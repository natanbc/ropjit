CFLAGS=-g
OUTPUT=ropjit

all: codegen
	${CC} ${CFLAGS} gadgets.s ropjit.s main.c -o ${OUTPUT}

codegen:
	./gadgets.py

run: all
	./${OUTPUT}
