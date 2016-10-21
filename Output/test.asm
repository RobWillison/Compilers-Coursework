.globl main

.text

main:
li $t0 1
sw $t0 0($gp)
li $t0 4
sw $t0 4($gp)
lw $t1 0($gp)
lw $t2 4($gp)
sltu $t0 $t2 $t1
sw $t0 8($gp)
lw $t2 8($gp)
beq $t2 $zero label1
li $t0 4
sw $t0 12($gp)
lw $ra 12($gp)
j label2
label1:
li $t0 1
sw $t0 16($gp)
li $t0 2
sw $t0 20($gp)
lw $t1 16($gp)
lw $t2 20($gp)
sltu $t0 $t2 $t1
sw $t0 24($gp)
lw $t2 24($gp)
beq $t2 $zero label3
li $t0 3
sw $t0 28($gp)
lw $ra 28($gp)
label3:
label2:
li $t0 8
sw $t0 32($gp)
lw $ra 32($gp)


li $v0 1
move $a0 $ra
syscall

li $v0, 10
syscall

.data
