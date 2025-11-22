bits 64
section .text
global store_context
global load_context

extern _context
extern _rip

store_context:
    mov rax, [_rip]
    mov [_context + 0x00], rax
    mov [_context + 0x10], rsi
    mov [_context + 0x18], rdi
    mov [_context + 0x20], r15
    mov [_context + 0x28], r14
    mov [_context + 0x30], r13
    mov [_context + 0x38], r12
    mov [_context + 0x40], r11
    mov [_context + 0x48], r10
    mov [_context + 0x50],  r9
    mov [_context + 0x58],  r8
    mov [_context + 0x60], rdx
    mov [_context + 0x68], rcx
    mov [_context + 0x70], rbx
    mov [_context + 0x78], rax
    mov [_context + 0x80], rbp
    mov [_context + 0x88], rsp

    mov rax, cr3
    mov [_context + 0x08], rax

    ret

load_context:
    cmp qword [_context], 0
    jz .end

    ;mov rax, [_context + 0x08]
    ;mov cr3, rax

    mov rsi, [_context + 0x10]
    mov rdi, [_context + 0x18]
    mov r15, [_context + 0x20]
    mov r14, [_context + 0x28]
    mov r13, [_context + 0x30]
    mov r12, [_context + 0x38]
    mov r11, [_context + 0x40]
    mov r10, [_context + 0x48]
    mov r9,  [_context + 0x50]
    mov r8,  [_context + 0x58]
    mov rdx, [_context + 0x60]
    mov rcx, [_context + 0x68]
    mov rbx, [_context + 0x70]
    mov rax, [_context + 0x78]
    
    mov rbp, [_context + 0x80]
    mov rsp, [_context + 0x88]

    jmp [_context + 0x00]

.end:
    ret