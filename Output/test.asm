# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:
li $t0 9
sw $t0 0($sp)
lw $t0 0($sp)
sw $t0 4($sp)
li $t0 3
sw $t0 8($sp)
lw $t0 8($sp)
sw $t0 12($sp)
lw $t0 4($sp)
sw $t0 16($sp)
lw $t0 12($sp)
sw $t0 20($sp)
lw $t1 16($sp)
lw $t2 20($sp)
mult $t1 $t2
mflo $t0
sw $t0 24($sp)
lw $t0 24($sp)
sw $t0 28($sp)
li $v0 1
lw $a0 28($sp)
syscall









li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
