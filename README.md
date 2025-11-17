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
- [x] Parse "APIC" table
- [x] (Other) Write a VFS and TMPFS and parse a USTAR Initrd archive
- [ ] Scheduling
- [ ] Multiprocessing with SMP
- [ ] Switching to userspace
- [ ] Write some basic syscalls
- [ ] Load x86_64 ELF binaries, static and relocatable
- [ ] End of x86_64 stuff

### Other
- [x] Write a VFS and TMPFS and parse a USTAR Initrd archive
- [x] Add support for /dev/XYZ files
- [ ] Write a PCI driver
- [ ] Write a PS2 keyboard driver
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

### Building the kernel
Check [BUILD_INSTRUCTIONS.md](https://github.com/Atlas-Software-Org/Terrakernel/blob/master/BUILD_INSTRUCTIONS.md)