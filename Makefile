BINNAME = kamby
TESTNAME = tests

all:
	@$(CC) -o $(BINNAME) $(BINNAME).c

run: all
	@./$(BINNAME)

test:
	@$(CC) -o $(TESTNAME) $(TESTNAME).c
	@$(CC) -shared -o $(TESTNAME)lib.so -fPIC $(BINNAME).c
	@echo "input" | ./$(TESTNAME)
	@$(MAKE) --no-print-directory clean

testmemory:
	@$(CC) -o $(TESTNAME) $(TESTNAME).c
	@$(CC) -shared -o $(TESTNAME)lib.so -fPIC $(BINNAME).c
	@echo "input" | valgrind ./$(TESTNAME)
	@$(MAKE) --no-print-directory clean

coverage:
	@$(CC) -fprofile-arcs -ftest-coverage -o $(TESTNAME) $(TESTNAME).c
	@$(CC) -shared -o $(TESTNAME)lib.so -fPIC $(BINNAME).c
	@echo "input" | ./$(TESTNAME)
	@{\
		output=$$(gcov $(TESTNAME).c | grep -A1 "'$(BINNAME).h'");\
		cat $(BINNAME).h.gcov | grep -C1 "#####";\
		echo "\n$$output";\
	}
	@$(MAKE) --no-print-directory clean

clean:
	@rm -f $(BINNAME)
	@rm -f $(TESTNAME)
	@rm -f *.gc*
	@rm -f output.out
	@rm -f testslib.so
