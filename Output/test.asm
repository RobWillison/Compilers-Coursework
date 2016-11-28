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
la $t1 function0
sw $t1 0($v0)
sw $fp 4($v0)
move $t3 $v0
move $a1 $fp
jal function0
lw $t0 4($fp)
jr $t0
function0:
move $t2 $a0
li $a0 48
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
li $t3 1
li $t3 4
move $t1 $t3
move $t2 $t3
sltu $t0 $t2 $t1
move $t3 $t0
move $t2 $t3
beq $t3 $zero label1
li $t3 4
move $v0 $t3
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
j label2
label1:
li $t3 2
li $t3 1
move $t1 $t3
move $t2 $t3
sltu $t0 $t2 $t1
move $t3 $t0
move $t2 $t3
beq $t3 $zero label3
li $t3 3
move $v0 $t3
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
label3:
label2:
li $t3 8
move $v0 $t3
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
