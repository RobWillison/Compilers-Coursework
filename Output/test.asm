.globl main

.text

main:
li $a0 20
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
la $t1 function0
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 16($fp)
move $a1 $fp
jal function0
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
lw $t1 0($t2)
sw $t1 12($fp)
li $a0 8
li $v0 9
syscall
la $t1 function2
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 16($fp)
lw $t0 16($fp)
sw $t0 20($fp)
lw $v0 16($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function2:
move $t2 $a0
li $a0 40
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
lw $t1 0($t2)
sw $t1 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
li $a0 4
li $v0 9
syscall
move $a0 $v0
lw $t0 12($fp)
sw $t0 0($v0)
addi $v0 $v0 4
move $t7 $fp
lw $t7 8($t7)
lw $t0 12($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
move $t0 $v0
sw $t0 20($fp)
li $a0 4
li $v0 9
syscall
move $a0 $v0
lw $t0 20($fp)
sw $t0 0($v0)
addi $v0 $v0 4
move $t7 $fp
lw $t7 8($t7)
lw $t0 12($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
move $t0 $v0
sw $t0 24($fp)
move $v0 $v0
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function0:
move $t2 $a0
li $a0 36
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
li $a0 8
li $v0 9
syscall
la $t1 function3
sw $t1 0($v0)
sw $fp 4($v0)
sw $v0 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
li $a0 4
li $v0 9
syscall
move $a0 $v0
lw $t0 12($fp)
sw $t0 0($v0)
addi $v0 $v0 4
move $t7 $fp
lw $t7 8($t7)
lw $t0 12($t7)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
move $t0 $v0
sw $t0 20($fp)
li $t0 2
sw $t0 24($fp)
li $a0 4
li $v0 9
syscall
move $a0 $v0
li $t0 2
sw $t0 0($v0)
addi $v0 $v0 4
lw $t0 20($fp)
lw $t1 0($t0)
lw $a1 4($t0)
jal $t1
move $t0 $v0
sw $t0 28($fp)
move $v0 $v0
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0
function3:
move $t2 $a0
li $a0 32
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
sw $a1 8($fp)
lw $t1 0($t2)
sw $t1 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
li $t0 10
sw $t0 20($fp)
lw $t1 12($fp)
li $t2 10
add $t0 $t1 $t2
sw $t0 24($fp)
lw $v0 24($fp)
lw $t0 4($fp)
lw $fp 0($fp)
jr $t0

.data
