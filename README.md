# wooby

A silly little game I'm trying to make because I've never actually finished a
game before.

# Building

You can cross-compile for Windows if mingw is installed, but the build system
assumes it's being run on linux. You can probably still build it on Windows you
just need to use WSL or something and then copy the .exe out.

First you have to bootstrap nob:

```bash
cc -o nob nob.c
```

Then you have to run nob, pass the "--windows" flag to build for windows

```bash
./nob
./nob --windows
```

There is a "-r" flag for running as soon as the build is complete, I just added
that for quick debugging.
