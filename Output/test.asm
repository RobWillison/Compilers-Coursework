.globl main

.text

main:
li $a0 24
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
li $a0 24
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
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
