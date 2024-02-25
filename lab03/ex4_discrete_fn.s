.globl f # this allows other files to find the function f

# f takes in two arguments:
# a0 is the value we want to evaluate f at
# a1 is the address of the "output" array (read the lab spec for more information).
# The return value should be stored in a0
f:
    # Your code here
    addi t0, a0, 3 # t0 = a0 + 3, a0 is the value we want to evaluate f at, t0 is the index of the array
    slli t0, t0, 2 # t0 = t0 * 4, t0 is the offset of the element in the array
    add t0, t0, a1 # t0 = t0 + a1, t0 is the address of the element in the array
    lw a0, 0(t0) # a0 = *t0, a0 is the value of the element in the array

    # This is how you return from a function. You'll learn more about this later.
    # This should be the last line in your program.
    jr ra
