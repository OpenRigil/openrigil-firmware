# openrigil-firmware

This is the firmware repo for OpenRigil. This includes accelerated crypto implementation and drivers for our peripherals.

## Build

First you should build a lateset riscv-gnu-toolchain with `rv32imac` ARCH and `ilp32` ABI.

```
# get riscv-gnu-toolchain repo then
cd riscv-gnu-toolchain
./configure --with-arch=rv32imac --with-abi=ilp32
```

After including the built binaries in your `PATH`, you could

```
cd openrigil-firmware
mkdir build; cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../riscv32.cmake ..
make -j`nproc`
```

Now you could find `openrigil.hex` in the artifacts. Now you should see the guide in [openrigil-rtl](https://github.com/OpenRigil/openrigil-rtl).

## DISCLAIMER

This is not secure and should not be used in production. Use with caution!
