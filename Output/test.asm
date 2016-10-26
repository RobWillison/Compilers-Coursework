.globl main

.text

main:
li $a0 52
li $v0 9
syscall
move $t0 $fp
move $fp $v0
sw $t0 0($fp)
sw $ra 4($fp)
li $t0 0
sw $t0 12($fp)
lw $t0 12($fp)
sw $t0 16($fp)
j label1
label2:
lw $t0 16($fp)
sw $t0 20($fp)
li $t0 1
sw $t0 24($fp)
lw $t1 20($fp)
lw $t2 24($fp)
add $t0 $t1 $t2
sw $t0 28($fp)
lw $t0 28($fp)
sw $t0 16($fp)
label1:
lw $t0 16($fp)
sw $t0 32($fp)
li $t0 5
sw $t0 36($fp)
lw $t1 32($fp)
lw $t2 36($fp)
sltu $t0 $t1 $t2
sw $t0 40($fp)
lw $t2 40($fp)
bne $t2 $zero label2
lw $t0 16($fp)
sw $t0 44($fp)
lw $v0 44($fp)
lw $t0 4($fp)
jr $t0

.data
