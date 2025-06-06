BINNAME = kamby
TESTNAME = tests

all:
	@$(CC) -o $(BINNAME) $(BINNAME).c

run: all
	@./$(BINNAME)

test:
	@$(CC) -fprofile-arcs -ftest-coverage -o $(TESTNAME) $(TESTNAME).c
	@$(CC) -shared -o $(TESTNAME)lib.so -fPIC $(BINNAME).c
	@echo "input" | $(TESTPREFIX) ./$(TESTNAME)
	@$(TESTPOST)
	@$(MAKE) --no-print-directory clean

testmemory: TESTPREFIX := valgrind
testmemory: test

coverage: TESTPOST := {\
	output=$$(gcov $(TESTNAME).c | grep -A1 "'$(BINNAME).h'");\
	cat $(BINNAME).h.gcov | grep -C1 "#####";\
	echo "\n$$output";\
}
coverage: test

coveragememory: TESTPREFIX := valgrind
coveragememory: coverage

wasm:
	@emcc -O3 -o $(BINNAME).wasm $(BINNAME).c

clean:
	@rm -f $(BINNAME)
	@rm -f $(BINNAME).wasm
	@rm -f $(TESTNAME)
	@rm -f $(TESTNAME).out
	@rm -f $(TESTNAME)lib.so
	@rm -f *.gc*
