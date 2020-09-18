CC := gcc
CXX := g++
CFLAGS := $(CFLAGS) -c -g -O2 -Wextra -Wall
CXXFLAGS := $(CXXFLAGS) $(CFLAGS)
#LDFLAGS ?=

INSTALLDIR ?= $(MAKETOP)/usr/local/bin
TARGETS ?= $(patsubst %.c,%,$(wildcard *.c)) \
	$(patsubst %.cpp,%,$(wildcard *.cpp)) \
	$(patsubst %.cc,%,$(wildcard *.cc))
define TARGETS_DEPEND
$(1): $(1).o
endef

INSTALL_EACH := $(patsubst %,install-%,$(TARGETS))

all: $(TARGETS)

$(foreach target,$(TARGETS),$(eval $(call TARGETS_DEPEND,$(target))))

install: $(INSTALL_EACH)

$(INSTALL_EACH): all
	myself=`echo $@ |sed -e 's/install-//'`;			\
	mkdir -p $(INSTALLDIR);						\
	cp $$myself $(INSTALLDIR)

clean:
	$(RM) -f $(TARGETS) *.o 

.gitignore: $(TARGETS)
	echo $(TARGETS) | sed -e 's/\s/\n/g' > $@

.PHONY: all install clean $(INSTALL_EACH)
