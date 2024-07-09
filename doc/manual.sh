#!/bin/bash
pdflatex barren && \
pdflatex barren && \
pdfunite cover-front.pdf blank.pdf barren.pdf blank.pdf cover-rear.pdf barren-manual.pdf
