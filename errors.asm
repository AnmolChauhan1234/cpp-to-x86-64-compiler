; ============================================
; errors.asm - runtime error handlers
; Calls print_string from print.asm
; ============================================
global overflow_error
global divzero_error

extern print_string        ; already defined in print.asm

section .data
overflow_msg db "Runtime Error: Integer Overflow", 10, 0
divzero_msg  db "Runtime Error: Divide by Zero", 10, 0

section .text

; -------------------------------
; overflow_error: prints overflow error and exits
; clobbers: RAX, RDI
overflow_error:
    mov rdi, overflow_msg
    call print_string
    mov rax, 60      ; sys_exit
    mov rdi, 1       ; exit code 1
    syscall

; -------------------------------
; divzero_error: prints divide by zero error and exits
; clobbers: RAX, RDI
divzero_error:
    mov rdi, divzero_msg
    call print_string
    mov rax, 60      ; sys_exit
    mov rdi, 2       ; exit code 1
    syscall
