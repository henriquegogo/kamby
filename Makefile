BINNAME = kamby
TESTNAME = tests

all:
	@$(CC) -o $(BINNAME) $(BINNAME).c

run: all
	@./$(BINNAME)

test:
	@$(CC) -o $(TESTNAME) $(TESTNAME).c
	@./$(TESTNAME)
	@$(MAKE) --no-print-directory clean

testmemory:
	@$(CC) -o $(TESTNAME) $(TESTNAME).c
	@valgrind ./$(TESTNAME)
	@$(MAKE) --no-print-directory clean

coverage:
	@$(CC) -fprofile-arcs -ftest-coverage -o $(TESTNAME) $(TESTNAME).c
	@./$(TESTNAME)
	@{\
		output=$$(gcov $(TESTNAME).c);\
		cat kamby.h.gcov | grep -C1 "#####";\
		echo "\n$$output";\
	}
	@$(MAKE) --no-print-directory clean

clean:
	@rm -f $(BINNAME)
	@rm -f $(TESTNAME)
	@rm -f *.gc*
	@rm -f output.out
