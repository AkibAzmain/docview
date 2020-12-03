SPHINX_BUILD = sphinx-build
DOXYGEN      = doxygen
SPHINXFLAGS  =
DOXYGENFLAGS =

html: xml
	$(SPHINX_BUILD) -M html . _build $(SPHINXFLAGS)

clean:
	rm -rf _build/* xml

%: build/xml build doxygen-xml makefile
	$(SPHINX_BUILD) -M $@ . _build $(SPHINXFLAGS)

xml: ../src/libdocview/docview.hpp ../src/libdocview/docview.h
	$(DOXYGEN) doxygen.conf $(DOXYGENFLAGS)
	touch xml

.PHONY: html clean makefile %