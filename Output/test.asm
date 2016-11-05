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
sw $a1 8($fp)
li $a0 0
li $v0 9
syscall
move $a0 $v0
jal function2
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function3:
move $t2 $a0
li $a0 16
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
move $t7 $fp
lw $t7 8($t7)
lw $t0 -1($t7)
sw $zero 12($fp)
lw $v0 12($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function2:
move $t2 $a0
li $a0 24
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
li $t0 6
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
sw $fp 20($fp)
li $a0 0
li $v0 9
syscall
move $a0 $v0
lw $a1 20($fp)
jal function3
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
main:
li $a0 24
li $v0 9
syscall
move $fp $v0
sw $ra 4($fp)
sw $fp 12($fp)
sw $fp 16($fp)
jal function1
lw $t0 4($fp)
jr $t0

.data
