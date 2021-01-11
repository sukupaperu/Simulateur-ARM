.global main
.text

main:
    mov r0, #0xff
    mrs r1,cpsr
    mrs r2,spsr


end:
    swi 0x123456
.data