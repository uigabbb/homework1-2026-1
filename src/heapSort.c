/* 힙 정렬 — 수업에서 다루지 않은 정렬 (J. W. J. Williams, 1964 · Floyd, 1964).
 *
 * 배열을 그대로 **최대 힙(max-heap)** 으로 본다. 인덱스 i의 자식은 2i+1, 2i+2다.
 * 포인터도 노드도 없이, 배열의 칸 번호만으로 완전 이진 트리가 된다.
 *
 *   1단계 · 힙 만들기(heapify)   아래 층 부모부터 거꾸로 올라가며 siftDown.
 *                               Floyd의 방법으로 O(n)이다.
 *   2단계 · 하나씩 꺼내기        루트(최댓값)를 맨 뒤와 바꾸고 힙 크기를 1 줄인 뒤
 *                               새 루트를 siftDown. n-1번 반복, 각각 O(log n).
 *
 * 선택 정렬(주제 02)과 뼈대가 같다 — "남은 것 중 최댓값을 골라 뒤에 놓는다".
 * 다른 점은 최댓값을 찾는 비용이다. 선택 정렬은 매번 O(n)을 훑지만, 힙은 이미
 * 절반쯤 정리된 트리라 O(log n)이면 다음 최댓값이 루트로 올라온다.
 *
 * 재귀도, 보조 배열도 없다. 추가 메모리는 원소 한 칸(tmp)뿐이다.
 */
#include "sort.h"

#include "sortctx.h"

/* a[root]에서 시작해, 크기 n인 힙의 성질이 깨진 곳을 아래로 내려가며 고친다.
 *
 * 교환을 되풀이하지 않고 "구멍"을 내린다. 내려갈 원소를 tmp에 들어 두고, 더 큰
 * 자식을 한 칸씩 위로 끌어올린 뒤, 멈춘 자리에 tmp를 놓는다. 삽입 정렬이
 * 교환 대신 한 칸씩 밀어내는 것과 같은 요령이다(이동 3회 → 1회). */
static void siftDown(SortCtx *c, size_t root, size_t n) {
    sortMove(c, c->tmp, sortElemAt(c, root));
    size_t hole = root;
    for (;;) {
        size_t child = 2 * hole + 1;
        if (child >= n) {
            break; /* 잎에 닿았다 */
        }
        /* 두 자식 중 큰 쪽을 고른다 (비교 1번) */
        if (child + 1 < n && sortCompareAt(c, child + 1, child) > 0) {
            child++;
        }
        /* 큰 자식이 내려가는 원소보다 크면 위로 올린다 (비교 1번) */
        if (sortCompareTmp(c, child) > 0) {
            sortMove(c, sortElemAt(c, hole), sortElemAt(c, child));
            hole = child;
        } else {
            break;
        }
    }
    if (hole != root) {
        sortMove(c, sortElemAt(c, hole), c->tmp);
    } else if (c->stats != NULL) {
        c->stats->moves--; /* 제자리였다면 tmp에 든 것은 움직인 것이 아니다 */
    }
}

void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 1단계: 힙 만들기. 잎(n/2 이후)은 이미 크기 1짜리 힙이라 건너뛴다. */
    for (size_t i = n / 2; i-- > 0;) {
        siftDown(&c, i, n);
    }
    /* 2단계: 루트(최댓값)를 맨 뒤로 보내고, 줄어든 힙을 다시 고친다.
     * 이 먼 거리 교환이 같은 값의 순서를 뒤섞는다 — 불안정한 이유. */
    for (size_t end = n - 1; end > 0; end--) {
        sortSwap(&c, 0, end);
        siftDown(&c, 0, end);
    }
    sortEnd(&c);
}
