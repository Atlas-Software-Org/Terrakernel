# The Terrakernel Project

Terrakernel is a hybrid x86_64 kernel.

During the development of TK the kernel will always be on version v1.0-rc0.

# TODO
*: WIP

### Initial stuff
- [x] Port printf implementation
- [x] Support for some COM1 serial output using port 0x3F8
- [x] End of initial stuff

### x86_64 Specific
- [ ] Write a GDT*
- [ ] Write an IDT
- [ ] Write a physical memory manager
- [ ] Write a virtual memory manager
- [ ] Write a heap
- [ ] Port uACPI
- [ ] Parse "APIC" table
- [ ] Switch to IOAPIC interrupts instead
- [ ] Write a PIT timer
- [ ] (MAYBE) Write an APIC timer
- [ ] (Other) Write a VFS and TMPFS and parse a USTAR Initrd archive
- [ ] Scheduling
- [ ] Multiprocessing with SMP
- [ ] Switching to userspace
- [ ] Write some basic syscalls
- [ ] Load x86_64 ELF binaries, static and relocatable
- [ ] End of x86_64 stuff

### Other
- [ ] Write a VFS and TMPFS and parse a USTAR Initrd archive
- [ ] Add support for /dev/XYZ files
- [ ] Write a PCI driver
- [ ] Write a PS2 keyboard driver
- [ ] Write a PS2 mouse driver
- [ ] Try to write an XHCI driver for USB device support
- [ ] Write some disk drivers, probably AHCI only for now
- [ ] Some filesystem drivers, probably FAT32 and maybe, maybe, maybe EXT3 or EXT4
- [ ] End of other

### Porting software
- [ ] Port binutils and coreutils
- [ ] Port DOOM :]
- [ ] Port a window manager (window server) (probably Xorg)
- [ ] Port anything else :]
- [ ] End of porting software
