#!/bin/bash

DOCUTILS_PATH="/usr/local/bin/docutils/tools"

${DOCUTILS_PATH}/rst2html.py <$1 >$1.html
${DOCUTILS_PATH}/rst2latex.py <$1 >$1.tex
pdflatex --shell-escape $1.tex

exit 0
