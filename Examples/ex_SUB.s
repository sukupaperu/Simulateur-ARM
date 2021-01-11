.global main
.text

main:
    mov r0, #5
    mov r1, #9
    sub r2, r1, r0

end:
    swi 0x123456
.data
