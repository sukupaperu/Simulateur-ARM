.global main
.text
main:
	mvn r0, #0x7
	mvn r1, r0
end:
	swi 0x123456
