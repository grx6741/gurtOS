.section .data

gdtr:
    .word 0  # 2 bytes for the limit
    .long 0  # 4 bytes for the base

.section .text
# Make the set_gdt label visible to the C linker.
.global gdt_set

gdt_set:
    # 4(%esp) <==3 first argument (limit)
    # 8(%esp) <==3 the second argument (base)

    movw 4(%esp), %ax
    movw %ax, gdtr

    movl 8(%esp), %eax
    movl %eax, gdtr+2

    lgdt gdtr

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $.flush

.flush:
    ret
