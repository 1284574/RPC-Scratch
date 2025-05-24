#include <stdio.h>
#include <stdbool.h>

#include "is_prime.h"

int main(void)
{
    // prompt user to enter a number:
    printf("Please enter a number: ");
    // read in user's number. Assume you enter a valid number
    int input;
    scanf("%d", &input);

    // check if it is prime
    if(is_prime(input))
    {
        printf("%d is prime\n", input);
    }
    else
    {
        printf("%d is not prime\n", input);
    }
    return 0;
}