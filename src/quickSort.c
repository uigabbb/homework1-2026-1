/* 퀵 정렬 — 주제 04 · 피벗을 제자리에 놓고, 양쪽을 각각 정렬한다.
 *
 *   QuickSort(A, lo, hi)
 *       if lo >= hi then return
 *       p <- Partition(A, lo, hi)
 *       QuickSort(A, lo, p-1)
 *       QuickSort(A, p+1, hi)
 *
 * 병합과 뼈대가 같은 재귀지만 일하는 자리가 반대다. 나누기(파티션)에서 일하고
 * 결합은 공짜다. 파티션은 수업 코드(02_quick_sort/quickSort.c)와 같은
 * "첫 원소를 피벗으로, 작은 것을 왼쪽으로 모으는" 방식이다.
 *
 * 수업 코드와 다른 점은 딱 하나, **피벗을 고르는 법**이다. 수업의 "지금 해 볼 것"
 * 2번처럼 구간에서 난수로 한 칸을 골라 맨 앞(lo)과 바꾼 뒤 같은 파티션을 돌린다.
 * 그러면 이미 정렬된 입력의 함정(O(n^2))이 사라진다. 난수 생성기도 수업
 * (01_random)의 minstd를 그대로 쓴다.
 */
#include "sort.h"

#include <stdint.h>

#include "sortctx.h"

QuickPivot quickSortPivot = PIVOT_RANDOM;

/* --- 난수: 주제 04의 minstd 생성기 ---------------------------------------- */

static int64_t state = 1;

static void setSeed(int64_t seed) {
    state = seed;
}

static int64_t nextRandom(void) {
    state = state * 16807 % 2147483647;
    return state;
}

/* --- 피벗 고르기 --------------------------------------------------------- */

/* a[lo..hi] 중 피벗으로 쓸 칸을 골라 a[lo]로 옮긴다. 파티션은 늘 a[lo]를 쓴다. */
static void choosePivot(SortCtx *c, size_t lo, size_t hi) {
    size_t p = lo;
    switch (quickSortPivot) {
        case PIVOT_FIRST:
            return; /* 수업 코드 그대로 — 아무것도 안 한다 */
        case PIVOT_RANDOM:
            p = lo + (size_t)(nextRandom() % (int64_t)(hi - lo + 1));
            break;
        case PIVOT_MEDIAN3: {
            size_t mid = lo + (hi - lo) / 2;
            /* 셋 중 가운데의 위치를 비교 2~3번으로 찾는다. */
            if (sortCompareAt(c, lo, mid) < 0) {
                if (sortCompareAt(c, mid, hi) < 0) {
                    p = mid;
                } else {
                    p = sortCompareAt(c, lo, hi) < 0 ? hi : lo;
                }
            } else {
                if (sortCompareAt(c, lo, hi) < 0) {
                    p = lo;
                } else {
                    p = sortCompareAt(c, mid, hi) < 0 ? hi : mid;
                }
            }
            break;
        }
    }
    if (p != lo) {
        sortSwap(c, lo, p);
    }
}

/* --- 파티션: 수업 코드와 같다 ---------------------------------------------- */

/* a[lo]를 피벗으로 삼아, 피벗보다 **작은** 것을 왼쪽으로 모은다.
 * 끝나면 피벗은 제자리(i)에 놓이고 그 자리는 확정된다.
 * 먼 거리를 건너뛰는 교환이 같은 값의 순서를 뒤섞는다 — 불안정한 이유. */
static size_t partition(SortCtx *c, size_t lo, size_t hi) {
    size_t i = lo;
    for (size_t j = lo + 1; j <= hi; j++) {
        if (sortCompareAt(c, j, lo) < 0) { /* a[j] < pivot */
            i++;
            if (i != j) {
                sortSwap(c, i, j);
            }
        }
    }
    if (i != lo) {
        sortSwap(c, lo, i);
    }
    return i;
}

static void quickSortRange(SortCtx *c, size_t lo, size_t hi, size_t depth) {
    if (lo >= hi) {
        return; /* 원소 하나면 이미 정렬 */
    }
    sortNoteDepth(c, depth);
    choosePivot(c, lo, hi);
    size_t p = partition(c, lo, hi);
    if (p > lo) { /* size_t라 p-1이 음수가 되지 않게 막는다 */
        quickSortRange(c, lo, p - 1, depth + 1);
    }
    quickSortRange(c, p + 1, hi, depth + 1);
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 시드를 매번 1로 되돌린다. 같은 입력이면 비교·이동 횟수가 재현된다.
     * 실전이라면 srand(time(NULL))처럼 시드를 바꿔야 공격자가 최악을 짤 수 없다. */
    setSeed(1);
    quickSortRange(&c, 0, n - 1, 1);
    sortEnd(&c);
}
