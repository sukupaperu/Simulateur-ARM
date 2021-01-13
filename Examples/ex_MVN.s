.global main
.text
main:
	# test de MVN
	# résultat attendu : r1 == NOT r0 == 0xfffffff8
	mvn r0, #0x7
	mvn r1, r0
end:
	swi 0x123456
