.global main
.text
main:
	# test de CMN
	# résultat attendu : CPSR[31-28] == 0xé == %0010
	mov r0, #0xfffffffe
	mov r1, #0x9
	cmn r0, r1
end:
	swi 0x123456
