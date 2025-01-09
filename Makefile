
TARGET	=	parsgen
DEPS	=	$(TARGET).deps
CC 	= 	gcc
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

all: $(TARGET)

%.o: %.c
	$(CC) -c $(OPT) $< -o $@

$(DEPS): $(OBJS:%.o=%.c)
	$(CC) -MM $^ > $(DEPS)

$(TARGET): $(OBJS) $(DEPS)
	$(CC) $(OPT) -o $@ $(OBJS)

scan.gen.h scanner.c: scanner.l
	flex scanner.l

scanner.o: scanner.c scan.gen.h
	$(CC) -c -g -std=c11 -Wno-implicit-function-declaration $< -o $@

include $(DEPS)

clean:
	rm -f scanner.c scan.gen.h $(TARGET) $(OBJS) $(DEPS)
