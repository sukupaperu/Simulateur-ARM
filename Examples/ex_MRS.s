.global main
.text

main:
    #lire le contenu de cpsr ou spsr
    # resultat attendu: r1 = 0x1d3 = 467, r2 = 0
    mov r0, #0xff
    mrs r1,cpsr
    mrs r2,spsr


end:
    swi 0x123456
.data
