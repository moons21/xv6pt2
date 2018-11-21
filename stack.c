#include "types.h"
#include "user.h"

// Prevent this function from being optimized, which might give it closed form
#pragma GCC push_options
#pragma GCC optimize ("O0")
static int
recurse(int n)
{
  if(n == 0)
    return 0;
  return n + recurse(n - 1);
}
#pragma GCC pop_options

int
main(int argc, char *argv[])
{
  int n, m;

  if(argc != 2){
    printf(1, "Usage: %s levels\n", argv[0]);
    exit();
  }

  n = atoi(argv[1]);
  printf(1, "Lab 3: Recursing %d levels\n", n);
  m = recurse(n);
  printf(1, "Lab 3: Yielded a value of %d\n", m);
  exit();
}


/*
long long fib(long long n){
  if (n == 0)
    return 1;
  if (n == 1)
     return 1;
  return fib(n-1) + fib(n - 2);
}

int nest(int n){
  printf(1, "%d \n", n);
  if (n - 1 != 0)
    nest(n - 1);
  return 0;
}
int main(int argc, char *argv[])
{
  //  int *i = malloc(2147459068 * sizeof(int));
  //  i[1] = 1;

  long long n = 0;
  if (argc > 1)
    n = atoi(argv[1]);
  //printf(1,"Finding the %d Fibonacci number\n", n);
  //printf(1,"Fib(%d) = %ld \n\n",n, fib(n)); 
  nest(n);
  exit();
}
*/
	
