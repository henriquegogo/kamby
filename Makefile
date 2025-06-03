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
	gcov $(TESTNAME).c
	@$(MAKE) --no-print-directory clean

clean:
	@rm -f $(BINNAME)
	@rm -f $(TESTNAME)
	@rm -f *.gc*
	@rm -f output.out
