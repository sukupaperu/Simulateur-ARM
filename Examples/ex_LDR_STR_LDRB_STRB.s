.global main
.text
main:
    mov r0, #48 @ on se servira de la valeur de ce registre pour la charger en mémoire
    mov r1, #52 @ on servira de la valeur de ce registre comme adresse mémoire cible (volontairement alignée sur 32 bits, dans le cas contraire provoque un plantage volontaire du simulateur si l'accès est de 32bits)
    mov r2, #28 @ on se servira de la valeur de ce registre comme valeur d'offset (décalage) pour le calcul de l'adresse mémoire

    # ldr + str, adressage sans décalage (sans offset)
    str r0, [r1] @ test réussi si la valeur de R0 se retrouve bien à l'adresse R1
    mov r3, #0
    ldr r3, [r1] @ test réussi si la valeur de R0 est bien contenue dans R3

    mov r1, #56
    strh r0, [r1] @ test réussi si la valeur de R0 se retrouve bien à l'adresse R1
    mov r3, #0
    ldrh r3, [r1] @ test réussi si la valeur de R0 est bien contenue dans R3

    mov r1, #60
    strb r0, [r1] @ test réussi si la valeur de R0 se retrouve bien à l'adresse R1
    mov r3, #0
    ldrb r3, [r1] @ test réussi si la valeur de R0 est bien contenue dans R3


    # ldr + str, adressage par registre et par décalage direct (positif puis négatif)
    mov r1, #64
    str r0, [r1, #4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + 4)
    mov r3, #0
    ldr r3, [r1, #4] @ test réussi si la valeur de R0 est bien contenue dans R3

    mov r1, #68
    str r0, [r1, #-4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 - 4)
    mov r3, #0
    ldr r3, [r1, #-4] @ test réussi si la valeur de R0 est bien contenue dans R3


    # ldr + str, adressage par registre et par décalage par registre (positif puis négatif)
    mov r1, #72
    str r0, [r1, r2] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + R2)
    mov r3, #0
    ldr r3, [r1, r2] @ test réussi si la valeur de R0 est bien contenue dans R3

    mov r1, #76
    str r0, [r1, -r2] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 - R2)
    mov r3, #0
    ldr r3, [r1, -r2] @ test réussi si la valeur de R0 est bien contenue dans R3

    # ldr + str, adressage par registre et décalage par registre + différents opérateurs type "bitshift"
    mov r1, #80
    str r0, [r1, r2, LSL #4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + (R2 << 4))
    mov r3, #0
    ldr r3, [r1, r2, LSL #4]  @ test réussi si la valeur de R0 est bien contenue dans R3
    #
    mov r1, #84
    mov r2, #448
    str r0, [r1, r2, LSR #4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + (R2 >> 4))
    mov r3, #0
    ldr r3, [r1, r2, LSR #4] @ test réussi si la valeur de R0 est bien contenue dans R3
    #
    mov r1, #88
    str r0, [r1, r2, ASR #4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + (R2 >> 4)) (avec extension du bit de signe de R2)
    mov r3, #0
    ldr r3, [r1, r2, ASR #4] @ test réussi si la valeur de R0 est bien contenue dans R3
    #
    mov r1, #92
    str r0, [r1, r2, ROR #4] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + (R2 ROTATION DROITE 4))
    mov r3, #0
    ldr r3, [r1, r2, ROR #4] @ test réussi si la valeur de R0 est bien contenue dans R3
    #
    mov r1, #96
    str r0, [r1, r2, RRX] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + EXTENSION_CARRY_FLAG(R2 >> 1))
    mov r3, #0
    ldr r3, [r1, r2, RRX] @ test réussi si la valeur de R0 est bien contenue dans R3


    # adressage pré-indexé
    mov r1, #500
    str r0, [r1, #16]! @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + 16) et que R1 est mis à jour (R1 = (R1 + 16))
    # adressage post-indexé
    mov r1, #504
    str r0, [r1], #16 @ test réussi si la valeur de R0 se retrouve bien à l'adresse R1 et que R1 est mis à jour (R1 = (R1 + 16))
    

    swi 0x123456
    