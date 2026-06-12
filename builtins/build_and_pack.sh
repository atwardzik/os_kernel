set -e
make clean
make all
make initramfs
find initramfs | cpio -o -H newc > initrfs.cio
# hexdump -v -e '1/1 "0x%02x, "' initramfs.cpio
