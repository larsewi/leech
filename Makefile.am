ACLOCAL_AMFLAGS = -I m4
SUBDIRS = leech doofus . tests

format:
	clang-format -i leech/*.c leech/*.h doofus/*.c tests/*.c

super-clean:
	git clean -fxd
