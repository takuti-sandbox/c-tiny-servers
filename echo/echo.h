#include <stdio.h>
#include <stdlib.h>

void err_func(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
