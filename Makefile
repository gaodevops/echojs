TOP=$(shell pwd)

include $(TOP)/build/config.mk

#SUBDIRS=external-deps node-llvm ejs-llvm lib runtime
SUBDIRS=external-deps node-llvm lib runtime

# run git submodule magic if somebody is antsy and doesn't type the magic incantation before typing make
all-local::
	@if [ ! -f pcre/configure.ac ]; then \
	  git submodule init; \
	  git submodule update; \
	fi

check:
	$(MAKE) -C test check

bootstrap:
	$(MAKE) -C lib/generated bootstrap

include $(TOP)/build/build.mk
