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
sw $v0 12($fp)
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
li $t0 0
sw $t0 12($fp)
lw $t2 12($fp)
beq $t2 $zero label1
li $v0 4
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
j label2
label1:
li $t0 1
sw $t0 16($fp)
lw $t2 16($fp)
beq $t2 $zero label3
li $v0 3
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
label3:
label2:
li $v0 8
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
