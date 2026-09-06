# QUICKSTART

This guide describes running a libhatchet version of "Hello World" on Ubuntu.

See the example directory for a program demonstrating more complex usage.

## Install Source and Tools

From the Ubuntu command line clone the head of the `main` branch and install
the build dependencies. (This does not install the WASM build tools.)

```bash
git clone https://github.com/whatchamacallem/libhatchet
cd libhatchet
./debian_packages.sh
```

## Run The Tests

Run this script to execute the full suite of test scripts. Three axe emoji
indicate success.

```bash
./testall.sh
```

## Hello World

From the repository top level directory, run these shell commands to delete any
existing `quickstart` directory, write `quickstart/hello_world/main.cpp` and
then compile and run it.

```bash
rm -rf quickstart
mkdir -p quickstart/hello_world
cat > quickstart/hello_world/main.cpp << 'EOF'
#include <hx/libhatchet.h>
int main(void) {
    hxinit();
    hxlog("hello world\n");
    hxexit(0);
}
EOF
g++ -std=c++23 -I include quickstart/hello_world/main.cpp src/*.cpp -o quickstart/hello_world/hello_world
./quickstart/hello_world/hello_world
```

Expected output:

```text
hello world
```

## Meson Hello World

Run these shell commands to reuse `quickstart/hello_world/main.cpp` from the
previous section and build it with Meson instead.

```bash
mkdir -p quickstart/hello_world/subprojects
ln -sfn ../../.. quickstart/hello_world/subprojects/libhatchet
cat > quickstart/hello_world/meson.build << 'EOF'
project('hello_world', ['c', 'cpp'], default_options : ['cpp_std=c++23'])
libhatchet = subproject('libhatchet')
libhatchet_dep = libhatchet.get_variable('libhatchet_dep')
hxwarning_args = libhatchet.get_variable('hxwarning_args')
executable('hello_world', ['main.cpp'], dependencies : libhatchet_dep, cpp_args : hxwarning_args)
EOF
meson setup quickstart/hello_world_meson quickstart/hello_world
meson compile -C quickstart/hello_world_meson
./quickstart/hello_world_meson/hello_world
```

Same expected output.

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
