#
# uzepede - a centipede clone for uzebox
# 
# Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>
# licensed under the GNU GPL v3 or later
#
# see https://github.com/mmitch/uzepede
#

warnings   := -Wall -Wextra -Wpedantic -Werror
libs       := 

CFLAGS  += $(warnings)
LDFLAGS += $(libs)

testsrcdir   := .
testbuilddir := ./build
testbindir   := ./bin

testsources  := $(wildcard $(testsrcdir)/*.c)
testobjects  := $(addprefix $(testbuilddir)/,$(notdir $(testsources:.c=.o)))
testdepends  := $(testobjects:.o=.d)
testbinaries := $(addprefix $(testbindir)/,$(notdir $(testsources:.c=)))

define gendep =
@echo dep $@
@set -e; rm -f $@; \
$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
sed -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -e ':loop;s,/[^/]\+/\.\./,/,g;t loop' < $@.$$$$ > $@; \
rm -f $@.$$$$
endef

define compile =
$(CC) $(CFLAGS) -c -o $@ $<
endef

define link =
$(CC) -o $@ $^ $(LDFLAGS)
endef

.PHONY: all clean test

all:	test

include $(testdepends)

test: $(testbinaries)
	@echo
	@echo starting tests
	@echo --------------
	@( for TEST in $^; do echo; echo test $$TEST:; $$TEST || exit 1; done; echo; echo PASS )

$(testbindir)/%: $(testbuilddir)/%.o
	$(link)

$(testbindir) $(testbuilddir):
	mkdir -p $@

$(testobjects) $(testdepends): | $(testbuilddir)

$(testbinaries): | $(testbindir)

clean:
	rm -f  $(testsrcdir)/*~
	rm -rf $(testbindir)
	rm -rf $(testbuilddir)

$(testbuilddir)/%.o: $(testsrcdir)/%.c $(testbuilddir)/%.d
	$(compile)

$(testbuilddir)/%.d: $(testsrcdir)/%.c
	$(gendep)

$(testobjects) $(testdepends): Makefile

