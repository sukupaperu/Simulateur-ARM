.global main
.text

main:
    # test de substaction inverse avec C
    # resultat attendu : 
    # r5 = 0xfffffffd = -2. r6 = 1
    
    mov r0, #1
    mov r1, #3
    rsc r5, r1, r0
    rsc r6, r1, #5

end:
    swi 0x123456
.data
