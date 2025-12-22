# The Terrakernel Project
## Made for Terra

Terrakernel is a hybrid x86_64 kernel.

During the development of TK the kernel will always be on version v1.0-rc0.
Terra is the operating system I'm planning to use terrakernel for.

# TODO
*: WIP

### Initial stuff
- [x] Port printf implementation
- [x] Support for some COM1 serial output using port 0x3F8
- [x] End of initial stuff

### x86_64 Specific
- [x] Write a GDT
- [x] Write an IDT
- [x] Write a physical memory manager
- [x] Write a virtual memory manager
- [x] Write a heap
- [x] Write a PIT timer
- [x] Switch to fully graphical (flanterm) messages and logs
- [x] Port uACPI
- [x] (Other) Write a VFS and TMPFS and parse a USTAR Initrd archive
- [ ] Scheduling (delayed)
- [x] Switching to userspace
- [x] Write some basic syscalls
- [ ] Load x86_64 ELF binaries, static and relocatable (copy from old version of TK)*
- [ ] End of x86_64 stuff (almost)

### Other
- [x] Write a VFS and TMPFS and parse a USTAR Initrd archive
- [x] Add support for /dev/XYZ files (use O_BUILTIN_DEVICE_FILE in kernel, also requires an additional parameter which is devpath, the path to the device directory, usually for disks, e.g., initrd creates two nodes, /dev/initrd (initrd disk image) and /initrd the directory itself)
- [x] Write a PCI driver
- [x] Write a PS2 keyboard driver
- [ ] Write a PS2 mouse driver
- [ ] Try to write an XHCI driver for USB device support
- [ ] Write some disk drivers, probably AHCI only for now
- [ ] Some filesystem drivers, probably FAT32 and maybe, maybe, maybe EXT3 or EXT4
- [ ] End of other

### Porting software
- [ ] Port binutils and coreutils
- [ ] Port DOOM
- [ ] Port a window manager (window server) (probably Xorg)
- [ ] Port anything else
- [ ] End of porting software
- [ ] End of project... or at least this version...

### Building the kernel
Check [BUILD_INSTRUCTIONS.md](https://github.com/Atlas-Software-Org/Terrakernel/blob/master/BUILD_INSTRUCTIONS.md)

### How many LoC?

```x86asm
     128 text files.
     126 unique files.
       2 files ignored.

github.com/AlDanial/cloc v 1.98  T=0.15 s (836.0 files/s, 274218.4 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               23           4462           1207          21163
C/C++ Header                    75           1424           2045           6834
C++                             22            733              0           3189
Assembly                         5             26              8            216
CMake                            1              0              0             21
-------------------------------------------------------------------------------
SUM:                           126           6645           3260          31423
-------------------------------------------------------------------------------
```
