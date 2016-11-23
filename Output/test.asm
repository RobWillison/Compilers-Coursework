.globl main

.text

main:
li $a0 28
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
li $a0 8
li $v0 9
syscall
la $t1 function2
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 16($fp)
li $a0 8
li $v0 9
syscall
la $t1 function3
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 20($fp)
li $a0 8
li $v0 9
syscall
la $t1 function0
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 24($fp)
move $a1 $fp
jal function0
lw $t0 4($fp)
jr $t0
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
li $v0 2
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
sw $a1 8($fp)
li $v0 1
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
li $v0 1
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function0:
move $t2 $a0
li $a0 72
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
move $t7 $fp
lw $t7 8($t7)
lw $t0 16($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
li $a0 0
li $v0 9
syscall
move $a0 $v0
move $t7 $fp
lw $t7 8($t7)
lw $t0 12($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
move $t0 $v0
sw $t0 12($fp)
lw $t1 -1($fp)
move $t2 $v0
mult $t1 $t2
mflo $t0
sw $t0 16($fp)
lw $t0 16($fp)
sw $t0 20($fp)
li $a0 0
li $v0 9
syscall
move $a0 $v0
move $t7 $fp
lw $t7 8($t7)
lw $t0 16($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
li $a0 0
li $v0 9
syscall
move $a0 $v0
move $t7 $fp
lw $t7 8($t7)
lw $t0 12($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
lw $t1 -1($fp)
move $t2 $v0
mult $t1 $t2
mflo $t0
sw $t0 24($fp)
lw $t1 20($fp)
lw $t2 24($fp)
add $t0 $t1 $t2
sw $t0 28($fp)
lw $v0 28($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
