.global main
.text
main:
	mov r0, #0xffffffff
	movs r1, #1
	adcs r4, r0, r1
	adcs r4, r0, r1
end:
	swi 0x123456
