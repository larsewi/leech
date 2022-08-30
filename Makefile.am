ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src . tests

format:
	clang-format -i src/*.c src/*.h tests/*.c

run:
	./src/simulate $(ARG)

gdb:
	libtool --mode=execute gdb --args ./src/simulate $(ARG)

valgrind:
	libtool --mode=execute valgrind --leak-check=full ./src/simulate $(ARG)

super-clean:
	git clean -fx
