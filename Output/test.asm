.globl run

.text


run:
jal main
move $a0 $v0
li $v0 1
syscall

li $v0, 10
syscall

test:
li $a0 16
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $t0 5
sw $t0 12($fp)
lw $v0 12($fp)
lw $t0 4($fp)
jr $t0
main:
li $a0 16
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
jal test
move $v0 $v0
lw $t0 4($fp)
jr $t0

.data
