/**
 * measureTaskSwitchを使用する前に、計測に用いる gettimeofday() の処理速度が
 * 問題ないか確認するためのテストプログラム
 * Compile:gcc -O2 -o measureGettime measureGettime.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "calcDiffs.h"

#define NUM_OF_REPEATS (10000)

int main(void)
{
    struct timeval tv[NUM_OF_REPEATS+1];

    int i;
    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
    }
    printf("--- Just call gettimeofday() %d times ---\n", NUM_OF_REPEATS);
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);

    volatile int vd, vd2;
    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
        vd = 100;
    }
    printf("--- Write to volatile with gettimeofday() %d times ---\n",
           NUM_OF_REPEATS);
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);

    for (i = 0; i < sizeof(tv)/sizeof(struct timeval); i++) {
        gettimeofday(&tv[i], NULL);
        vd2 = vd;
    }
    printf("--- Read from volatile with gettimeofday() %d times ---\n",
           NUM_OF_REPEATS);
    calcDiffs(tv, sizeof(tv)/sizeof(struct timeval)-1);

    dummy(vd2);

    
    return 0;
}
