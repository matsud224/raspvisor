# raspvisor
A simple type-1 hypervisor on Raspberry Pi 3 (aarch64)

*Warning: This is a hobby project and not for practical use.*

# Setup
First, write a Raspberry Pi OS (previously called Raspbian) image to your SD card to make partitions and install firmwares.
```
$ export SD_BOOT_DIR=/path/to/bootpartiton/on/sdcard
$ export SD_ROOTFS_DIR=/path/to/rootfspartiton/on/sdcard
$ make install
```

Programs runs on a hypervisor startup are hard-coded in `src/main.c`. In `example` directory, following programs are found.
* test_binary : issues hypervisor call once.
* echo : UART echo back (based on [raspberry-pi-os/lesson02](https://github.com/s-matyukevich/raspberry-pi-os/tree/master/src/lesson02))
* mini-os : mini operating system which has process scheduler, interrupt and virtual memory support (based on [raspberry-pi-os/lesson06](https://github.com/s-matyukevich/raspberry-pi-os/tree/master/src/lesson06))

Enter each directory and `make` to build. Then copy `*/bin` file to `SD_BOOT_DIR`.

# License

This project is under the MIT license.

Source codes and examples (echo and mini-os) are mostly based on [Sergey Matyukevich's raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os).
