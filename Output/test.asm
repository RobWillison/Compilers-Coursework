.globl main

.text

main:
move $t2 $a0
li $a0 24
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $a0 8
li $v0 9
syscall
move $a0 $v0
li $t0 3
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 0($v0)
addi $v0 $v0 4
li $t0 2
sw $t0 16($fp)
lw $t0 16($fp)
sw $t0 0($v0)
addi $v0 $v0 4
jal times
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
times:
move $t2 $a0
li $a0 32
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
lw $t1 0($t2)
sw $t1 20($fp)
lw $t1 4($t2)
sw $t1 24($fp)
lw $t0 20($fp)
sw $t0 28($fp)
lw $t0 24($fp)
sw $t0 32($fp)
lw $t1 28($fp)
lw $t2 32($fp)
mult $t1 $t2
mflo $t0
sw $t0 36($fp)
lw $v0 36($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
