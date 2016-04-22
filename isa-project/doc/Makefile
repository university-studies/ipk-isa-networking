NAME=manual
CITACIE=literatura.bib

all:
	#vlna -v KkSsVvZzOoUuAIai $(NAME).tex
	#vlna -v KkSsVvZzOoUuAIai $(CITACIE) 
	
	latex $(NAME).tex

	bibtex $(NAME).aux

	latex $(NAME).tex
	latex $(NAME).tex

	dvips -t a4 $(NAME).dvi
	ps2pdf $(NAME).ps

clean:
	rm -f $(NAME).dvi $(NAME).log $(NAME).ps $(NAME).aux $(NAME).pdf \
		$(NAME).bbl $(NAME).blg $(NAME).toc 

