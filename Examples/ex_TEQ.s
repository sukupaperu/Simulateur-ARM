.global main
.text
main:
	mov r0, #0xc
	mov r1, #0xc
	teq r0, r1
	mov r1, #0xd
	teq r0, r1
end:
	swi 0x123456
