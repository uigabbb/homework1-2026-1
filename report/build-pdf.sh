#!/bin/sh
# REPORT.md -> REPORT.pdf (pandoc + wkhtmltopdf). report/ 안에서 실행한다.
set -e
cd "$(dirname "$0")"
pandoc REPORT.md -f markdown+gfm_auto_identifiers-implicit_figures -t html5 -s \
    --css pdf.css --metadata title="정렬 비교 보고서" -o report.html
python3 -c "import re;p='report.html';s=open(p).read();open(p,'w').write(re.sub(r'<header id=\"title-block-header\">.*?</header>','',s,flags=re.S))"
wkhtmltopdf -q --enable-local-file-access --encoding utf-8 -s A4 -T 16mm -B 16mm -L 15mm -R 15mm \
    --footer-center "[page] / [topage]" --footer-font-size 8 report.html REPORT.pdf
rm -f report.html
