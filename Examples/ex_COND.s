.global main
.text

main:
	mov r0, #1
    subs r0, r0, #2
    addmis r0, #10

	#NE  
    movS r0, #1
    movS r1, #2
    movS r2, #2
    cmp r1, r0
    moveq r3, #2
    movne r3, #1    

    #EQ     
    mov r1, #2
    mov r2, #2
    cmp r1, r2
    moveq r4, #2
    movne r4, #1 

    #CS     
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    addcs r0, r0, r1


    #HS   
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVHS r3, #3

    #CC     
    MOVCC r3, #3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVCC r4, #3

    #LO    
    MOVLO r3, #3
    movs r0, #0xffffffff
    movs r1, #2
    adds r0, r1, r0
    MOVLO r4, #3

    #MI            
    MOVMI r3, #3
    movs r0, #0
    movs r1, #2
    subs r0, r1
    MOVMI r4, #3

    #PL          
    MOVPL r3, #3
    movs r0, #0
    movs r1, #2
    subs r0, r1
    MOVPL r4, #4

    #VS          
    MOVVS r3, #3
    movs r0, #0x7fffffff
    movs r1, #2
    adds r0, r1, r0
    MOVVS r4, #4

    #VC          
    MOVVC r3, #3
    movs r0, #0x7fffffff
    movs r1, #2
    adds r0, r1, r0
    MOVVC r4, #4

    #HI
    MOVHI r3, #3
    movs r0, #0x7fffffff
    movs r1, #2
    movs r2, #3
    adds r0, r1, r0
    MOVHI r4, #4

    #LS     
    mov r0, #0
    mov r1, #2
    mov r2, #2
    cmp r1, r2
    movLS r4, #4    
    cmp r0, r1          
    movLS r5, #5 

    #GE
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

    #LT
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
    
    #GT
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
    
    #LE
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
    
    #AL
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