all:
	rm -f .partmode .part*
	if [ -n '$(PART)' ]; then touch .partmode .part$(PART); fi
	pdflatex path-tracing
	pdflatex path-tracing
	if [ -n '$(PART)' ]; then mv path-tracing.pdf path-tracing-`printf %02d $(PART)`.pdf; fi

clean:
	for i in aux log nav out pdf snm toc vrb; do rm -f path-tracing*.$$i; done
