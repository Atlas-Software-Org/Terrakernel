global gdt_load
global tss_load

gdt_load:
    lgdt [rdi]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push 0x08
    lea rax, [end]
    push rax
    retfq
end:
    ret

tss_load:
    mov ax, 0x28
    ltr ax
    ret
