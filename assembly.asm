jmp [53]
;=====================Change Line 37 To Change Array Length==================
; This example sorts an array
mov bx [3]	; fun [4] 2 - Fill array (lower bound, upper bound)
get
mov [bx] ax
add bx 1
cmp bx [2]
jbe [6]
ret [250]
			; 16

mov bx [17]	; fun [18] 2 - Print array (lower bound, upper bound)
mov ax [bx]
put
add bx 1
cmp bx [16]
jbe [20]
ret [250]
			; 30
mov bx [30]	; fun [31] 1 - swaps 2 memory locations
mov cx [bx]
add bx 1
mov dx [bx]
sub bx 1
cmp cx dx
mov ax 0
jbe [50]
mov ax 1
mov [bx] dx
add bx 1
mov [bx] cx
ret [198]

mov cx 200 ; starting point of array (feel free to change this)
mov [150] cx
mov dx 10   ; length of array (feel free to change this too)
mov [151] dx
add dx cx
sub dx 1
fun [4] 2
cx
dx
mov cx [150]; 69
mov ax 0
mov [199] ax
fun [31] 1 ; 75
cx
mov ax [199]
add ax [198]
mov [199] ax
add cx 1
cmp cx dx
jb [75]
cmp ax 0
ja [69]
fun [18] 2
[150]
dx
halt
