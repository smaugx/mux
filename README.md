# mux
some try about network-programming.


# build

first, you should get the source code:

```
git clone https://github.com/smaugx/mux.git
cd mux
```

we support to build using two tools: **scons** 、**cmake**

## use scons

you just need runing:

```
scons
```

then check dir `sbuild`, all targets will be this dir.

if you can't find this command in your machine, mayby you should install this tool first:

```
pip install scons
```

Is it super easy?

if you think using scons to  build is slow, try this:

```
scons -j4
```

> similar to `make -j4`.


if you want to clean the build-objects, try this:

```
scons -c
```

Good Luck!


## use cmake

usually, run this:

```
mkdir cbuild
cd cbuild
cmake ..
make -j4
```

then check `cbuild/bin` or `cbuild/lib` dir.

