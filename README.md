# ROP_Python

A Houdini ROP that executes an embedded python script

## How To Build

On a fresh clone, don't forget to update git submodules:
```bash
git submodule update --init
```

To build using a standard houdini install:
``bash
scons with-houdini=15.5.565
```

To build using a non standard houdini installation directory:
```bash
scons with-houdini=/usr/local/sidefix/houdini/15.5.565
```
*Note*: On OSX, this should be the path of the Houdini Framework

To list all available build targets and options:
```bash
scons -h
```


