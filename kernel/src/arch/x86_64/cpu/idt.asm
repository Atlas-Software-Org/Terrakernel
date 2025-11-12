global idt_load
global exception_stub_table

extern exception_handler

idt_load:
    lidt [rdi]
    ret

%macro idt_exception_noerr 1
global idt_exception_noerr_%1
idt_exception_noerr_%1:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, [rsp + 0]
    mov rsi, %1
    mov rdx, [rsp + 1*8]
    mov r8, [rsp + 4*8]
    mov r9, [rsp + 5*8]
    call exception_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
%endmacro

%macro idt_exception_err 1
global idt_exception_err_%1
idt_exception_err_%1:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, [rsp + 14*8]
    mov rsi, %1
    mov rdx, [rsp + 13*8]
    mov r8, cr2
    mov r9, cr3
    call exception_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
%endmacro

idt_exception_noerr_0:
idt_exception_noerr_1:
idt_exception_noerr_2:
idt_exception_noerr_3:
idt_exception_noerr_4:
idt_exception_noerr_5:
idt_exception_noerr_6:
idt_exception_noerr_7:
idt_exception_err_8:
idt_exception_noerr_9:
idt_exception_err_10:
idt_exception_err_11:
idt_exception_err_12:
idt_exception_err_13:
idt_exception_err_14:
idt_exception_noerr_15:
idt_exception_noerr_16:
idt_exception_err_17:
idt_exception_noerr_18:
idt_exception_noerr_19:
idt_exception_noerr_20:
idt_exception_noerr_21:
idt_exception_noerr_22:
idt_exception_noerr_23:
idt_exception_noerr_24:
idt_exception_noerr_25:
idt_exception_noerr_26:
idt_exception_noerr_27:
idt_exception_noerr_28:
idt_exception_noerr_29:
idt_exception_noerr_30:
idt_exception_noerr_31:

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
