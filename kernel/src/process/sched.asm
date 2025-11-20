section .bss
scratch: resq 2

section .text
global switch_thread
switch_thread:
    cli

    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 48], rsp
    mov [rdi + 56], rbp
    mov [rdi + 88], r8
    mov [rdi + 96], r9
    mov [rdi + 104], r10
    mov [rdi + 112], r11
    mov [rdi + 120], r12
    mov [rdi + 128], r13
    mov [rdi + 136], r14
    mov [rdi + 144], r15

    pushfq
    pop qword [rdi + 72]

    lea rax, [rel .save_rip]
    mov [rdi + 64], rax

    mov rax, cr3
    mov [rdi + 80], rax

.save_rip:
    mov rdx, rsi

    mov rax, [rsi + 0]
    mov rbx, [rsi + 8]
    mov rcx, [rsi + 16]
    mov r8,  [rsi + 88]
    mov r9,  [rsi + 96]
    mov r10, [rsi + 104]
    mov r11, [rsi + 112]
    mov r12, [rsi + 120]
    mov r13, [rsi + 128]
    mov r14, [rsi + 136]
    mov r15, [rsi + 144]
    mov rbp, [rsi + 56]
    mov rsp, [rsi + 48]

    mov rax, [rsi + 32]
    mov [scratch], rax
    mov rax, [rsi + 40]
    mov [scratch + 8], rax

    mov rax, [rdx + 80]
    mov rcx, cr3
    cmp rax, rcx
    je .skip_cr3
    mov cr3, rax
.skip_cr3:
    mov rax, [rdx + 72]
    push rax
    popfq

    mov rsi, [scratch]
    mov rdi, [scratch + 8]

    jmp qword [rdx + 64]