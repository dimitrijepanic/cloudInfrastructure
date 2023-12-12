.SUFFIXES:	.pdf .md

.md.pdf:
# add --listing for captions in listings...
	pandoc --number-sections $*.md -f markdown -o $*.pdf


.PHONY: all clean mrproper

all: report.pdf

clean:
	rm -fv *.aux *.bbl *.blg *.log *.out *.toc

mrproper: clean
	rm -fv *.pdf