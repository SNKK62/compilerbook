# compilerbook

a source code I learned from 'https://www.sigbus.info/compilerbook' : the tutorial of building compiler of clang for people who wants to know low layer programs.

### How to setup

```sh
$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
```

### Running docker image sample

```sh
$ docker run --rm compilerbook ls /
```

### test with container

```sh
$ docker run --rm -v $PWD/9cc:/9cc -w /9cc compilerbook make test
```

### run container with interactive mode

```sh
$ docker run --rm -it -v $PWD/9cc:/9cc -w /9cc compilerbook
```
