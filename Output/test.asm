.globl main

.text

main:
li $a0 16
li $v0 9
syscall
move $fp $v0
sw $ra 4($fp)
li $a0 8
li $v0 9
syscall
la $t1 function1
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 12($fp)
move $a1 $fp
jal function1
lw $t0 4($fp)
jr $t0
function1:
move $t2 $a0
li $a0 48
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
li $t0 1
sw $t0 12($fp)
li $t0 4
sw $t0 16($fp)
lw $t1 12($fp)
lw $t2 16($fp)
sltu $t0 $t2 $t1
sw $t0 20($fp)
lw $t2 20($fp)
beq $t2 $zero label1
li $t0 4
sw $t0 24($fp)
lw $v0 24($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
j label2
label1:
li $t0 2
sw $t0 28($fp)
li $t0 1
sw $t0 32($fp)
lw $t1 28($fp)
lw $t2 32($fp)
sltu $t0 $t2 $t1
sw $t0 36($fp)
lw $t2 36($fp)
beq $t2 $zero label3
li $t0 3
sw $t0 40($fp)
lw $v0 40($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
label3:
label2:
li $t0 8
sw $t0 44($fp)
lw $v0 44($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
