"""힙 정렬 설명용 그림 — 배열과 그것이 뜻하는 완전 이진 트리를 나란히 그린다.

    python3 tools/heaptree.py

report/heap-array-tree.svg 를 쓴다. 표준 모듈만 쓴다.
배열 값은 수업 예제 배열(2 8 5 9 1 10 7 6 4 3)과, 그것을 힙으로 만든 결과다.
"""

from pathlib import Path

import svgchart as sc

ROOT = Path(__file__).resolve().parents[1]
BEFORE = [2, 8, 5, 9, 1, 10, 7, 6, 4, 3]
AFTER = [10, 9, 7, 8, 3, 5, 2, 6, 4, 1]   # heapSort 1단계가 만든 최대 힙
BLUE, GREEN, ORANGE = "#0072B2", "#009E73", "#E69F00"


def tree(c, values, ox, oy, width, title, color, bad=()):
    """values를 완전 이진 트리로 그린다. bad에 든 칸은 힙 성질이 깨진 부모."""
    c.text(ox + width / 2, oy - 36, title, size=13, anchor="middle", weight="600")
    pos = {}
    for i in range(len(values)):
        level = (i + 1).bit_length() - 1
        first = 2 ** level - 1
        slots = 2 ** level
        x = ox + width * ((i - first) + 0.5) / slots
        y = oy + level * 62
        pos[i] = (x, y)
    for i in range(1, len(values)):
        (x1, y1), (x2, y2) = pos[(i - 1) // 2], pos[i]
        c.line(x1, y1, x2, y2, stroke=sc.MUTED, width=1.2)
    for i, v in enumerate(values):
        x, y = pos[i]
        fill = ORANGE if i in bad else color
        c.parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="17" fill="{fill}"/>')
        c.text(x, y + 5, str(v), size=13, fill="#ffffff", anchor="middle", weight="600")
        c.text(x + 20, y - 12, f"[{i}]", size=9.5, fill=sc.MUTED)


def array(c, values, ox, oy, color):
    for i, v in enumerate(values):
        x = ox + i * 32
        c.rect(x, oy, 30, 30, color)
        c.text(x + 15, oy + 20, str(v), size=13, fill="#ffffff", anchor="middle", weight="600")
        c.text(x + 15, oy + 46, str(i), size=10, fill=sc.MUTED, anchor="middle")


def main():
    c = sc.Canvas(720, 470, "배열이 곧 트리다 — 칸 i의 자식은 2i+1, 2i+2",
                  "왼쪽: 수업 예제 배열 그대로 (주황 = 자식보다 작은 부모, 힙 성질 위반)"
                  " · 오른쪽: 1단계(힙 만들기)가 끝난 최대 힙")
    # 힙 성질(부모 >= 자식)이 깨진 부모를 찾는다.
    bad = {i for i in range(len(BEFORE)) for ch in (2 * i + 1, 2 * i + 2)
           if ch < len(BEFORE) and BEFORE[ch] > BEFORE[i]}
    tree(c, BEFORE, 20, 120, 330, "힙 만들기 전", BLUE, bad)
    tree(c, AFTER, 370, 120, 330, "최대 힙 (루트 = 최댓값)", GREEN)
    array(c, BEFORE, 30, 380, BLUE)
    array(c, AFTER, 380, 380, GREEN)
    c.text(360, 458, "배열 칸 번호 · 포인터 없이 인덱스 계산만으로 부모·자식을 오간다",
           size=11, fill=sc.MUTED, anchor="middle")
    out = ROOT / "report" / "heap-array-tree.svg"
    c.save(out)
    print(f"wrote {out.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
