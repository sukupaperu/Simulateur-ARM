.global main
.text

main:
    # test avec 0xffffffff +1
    # le 0xffffffff est la valeur maximum dans le registre, 
    # avec l'addition, il y a un debordement
    # premier r4 = 0, Z set, C set 
    # deuxieme r4 = 1, Z clear, C set
    movS r0, #4294967295
    movS r1, #1
    adcS r4, r0, r1
    adcS r4, r0, r1

end:
    swi 0x123456
.data