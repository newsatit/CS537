//Tests opening small number of files

#include "types.h"
#include "user.h"
#include "param.h"


void openfile(char *file, int howmany) {
  int i;
  
  for (i = 0; i < howmany; i++)
  {
      int fd = open(file, 0);
	  close(fd);
  }
  int fd = open(file, 0);
  fd = open(file, 0);
  printf(1, "%d\n", fd);
}

int
main(int argc, char *argv[])
{
  int rc1 = getopenedcount();
  int c1 = getclosedcount();
  openfile("README", 5);
  int rc2 = getopenedcount();
  int c2 = getclosedcount();
  if((rc2 - rc1) == 5)
	  printf(1, "TEST PASSED\n");
  else
	  printf(1, "TEST FAILED: rc2 - rc1 = %d, expected 5, %s: %d\n", rc2 - rc1 , __FILE__, __LINE__);
  printf(1, "open %d close %d\n", rc2-rc1, c2-c1);
  exit();
}
