.global main
.text
main:
	# test de ORR
	# résultat attendu : r2 == r0 OR r1 == 0xd
	mov r0, #0xc
	mov r1, #0x9
	orrs r2, r0, r1
end:
	swi 0x123456
