/**
 * @file calcDiffs.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/**
 * @brief timeval構造体配列を受け取り、順次差分を計算する
 * @param[in] tv 差分計算対象が順番に並ぶ配列
 * @param[in] size 差分計算個数
 * @detail 計算は、tv[1]-tv[0],...,tv[size]-tv[size-1] の範囲で行うため
 * sizeは(実際の配列長-1)以下でなければならない。
 */
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

void dummy(volatile int v)
{
    printf("%s():v=%d\n", __FUNCTION__, v);
}

        
