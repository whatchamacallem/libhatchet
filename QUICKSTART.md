# QUICKSTART

This guide describes running a libhatchet version of "Axe World" on Ubuntu.

See the example directory for a program demonstrating more complex usage.

## Install Source and Tools

From the Ubuntu command line clone the head of the `main` branch and install
the build dependencies.

```bash
git clone https://github.com/whatchamacallem/libhatchet
cd libhatchet
./install_deb.sh --sdks
```

## Run The Tests

Run this script to execute the full suite of test scripts. Three axe emoji
indicate success.

```bash
./test_all.sh
```

## Axe World

From the repository top level directory, run these shell commands to delete any
existing `quickstart` directory, write `quickstart/axe_world/main.cpp` and
then compile and run it.

```bash
rm -rf quickstart
mkdir -p quickstart/axe_world
cat > quickstart/axe_world/main.cpp << 'EOF'
#include <hx/libhatchet.h>
int main(void) {
    hxinit();
    hxlog_verbose("🪓🪓🪓\n");
    hxexit(0);
}
EOF
g++ -std=c++23 -I include quickstart/axe_world/main.cpp src/*.cpp -o quickstart/axe_world/axe_world
./quickstart/axe_world/axe_world
```

Expected output: 🪓🪓🪓

## Meson Axe World

Run these shell commands to reuse `quickstart/axe_world/main.cpp` from the
previous section and build it with Meson instead.

```bash
mkdir -p quickstart/axe_world/subprojects
ln -sfn ../../.. quickstart/axe_world/subprojects/libhatchet
cat > quickstart/axe_world/meson.build << 'EOF'
project('axe_world', ['c', 'cpp'], default_options : ['cpp_std=c++23'])
libhatchet = subproject('libhatchet')
libhatchet_dep = libhatchet.get_variable('libhatchet_dep')
hxwarning_args = libhatchet.get_variable('hxwarning_args')
executable('axe_world', ['main.cpp'], dependencies : libhatchet_dep, cpp_args : hxwarning_args)
EOF
meson setup quickstart/axe_world_meson quickstart/axe_world
meson compile -C quickstart/axe_world_meson
./quickstart/axe_world_meson/axe_world
```

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
