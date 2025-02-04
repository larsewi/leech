ACLOCAL_AMFLAGS = -I m4
SUBDIRS = lib bin . tests

format:
	clang-format -i lib/*.{c,h} bin/*.{c,h} tests/*.c tests/unit/*.c
	black *.py tests/*.py

super-clean:
	git clean -fxd
	rm -rf simulate/**/.leech
