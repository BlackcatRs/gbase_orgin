#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  // printf("%d\n", argc);
  char buffer[50];

  // converts the initial part of the string in str to an unsigned long int
  // value according to the given base
  long int accumulator = strtol(argv[1], (char **)NULL, 2);

  sprintf(buffer, "%lu", accumulator);
  printf("%s\n", buffer);

  // int accumulator = 10110011;


  return 0;
}

/*
1011 = 11
0000 = 0
----
0000


1011 = 11
0001 = 1
----
0001


1011 = 11
0010 = 2
----
0010

1011 = 11
0100 = 4
----
0000


1011 = 11
1000 = 8
----
1000



                    0011
                    1000 0100 0010 0001



2 = 0100
3 = 1000


000 0000 0000 0000 0000 0000 0000 0001
100 0000 0000 0000 0000 0000 0000 0000
*/
