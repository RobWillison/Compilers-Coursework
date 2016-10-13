# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:
li $t2, 25		# Load immediate value (25)
sw $t2 ($sp)

li $t2, 10		# Load immediate value (25)
sw $t2 4($sp)

lw $t0 ($sp)
lw $t1 4($sp)

add $t0,$t0,$t1

li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
