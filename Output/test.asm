.globl main

.text

main:
li $t0 8
sw $t0 0($gp)
lw $ra 0($gp)
lw $ra 0($gp)


li $v0 1
move $a0 $ra
syscall

li $v0, 10
syscall

.data
