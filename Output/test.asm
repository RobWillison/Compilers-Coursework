.globl main

.text

main:
li $a0 24
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $t0 8
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
lw $t0 16($fp)
sw $t0 20($fp)
lw $v0 20($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
