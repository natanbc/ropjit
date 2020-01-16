CFLAGS= -g -I .
OUTPUT=ropjit

all: codegen
	${CC} ${CFLAGS} gadgets.s ropjit.s examples/helloworld.c -o ${OUTPUT}

codegen:
	./gadgets.py

run: all
	./${OUTPUT}

clean:
	rm gadgets.s gadgets.h ${OUTPUT}
