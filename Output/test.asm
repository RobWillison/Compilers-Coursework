.globl main

.text

times2:
move $t2 $a0
li $a0 48
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
lw $t1 0($t2)
sw $t1 12($fp)
j label1
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
sw $t1 16($fp)
lw $t1 4($t2)
sw $t1 20($fp)
lw $t0 12($fp)
sw $t0 24($fp)
lw $t0 20($fp)
sw $t0 28($fp)
lw $t1 24($fp)
lw $t2 28($fp)
mult $t1 $t2
mflo $t0
sw $t0 32($fp)
lw $v0 32($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
label1:
li $a0 8
li $v0 9
syscall
move $a0 $v0
lw $t0 -1($fp)
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
main:
move $t2 $a0
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
move $a0 $v0
li $t0 3
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 0($v0)
addi $v0 $v0 4
jal times2
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
