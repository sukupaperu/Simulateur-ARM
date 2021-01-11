.global main
.text

main:
    
    movS r0, #4294967295
    movS r1, #1
    adcS r4, r0, r1
    adcS r4, r0, r1

end:
    swi 0x123456
.data