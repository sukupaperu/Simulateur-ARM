.global main
.text
main:
	# test de TST
	# résultat attendu : CPSR[31-28] == 0x2 == %0010
	mov r0, #0xc
	mov r1, #0x9
	tst r0, r1
end:
	swi 0x123456
