.globl main

.text

function1:
move $t2 $a0
li $a0 16
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $a0 0
li $v0 9
syscall
move $a0 $v0
jal function18081904
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function2:
move $t2 $a0
li $a0 16
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $t0 4
sw $t0 8($fp)
lw $v0 8($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
