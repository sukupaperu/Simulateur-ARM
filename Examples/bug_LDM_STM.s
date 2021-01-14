.global main
.text
    mov r0, #800
    
    # decrement after
    stmda r0, {r0-r15}
    ldmda r0, {r0-r15} @ cette instruction cause un bug inattendu dans le simulateur (cf document)

    # decrement before
    @ stmdb r0, {r0-r15}
    @ ldmdb r0, {r0-r15} @ cette instruction cause le même bug inattendu dans le simulateur

    @ en temps normal ces instructions devraient être "visibles" dans GDB, mais pas ici
    mov r2, #5
    mov r3, #6
    mov r4, #7
    mov r5, #8

    swi 0x123456
