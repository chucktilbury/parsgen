

CC = gcc
OBJS	=	scanner.o \
			main.o \
			parser.o \
			ast.o \
			errors.o \
			memory.o \
			regurge.o \
			scanner_support.o \
			pointer_list.o

OPT = -g -std=c11 -Wall -Wextra -Wpedantic -pedantic

%.o: %.c
	$(CC) -c $(OPT) $< -o $@

simp: $(OBJS)
	$(CC) $(OPT) -o $@ $^

scan.gen.h scanner.c: scanner.l
	flex scanner.l

scanner.o: scanner.c scan.gen.h
	$(CC) -c -g -std=c11 -Wno-implicit-function-declaration $< -o $@

parser.o: parser.c parser.h
ast.o: ast.c ast.h
pointer_list.o: pointer_list.c pointer_list.h
errors.o: errors.c errors.h
memory.o: memory.c memory.h
regurge.o: regurge.c regurge.h
scanner_support.o: scanner_support.c scanner.h
main.o: main.c

clean:
	rm -f scanner.c scan.gen.h simp $(OBJS)
