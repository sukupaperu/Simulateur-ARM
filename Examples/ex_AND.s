.global main
.text

main:
    mov r0, #5
    AND r0, #9
end:
    swi 0x123456
.data