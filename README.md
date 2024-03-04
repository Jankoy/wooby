# wooby

A silly little game I'm trying to make because I've never actually finished a
game before.

# Building

First you have to bootstrap nob, use mingw if you are on windows:

```bash
cc -o nob nob.c
```

Then you have to run nob, pass the "--windows" flag to build for windows from
linux or the "--linux" flag to build for linux from windows. It should
auto-detect your platform if you are not cross-compiling though. No mac support.

```bash
./nob
./nob --windows
```

There is a "-r" flag for running as soon as the build is complete, I just added
that for quick debugging, it should be the last argument as everything
afterwards is passed to the executable as arguments.
