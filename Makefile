all: bin/gfwrap

# Build the binary from the source file in `src/`.
bin/gfwrap: src/main.c
	@mkdir -p bin 2>nul || if not exist bin mkdir bin
	gcc -o bin/gfwrap src/*.c

# test: test.c
# # 	gcc -lrt -lm -o test test.c calculator.c utils.c history.c
# 	gcc test.c calculator.c utils.c history.c -lm -o test

# memtest: main.c
# 	gcc -fsanitize=address -g -o memapp main.c calculator.c utils.c history.c

clean:
	rm -f bin/gfwrap