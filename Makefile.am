ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src . tests

format:
	clang-format -i src/*.c src/*.h tests/*.c

run:
	./src/simulate testid

gdb:
	libtool --mode=execute gdb --args ./src/simulate testid

valgrind:
	libtool --mode=execute valgrind --leak-check=full ./src/simulate testid

super-clean:
	git clean -fx
