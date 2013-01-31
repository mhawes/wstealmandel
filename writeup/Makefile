DOCUMENT  = document

LATEX     = pdflatex
BIBTEX    = bibtex
PDFVIEWER = evince

DOCDIR    = ../doc/

LATEXOPTS = -output-directory ../doc/

$(DOCUMENT): 
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex
	$(BIBTEX) $(DOCUMENT).aux
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex

v: $(DOCUMENT).pdf
	$(PDFVIEWER) $(DOCUMENT).pdf &

.PHONY: clean

clean:
	rm -rf $(DOCUMENT).aux
	rm -rf $(DOCUMENT).toc
	rm -rf $(DOCUMENT).bbl
	rm -rf $(DOCUMENT).blg
	rm -rf $(DOCUMENT).pdf

