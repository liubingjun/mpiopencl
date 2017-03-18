#include <stdio.h>
#include <stdlib.h>
#include <assert.h>s
#include "mix.h"

int main(int argc, char *argv[]){
  MPI_Init(&argc, &argv);
  mix(argc,argv);
  mix(argc,argv);
  MPI_Finalize();
  return 0;
}
