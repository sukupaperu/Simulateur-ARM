.global main
.text
main:
	mov r0, #0xc
	mov r1, #0x9
	cmp r0, r1
end:
	swi 0x123456
