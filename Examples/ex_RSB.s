.global main
.text

main:
    # r1 = r1 - r0
    # resultat attendu = 4
    mov r0, #5
    mov r1, #9
    rsb r1, r0, r1

end:
    swi 0x123456
.data
