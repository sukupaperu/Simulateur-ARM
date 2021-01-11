.global main
.text

main:
    
    
    mov r0, #1
    mov r1, #3
    rsc r5, r1, r0
    rsc r6, r1, #5

end:
    swi 0x123456
.data