/**
 * 3msec毎に周期起動させた時に起動時刻の平均・最大値・最小値を計測する
 * パラメータとして、下記のスケジューリングポリシーを指定可能
 *   -d: SCHED_DEADLINE  Realtime中、最大優先度 (Realtime)
 *   -f: SCHED_FIFO      指定可能な最大優先度(=99)で起動 (Realtime)
 *   -r: SCHED_RR        指定可能な最大優先度(=99)で起動 (Realtime)
 *   -o: SCHED_OTHER     通常のtime slice RR (NOT realtime)
 * <注意>
 *   動作確認は下記の環境で行った。
 *     Linux ubuntu 5.3.0-53-generic #47~18.04.1-Ubuntu
 *   他の環境で動作させる場合は、Linux kernelのソースを調査して、SCHED_DEADLINE
 *   と struct sched_attr の定義を確認し、必要に応じて修正すること。
 * Compile:gcc -O2 -o measureTaskSwitch measureTaskSwitch.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <asm/types.h>
#include <unistd.h>

#define SCHED_DEADLINE  (6)

struct sched_attr {
    uint32_t size;              /* この構造体のサイズ */
    uint32_t sched_policy;      /* ポリシー (SCHED_*) */
    uint64_t sched_flags;       /* フラグ */
    int32_t sched_nice;        /* nice 値 (SCHED_OTHER,
                              SCHED_BATCH) */
    uint32_t sched_priority;    /* 静的優先度 (SCHED_FIFO,
                              SCHED_RR) */
    /* 残りのフィールドは SCHED_DEADLINE 用である */
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
    /* Utilization hints */
    uint32_t sched_util_min;
    uint32_t sched_util_max;
};

void calcDiffs(struct timeval* tv, int size)
{
    long long *diffs = (long long*)malloc(size * sizeof(long long));
    long long maxv = 0;
    long long minv = 1000000000;
    long long sum = 0;
    int i;
    for (i = 0; i < size; i++) {
        long long sec = tv[i+1].tv_sec - tv[i].tv_sec;
        long long usec = tv[i+1].tv_usec - tv[i].tv_usec;
        if (usec >= 0) {
            diffs[i] = sec*1000000 + usec;
        } else {
            diffs[i] = (sec-1)*1000000 + (1000000+usec);
        }
        if (maxv < diffs[i])
            maxv = diffs[i];
        if (minv > diffs[i])
            minv = diffs[i];
        sum += diffs[i];
    }

    double ave = (double)sum/size;
    printf("ave = %lf [us]\n", ave);
    printf("max = %lld [us]\n", maxv);
    printf("min = %lld [us]\n", minv);
}

void setAttrForDeadline(struct sched_attr* attr)
{
    memset(attr, 0, sizeof(struct sched_attr));
    attr->size = sizeof(struct sched_attr);
    attr->sched_policy = SCHED_DEADLINE;
    attr->sched_runtime = 30000000; // 30msec
    attr->sched_deadline = 30000000; // 30msec
    attr->sched_period = 30000000; // 30msec
}

void setAttrForOther(struct sched_attr* attr)
{
    memset(attr, 0, sizeof(struct sched_attr));
    attr->size = sizeof(struct sched_attr);
    attr->sched_policy = SCHED_OTHER;
    attr->sched_nice = -20; // Highest
}

void setAttrForFifo(struct sched_attr* attr)
{
    memset(attr, 0, sizeof(struct sched_attr));
    attr->size = sizeof(struct sched_attr);
    attr->sched_policy = SCHED_FIFO;
    attr->sched_priority = sched_get_priority_max(SCHED_FIFO);
}

void setAttrForRoundRobin(struct sched_attr* attr)
{
    memset(attr, 0, sizeof(struct sched_attr));
    attr->size = sizeof(struct sched_attr);
    attr->sched_policy = SCHED_RR;
    attr->sched_priority = sched_get_priority_max(SCHED_RR);
}

void usagePrint(char* name)
{
    fprintf(stderr, "Usage:%s [sched]\n", name);
    fprintf(stderr, " sched:\n");
    fprintf(stderr, "  -d:SCHED_DEADLINE\n");
    fprintf(stderr, "  -f:SCHED_FIFO\n");
    fprintf(stderr, "  -r:SCHED_RR\n");
    fprintf(stderr, "  -o:SCHED_OTHER\n");
    fprintf(stderr, "  (default:SCHED_DEADLINE)\n");
}

int main(int ac, char* av[])
{
    struct sched_attr attr;
    int i;

    int mode = 0; // SCHED_DEADLINE
    char schedPolicy[100];
    strcpy(schedPolicy, "SCHED_DEADLINE");
    
    if (ac > 2) {
        usagePrint(av[0]);
        return -1;
    } else if (ac == 2) {
        if (strcmp(av[1], "-d") == 0) {
            mode = 0; // SCHED_DEADLINE
            strcpy(schedPolicy, "SCHED_DEADLINE");
        } else if (strcmp(av[1], "-o") == 0) {
            mode = 1; // SCHED_OTHER
            strcpy(schedPolicy, "SCHED_OTHER");
        } else if (strcmp(av[1], "-f") == 0) {
            mode = 2; // SCHED_FIFO
            strcpy(schedPolicy, "SCHED_FIFO");
        } else if (strcmp(av[1], "-r") == 0) {
            mode = 3; // SCHED_RR
            strcpy(schedPolicy, "SCHED_RR");
        } else {
            usagePrint(av[0]);
            return -1;
        }
    }
    switch (mode) {
        case 0:
            setAttrForDeadline(&attr);
            break;
        case 1:
            setAttrForOther(&attr);
            break;
        case 2:
            setAttrForFifo(&attr);
            break;
        case 3:
            setAttrForRoundRobin(&attr);
            break;
    }
    
    if (syscall(__NR_sched_setattr, 0, &attr, 0)) {
        int errnum = errno;
        char* errstr = strerror(errnum);
        printf("Error:sched_setattr():errno=%d,errstr=%s\n", errnum, errstr);
        return -1;
    }

    struct timeval tv[101];
    const struct timespec sp = {
        .tv_sec = 0,
        .tv_nsec = 30000000, // 30 msec
    };
    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
        nanosleep(&sp, NULL);
    }

    // Calculate diffs
    fprintf(stderr, "-- Sched:%s --\n", schedPolicy);
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);
    
    return 0;
}

