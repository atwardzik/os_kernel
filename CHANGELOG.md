# Changelog
# v0.1.0 (2026-06-07)

### Features

* add basic file structure for next
  features” ([23b5068](https://gitlab.com/atwardzik/os_kernel/commit/23b506899aaa920e877e591e62905ea0f58ec048))
* add basic PIO codes for generating VGA signals for both 640x480 and
  800x600 ([e2fc266](https://gitlab.com/atwardzik/os_kernel/commit/e2fc26613fb781d3b65948042be8300445633e6b))
* add basic syscalls
  codes ([bcbadf2](https://gitlab.com/atwardzik/os_kernel/commit/bcbadf2b5225c6de1a1af8ae9a447e0c6ff53d0a))
* add runtime configuration
  struct ([30cbc8c](https://gitlab.com/atwardzik/os_kernel/commit/30cbc8c31e5ec00bd1e501f56a95d3eeecd4ee38))
* add script for extracting mnemonics from disassembly view for future use in assembler
  implementation ([a8e1857](https://gitlab.com/atwardzik/os_kernel/commit/a8e1857258fb4438e451d895ef89a753cc53565a))
* **allocator:** add max size
  check ([4436df4](https://gitlab.com/atwardzik/os_kernel/commit/4436df48020a55835ad2c9714f2998f51abd318b))
* **builtins:** add advanced
  cat ([11fb26b](https://gitlab.com/atwardzik/os_kernel/commit/11fb26b8c49d77850c732212ddbf17701eefc89b))
* **builtins:** add advanced
  echo ([a56c4a6](https://gitlab.com/atwardzik/os_kernel/commit/a56c4a6617c496854086d7472e6ab93b4e543f86))
* **builtins:** add
  kill ([87f7111](https://gitlab.com/atwardzik/os_kernel/commit/87f7111a18e8179ee6cf4823c453d51cc6eb901a))
* **builtins:** implement basic shell as a user process, possible to compile at any
  machine ([bfd7d21](https://gitlab.com/atwardzik/os_kernel/commit/bfd7d21a318f2e389b0288000c3b34166ba9c250))
* **builtins:** run user defined shell from
  init ([8747800](https://gitlab.com/atwardzik/os_kernel/commit/8747800ceacc1448d51111e23a61b749810c802e))
* **error:** implement basic kernel panic with macro for unimplemented
  features ([937546a](https://gitlab.com/atwardzik/os_kernel/commit/937546a6f2ef475c89dffab8aefca82c169756b3))
* **eth:** add receiving in
  TCP ([7f5ac60](https://gitlab.com/atwardzik/os_kernel/commit/7f5ac60605c1c1566a23bd65772bf40467db0e20))
* **eth:** add simple example of a www
  server ([7d23818](https://gitlab.com/atwardzik/os_kernel/commit/7d23818444bb374ca5a456f1fb3e5ee4ead2e0d1))
* **eth:** implement basic driver for wiznet ethernet chip containing ARP and PING
  support ([ed5dec2](https://gitlab.com/atwardzik/os_kernel/commit/ed5dec297775b8e5397890a5192b1be43a28eb6b))
* **eth:** implement TCP server establishing connection and sending
  data ([bb7c8fb](https://gitlab.com/atwardzik/os_kernel/commit/bb7c8fb4542a6851f159292c948539abb4a75789))
* **eth:** increase speed by setting SPICLK to about 3MHz. Use full ring buffer on chip to transmit
  data ([a942cbe](https://gitlab.com/atwardzik/os_kernel/commit/a942cbe65863152e82ffddaeb1ffdba6d8573965))
* **fs:** add basic
  ramfs ([e725ab4](https://gitlab.com/atwardzik/os_kernel/commit/e725ab4fd01fd16f625fb275871cb751e190620a))
* **fs:** add directories
  support ([b3246ea](https://gitlab.com/atwardzik/os_kernel/commit/b3246ea78d79f77e9842629a47a62df220b201da))
* **fs:** add FAT16 basic file creation and
  write ([09ce7d2](https://gitlab.com/atwardzik/os_kernel/commit/09ce7d20bfab972a50de478ccf74149568d28082))
* **fs:** add length of the buffer while reading bytes from the sd
  card ([2495f6c](https://gitlab.com/atwardzik/os_kernel/commit/2495f6cfd1cab494f8ec3037adac21273564a739))
* **fs:** add lookup for root directory in FAT16; add owned inode queue to process; WIP in trying to read FAT
  chain ([eddf1eb](https://gitlab.com/atwardzik/os_kernel/commit/eddf1eb4b3e46fbc51778ae76835fa254f70a698))
* **fs:** add parent inode to each inode for fs
  traversal ([93aeb8b](https://gitlab.com/atwardzik/os_kernel/commit/93aeb8b37f791d8d1c6fdaf21abe5690684675ac))
* **fs:** add quasi-mounting
  FAT16 ([5f74bd9](https://gitlab.com/atwardzik/os_kernel/commit/5f74bd996df6c63c5c7cab582b1e04df5adc1aa7))
* **fs:** add readdir, lseek and ensure already opened filedescriptors are not
  duplicated ([0dd22d0](https://gitlab.com/atwardzik/os_kernel/commit/0dd22d05fb05ab3fa6ff9a6c732eff38b4540b4c))
* **fs:** add WIP following FAT chain and reading corresponding
  clusters ([ce096d3](https://gitlab.com/atwardzik/os_kernel/commit/ce096d31223616472ac98c5fbc533ac7c1d0b374))
* **fs:** cache current File Allocation Table
  sector ([ab1e945](https://gitlab.com/atwardzik/os_kernel/commit/ab1e945cadddd7f6480b5d9f51f09e0ba436bb5e))
* **fs:** implement all functions for reading FAT16 root
  directory ([99f8d50](https://gitlab.com/atwardzik/os_kernel/commit/99f8d50745cfd7256570acbacf62e9ff5205f0dd))
* **fs:** read FAT16 boot
  sector ([24bc770](https://gitlab.com/atwardzik/os_kernel/commit/24bc7709dfaf7d2a328b437a40ac68fa91a8b375))
* **fs:** update root directory entries (size) after writing to
  file ([b6292e7](https://gitlab.com/atwardzik/os_kernel/commit/b6292e76181d57835f303df85925f030e2b37e32))
* **fs:** WIP traverse FAT table while trying to read
  files ([9a56bb0](https://gitlab.com/atwardzik/os_kernel/commit/9a56bb064c3740608db82c99e3fea6415d5b2e00))
* implement extended ansi color
  codes ([b083b44](https://gitlab.com/atwardzik/os_kernel/commit/b083b445143d922cdaf56096748f2e8077d758ba))
* implement handler for hardware
  divider ([d516cb4](https://gitlab.com/atwardzik/os_kernel/commit/d516cb46ea884ad01a74e0d0e3e36ff38386c91d))
* implement loading pio program to pio instruction
  memory ([fb6e988](https://gitlab.com/atwardzik/os_kernel/commit/fb6e988f341daa3ec43b9421facd5f5eaca81260))
* **init:** add decompression of cpio files containing initramfs with some programs and allow running those
  programs ([f2c7d3b](https://gitlab.com/atwardzik/os_kernel/commit/f2c7d3b4bf7cbabc57e5ec1698656e1fa54e1418))
* **kernel:** print pretty startup
  messages ([f151c77](https://gitlab.com/atwardzik/os_kernel/commit/f151c77a50d12f989465ea6b37d5a6aa8af09e9f))
* **keyboard:** add a wrapper function for initializing serial
  communication ([ade1664](https://gitlab.com/atwardzik/os_kernel/commit/ade1664a4c82f7c916f2177247ad010073cbf636))
* **keyboard:** add multiple byte code sequence
  detection ([69190f8](https://gitlab.com/atwardzik/os_kernel/commit/69190f810cfc001500533d2b3588a72a3956c095))
* **keyboard:** add proper interrupt handling for serial
  communication ([ce6bf59](https://gitlab.com/atwardzik/os_kernel/commit/ce6bf594feff68c73ab403656ad9f5c45a8bc33a))
* **keyboard:** correctly interpret 2-byte sequences like
  arrows ([6ad1b90](https://gitlab.com/atwardzik/os_kernel/commit/6ad1b90129f4c1857270d990b10ddbd0efd5ab4b))
* **keyboard:** run program on state
  machine ([832a49b](https://gitlab.com/atwardzik/os_kernel/commit/832a49b9b41d24488e4e9989e8d393d6b4654283))
* **libc:** add
  strncasecmp ([7e2e58a](https://gitlab.com/atwardzik/os_kernel/commit/7e2e58a87296445603a13a631e0ab3ae2f2a1a8e))
* **loader:** add basic elf file
  parser ([878c965](https://gitlab.com/atwardzik/os_kernel/commit/878c965b5c2b00f9b187540245f52d302640beee))
* **loader:** add temporary r9 page for rwpi of dynamic libraries. Add linker flag to prevent PLT creation in .so
  files ([b14ffbe](https://gitlab.com/atwardzik/os_kernel/commit/b14ffbe688dba8a63933abab8ba38fbef1b4c3df))
* **loader:** apply relocations to the loaded
  file ([232e90e](https://gitlab.com/atwardzik/os_kernel/commit/232e90e42825f150ce61b1f3fe21c5633d99610c))
* **loader:** find _start in executables while
  loading ([ee86daa](https://gitlab.com/atwardzik/os_kernel/commit/ee86daa475f27ea1c6a483229aa86c250d8de59b))
* **loader:** parse dynamic elf files
  correctly ([7a9f562](https://gitlab.com/atwardzik/os_kernel/commit/7a9f562b6d638ef780ab40519f3f6cacc6e4bfe1))
* **loader:** parse dynamic library in search for symbols needed by an
  executable ([69e66e5](https://gitlab.com/atwardzik/os_kernel/commit/69e66e5322469763a24d3aff32f53707a2032659))
* **loader:** update builtins to use dynamic executables; load executables from elf
  files ([302bf17](https://gitlab.com/atwardzik/os_kernel/commit/302bf176a7340c65b9daf65b5bc04b2cc08afbd5))
* **memory:** add precise memory
  map ([c57c063](https://gitlab.com/atwardzik/os_kernel/commit/c57c0639a0ae5394d6549c637c2cf52bc7632b6e))
* **memory:** add reading mbr of selected hard
  drive ([ec1cdd6](https://gitlab.com/atwardzik/os_kernel/commit/ec1cdd63ed0e7f8927151350c915c8fe84e27f19))
* **memory:** implement simple hard drive
  interface ([08921a8](https://gitlab.com/atwardzik/os_kernel/commit/08921a81a494d60c846f394fbba546283838417a))
* move out initramfs from .code
  section ([fb095e8](https://gitlab.com/atwardzik/os_kernel/commit/fb095e8f7fb7a6823d4f0b22c03903c04a59e55d))
* **network:** add TCP server
  interface ([11623d5](https://gitlab.com/atwardzik/os_kernel/commit/11623d55d4a6f2ebac003081f9cc880cc30456cb))
* **network:** implement connect syscall and block process while waiting for
  input ([246698f](https://gitlab.com/atwardzik/os_kernel/commit/246698f81f55369ff862d25fc16cbdb359b30bae))
* **newlib:** add more proper syscalls for read and write so that it is now possible to write in the middle of
  text ([8f9d4d5](https://gitlab.com/atwardzik/os_kernel/commit/8f9d4d58e89c0b406cfa743e3cfc4143dd7fc67c))
* **newlib:** add trivial mocks of syscalls to import
  newlib ([67c29ad](https://gitlab.com/atwardzik/os_kernel/commit/67c29ad4c453fbc9a1ab75f8b367ffea3e642a8d))
* **pio:** implement setting consecutive bits as
  outputs ([f8257da](https://gitlab.com/atwardzik/os_kernel/commit/f8257da00cd296b8c441f77a7e1e36db7f922024))
* **process:** add file operations to
  processes ([45ea1e8](https://gitlab.com/atwardzik/os_kernel/commit/45ea1e85fb2261fab725d6d84862d673e7546d8b))
* **process:** add idle process and fix handling signals in context switching
  function ([247dfc8](https://gitlab.com/atwardzik/os_kernel/commit/247dfc871eef49d0af1cb4da043c1fec1ec1d5ce))
* **process:** add info about process pages to the process'
  struct ([b28c57e](https://gitlab.com/atwardzik/os_kernel/commit/b28c57e9b850fca4ede995a83272a69732302da7))
* **process:** add kernel stack to the process and use it in order to perform blocking on a
  resource ([115f250](https://gitlab.com/atwardzik/os_kernel/commit/115f2506ebe8a2d11b02de90a3bd829002e43b24))
* **process:** add killing processes syscall and fix the tty_read to be POSIX compliant (errno on signal
  interrupt) ([e576a32](https://gitlab.com/atwardzik/os_kernel/commit/e576a32158000941c5144b76d1716ddd7c55a330))
* **process:** add place for process kernel stack so as to return to kernel mode in the
  feature ([d929c75](https://gitlab.com/atwardzik/os_kernel/commit/d929c75dcd0f5c3ea834eb6107eb0dfaef106abb))
* **process:** add proper exit
  function ([6943d16](https://gitlab.com/atwardzik/os_kernel/commit/6943d1692ec09209be4f57ba9062432b19aef426))
* **process:** add state saving and context switch when exiting blocked
  syscall ([d9a93aa](https://gitlab.com/atwardzik/os_kernel/commit/d9a93aaffedf6914e484db81d2015e5e0daa1b86))
* **process:** add wrappers for blocking
  syscalls ([f9ec902](https://gitlab.com/atwardzik/os_kernel/commit/f9ec902116f58627d7aeaed388ece84030b93a32))
* **processes:** set processes working on hand, TO BE FIXED AT FULL
  SPEED ([becbe36](https://gitlab.com/atwardzik/os_kernel/commit/becbe36e13eece13ecc50a6f9f77d2d39410bb9c))
* **process:** implement basic functions / mocks for context
  switching ([13fd87f](https://gitlab.com/atwardzik/os_kernel/commit/13fd87f73d6c5db02ab03b06c2c523db91b0aedb))
* **process:** implement basic process spawn
  function ([69a00a3](https://gitlab.com/atwardzik/os_kernel/commit/69a00a3ad367615366c792d35f3a1defcf6914a8))
* **process:** implement most of state recall in the
  signal_resource ([e0c2b7c](https://gitlab.com/atwardzik/os_kernel/commit/e0c2b7ce79a02289db7a36f271806f8b96234498))
* **process:** implement process
  creation ([36fd293](https://gitlab.com/atwardzik/os_kernel/commit/36fd2933d3d11e88a088b0ddf51ee8bfe1db46af))
* **process:** implement returning to
  syscall_block_return_point ([2efb029](https://gitlab.com/atwardzik/os_kernel/commit/2efb029a7d60a55c1c51221ffbb636178b27ae07))
* **process:** implement simple context
  switching ([353a765](https://gitlab.com/atwardzik/os_kernel/commit/353a765529f8cecd50fe6cd3795b30a318029965))
* **process:** implement syscall
  wait ([fbf0400](https://gitlab.com/atwardzik/os_kernel/commit/fbf040058207ecd9bcaad4d9b0f95288199d68be))
* **process:** implement wait_event_interruptible - save the state of the kernel mode and force context
  switch ([6cc8906](https://gitlab.com/atwardzik/os_kernel/commit/6cc8906067fab802ed2ccf507fab75af19ecc8a9))
* **process:** implement wrapper for signal resource
  function ([5a76065](https://gitlab.com/atwardzik/os_kernel/commit/5a760659894613f0b3ac3cd381140cadb75d2a60))
* **process:** pass arguments to spawned
  process ([eba8758](https://gitlab.com/atwardzik/os_kernel/commit/eba8758c47c447f6d8b8a329fa862da15f8272d0))
* **screen:** implement basic, buggy screen
  buffer ([3731e7c](https://gitlab.com/atwardzik/os_kernel/commit/3731e7c843bd67721c57fa89993867cc07c46c22))
* **sd:** add simple SD card
  driver ([656bfea](https://gitlab.com/atwardzik/os_kernel/commit/656bfea8293ed37e93445588261512aa4d3199bf))
* **sd:** set faster closk, about
  15 [MHz] ([fdef6bd](https://gitlab.com/atwardzik/os_kernel/commit/fdef6bd254001effe5b4347cbd1b76af24b919e0))
* **signal:** implement basic termination signals for
  processes ([56164a7](https://gitlab.com/atwardzik/os_kernel/commit/56164a7862f1c35dd1986e2a3c3a70d5aea1ad39))
* **signal:** implement
  SIGINT ([d9b3649](https://gitlab.com/atwardzik/os_kernel/commit/d9b364991faae5a4807e4d4590e7d1f2aa54b849))
* **signal:** implement signal handlers replacement (signal
  syscall) ([e3a0407](https://gitlab.com/atwardzik/os_kernel/commit/e3a040752d738f9baa3faeb38c298192b1f5cd7b))
* **signal:** implement signal
  handling ([5abdd16](https://gitlab.com/atwardzik/os_kernel/commit/5abdd16f60e510a91e8f5c0077c8d57056f07be3))
* **spi:** add init for
  spi ([60162fc](https://gitlab.com/atwardzik/os_kernel/commit/60162fca2b8c2a4949171febb65cb80ca0d199a4))
* **spi:** add simple SD card driver for getting CMD58
  output ([f70132e](https://gitlab.com/atwardzik/os_kernel/commit/f70132e02e0399dda1ec37ae34f41b90ee906b8b))
* **stdio:** add backspace
  management ([6e45eab](https://gitlab.com/atwardzik/os_kernel/commit/6e45eab77e26a80ccff5b45c71c89f12db00c0bb))
* **stdio:** add basic support for user
  input ([4aedbb2](https://gitlab.com/atwardzik/os_kernel/commit/4aedbb2ee07e2eec39976255a7dc75722cfc8d68))
* **stdio:** add german diacritical
  signs ([f3413a9](https://gitlab.com/atwardzik/os_kernel/commit/f3413a9e7984a3bf1caa91a7841a22fbd38ec800))
* **stdio:** add read and write support to stdin stdout (
  vga) ([f10af41](https://gitlab.com/atwardzik/os_kernel/commit/f10af414d04f64d59646696629772a12079f0a27))
* **syscall:** implement fstat and
  getcwd ([3478aa3](https://gitlab.com/atwardzik/os_kernel/commit/3478aa3f4674f3d77d4eea28114228c2c94b8ae2))
* **tty:** treat tty as a file, place it under /dev/tty and create file descriptors in init
  process ([35439a7](https://gitlab.com/atwardzik/os_kernel/commit/35439a77fc54cfe189111b5e5e9ddb62adc78794))
* **uart:** add uart0 irq for receiving
  characters ([7f1c037](https://gitlab.com/atwardzik/os_kernel/commit/7f1c037e3d5736c04a1e64e88790cc60301de559))
* **vga:** add 6 bit mode and printing
  letters ([d2d2160](https://gitlab.com/atwardzik/os_kernel/commit/d2d21608b8b9b0c70aaa76a4d2ff22e1aa7752fb))
* **vga:** add logo to the top of
  screen ([69f483d](https://gitlab.com/atwardzik/os_kernel/commit/69f483d2a04fe0848819b0e30622fd0dd5a58ece))
* **vga:** add PIO implementation and fix clocks for
  rp2350? ([5497fd7](https://gitlab.com/atwardzik/os_kernel/commit/5497fd736650e387e641531e48c665891c363eac))
* **vga:** add red screen
  generation ([ad9dad0](https://gitlab.com/atwardzik/os_kernel/commit/ad9dad0bd62a3e40a83d9efb3e797e058b371aa6))
* **vga:** add vsync signal
  generation ([7c48cbe](https://gitlab.com/atwardzik/os_kernel/commit/7c48cbe2f3b8a8ca276e165109376d55785452cb))
* **vga:** add waiting for character with cursor
  blinking ([5ae569a](https://gitlab.com/atwardzik/os_kernel/commit/5ae569aee0b845463dddcf978518b9cb59e01d00))
* **vga:** implement function for printing onto the
  screen ([8a4b0c6](https://gitlab.com/atwardzik/os_kernel/commit/8a4b0c6a6dcb17ea39d503f7288d40b0e037cbb5))
* **vga:** implement WORKING VGA
  hsync ([feb8104](https://gitlab.com/atwardzik/os_kernel/commit/feb81043f2655958c4460396ea26118a52ecfe21))
* **vga:** made vga finally
  work ([99661ff](https://gitlab.com/atwardzik/os_kernel/commit/99661ff13d681cbfc44ec140d4713064b52ec95d))

### Bug Fixes

* add addresses for ISR for the keyboard on
  rp2350 ([7ff9184](https://gitlab.com/atwardzik/os_kernel/commit/7ff9184339cf31aec0172ec01fe3ef9b0a1f0197))
* add bss section
  zeroing ([6362e61](https://gitlab.com/atwardzik/os_kernel/commit/6362e615c97e40f22adb5a67956952dd444fdb24))
* add bss section zeroing in startup
  routine ([248fd80](https://gitlab.com/atwardzik/os_kernel/commit/248fd802f1c7661f02fd594d96cb07277ac94a3a))
* add final fixes for rp2350, inclusive
  CLK_PERI ([c7a18c0](https://gitlab.com/atwardzik/os_kernel/commit/c7a18c0264a37f37b6dfcef7917fd454fbeddec1))
* add fixes for compatibility with both rp2040 and
  rp2350 ([f63362b](https://gitlab.com/atwardzik/os_kernel/commit/f63362b75d46479cfd31aa4b765f8a2c25412735))
* add syscall trampoline for preserving frames in the
  gdb ([ada1ae9](https://gitlab.com/atwardzik/os_kernel/commit/ada1ae9719c39573c1ca673d80a9c7e38ba7e5c2))
* **allocator:** repair freeing memory, set proper allocator size
  decrementation ([a8301a5](https://gitlab.com/atwardzik/os_kernel/commit/a8301a5ebede83292ff67bb183988a9c8af40df6))
* **builtins:** add proper exit in _
  start ([402516e](https://gitlab.com/atwardzik/os_kernel/commit/402516e218c94189f51d65432505b3d95ab59763))
* **builtins:** add proper jump to main in the
  crt0 ([f5020c9](https://gitlab.com/atwardzik/os_kernel/commit/f5020c994b70befc1135c58e4b29820fdba59118))
* **eth:** change addresses (offsets) on chip to point to valid
  data ([427ee0c](https://gitlab.com/atwardzik/os_kernel/commit/427ee0cbcf63722e7f3978547080ac379224b566))
* **eth:** change from polling to irq mode so that the process can sleep while waiting for a TCP connection; add close
  for FileOperations ([33d97be](https://gitlab.com/atwardzik/os_kernel/commit/33d97bebc776c616cb02e6d7d66dac2820169c3b))
* **eth:** change tx/rx buffer size per socket for equal values of
  2KB ([60465ef](https://gitlab.com/atwardzik/os_kernel/commit/60465efb560824e0aa4cedd29e48a7bc9257f4a4))
* **ethernet:** remove concurrency issue by adding mutex locks on transactions with wiznet
  chip ([15f7ea9](https://gitlab.com/atwardzik/os_kernel/commit/15f7ea92f5d7b1d09fc2ccdbd3aa331e2e56cb3f))
* **fs:** add info about file owner when getting a file descriptor in order to dealloc the file handler properly on
  process exit ([6593bd2](https://gitlab.com/atwardzik/os_kernel/commit/6593bd2ff64f7758ba1b995f0f3fadaa3e966829))
* **fs:** change inode index held in the DirectoryEntry to the direct pointer of the
  inode ([9ea7a5a](https://gitlab.com/atwardzik/os_kernel/commit/9ea7a5a4579710869df09d9675e5ea3392b0def8))
* **fs:** change stack buffer to dynamic buffer while lookup root
  directory ([edb9226](https://gitlab.com/atwardzik/os_kernel/commit/edb9226fe1343c69302a5b04a1c4afb888c305ef))
* **fs:** clean Inode struct initialization (with zeroes) and add kstrtok to fix the issue with signal that the linker
  is trying to poll from the
  newlibc ([c6eaa28](https://gitlab.com/atwardzik/os_kernel/commit/c6eaa283442d4f89e81f2e37c9dca1ef2067a263))
* **fs:** mount fat16 partition in the parent
  ramfs ([004d75d](https://gitlab.com/atwardzik/os_kernel/commit/004d75d825433cbadfda596555670ce7bb32cc17))
* **fs:** set inode cache to by dynamic (
  inode_table) ([b527cfe](https://gitlab.com/atwardzik/os_kernel/commit/b527cfe1060623ea5971538c1ca3466b4ca23efe))
* **fs:** update f_pos correctly; add memcpy for loading bytes from next clusters into specified
  buffer ([d779dd7](https://gitlab.com/atwardzik/os_kernel/commit/d779dd790d69fc06b9f600be9c07881357d1c9c2))
* **keyboard:** add arbitrary clock divisor for the PIO block to avoid synchronization
  issues ([1dcfd92](https://gitlab.com/atwardzik/os_kernel/commit/1dcfd9285079ae9a434e92005ad76f83d6147bfc))
* **keyboard:** repair hardware issue of swapped pins by applying JMP_PIN to the state machine
  EXECCTRL_JMP_PIN ([36322de](https://gitlab.com/atwardzik/os_kernel/commit/36322de0e1e11f4e40dc52066ec4807c2df3931c))
* **keyboard:** set up IRQ properly for the
  PIO ([e3027c1](https://gitlab.com/atwardzik/os_kernel/commit/e3027c14b390ed847e9e86459dcd44fc3ee50007))
* **libc:** add proper found index to the returned pointer in
  strchr ([bb7c528](https://gitlab.com/atwardzik/os_kernel/commit/bb7c528618233522ea85a7f81231b793f7667199))
* **loader:** set executable inode to be VFS_Inode - now executables can be loaded from any
  filesystem ([2999322](https://gitlab.com/atwardzik/os_kernel/commit/2999322755bfd00793c941e3fd1461086771e823))
* **memory:** repair condition in a while loop in
  krealloc ([e7e41db](https://gitlab.com/atwardzik/os_kernel/commit/e7e41db421b86e5f7621047367d65af475b03456))
* **newlib:** add shifts for
  deletion ([e5f8f10](https://gitlab.com/atwardzik/os_kernel/commit/e5f8f105e6116de27bb77d5016389260088d614e))
* **pio:** implement proper program loading for
  PIO ([f21b917](https://gitlab.com/atwardzik/os_kernel/commit/f21b917826a6cd98ff880ffce882d0ece4ee04dc))
* **process:** add new children to empty places in children
  array ([fcaa303](https://gitlab.com/atwardzik/os_kernel/commit/fcaa303a7bcd6b8d36781030a0f937d7762d9bc4))
* **process:** after creation set process to be READY and run only READY processes in round robin
  algorithm ([eeed959](https://gitlab.com/atwardzik/os_kernel/commit/eeed959e52c66885b214efa1de4a2b1f0c9e76f4))
* **processes:** implement proper context
  switching ([49309b5](https://gitlab.com/atwardzik/os_kernel/commit/49309b57731deb59cd051d511de7c54f5ab6902e))
* **process:** repair saving and recalling from stack high registers on a blocking
  process ([34327fb](https://gitlab.com/atwardzik/os_kernel/commit/34327fbd1a1c47353b4a14ceb2e0e5981a04f0b0))
* **process:** replace modulo operation to builtin mod
  function ([095d655](https://gitlab.com/atwardzik/os_kernel/commit/095d655ee0268a175c87cf7abe8e9164bfa4c956))
* **process:** set fp to be relative to sp and add thumb bit to the return point
  address ([a223080](https://gitlab.com/atwardzik/os_kernel/commit/a22308047a01a0f3932e886d186417c68651d90d))
* repair vga last line
  printing ([65ef650](https://gitlab.com/atwardzik/os_kernel/commit/65ef650761710746202bfb2b7a934b43d9b8fea3))
* **spi:** repair transmitting by applying full duplex mode (draining rx
  fifo) ([af8ca9b](https://gitlab.com/atwardzik/os_kernel/commit/af8ca9b6dc64fe769bf305299706b4072e830b57))
* **spi:** wait for data to be available in spi
  rx/tx ([273b074](https://gitlab.com/atwardzik/os_kernel/commit/273b07457e0bf17371db1d3f3ed6c9749688998b))
* **stdio:** add checks in the last line of screen, optimize UART
  communication ([23119d9](https://gitlab.com/atwardzik/os_kernel/commit/23119d95b6ff75289f49b8392f69bcb2bf0bae64))
* **stdio:** change screen writer to send arrows to uart properly, add position clearing after screen clearing, add
  nicer cursor to VGA
  screen ([e12c4f2](https://gitlab.com/atwardzik/os_kernel/commit/e12c4f259e6cc3f8e91bd43dcf8695ab376d5b17))
* **stdio:** remove writing right arrow character onto the screen after the end of the string and ensure the keyboard
  buffer is zeroed out on time (while waiting for the new
  character) ([edfffc5](https://gitlab.com/atwardzik/os_kernel/commit/edfffc5f36d05a95e3df3df34fa948174bda53f0))
* **stdio:** repair background color
  recognition ([6b974d6](https://gitlab.com/atwardzik/os_kernel/commit/6b974d638bcafc9f4aa078d5cebb27f62e510de9))
* **stdio:** repair foreground
  colors ([bfec75b](https://gitlab.com/atwardzik/os_kernel/commit/bfec75b928ff776d50756a065798fb246321a2bb))
* **stdio:** repair off by one errors for
  deletion ([9d9eaa7](https://gitlab.com/atwardzik/os_kernel/commit/9d9eaa767b5a56e3ea842e75f4191f68d1ee6d84))
* **tty:** add constraints for UART communication, replacing DEL with \b and decode escape sequences coming from
  uart ([c8e1958](https://gitlab.com/atwardzik/os_kernel/commit/c8e1958cad7622b65db8abe69956a12cbbaa2231))
* **tty:** repair cursor and receive data directly from keyboard
  driver ([4dedd98](https://gitlab.com/atwardzik/os_kernel/commit/4dedd98825ca1249a2a19ed10b67adfa35306b1c))
* **vga:** get rid of hardcoded gpio; pass as function parameters
  instead ([b364de3](https://gitlab.com/atwardzik/os_kernel/commit/b364de39416b194402ddc45cf704e375275e4b02))
* **vga:** repair cursor
  blinking ([ba8dce9](https://gitlab.com/atwardzik/os_kernel/commit/ba8dce937d9b4dcfa356d343d605708acba00b72))
* **vga:** set white cursor as
  default ([d2dfb1c](https://gitlab.com/atwardzik/os_kernel/commit/d2dfb1cfd6a8e23c36cbed81d8669d1a34baba24))
* **vga:** set white text as
  default ([27c39c8](https://gitlab.com/atwardzik/os_kernel/commit/27c39c88d261a79d65dd4921a03f51b7d6c3b40b))

