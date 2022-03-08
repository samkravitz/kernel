# maestro

## My symphony in C.

### Requirements
- [qemu](https://www.qemu.org/) (specifically qemu-system-i386)
- [nasm](https://nasm.us/)
- [e2tools](https://github.com/e2tools/e2tools)
- An i686 cross-compiler (on x64 Linux machines, this should be achievable with some gcc flags)
- At least 512M of space for the disk image

### Usage
create disk image: (this only needs to be done once) \
`make disk` \
build: \
`make` \
launch: \
`make start`
