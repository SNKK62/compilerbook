# compilerbook

a source code I learned from 'https://www.sigbus.info/compilerbook' : the tutorial of building compiler of clang for people who wants to know low layer programs.

### How to setup

```sh
$ docker build ./ -t compilerbook
```

### Running docker image sample

```sh
$ docker run --rm compilerbook ls /
```

### test with container

```sh
$ make it
$ make 9cc
$ make test
```

### run container with interactive mode

```sh
$ docker run --rm -it -v $PWD/9cc:/9cc -w /9cc compilerbook
```
