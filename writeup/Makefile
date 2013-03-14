DOCUMENT   = document

LATEX      = pdflatex
BIBTEX     = bibtex
PDFVIEWER  = evince
GLOSSARIES = makeglossaries

DOCDIR    = ../doc/

LATEXOPTS = -output-directory ../doc/

$(DOCUMENT): 
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex
	$(BIBTEX) $(DOCUMENT).aux
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex
	$(GLOSSARIES) $(DOCUMENT)
	$(LATEX) $(LATEXOPTIONS) $(DOCUMENT).tex

v: $(DOCUMENT).pdf
	$(PDFVIEWER) $(DOCUMENT).pdf &

.PHONY: clean

clean:
	rm -rf *.aux *.toc *.bbl *.blg *.pdf *.glo

