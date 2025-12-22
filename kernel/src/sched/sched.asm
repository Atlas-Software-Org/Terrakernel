section .text
global switchTask

; void switchTask(Registers* oldRegs, Registers* newRegs)
; rdi = oldRegs, rsi = newRegs
switchTask:
    ; --- Save general-purpose registers to oldRegs ---
    mov rax, [rsp]           ; save old RIP from stack temporarily
    mov [rdi + 48], rsp      ; save RSP
    mov [rdi + 56], rbp      ; save RBP
    mov [rdi + 0], rax       ; save RIP
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 88], r8
    mov [rdi + 96], r9
    mov [rdi + 104], r10
    mov [rdi + 112], r11
    mov [rdi + 120], r12
    mov [rdi + 128], r13
    mov [rdi + 136], r14
    mov [rdi + 144], r15

    ; Save RFLAGS and CR3
    pushfq
    pop rax
    mov [rdi + 72], rax      ; save RFLAGS
    mov rax, cr3
    mov [rdi + 80], rax      ; save CR3

    ; --- Load registers from newRegs ---
    mov rax, [rsi + 0]
    mov rbx, [rsi + 8]
    mov rcx, [rsi + 16]
    mov rdx, [rsi + 24]
    mov rsi, [rsi + 32]
    mov rdi, [rsi + 40]
    mov rbp, [rsi + 56]
    mov rsp, [rsi + 48]
    mov r8,  [rsi + 88]
    mov r9,  [rsi + 96]
    mov r10, [rsi + 104]
    mov r11, [rsi + 112]
    mov r12, [rsi + 120]
    mov r13, [rsi + 128]
    mov r14, [rsi + 136]
    mov r15, [rsi + 144]

    ; Load CR3 if set
    mov rax, [rsi + 80]
    test rax, rax
    jz .skip_cr3_load
    mov cr3, rax
.skip_cr3_load:

    ; Restore RFLAGS
    mov rax, [rsi + 72]
    push rax
    popfq

    ; Jump to the new RIP
    mov rax, [rsi + 0]
    jmp rax
