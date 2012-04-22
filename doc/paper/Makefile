RM = rm -f
LATEXMK ?= latexmk

.PHONY: clean

doc.pdf: doc.tex doc.bib
	$(LATEXMK) -f -pdf $<

clean:
	$(LATEXMK) -c
	$(RM) doc.bbl
