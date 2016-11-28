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
li $a0 40
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
li $t3 4
move $t4 $t3
li $t5 1
move $t6 $t5
move $t7 $t4
move $s0 $t6
move $t1 $t7
move $t2 $s0
mult $t1 $t2
mflo $t0
move $s1 $t0
move $v0 $s1
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
