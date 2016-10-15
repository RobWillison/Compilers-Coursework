# Declare main as a global function
.globl main

.text

# The label 'main' represents the starting point
main:
li $a0, 25		# Load immediate value (25)
li $v0, 1
syscall

li $v0, 10 # Sets $v0 to "10" to select exit syscall
syscall # Exit

.data
