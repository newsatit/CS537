#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "open: (%d), close: (%d)", getopenedcount(), getclosedcount()); 
  exit();
}
