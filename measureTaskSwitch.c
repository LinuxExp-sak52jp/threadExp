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
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <asm/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "calcDiffs.h"

#ifdef SCHED_DEADLINE
#  undef SCHED_DEADLINE
#endif // SCHED_DEADLINE
#define SCHED_DEADLINE  (6)


//! 繰り返し回数
#define NUM_OF_REPEATS (1000)

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

int setAttrForDeadline(struct sched_attr* attr)
{
    memset(attr, 0, sizeof(struct sched_attr));
    attr->size = sizeof(struct sched_attr);
    attr->sched_policy = SCHED_DEADLINE;
    attr->sched_runtime = 30000000; // 30msec
    attr->sched_deadline = 30000000; // 30msec
    attr->sched_period = 30000000; // 30msec

    if (syscall(__NR_sched_setattr, 0, attr, 0)) {
        perror("setAttrForDeadline()");
        return -1;
    }
    return 0;
}

int setScheduler(struct sched_param *param, int policy)
{
    memset(param, 0, sizeof(struct sched_param));

    if (policy != SCHED_OTHER) {
        param->sched_priority = sched_get_priority_max(policy);
    }
    if (sched_setscheduler(0, policy, param)) {
        perror("setScheduler()");
        return -1;
    }
    return 0;
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

void dummyLoad(void)
{
    volatile int d;
    int fd = open("/dev/null", O_WRONLY|O_SYNC);
    if (fd < 0) {
        perror("dummyLoad()");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 500000; i++) {
        d = i;
        write(fd, (void *)&d, sizeof(d));
    }
    
}

int main(int ac, char* av[])
{
    struct sched_attr attr;
    struct sched_param param;
    int i;

    int mode = 0; // SCHED_DEADLINE
    int policy = SCHED_DEADLINE;
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
            policy = SCHED_OTHER;
            strcpy(schedPolicy, "SCHED_OTHER");
        } else if (strcmp(av[1], "-f") == 0) {
            mode = 2; // SCHED_FIFO
            policy = SCHED_FIFO;
            strcpy(schedPolicy, "SCHED_FIFO");
        } else if (strcmp(av[1], "-r") == 0) {
            mode = 3; // SCHED_RR
            policy = SCHED_RR;
            strcpy(schedPolicy, "SCHED_RR");
        } else {
            usagePrint(av[0]);
            return -1;
        }
    }
    int ret = 0;
    switch (mode) {
        case 0:
            ret = setAttrForDeadline(&attr);
            break;
        default:
            ret = setScheduler(&param, policy);
            break;
    }
    if (ret) {
        printf("setAttr() or setScheduler() failed\n");
        return -1;
    }

    struct timeval tv[NUM_OF_REPEATS+1];
    const struct timespec sp = {
        .tv_sec = 0,
        .tv_nsec = 30000000, // 30 msec
    };
    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
        dummyLoad();
        nanosleep(&sp, NULL);
    }

    // Calculate diffs
    fprintf(stderr, "-- Sched:%s --\n", schedPolicy);
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);
    
    return 0;
}

