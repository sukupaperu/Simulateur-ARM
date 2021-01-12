.global main
.text
main:
    mov r0, #0b1100
    mov r1, #0b110100
    mov r2, #0b11100
    
    # ldr + str, adressage sans décalage (sans offset)
    str r0, [r1]
    strh r0, [r1]
    strb r0, [r1]
    ldr r3, [r1]
    ldrh r4, [r1]
    ldrb r5, [r1]


    # ldr + str, adressage par registre et par décalage direct (positif puis négatif)
    str r0, [r1, #4]
    ldr r3, [r1, #4]
    str r0, [r1, #-4]
    ldr r3, [r1, #-4]


    # ldr + str, adressage par registre et par décalage par registre (positif puis négatif)
    str r0, [r1, r2]
    ldr r3, [r1, r2]
    str r0, [r1, -r2]
    ldr r3, [r1, -r2]

    # ldr + str, adressage par registre et décalage par registre + différents opérateurs type "bitshift"
    str r0, [r1, r2, LSL #4]
    ldr r3, [r1, r2, LSL #4]
    #
    mov r2, #0b111000000
    str r0, [r1, r2, LSR #4]
    ldr r3, [r1, r2, LSR #4]
    #
    str r0, [r1, r2, ASR #4]
    ldr r3, [r1, r2, ASR #4]
    #
    str r0, [r1, r2, ROR #4]
    ldr r3, [r1, r2, ROR #4]
    #
    str r0, [r1, r2, RRX]
    ldr r3, [r1, r2, RRX]


    # adressage pré-indexé
    mov r1, #0x34
    ldr r0, [r1, #16]!
    # adressage post-indexé
    mov r1, #0x34
    ldr r0, [r1], #16
    mov r1, #0x34


    # cas spécifiques avec strh et ldrh
        # adressage par registre et décalage direct
    strh r3, [r1, #16]
    ldrh r0, [r1, #16]
        # adressage par registre et décalage par registre
    strh r3, [r1, r2]
    ldrh r0, [r1, r2]
        # adressage pré-indexé
    mov r1, #0x34
    strh r0, [r1, #16]!
        # adressage post-indexé
    mov r1, #0x34
    strh r0, [r1], #16

    # stm et ldm
    mov r0, #0b11010100
    mov r1, #56
    mov r2, #77
    mov r3, #45
    mov r4, #182
    mov r5, #88
    mov r6, #7
    mov r7, #456
    mov r8, #112
    mov r9, #344
    mov r10, #48
    mov r11, #77
    mov r12, #78
        # increment after
    stmia r0, {r0-r15}
    ldmia r0, {r0-r15}
        # increment before
    stmib r0, {r0-r15}
    ldmib r0, {r0-r15}
        # decrement after
    stmda r0, {r0-r15}
    @ ldmda r0, {r0-r15} // ne fonctionne pas
        # decrement before
    stmdb r0, {r0-r15}
    @ ldmdb r0, {r0-r15} // ne fonctionne pas
        # base register written back
    // stmdb r0!, {r0-r15} // ne fonctionne pas

    swi 0x123456
