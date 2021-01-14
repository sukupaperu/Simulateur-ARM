.global main
.text

main:
    # 5 = 0b101
    # 9 = 0b1001
    # resultat attendu r0 = 1
    mov r0, #5
    AND r0, #9
end:
    swi 0x123456
.data
