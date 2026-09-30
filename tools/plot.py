"""비교 결과를 그래프(SVG)로 그린다.

    make charts          # 또는
    python3 tools/plot.py

src/main.out --csv 를 돌려 측정값을 받아 report/ 아래에 SVG를 쓴다. 사람이 읽는
표를 파싱하지 않고 CSV를 쓰는 이유는, 표의 모양이 바뀌어도 그래프가 깨지지
않게 하려는 것이다.

표준 모듈만 쓴다. 그림은 tools/svgchart.py가 직접 찍어 낸다.
"""

import csv
import math
import io
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import svgchart  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "src" / "main.out"
OUT_DIR = ROOT / "report"
ALGOS = ["mergeSort", "quickSort", "heapSort"]
KIND_LABEL = {
    "random": "무작위",
    "sorted": "정렬됨",
    "reversed": "역순",
    "few-unique": "중복많음",
}


def run_csv(flag):
    """측정 프로그램을 돌려 CSV를 읽는다."""
    if not BINARY.exists():
        subprocess.run(["make", "src/main.out"], cwd=ROOT, check=True)
    result = subprocess.run([str(BINARY), flag], cwd=ROOT, check=True,
                            capture_output=True, text=True)
    return list(csv.DictReader(io.StringIO(result.stdout)))


def load_rows():
    """측정 프로그램을 돌려 CSV를 읽는다. 없으면 make가 만들게 한다."""
    rows = run_csv("--csv")
    for row in rows:
        for key in ("n", "compares", "moves", "extraBytes", "maxDepth"):
            row[key] = int(row[key])
        row["millis"] = float(row["millis"])
    return rows


def pick(rows, **conditions):
    return [r for r in rows if all(r[k] == v for k, v in conditions.items())]


def by_algo(rows, field, keys, key_field):
    """{알고리즘: [키 순서대로의 값]} 으로 모은다."""
    table = {}
    for algo in ALGOS:
        values = []
        for key in keys:
            match = [r for r in rows if r["algo"] == algo and r[key_field] == key]
            values.append(match[0][field] if match else 0)
        table[algo] = values
    return table


def main():
    OUT_DIR.mkdir(exist_ok=True)
    rows = load_rows()
    # 그래프와 보고서의 표가 같은 실행에서 나오도록 측정값을 그대로 남긴다.
    with open(OUT_DIR / "results.csv", "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    growth = pick(rows, scope="growth")
    kinds = pick(rows, scope="kinds")
    sizes = sorted({r["n"] for r in growth})
    kind_keys = ["random", "sorted", "reversed", "few-unique"]
    shape_labels = [KIND_LABEL[k] for k in kind_keys]
    made = []

    # 1. n에 따라 자라는 모양 (로그-로그: 기울기가 곧 복잡도 지수)
    for field, unit, noun, stem in (
            ("compares", "비교 횟수", "비교 횟수", "growth-compares"),
            ("moves", "이동 횟수", "이동 횟수", "growth-moves"),
            ("millis", "시간 (ms)", "걸린 시간", "growth-time")):
        made.append(svgchart.line_chart(
            OUT_DIR / f"{stem}.svg",
            f"n이 커질 때 {noun} — 로그-로그 축",
            "무작위 입력 · 기울기가 곧 복잡도 지수다 (n log n이면 1.0보다 조금 크다)",
            sizes, by_algo(growth, field, sizes, "n"), "n (원소 개수)", unit))

    # 2. 비교 횟수를 n log2 n으로 나눈 값 — 상수 계수가 보인다
    norm = {a: [v / (n * math.log2(n)) for v, n in zip(vals, sizes)]
            for a, vals in by_algo(growth, "compares", sizes, "n").items()}
    made.append(svgchart.line_chart(
        OUT_DIR / "growth-compares-per-nlogn.svg",
        "비교 횟수 ÷ n log₂n — 복잡도가 같을 때 남는 것은 상수다",
        "무작위 입력 · x는 로그 축 · 병합 ≈ 0.9, 퀵 ≈ 1.2(점근 1.39), 힙 ≈ 1.8(점근 2)",
        sizes, norm, "n (원소 개수)", "비교 / n log₂n",
        log_axes=False, annotate_slope=False, log_x=True))

    # 3. 시간을 n log2 n으로 나눈 값 (ns) — 캐시가 끼어드는 자리
    tnorm = {a: [v * 1e6 / (n * math.log2(n)) for v, n in zip(vals, sizes)]
             for a, vals in by_algo(growth, "millis", sizes, "n").items()}
    made.append(svgchart.line_chart(
        OUT_DIR / "growth-time-per-nlogn.svg",
        "시간 ÷ n log₂n (ns) — 같은 n log n인데 누가 먼저 느려지나",
        "무작위 입력 · x는 로그 축 · 평평하면 이론대로, 오르면 캐시 · 메모리 비용이 끼어든 것",
        sizes, tnorm, "n (원소 개수)", "ns / (n log₂n)",
        log_axes=False, annotate_slope=False, log_x=True))

    # 4. 입력 모양별 (n = 100,000). 퀵의 중복많음이 수십 배라 로그 축을 같이 그린다.
    columns = (
        ("millis", "시간 (ms)", "input-shapes-time", "걸린 시간", svgchart.ms),
        ("compares", "비교 횟수", "input-shapes-compares", "비교 횟수", svgchart.si),
        ("moves", "이동 횟수", "input-shapes-moves", "이동 횟수", svgchart.si),
    )
    for field, unit, stem, label, fmt in columns:
        data = by_algo(kinds, field, kind_keys, "input")
        made.append(svgchart.grouped_bar_chart(
            OUT_DIR / f"{stem}-log.svg",
            f"입력 모양에 따른 {label} — 로그 축",
            "n = 100,000 · 눈금 한 칸이 10배다",
            shape_labels, data, unit, log_scale=True, value_label=fmt))
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "input-shapes-time.svg",
        "입력 모양에 따른 걸린 시간 — 선형 축",
        "n = 100,000 · 퀵 정렬의 '중복많음' 한 막대가 나머지를 모두 바닥으로 누른다",
        shape_labels, by_algo(kinds, "millis", kind_keys, "input"), "시간 (ms)",
        value_label=svgchart.ms))

    # 5. 재귀 깊이 (스택 메모리의 대리 지표)
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "input-shapes-depth-log.svg",
        "입력 모양에 따른 재귀 깊이 — 로그 축",
        "n = 100,000 · 병합은 log₂n+1로 고정, 힙은 반복문뿐이라 1, 퀵은 입력을 탄다",
        shape_labels, by_algo(kinds, "maxDepth", kind_keys, "input"),
        "재귀 깊이", log_scale=True))

    # 6. 피벗 실험 — 퀵 정렬만, 피벗 고르는 법을 바꿔 가며
    pivots = run_csv("--pivot")
    for row in pivots:
        for key in ("n", "compares", "moves", "maxDepth"):
            row[key] = int(row[key])
        row["millis"] = float(row["millis"])
    with open(OUT_DIR / "pivot.csv", "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(pivots[0]))
        w.writeheader()
        w.writerows(pivots)
    pivot_label = {"first": "첫 원소 (수업 코드)", "median3": "셋 중 가운데",
                   "random": "랜덤 (이 과제)"}
    series = {pivot_label[p]: [next(r["compares"] for r in pivots
                                    if r["input"] == k and r["pivot"] == p)
                               for k in kind_keys]
              for p in ("first", "median3", "random")}
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "pivot-compares.svg",
        "피벗 고르는 법에 따른 퀵 정렬의 비교 횟수 — 로그 축",
        "n = 10,000 · 정렬됨·역순에서 첫 원소 피벗은 n(n-1)/2 = 49,995,000회",
        shape_labels, series, "비교 횟수", log_scale=True))

    for path in made:
        print(f"wrote {Path(path).relative_to(ROOT)}")


if __name__ == "__main__":
    main()
