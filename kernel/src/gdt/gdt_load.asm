[BITS 64]

global gdt_load
gdt_load:
    ; rdi = pointer to GdtPtr
    ; rsi = TSS selector

    lgdt [rdi]        ; load GDT
    mov ax, si        ; load TSS selector into ax
    ltr ax            ; load TSS
    ret
