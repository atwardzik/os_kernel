set -e
make clean
make dirs
mkdir -p build
make all
find initramfs | cpio -o -H newc > initramfs.cpio
# hexdump -v -e '1/1 "0x%02x, "' initramfs.cpio
