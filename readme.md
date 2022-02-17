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
|file|size|gzip|
|---|---|---|
|deflate.c|2.05KB|49B|
|deflate.h|132B|92B|
```

## TODO

- Add compressed output from [brotli](https://gitub.com/google/brotli)

## License

[MIT](license)
