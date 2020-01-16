CFLAGS=-g
OUTPUT=ropjit

ifeq (${EXAMPLE},)
EXAMPLE=helloworld
endif

all: codegen
	${CC} ${CFLAGS}             \
        -I generated -I include \
        generated/gadgets.s     \
        generated/gadgets.c     \
        src/ropjit.s            \
        examples/${EXAMPLE}.c   \
        -o ${OUTPUT}

codegen:
	mkdir -p generated
	./gadgets.py

run: all
	./${OUTPUT}

clean:
	rm -rf generated ${OUTPUT}
