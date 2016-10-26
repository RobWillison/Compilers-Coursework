.globl main

.text

test:
li $a0 20
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
lw $t0 -1($fp)
sw $t0 12($fp)
lw $v0 12($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
main:
li $a0 20
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $a0 4
li $v0 9
syscall
move $t1 $v0
li $t0 6
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 0($t1)
addi $t1 $t1 4
jal test
move $v0 $v0
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
