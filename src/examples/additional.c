#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int fibo = fibonacci(atoi(argv[1]));
  int max4 = max_of_four_int(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

  printf("%d %d\n", fibo, max4);

  return EXIT_SUCCESS;
}
