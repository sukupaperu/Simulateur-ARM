.global main
.text
main:
	mov r0, #0xc
	mov r1, #0x9
	bics r2, r1
end:
	swi 0x123456
