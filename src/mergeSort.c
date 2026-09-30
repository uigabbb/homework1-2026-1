/* 병합 정렬 — 주제 03 · 반으로 나눠 각각 정렬하고, 정렬된 두 절반을 합친다.
 *
 * 수업 코드(01_merge_sort/mergeSort.c)를 그대로 옮기되, 원소 타입을 모르는
 * 공통 인터페이스(void * + 비교 함수) 위에서 돌도록 바꿨다.
 *
 *   MergeSort(A, lo, hi)
 *       if lo >= hi then return
 *       mid <- (lo + hi) / 2
 *       MergeSort(A, lo, mid)
 *       MergeSort(A, mid+1, hi)
 *       Merge(A, lo, mid, hi)
 *
 * 나누기는 공짜(위치로 자른다)이고, 일은 결합(병합)에서 한다.
 */
#include "sort.h"

#include <stdlib.h>

#include "sortctx.h"

/* a[lo..mid]와 a[mid+1..hi]는 각각 정렬되어 있다. temp를 거쳐 합친다. */
static void merge(SortCtx *c, char *temp, size_t lo, size_t mid, size_t hi) {
    size_t i = lo;
    size_t j = mid + 1;
    size_t k = lo;
    const size_t sz = c->size;

    while (i <= mid && j <= hi) {
        /* '<='라서 같은 값이면 왼쪽이 먼저 나간다. 이 한 글자가 안정성이다. */
        if (sortCompareAt(c, i, j) <= 0) {
            sortMove(c, temp + (k++) * sz, sortElemAt(c, i++));
        } else {
            sortMove(c, temp + (k++) * sz, sortElemAt(c, j++));
        }
    }
    /* 한쪽이 바닥나면 남은 쪽은 비교 없이 부어 넣는다. */
    while (i <= mid) {
        sortMove(c, temp + (k++) * sz, sortElemAt(c, i++));
    }
    while (j <= hi) {
        sortMove(c, temp + (k++) * sz, sortElemAt(c, j++));
    }
    /* temp에서 제자리로 돌려놓는다. 병합 한 번에 이동이 원소 수의 2배인 이유. */
    for (k = lo; k <= hi; k++) {
        sortMove(c, sortElemAt(c, k), temp + k * sz);
    }
}

static void mergeSortRange(SortCtx *c, char *temp, size_t lo, size_t hi, size_t depth) {
    if (lo >= hi) {
        return; /* 원소 하나면 이미 정렬 */
    }
    sortNoteDepth(c, depth);
    size_t mid = lo + (hi - lo) / 2; /* (lo+hi)/2는 overflow가 날 수 있다 */
    mergeSortRange(c, temp, lo, mid, depth + 1);
    mergeSortRange(c, temp, mid + 1, hi, depth + 1);
    merge(c, temp, lo, mid, hi);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 병합 정렬의 값: 입력과 같은 크기의 temp. 퀵·힙에는 없는 O(n) 메모리다. */
    char *temp = (char *)malloc(n * size);
    if (temp == NULL) {
        sortEnd(&c);
        return;
    }
    if (stats != NULL) {
        stats->extraBytes += n * size;
    }
    mergeSortRange(&c, temp, 0, n - 1, 1);
    free(temp);
    sortEnd(&c);
}
