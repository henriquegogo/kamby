BINNAME = kamby
LIBNAME = lib$(BINNAME).so

all: lib bin

lib:
	$(CC) -c -fpic -o $(BINNAME).o $(BINNAME).c
	$(CC) -shared -o $(LIBNAME) $(BINNAME).o
	rm $(BINNAME).o

bin:
	$(CC) -o $(BINNAME) cli.c $(BINNAME).c

shared: lib
	$(CC) -o $(BINNAME) cli.c -L`pwd` -lkamby

static:
	$(CC) -static -o $(BINNAME) cli.c $(BINNAME).c

clean:
	test -e $(BINNAME) && rm $(BINNAME) || true
	test -e $(LIBNAME) && rm $(LIBNAME) || true
	test -e tests && rm tests || true

script: bin
	@./$(BINNAME) ./script.kmb
	@make clean >> /dev/null

test: lib
	$(CC) -o tests tests.c -L`pwd` -lkamby
	@LD_LIBRARY_PATH=`pwd` ./tests
	@make clean >> /dev/null

prompt: bin
	@./$(BINNAME)
	@make clean >> /dev/null

debug:
	$(CC) -g -o $(BINNAME) cli.c $(BINNAME).c
	gdb ./$(BINNAME)
	@make clean >> /dev/null

wasm:
	emcc --no-entry -O1 -o kamby.wasm run.c kamby.c -sEXPORTED_FUNCTIONS=_ka_run -sEXPORTED_RUNTIME_METHODS=cwrap
