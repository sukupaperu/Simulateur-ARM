.global main
.text

main:
    # 5 = 0b101, 3=0b11
    #resultat attendu: r0 = 6 =0b110
    mov r0, #5
    eor r0, #3
end:
    swi 0x123456
.data
