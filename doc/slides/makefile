slides.html: figs/* slides.md makefile
	pandoc -t revealjs --embed-resources --standalone -o slides.html slides.md

.PHONY: clean
clean:
	rm -f slides.html

