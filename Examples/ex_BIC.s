.global main
.text
main:
	# test de BIC
	# résultat attendu : r2 == 0xc & ~0x9 == 0x4 soit %1100 & ~%1001 == %0100
	mov r0, #0xc
	mov r1, #0x9
	bics r2, r0, r1
end:
	swi 0x123456
