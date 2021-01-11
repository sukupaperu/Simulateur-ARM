.global main
.text
main:
	mov r0, #0xc
	mov r1, #0x9
	orrs r2, r0, r1
	bics r2, #0x6
end:
	swi 0x123456
