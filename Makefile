
HIDE	=	@
TARGET	=	parsgen
DEPS	=	$(TARGET).deps
CC 	= 	clang
OBJS	=	scanner.o \
		main.o \
		parser.o \
		ast.o \
		errors.o \
		memory.o \
		regurge.o \
		emit.o \
		emit_pass1.o \
		emit_pass2.o \
		scanner_support.o \
		pointer_list.o

DEBUG	=	-g
OPT 	= 	$(DEBUG) -std=c11 -Wall -Wextra -Wpedantic -pedantic

all: $(TARGET)

%.o: %.c
	@echo "build $@"
	$(HIDE)$(CC) -c $(OPT) $< -o $@

$(DEPS): $(OBJS:%.o=%.c)
	@echo "make depends"
	$(HIDE)$(CC) -MM $^ > $(DEPS)

$(TARGET): $(OBJS) $(DEPS)
	@echo "make $(TARGET)"
	$(HIDE)$(CC) $(OPT) -o $@ $(OBJS)

scan.gen.h scanner.c: scanner.l
	@echo "build scanner.l"
	$(HIDE)flex scanner.l

scanner.o: scanner.c scan.gen.h
	@echo "build $@"
	$(HIDE)$(CC) -c -g -std=c11 -Wno-implicit-function-declaration $< -o $@

include $(DEPS)

clean:
	@echo "clean"
	@rm -f scanner.c scan.gen.h $(TARGET) $(OBJS) $(DEPS)
