section .text
global switchTask

; void switchTask(Registers* oldRegs, Registers* newRegs)
switchTask:
    ; rdi = oldRegs
    ; rsi = newRegs

    ; --- Save current registers into oldRegs ---
    pushfq
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, cr3
    push rax

    mov rdx, rdi        ; oldRegs pointer
    xchg [rdx + 0], rax
    xchg [rdx + 8], rbx
    xchg [rdx + 16], rcx
    xchg [rdx + 24], rdx
    xchg [rdx + 32], rsi
    xchg [rdx + 40], rdi
    xchg [rdx + 48], rsp
    xchg [rdx + 56], rbp
    xchg [rdx + 88], r8
    xchg [rdx + 96], r9
    xchg [rdx + 104], r10
    xchg [rdx + 112], r11
    xchg [rdx + 120], r12
    xchg [rdx + 128], r13
    xchg [rdx + 136], r14
    xchg [rdx + 144], r15
    xchg [rdx + 80], rax   ; CR3

    pop rax
    popfq
    push rax
    pushfq
    push rax
    mov rax, [rdx + 72]
    pop rax                 ; rflags
    pop rax
    mov rax, [rsp]          ; temporary RIP storage
    xchg [rdx + 64], rax    ; RIP

    ; --- Load new registers from newRegs ---
    mov rdx, rsi            ; newRegs pointer
    xchg rax, [rdx + 0]
    xchg rbx, [rdx + 8]
    xchg rcx, [rdx + 16]
    xchg rdx, [rdx + 24]
    xchg rsi, [rdx + 32]
    xchg rdi, [rdx + 40]
    xchg rbp, [rdx + 56]
    xchg rsp, [rdx + 48]
    xchg r8,  [rdx + 88]
    xchg r9,  [rdx + 96]
    xchg r10, [rdx + 104]
    xchg r11, [rdx + 112]
    xchg r12, [rdx + 120]
    xchg r13, [rdx + 128]
    xchg r14, [rdx + 136]
    xchg r15, [rdx + 144]
    push rax
    mov rax, [rdx + 80]
    mov cr3, rax
    pop rax

    ; --- Restore RIP and RFLAGS properly ---
    push rax
    mov rax, [rdx + 72]
    push rax
    pop rax
    popfq
    jmp [rdx + 64]           ; jump to new task RIP
