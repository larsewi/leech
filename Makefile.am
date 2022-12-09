ACLOCAL_AMFLAGS = -I m4
SUBDIRS = lib bin . tests

format:
	clang-format -i lib/*.c lib/*.h bin/*.c tests/*.c

super-clean:
	git clean -fxd
