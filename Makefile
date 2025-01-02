all:
	@gcc -o kamby kamby.c

musl:
	@musl-gcc -static -o kamby kamby.c
	@strip kamby

memcheck: all
	@valgrind --leak-check=full -s ./kamby

test:
	@gcc -o tests tests.c
	@valgrind ./tests
	@rm -f tests

clean:
	@rm -f kamby
	@rm -f tests

.PHONY : all musl memcheck test clean
