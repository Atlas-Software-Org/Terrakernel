bits 64

global idt_load
global exception_stub_table

extern exception_handler

idt_load:
    lidt [rdi]
    ret

%macro idt_exception_noerr 1
global idt_exception_noerr_%1
idt_exception_noerr_%1:
    mov rdi, rsp
    mov rsi, %1
    xor rdx, rdx
    mov r8, cr2
    mov r9, cr3
    call exception_handler
    iretq
%endmacro

%macro idt_exception_err 1
global idt_exception_err_%1
idt_exception_err_%1:
    mov rdi, rsp
    mov rsi, %1
    mov rdx, [rsp + 24]
    mov r8, cr2
    mov r9, cr3
    call exception_handler
    add rsp, 8
    iretq
%endmacro

idt_exception_noerr 0
idt_exception_noerr 1
idt_exception_noerr 2
idt_exception_noerr 3
idt_exception_noerr 4
idt_exception_noerr 5
idt_exception_noerr 6
idt_exception_noerr 7
idt_exception_err 8
idt_exception_noerr 9
idt_exception_err 10
idt_exception_err 11
idt_exception_err 12
idt_exception_err 13
idt_exception_err 14
idt_exception_noerr 15
idt_exception_noerr 16
idt_exception_err 17
idt_exception_noerr 18
idt_exception_noerr 19
idt_exception_noerr 20
idt_exception_noerr 21
idt_exception_noerr 22
idt_exception_noerr 23
idt_exception_noerr 24
idt_exception_noerr 25
idt_exception_noerr 26
idt_exception_noerr 27
idt_exception_noerr 28
idt_exception_noerr 29
idt_exception_noerr 30
idt_exception_noerr 31

section .rodata
exception_stub_table:
    dq idt_exception_noerr_0
    dq idt_exception_noerr_1
    dq idt_exception_noerr_2
    dq idt_exception_noerr_3
    dq idt_exception_noerr_4
    dq idt_exception_noerr_5
    dq idt_exception_noerr_6
    dq idt_exception_noerr_7
    dq idt_exception_err_8
    dq idt_exception_noerr_9
    dq idt_exception_err_10
    dq idt_exception_err_11
    dq idt_exception_err_12
    dq idt_exception_err_13
    dq idt_exception_err_14
    dq idt_exception_noerr_15
    dq idt_exception_noerr_16
    dq idt_exception_err_17
    dq idt_exception_noerr_18
    dq idt_exception_noerr_19
    dq idt_exception_noerr_20
    dq idt_exception_noerr_21
    dq idt_exception_noerr_22
    dq idt_exception_noerr_23
    dq idt_exception_noerr_24
    dq idt_exception_noerr_25
    dq idt_exception_noerr_26
    dq idt_exception_noerr_27
    dq idt_exception_noerr_28
    dq idt_exception_noerr_29
    dq idt_exception_noerr_30
    dq idt_exception_noerr_31
