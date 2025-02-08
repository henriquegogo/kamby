all:
	@gcc -o kamby kamby.c

musl:
	@musl-gcc -static -o kamby kamby.c
	@strip kamby

memcheck: all
	@gcc -o tests tests.c
	@valgrind --leak-check=full -s ./tests
	@rm -f tests

test:
	@gcc -o tests tests.c
	@valgrind ./tests
	@rm -f tests

clean:
	@rm -f kamby
	@rm -f tests

.PHONY : all musl memcheck test clean
