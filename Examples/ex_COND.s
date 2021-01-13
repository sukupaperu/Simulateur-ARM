.global main
.text

main:
    # tous les instructions de conditions a execute en dependant les flags.
    # les calculs ou les comparasons peuvent midifie les flags dans les cas differents.
	mov r0, #1
    subs r0, r0, #2
    addmis r0, #10

    # NE et EQ lire le flag Z. La comparason cmp modifie le Z si les deux valeurs sont identiques.
	# NE
    # Apres la comparason, le ne s'effectue que les deux valeurs sont egale
    # resultat attendu : r3 = 1
    movS r0, #1
    movS r1, #2
    cmp r1, r0
    moveq r3, #2
    movne r3, #1    

    # EQ     
    # resultat attendu : r4 = 2
    mov r1, #2
    mov r2, #2
    cmp r1, r2
    moveq r4, #2
    movne r4, #1 


    # CS     
    # apres le adds, C set, r0 = 1
    # resultat attendu : r0 = 3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    addcs r0, r0, r1

    # HS   
    # apres le adds, C set, r0 = 1
    # resultat attendu : r0 = 3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVHS r3, #3

    # CC    
    # le premier CC : C clear, donc r3 = 3
    # le deuxieme CC : C set, donc r4 ne change pas de valeur
    MOVCC r3, #3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVCC r4, #3

    # LO   
    # (Pariel avec CC)
    # le premier LO : C clear, donc r3 = 3
    # le deuxieme LO : C set, donc r4 ne change pas de valeur
    MOVLO r3, #3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVLO r4, #3

    # MI  
    # premier MI : N clear, donc r3 ne change pas de valeur
    # avec subs : N set, C set
    # deuxieme MI : r4 = 3
    MOVMI r3, #3
    movs r0, #0
    movs r1, #2
    subs r0, r1
    MOVMI r4, #3

    # PL    
    # (inverse de MI)
    # premier PL : N clear, donc r3 = 3
    # avec subs : N set, C set
    # deuxieme PL : r4 ne change pas de valeur   
    MOVPL r3, #3
    movs r0, #0
    movs r1, #2
    subs r0, r1
    MOVPL r4, #4

    # VS 
    # premier VS : V clear, donc r3 ne change pas de valeur  
    # apres adds :  N set, C set, V set 
    # deuxieme VS : r4 = 4
    MOVVS r3, #33
    movs r0, #0x7fffffff
    movs r1, #2
    adds r0, r1, r0
    MOVVS r4, #4

    # VC   
    # (inverse de VS)
    # premier VS : V clear, donc r3 = 33 
    # apres adds :  N set, C set, V set 
    # deuxieme VS : r4 ne change pas de valeur        
    MOVVC r3, #33
    movs r0, #0x7fffffff
    movs r1, #2
    adds r0, r1, r0
    MOVVC r4, #4

    # HI
    # premier HI : C clear, Z clear, donc r3 ne change pas de valeur
    # apres adds, N set, Z clear,C set, V set
    # deuxieme HI : r4 = 4
    MOVHI r3, #3
    movs r0, #0x7fffffff
    movs r1, #2
    movs r2, #3
    adds r0, r1, r0
    MOVHI r4, #4

    # LS  
    # premier LS : C set, Z set, r1 = r2, donc r4 = 4
    # deuxieme LS : C clear, Z clear, r0 < r1, donc r5 = 5  
    mov r0, #0
    mov r1, #2
    mov r2, #2
    cmp r1, r2
    movLS r4, #4    
    cmp r0, r1          
    movLS r5, #5 

    # GE, LT, GT, LE traitent les comparasons entre les entriers signed
    # GE
    # dans la comparason, si op1 >= op2, donc ge execute
    # resultat attendu:
    #   r4 = 4, r6 = 6, r8 =8
    #   r5, r7 ne changent pas de valeur
    movS r0, #0
    movS r1, #2
    movS r2, #2
    cmp r1, r2
    movGE r4, #4
    cmp r0, r1
    movGE r5, #5 
    cmp r1, r0
    movGE r6, #6 
    MOVS r0, #-1
    movs r2, #-2
    cmp r2, r0
    movGE r7, #7
    cmp r0, r2
    movGE r8, #8

    # LT
    # (inverse de GE)
    # dans la comparason, si op1 < op2, donc le ge execute
    # resultat attendu:
    #   r5 = 5, r7 = 7
    #   r4, r6, r8 ne changent pas de valeur
    movS r0, #0
    movS r1, #2
    movS r2, #2
    cmp r1, r2
    movLT r4, #4
    cmp r0, r1
    movLT r5, #5 
    cmp r1, r0
    movLT r6, #6 

    MOVS r0, #-1
    movs r2, #-2
    cmp r2, r0
    movLT r7, #7
    cmp r0, r2
    movLT r8, #8
    
    # GT
    # dans la comparason, si op1 > op2, donc le ge execute
    # resultat attendu:
    #   r6 = 6, r8 = 8
    #   r4, r5, r7 ne changent pas de valeur
    movS r0, #0
    movS r1, #2
    movS r2, #2
    cmp r1, r2
    movGT r4, #4
    cmp r0, r1
    movGT r5, #5 
    cmp r1, r0
    movGT r6, #6 

    MOVS r0, #-1
    movs r2, #-2
    cmp r2, r0
    movGT r7, #7
    cmp r0, r2
    movGT r8, #8
    
    # LE
    # (inverse de GT)
    # dans la comparason, si op1 <= op2, donc le ge execute
    # resultat attendu:
    #   r4 =4, r5 = 5, r7 = 7
    #   r6, r8 ne changent pas de valeur
    movS r0, #0
    movS r1, #2
    movS r2, #2
    cmp r1, r2
    movLE r4, #4
    cmp r0, r1
    movLE r5, #5 
    cmp r1, r0
    movLE r6, #6 

    MOVS r0, #-1
    movs r2, #-2
    cmp r2, r0
    movLE r7, #7
    cmp r0, r2
    movLE r8, #8
    
    # AL
    # il execute toujours
    # resultat attendu : r4 = 4, r5 = 5, r6 = 6
    movS r0, #0
    movS r1, #2
    movS r2, #2
    cmp r1, r2
    movAL r4, #4
    cmp r0, r1
    movAL r5, #5 
    cmp r1, r0
    movAL r6, #6 
    
end:
    swi 0x123456
.data