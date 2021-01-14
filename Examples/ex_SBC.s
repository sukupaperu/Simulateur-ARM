.global main
.text

main:
    # test de substaction avec C
    # resultat attendu : 
    # r5 = 1,r6 = 0xfffffffd = -2.     
    
    mov r0, #1
    mov r1, #3
    sbc r5, r1, r0
    sbc r6, r1, #5

end:
    swi 0x123456
.data
