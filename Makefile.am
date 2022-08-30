ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src . tests

format:
	clang-format -i src/*.c src/*.h tests/*.c

gdb: src/simulate
	libtool --mode=execute gdb --args ./src/simulate -dv test-id

valgrind: src/simulate
	libtool --mode=execute valgrind --leak-check=full ./src/simulate -dv test-id
