.global main
.text

main:
    
    # Test de la fonction ADD
    # résultat attendu : r4 == 0x1 CPSR[31-28] == %0010 == 0x2
    mov r0, #4294967295
    mov r1, #2
    addS r4, r0, r1

end:
    swi 0x123456
.data