.global main
.text

    # test de B et BL
    # résultat attendu : le programme se termine lorsque r0 == 0
    # on passe cinq fois dans la boucle decr
    # à chaque passage on peut observer la mise à jour de r14 par bl
decr:
    subs r0, r0, #1
    mov pc, lr

main:
    mov r0, #5
loop:
    bl decr
    bne loop
end:
    swi 0x123456
.data
