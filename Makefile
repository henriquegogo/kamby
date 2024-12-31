all:
	@gcc -o kamby kamby.c

musl:
	@musl-gcc -static -o kamby kamby.c
	@strip kamby

memcheck: all
	@valgrind --leak-check=full -s ./kamby
