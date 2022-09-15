all:
	# Build the 10 parts of the slides
	for i in 01 02 03 04 05 06 07 08 09 10; do \
		make -C slides PART=`expr 1$$i - 100` ; \
		mv slides/path-tracing-$$i.pdf . ; \
	done
	# Build the complete slide set
	make -C slides
	mv slides/path-tracing.pdf .
	# Build the tutorials with material archives
	for i in 01 02 03 04 05 06 07 08 09 10; do \
		cd tutorial ; \
		pdflatex $$i ; \
		pdflatex $$i ; \
		zip -9 -v -r material-$$i.zip material-$$i ; \
		cd .. ; \
		mv tutorial/$$i.pdf tutorial-$$i.pdf ; \
		mv tutorial/material-$$i.zip . ; \
	done
