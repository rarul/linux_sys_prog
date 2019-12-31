SUBDIRS += $(dir $(shell find . -mindepth 2 -name "Makefile"))

all: $(SUBDIRS)
clean: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(SUBDIRS)
