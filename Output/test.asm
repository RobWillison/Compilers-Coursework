.globl main

.text

main:
move $t2 $a0
li $a0 32
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
li $t0 5
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 0($v0)
addi $v0 $v0 4
jal cplus
lw $t0 -1($fp)
sw $t0 16($fp)
li $a0 4
li $v0 9
syscall
move $a0 $v0
li $t0 2
sw $t0 20($fp)
lw $t0 20($fp)
sw $t0 0($v0)
addi $v0 $v0 4
jal 
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
cplusa:
move $t2 $a0
li $a0 28
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
lw $t1 0($t2)
sw $t1 12($fp)
lw $t0 -1($fp)
sw $t0 16($fp)
lw $t0 12($fp)
sw $t0 20($fp)
lw $t1 16($fp)
lw $t2 20($fp)
add $t0 $t1 $t2
sw $t0 24($fp)
lw $v0 24($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
cplus:
move $t2 $a0
li $a0 20
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
lw $t1 0($t2)
sw $t1 12($fp)
lw $t0 -1($fp)
sw $t0 16($fp)
lw $v0 16($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
