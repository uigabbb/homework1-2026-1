/* 정렬 비교 과제 — 병합 · 퀵 · 힙 정렬을 하나의 공통 인터페이스로 묶는다.
 *
 * C에는 interface·class가 없다. 대신 **함수 포인터를 담은 구조체**를 쓴다.
 * 비교 규약은 표준 라이브러리의 qsort와 똑같이 맞췄다.
 *
 *   mergeSort  수업(주제 03)에서 배운 정렬 — 위치로 나누고 병합에서 일한다
 *   quickSort  수업(주제 04)에서 배운 정렬 — 값으로 나누고 파티션에서 일한다
 *   heapSort   수업에서 다루지 않은 정렬 — 힙이라는 자료구조가 일한다
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

/* 비교 함수. qsort와 같은 규약: a<b면 음수, a==b면 0, a>b면 양수. */
typedef int (*SortCompare)(const void *a, const void *b);

/* 한 번 정렬하는 동안 모인 측정값. 시간은 바깥(bench)에서 잰다. */
typedef struct SortStats {
    size_t compares;   /* 비교 함수를 부른 횟수 */
    size_t moves;      /* 원소를 복사한 횟수 (교환 한 번은 3) */
    size_t extraBytes; /* 입력 배열 밖에 잡은 작업 공간의 최대 바이트 (힙 메모리) */
    size_t maxDepth;   /* 재귀 깊이의 최댓값. 반복문만 쓰면 1 (스택 메모리의 대리 지표) */
} SortStats;

/* 정렬 한 가지. 이 구조체가 이 과제의 "인터페이스"다. */
typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;  /* 평균 / 최악 시간복잡도 */
    const char *spaceComplexity; /* 추가 메모리 */
    int stable;                  /* 안정 정렬이라고 주장하는 값. 테스트가 실측과 맞춰 본다 */
    void (*sort)(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
} SortAlgorithm;

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

/* 퀵 정렬의 피벗 고르는 법 (주제 04, "피벗을 어떻게 고르나").
 * 기본은 랜덤. 피벗 실험(main.c --pivot)이 다른 값을 넣어 본다. */
typedef enum QuickPivot {
    PIVOT_RANDOM, /* 옵션 1: 구간에서 무작위 — 기본값 */
    PIVOT_FIRST,  /* 수업 코드 그대로: 첫 원소 */
    PIVOT_MEDIAN3 /* 옵션 2: 앞·가운데·뒤 셋 중 가운데 */
} QuickPivot;
extern QuickPivot quickSortPivot;

void sortStatsReset(SortStats *stats);
int sortCompareInt(const void *a, const void *b); /* int 배열용 기본 비교 함수 */

#endif /* SORT_H */
