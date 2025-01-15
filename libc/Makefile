CC = ${CONOS_CC}
AS = ${CONOS_AS}
AR = ${CONOS_AR}
LD = ${CONOS_LD}

all: libc.a
	${CC} test2.c -o test2 -Iinclude -Llib -lc

libc.a:
	${CC} -c src/test.c -o src/test.o
	${AS} crt0.S crti.S crtn.S -o crt0.o
	${AR} rcs lib/libc.a src/test.o crt0.o

clean:
	rm -rf *.o *.a
	rm -rf ./lib/*.o ./lib/*.a
	rm -rf ./src/*.o ./src/*.a