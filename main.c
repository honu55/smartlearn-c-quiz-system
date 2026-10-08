#include <stdio.h>
int main()
{
    int num;
    printf("Enter an Integer: ");
    scanf("%d", &num);

    if (num > 0)
    {
        printf("%d is Positive.\n", num);
    }
    else if (num == 0)
    {
        printf("%d is Zero.\n", num);
    }
    else
    {
        printf("%d is Negative.\n", num);
    }

    if (num % 2 == 0)
    {
        printf("%d is Even.\n", num);
    }
    else
    {
        printf("%d is Odd.\n", num);
    }

    if (num % 3 == 0 && num % 5 == 0)
    {
        printf("%d is divisible by both 3 and 5.\n", num);
    }
    else if (num % 3 == 0)
    {
        printf("%d is divisible by only 3.\n", num);
    }
    else if (num % 5 == 0)
    {
        printf("%d is divisible by only 5.\n", num);
    }
    else
    {
        printf("%d is divisible by neither.\n", num);
    }
    return 0;
}
