.global main
.text
alea:
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
    mov PC, LR
vide:
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    mov PC, LR
main:
    @ mov r0, #212 @ R0 nous servira comme adresse de base pour le chargement des registres en mémoire (alignée sur 32bits)
    bl alea @ on met des valeurs arbitraires dans les registres (sauf registres R0 et R15)
    # increment after
    stmia r0, {r0-r15} @ on doit retrouver les registres R0 à R15 stockés respectivement de l'adresse R0 à (R0 + 15*4)
    bl vide @ (0) vide les registres (sauf registres R0 et R15)
    ldmia r0, {r0-r14} @ les registres R0 à R14 doivent être restaurés (tels qu'ils l'étaient avant d'être vidés)
    @ on ne restaure pas PC sinon le programme forme une boucle infini puisque la valeur de PC stockée en mémoire pointe
    @ sur l'instruction ldmia elle-même (pour la même raison on fera ce choix par la suite)

    # increment before
    stmib r0, {r0-r15} @ on doit retrouver les registres R0 à R15 stockés respectivement de l'adresse (R0 + 4) à (R0 + 16*4)
    bl vide @ (1) vide les registres (sauf registres R0 et R15)
    ldmib r0, {r0-r14} @ les registres R0 à R14 doivent être restaurés (tels qu'ils l'étaient avant d'être vidés)

    mov r0, #800
    # decrement after
    stmda r0, {r0-r15} @ on doit retrouver les registres R0 à R15 stockés respectivement de l'adresse (R0) à (R0 - 15*4)
    bl vide @ (2) vide les registres (sauf registres R0 et R15)
    ldmda r0, {r0-r14} @ les registres R0 à R14 doivent être restaurés (tels qu'ils l'étaient avant d'être vidés)
    @ note: ldmda r0, {r0-r15} ne peut pas être vérifié car cette instruction cause un bug inattendu dans le simulateur

    # decrement before
    stmdb r0, {r0-r15} @ on doit retrouver les registres R0 à R15 stockés respectivement de l'adresse (R0 - 4) à (R0 - 16*4)
    bl vide @ (3) vide les registres (sauf registres R0 et R15)
    ldmdb r0, {r0-r14} @ les registres R0 à R14 doivent être restaurés (tels qu'ils l'étaient avant d'être vidés)
    @ note: ldmdb r0, {r0-r15} ne peut pas être vérifié car cette instruction cause un bug inattendu dans le simulateur

    # base register written back (pour les 4 cas d'incrémentation possible)
    stmia r0!, {r0-r15} @ R0 doit être mis à jour avec l'adresse R0 + 16*4
    stmda r0!, {r0-r15} @ R0 doit être mis à jour avec l'adresse R0 - 16*4
    stmib r0!, {r0-r15} @ R0 doit être mis à jour avec l'adresse R0 + 16*4
    stmdb r0!, {r0-r15} @ R0 doit être mis à jour avec l'adresse R0 - 16*4

    swi 0x123456
