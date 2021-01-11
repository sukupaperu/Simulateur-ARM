.global main
.text

main:
    mov r0, #5
    eor r0, #3
end:
    swi 0x123456
.data
