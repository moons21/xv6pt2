#include "types.h"
#include "user.h"

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
  
  long long n = 0;
  if (argc > 1)
    n = atoi(argv[1]);
  //printf(1,"Finding the %d Fibonacci number\n", n);
  //printf(1,"Fib(%d) = %ld \n\n",n, fib(n)); 
  nest(n);
  exit();
}
	
