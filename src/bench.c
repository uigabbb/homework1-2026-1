#include "bench.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

int recordCompare(const void *a, const void *b) {
    int x = ((const Record *)a)->key;
    int y = ((const Record *)b)->key;
    return (x > y) - (x < y);
}

const char *inputKindName(InputKind kind) {
    switch (kind) {
        case INPUT_RANDOM:     return "무작위";
        case INPUT_SORTED:     return "정렬됨";
        case INPUT_REVERSED:   return "역순";
        case INPUT_FEW_UNIQUE: return "중복많음";
        default:               return "?";
    }
}

void makeInput(Record *a, size_t n, InputKind kind, unsigned seed) {
    srand(seed); /* 씨앗을 고정해 매번 같은 입력을 쓴다 */
    for (size_t i = 0; i < n; i++) {
        switch (kind) {
            case INPUT_RANDOM:     a[i].key = rand(); break;
            case INPUT_SORTED:     a[i].key = (int)i; break;
            case INPUT_REVERSED:   a[i].key = (int)(n - i); break;
            case INPUT_FEW_UNIQUE: a[i].key = rand() % 8; break;
            default:               a[i].key = 0; break;
        }
        a[i].tag = (int)i; /* 입력 순서를 새겨 둔다 */
    }
}

int recordsSorted(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key > a[i].key) {
            return 0;
        }
    }
    return 1;
}

int recordsStable(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key == a[i].key && a[i - 1].tag > a[i].tag) {
            return 0;
        }
    }
    return 1;
}

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps) {
    BenchResult result;
    result.algo = algo;
    result.n = n;
    result.millis = 0.0;
    sortStatsReset(&result.stats);
    result.sorted = 0;
    result.stable = 0;

    if (reps < 1) {
        reps = 1;
    }
    Record *work = (Record *)malloc((n > 0 ? n : 1) * sizeof(Record));
    if (work == NULL) {
        return result;
    }

    clock_t spent = 0;
    for (int t = 0; t < reps; t++) {
        memcpy(work, input, n * sizeof(Record));
        /* 복사가 끝난 뒤에 시계를 본다. 재는 것은 정렬뿐이다. */
        clock_t begin = clock();
        algo->sort(work, n, sizeof(Record), recordCompare, &result.stats);
        spent += clock() - begin;
    }

    result.millis = (double)spent * 1000.0 / CLOCKS_PER_SEC / reps;
    result.sorted = recordsSorted(work, n);
    result.stable = recordsStable(work, n);
    free(work);
    return result;
}
