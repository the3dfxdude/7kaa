ACLOCAL_AMFLAGS = --install -I m4

SUBDIRS = data include src po

doc_DATA = COPYING README

# these are included recursively by dist
EXTRA_DIST = doc packaging tools

if COMPILER_IS_EMSCRIPTEN
all: emscripten
endif

emscripten:
	$(CXX) src/7kaa.wasm $(EMSCRIPTEN_LD_FLAGS)
