; From OSDev -> https://wiki.osdev.org/ATA_read/write_sectors
%define C_READ_FUNC
%ifndef C_READ_FUNC
global AtaLbaRead
global AtaLbaWrite
AtaLbaRead:
	mov eax, [esp + 4]
	mov edi, [esp + 8]
	mov ecx, [esp + 12]
	pushfd
	and eax, 0x0fffffff
	push eax
	push ebx
	push ecx
	push edx
	push edi
	
	mov ebx, eax			; Save LBA in EBX
	
	mov edx, 0x01f6			; Port to send drive and bits 24-27 of LBA
	shr eax, 24				; Get bits 24-27 in AL
	or al, 0xe0				; Set bit 6 in AL for LBA Mode
	out dx, al
	
	mov edx, 0x01f2			; Port to send number of sectors
	mov al, cl				; Get number of sectors from CL
	out dx, al
	
	mov edx, 0x01f3			; Port to send bits 0-7 of LBA
	mov eax, ebx			; Get LBA from EBX
	out dx, al
	
	mov edx, 0x01f4			; Port to send bits 8-15 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 8				; Get bits 8-15 in AL
	out dx, al
	
	mov edx, 0x01f5			; Port to send bits 16-23 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 16				; Get bits 16-23 in AL
	out dx, al
	
	mov edx, 0x01f7			; Command port
	mov al, 0x20			; Read with Retry
	out dx, al
	
.still_going:
	in al, dx
	test al, 8				; The sector buffer requires servicing...
	jz .still_going			; until the sector buffer is ready
	
	mov eax, 256			; Read 256 Words = 1 Sector (512 bytes = 0.5 KiB)
	xor bx, bx				; synonymous to bx = 0
	mov bl, cl				; Read CL sectors
	mul bx
	mov ecx, eax			; ECX is counter for INSW
	mov edx, 0x1f0			; Data port, I/O
	rep insw				; in to ESI
	
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	popfd
	
	ret
	
AtaLbaWrite:
	mov eax, [esp + 4]
	mov esi, [esp + 8]
	mov ecx, [esp + 12]
	pushfd
	and eax, 0x0fffffff
	push eax
	push ebx
	push ecx
	push edx
	push edi
	
	mov ebx, eax			; Save LBA in EBX
	
	mov edx, 0x01f6			; Port to send drive and bits 24-27 of LBA
	shr eax, 24				; Get bits 24-27 in AL
	or al, 0xe0				; Set bit 6 in AL for LBA Mode
	out dx, al
	
	mov edx, 0x01f2			; Port to send number of sectors
	mov al, cl				; Get number of sectors from CL
	out dx, al
	
	mov edx, 0x01f3			; Port to send bits 0-7 of LBA
	mov eax, ebx			; Get LBA from EBX
	out dx, al
	
	mov edx, 0x01f4			; Port to send bits 8-15 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 8				; Get bits 8-15 in AL
	out dx, al
	
	mov edx, 0x01f5			; Port to send bits 16-23 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 16				; Get bits 16-23 in AL
	out dx, al
	
	mov edx, 0x01f7			; Command port
	mov al, 0x30			; Write with Retry
	out dx, al
	
.still_going:
	in al, dx
	test al, 8				; The sector buffer requires servicing...
	jz .still_going			; until the sector buffer is ready
	
	mov eax, 256			; Write 256 Words = 1 Sector (512 bytes = 0.5 KiB)
	xor bx, bx				; synonymous to bx = 0
	mov bl, cl				; Read CL sectors
	mul bx
	mov ecx, eax			; ECX is counter for OUTSW
	mov edx, 0x1f0			; Data port, I/O
	rep outsw				; in to ESI
	
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	popfd
	
	ret

	
global AtaLbaReadS
global AtaLbaWriteS
AtaLbaReadS:
	mov eax, [esp + 4]
	mov edi, [esp + 8]
	pushfd
	and eax, 0x0fffffff
	push eax
	push ebx
	push ecx
	push edx
	push edi
	
	mov ebx, eax			; Save LBA in EBX
	
	mov edx, 0x01f6			; Port to send drive and bits 24-27 of LBA
	shr eax, 24				; Get bits 24-27 in AL
	or al, 0xe0				; Set bit 6 in AL for LBA Mode
	out dx, al
	
	mov edx, 0x01f2			; Port to send number of sectors
	mov al, 1				; Get number of sectors from CL
	out dx, al
	
	mov edx, 0x01f3			; Port to send bits 0-7 of LBA
	mov eax, ebx			; Get LBA from EBX
	out dx, al
	
	mov edx, 0x01f4			; Port to send bits 8-15 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 8				; Get bits 8-15 in AL
	out dx, al
	
	mov edx, 0x01f5			; Port to send bits 16-23 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 16				; Get bits 16-23 in AL
	out dx, al
	
	mov edx, 0x01f7			; Command port
	mov al, 0x20			; Read with Retry
	out dx, al
	
.still_going:
	in al, dx
	test al, 8				; The sector buffer requires servicing...
	jz .still_going			; until the sector buffer is ready
	
	mov ecx, 256
	mov edx, 0x1f0			; Data port, I/O
	rep insw				; in to ESI
	
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	popfd
	
	ret
	
AtaLbaWriteS:
	mov eax, [esp + 4]
	mov esi, [esp + 8]
	pushfd
	and eax, 0x0fffffff
	push eax
	push ebx
	push ecx
	push edx
	push edi
	
	mov ebx, eax			; Save LBA in EBX
	
	mov edx, 0x01f6			; Port to send drive and bits 24-27 of LBA
	shr eax, 24				; Get bits 24-27 in AL
	or al, 0xe0				; Set bit 6 in AL for LBA Mode
	out dx, al
	
	mov edx, 0x01f2			; Port to send number of sectors
	mov al, 1				; Get number of sectors from CL
	out dx, al
	
	mov edx, 0x01f3			; Port to send bits 0-7 of LBA
	mov eax, ebx			; Get LBA from EBX
	out dx, al
	
	mov edx, 0x01f4			; Port to send bits 8-15 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 8				; Get bits 8-15 in AL
	out dx, al
	
	mov edx, 0x01f5			; Port to send bits 16-23 of LBA
	mov eax, ebx			; Get LBA from EBX
	shr eax, 16				; Get bits 16-23 in AL
	out dx, al
	
	mov edx, 0x01f7			; Command port
	mov al, 0x30			; Write with Retry
	out dx, al
	
.still_going:
	in al, dx
	test al, 8				; The sector buffer requires servicing...
	jz .still_going			; until the sector buffer is ready
	
	mov ecx, 256			; ECX is counter for OUTSW
	mov edx, 0x1f0			; Data port, I/O
	rep outsw				; in to ESI
	
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	popfd
	
	ret
%endif