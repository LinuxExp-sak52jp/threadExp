/**
 * measureTaskSwitchを使用する前に、計測に用いる gettimeofday() の処理速度が
 * 問題ないか確認するためのテストプログラム
 *
 * Compile:gcc -O2 -o measureGettime measureGettime.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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

int main(void)
{
    struct timeval tv[101];

    int i;
    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
    }

    // Calculate diffs
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);
    
    return 0;
}
