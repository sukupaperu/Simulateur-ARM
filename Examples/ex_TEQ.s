.global main
.text
main:
	# test de TEQ
	# résultat attendu : après le premier TEQ, on a CPSR[31-28] == 0x6 == %0110 car r0 == r1
	# après le second TEQ, on a CPSR[31-28] == 0x2 == %0010 car r0 != r1
	mov r0, #0xc
	mov r1, #0xc
	teq r0, r1
	mov r1, #0xd
	teq r0, r1
end:
	swi 0x123456
