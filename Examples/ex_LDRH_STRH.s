.global main
.text
main:
    mov r0, #48 @ on se servira de la valeur de ce registre pour la charger en mémoire
    mov r1, #52 @ on servira de la valeur de ce registre comme adresse mémoire cible (volontairement alignée sur 32 bits)
    mov r2, #28 @ on se servira de la valeur de ce registre comme valeur d'offset (décalage) pour le calcul de l'adresse mémoire

    # cas spécifiques avec strh et ldrh

        # adressage par registre et décalage direct
    strh r0, [r1, #16] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + 16)
    mov r3, #0
    ldrh r3, [r1, #16] @ test réussi si la valeur de R0 est bien contenue dans R3

        # adressage par registre et décalage par registre
    mov r1, #56
    strh r0, [r1, r2] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + R2)
    mov r3, #0
    ldrh r3, [r1, r2] @ test réussi si la valeur de R0 est bien contenue dans R3

        # adressage par registre et décalage par registre (négatif)
    mov r1, #60
    strh r0, [r1, r2] @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + R2)
    mov r3, #0
    ldrh r3, [r1, r2] @ test réussi si la valeur de R0 est bien contenue dans R3

        # adressage pré-indexé
    mov r1, #64
    strh r0, [r1, #16]! @ test réussi si la valeur de R0 se retrouve bien à l'adresse (R1 + 16) et que R1 est mis à jour (R1 = (R1 + 16))
        # adressage post-indexé
    mov r1, #78
    strh r0, [r1], #16 @ test réussi si la valeur de R0 se retrouve bien à l'adresse R1 et que R1 est mis à jour (R1 = (R1 + 16))


    swi 0x123456
    