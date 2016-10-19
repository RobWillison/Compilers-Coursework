# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:

li $t0 1
sw $t0 0($gp)
li $t0 3
sw $t0 4($gp)
lw $t1 0($gp)
lw $t2 4($gp)
add $t0 $t1 $t2
sw $t0 8($gp)
lw $t0 8($gp)
sw $t0 12($gp)
lw $t0 12($gp)
sw $t0 16($gp)
li $t0 3
sw $t0 20($gp)
lw $t1 16($gp)
lw $t2 20($gp)
mult $t1 $t2
mflo $t0
sw $t0 24($gp)
lw $ra 24($gp)
li $v0 1
move $a0 $ra
syscall


li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
