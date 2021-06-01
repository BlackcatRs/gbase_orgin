#include <stdio.h>


int main(int argc, char const *argv[]) {
  char buffer[50];
  long int accumulator = 26;

  sprintf(buffer, "%lo", accumulator);
  printf("%s\n", buffer);
  return 0;
}
