# filesizemd

> files' sizes as a markdown table (CLI)

[![Build](https://github.com/barelyhuman/filesizemd/actions/workflows/test.yml/badge.svg)](https://github.com/barelyhuman/filesizemd/actions/workflows/test.yml)

## Build

You'll need [zlib](https://zlib.net/) to be able to compile the binary

```sh
# to build
make
```

## Usage

```sh
# if you tty expansion for glob is disabled
filesizemd lib/*

# if it's enabled and the shell is expanding the glob
filesizemd "lib/*"
```

**Output**

```
|file|size|gzip|brotli|
|---|---|---|---|
|deflate.c|2.05KB|893B|948B|
|deflate.h|131B|91B|87B|
```

## License

[MIT](license)
