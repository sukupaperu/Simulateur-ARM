.global main
.text

main:
    # test de substraction normale
    # sub r2, r1, r0 : si r1<r0, la compilation affiche erreur
    # resultat attendu: r2 = 4 
    mov r0, #5
    mov r1, #9
    sub r2, r1, r0

end:
    swi 0x123456
.data
