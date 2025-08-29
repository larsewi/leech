# Contributing

## Dependencies:

### On Debian:
```
sudo apt-get install autoconf automake build-essential libtool-bin check \
clang-format pkg-config libpq-dev
```

### On macOS:
```
brew install autoconf automake libtool check clang-format pkg-config libpq
```

## Build:
```
./bootstrap.sh
./configure --enable-debug \
    --with-check-framework \
    --with-test-binary \
    --with-csv-module \
    --with-psql-module
make
```

## Run unit tests with GDB:
```
cd tests/
libtool --mode=execute gdb --args ./unit_test no-fork
```

## Release new version

Bump version number in _configure.ac_, e.g.:

```
-AC_INIT([leech], [0.1.23], [https://github.com/larsewi/leech/issues], [leech],
+AC_INIT([leech], [0.1.24], [https://github.com/larsewi/leech/issues], [leech],
```

Create Pull Request and wait for it to be merged

```
git add configure.ac
git commit -s -m "Bumped version number from 0.1.23 to 0.1.24"
gh pr create
```

Maintainer will (hopefully) continue the release process from here

## Maintainer release instructions

```
git checkout latest
git fetch --all
git rebase origin/master
git push
git clean -fxd
./bootstrap.sh
./configure
make dist
sha256sum leech-0.1.24.tar.gz # Add output to release notes later
```

- Try compiling the tarball and run tests
- Goto https://github.com/larsewi/leech/releases and click `Draft a new release`
- In chose tag, write `v0.1.24`, target `latest`
- Click `Generate release notes`
- Upload tarball `leech-0.1.24.tar.gz`
- Try to unpack tarball `tar -xf leech-0.1.24.tar.gz`
    * Run `make check` after configuring with non and all configure options
- Click publish release
- Done!
