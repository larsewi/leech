ACLOCAL_AMFLAGS = -I m4
SUBDIRS = leech simulate . tests

format:
	clang-format -i leech/*.c leech/*.h simulate/*.c simulate/*.h tests/*.c

run: simulate/simulate
	./simulate/simulate -d testid

gdb: simulate/simulate
	libtool --mode=execute gdb --args ./simulate/simulate -d testid

valgrind: simulate/simulate
	libtool --mode=execute valgrind --leak-check=full ./simulate/simulate -d testid

super-clean:
	git clean -fx
