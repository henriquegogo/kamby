BINNAME = kamby
TESTNAME = tests

all:
	@$(CC) -o $(BINNAME) $(BINNAME).c

run: all
	@./$(BINNAME)

test:
	@$(CC) -fprofile-arcs -ftest-coverage -o $(TESTNAME) $(TESTNAME).c
	@$(CC) -shared -o $(TESTNAME)lib.so -fPIC $(BINNAME).c
	@echo "99" | $(TESTPREFIX) ./$(TESTNAME) 2>&1 | \
		grep --color=never -E "^(==.*(total heap usage|ERROR SUMMARY)|[^=]|^$$)" |\
		sed 's/==[^=]*==[^:]*: //'
	@$(TESTPOST)
	@$(MAKE) --no-print-directory clean

testmemory: TESTPREFIX := valgrind
testmemory: test

coverage: TESTPOST := \
	output=$$(gcov $(TESTNAME).c | grep -A1 "'$(BINNAME).h'");\
	cat $(BINNAME).h.gcov | grep -C1 "#####";\
	echo "\n$$output";
coverage: test

coveragememory: TESTPREFIX := valgrind
coveragememory: coverage

wasm:
	@emcc -O3 -o $(BINNAME).html $(BINNAME).c -sSTACK_SIZE=2mb

clean:
	@rm -f $(BINNAME) $(BINNAME).wasm $(BINNAME).html $(BINNAME).js
	@rm -f $(TESTNAME) $(TESTNAME).out $(TESTNAME)lib.so
	@rm -f *.gc*
