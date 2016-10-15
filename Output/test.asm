# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:

li $t0 14
sw $t0 0($sp)
li $t0 11
sw $t0 4($sp)
lw $t1 0($sp)
lw $t2 4($sp)
sltu $t0 $t2 $t1
sw $t0 8($sp)
lw $t1 0($sp)
lw $t2 4($sp)
sub $t0 $t1 $t2
sltu $t0 $zero $t0
xori $t1 $t0 1
lw $t2 8($sp)
or $t0 $t2 $t1
sw $t0 16($sp)
lw $t0 16($sp)
sw $t0 20($sp)
li $v0 1
lw $a0 20($sp)
syscall

















li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
