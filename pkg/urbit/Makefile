include config.mk

jets = jets/tree.c $(wildcard jets/*/*.c)
noun = $(wildcard noun/*.c)
vere = $(wildcard vere/*.c)
daemon = $(wildcard daemon/*.c)
worker = $(wildcard worker/*.c)

common  = $(jets) $(noun) $(vere)
headers = $(shell find include -type f)

common_objs = $(shell echo $(common) | sed 's/\.c/.o/g')
daemon_objs = $(shell echo $(daemon) | sed 's/\.c/.o/g')
worker_objs = $(shell echo $(worker) | sed 's/\.c/.o/g')

all_objs = $(common_objs) $(daemon_objs) $(worker_objs)
all_srcs = $(common) $(daemon) $(worker)
all_exes = ./noun_tests ./hash_tests ./hashtable_tests ./urbit ./urbit-worker

# -Werror promotes all warnings that are enabled into errors (this is on)
# -Wconversion warns on all casts that might lose data. DO NOT TURN THIS OFF. Use explicit cast funcs in allocate.h.
# -Wall issues all types of errors.  This is off (for now)
CFLAGS := $(CFLAGS) -Werror -Wconversion

################################################################################

.PHONY: all test clean mkproper

################################################################################

all: urbit urbit-worker hashtable_tests hash_tests

test: noun_tests hashtable_tests hash_tests
	./noun_tests
	./hashtable_tests
	./hash_tests

clean:
	rm -f ./tags $(all_objs) $(all_exes)

mrproper: clean
	rm -f config.mk include/config.h

################################################################################

noun_tests: $(common_objs) tests/noun_tests.o
	@echo CC -o $@
	@$(CC) $^ $(LDFLAGS) -o $@

hashtable_tests: $(common_objs) tests/hashtable_tests.o
	@echo CC -o $@
	@$(CC) $^ $(LDFLAGS) -o $@

hash_tests: $(common_objs) tests/hash_tests.o
	@echo CC -o $@
	@$(CC) $^ $(LDFLAGS) -o $@

urbit: $(common_objs) $(daemon_objs)
	@echo CC -o $@
	@$(CC) $^ $(LDFLAGS) -o $@

urbit-worker: $(common_objs) $(worker_objs)
	@echo CC -o $@
	@$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c $(headers)
	@echo CC $<
	@$(CC) -I./include $(CFLAGS) -c $< -o $@

tags: $(all_srcs) $(headers)
	ctags $^
