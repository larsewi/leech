# leech

![logo](logo.jpeg)

```mermaid
    graph TD;
            C1-->S;
            C2-->S;
            C3-->S;
```

## Prerequesites

### On debian:
Dependencies:
```
sudo apt-get install autoconf automake build-essential libtool-bin check \
clang-format pkg-config libssl-dev
```

### On macOS:
Dependencies:
```
brew install autoconf automake libtool openssl check clang-format pkg-config
```

For compilers to find OpenSSL:
```
export LDFLAGS="-L$(brew --prefix openssl)/lib"
export CPPFLAGS="-I$(brew --prefix openssl)/include"
```

For pkg-config to find OpenSSL:
```
export PKG_CONFIG_PATH="$(brew --prefix openssl)/lib/pkgconfig"
```

## Install
`./bootstrap.sh && ./configure --enable-debug && make && sudo make install`



## TODO: [doxygen](https://www.gnu.org/software/autoconf-archive/ax_prog_doxygen.html)

## Block

```json
{
    "version": "<VERSION_NUMBER>",
    "parent": "<PARENT_IDENTIFIER>",
    "timestamp": "<TIMESTAMP>",
    "payload" : [ ... ]
}
```

## Delta

```json
{
    "type": "<PAYLOAD_TYPE>",
    "version": "<VERSION_NUMBER>",
    "table": "<TABLE_IDENTIFIER>",
    "insert": {
        "<PRIMARY_KEY>": "<SUBSIDIARY_KEY>",
        ...
    },
    "update": {
        "<PRIMARY_KEY>": "<SUBSIDIARY_KEY>",
        ...
    },
    "delete": {
        "<PRIMARY_KEY>": "<SUBSIDIARY_KEY>",
        ...
    }
}
```

## Rebase

```json
{
    "type": "<PAYLOAD_TYPE>",
    "version": "<VERSION_NUMBER>",
    "id": "<TABLE_IDENTIFIER>",
    "records": [
        "<TABLE_RECORD>",
        ...
    ]
}
```
