all: default

default:
	find ./figs -name "*.tex" ! -name "pgfplotstyles.tex" -execdir latexmk -pdf {} \;
	latexmk -pdf paper.tex

clean:
	find . -name "*.aux" -delete \
		-o -name "*.auxlock" -delete \
		-o -name "*.bbl" -delete \
		-o -name "*.blg" -delete \
		-o -name "*.cut" -delete \
		-o -name "*.fdb_latexmk" -delete \
		-o -name "*.fls" -delete \
		-o -name "*.log" -delete \
		-o -name "*.nav" -delete \
		-o -name "*.out" -delete \
		-o -name "*.pdf" -delete \
		-o -name "*.snm" -delete \
		-o -name "*.toc" -delete \
		-o -name "*.xcp" -delete
