#4th
addi $t1, $0, 200
addi $t8, $0, 42
sw $t8, 8($t1)
addi $t3, $0, 3
addi $t4, $0, 12
addi $t7, $0, 100
sw $t7, 4($t1)
lw $t0, 4($t1)
add $t2, $t3, $t4
mul $t5, $t0, $t2
sw $t5, 4($t1)
lw $t6, 8($t1)
add $s0, $t6, $t5

#3rd
#addi $1, $0, 2
#addi $2, $0, 1
#sw $1 , 1002($1) 
#sw $2 , 1006($1) 
#add $3, $2, $1 
#sub $4, $2, $1 
#sw $3 , 1011($2) 
#sw $4 , 1015($2) 
#lw $5 , 1002($1)  
#lw $6 , 1006($1) 
#lw $7 , 1010($1)  
#lw $8 ,  1014($1)  
#add $7, $7, $8 
#add $6, $6, $7 
#add $5, $6, $5


#2
#addi $1, $0, 2
#addi $2, $0, 1
#beq $1 , $2 , one 
#sw $1 , 1023($2) 
#addi $2 , $2 , 1 
#bne $1 , $2 , one 
#beq $1 , $2 , two 
#one: addi $2 , $2 , -1 
#two: lw $3 , 1022($2)

#1st
#addi $1, $0, 2
#addi $2, $0, 1
#sw $1 , 1023($2) 
#add $1,$1,$1 
#sub $2 , $1, $2 
#lw $3, 1021($2)

#main:
#	addi	$t0, $0, 10
#	add		$t1, $0, $0
#	addi	$s1, $zero, 320
#loop:
#	beq		$t1, $t0, exit
#	sw		$s0, 12($s1)
#	addi	$s0, $s0, 100
#	addi	$t1, $t1, 1
#	j		loop
#
#exit:
