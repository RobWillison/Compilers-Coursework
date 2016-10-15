# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:
li $t0 8
sw $t0 0($sp)
li $t0 4
sw $t0 4($sp)
li $t0 2
sw $t0 8($sp)
lw $t1 4($sp)
lw $t2 8($sp)
div $t1 $t2
mflo $t0
sw $t0 12($sp)
lw $t1 0($sp)
lw $t2 12($sp)
add $t0 $t1 $t2
sw $t0 16($sp)
lw $t0 16($sp)
sw $t0 20($sp)
li $v0 1
lw $a0 20($sp)
syscall






li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
