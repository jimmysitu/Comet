#include <machine/syscall.h>
#include <sys/types.h>

#define STATUSREG_ADDRESS 0x300004
#define STDOUTREG_ADDRESS 0x300000

#define EXITFLAG 0

/* Write to a file.  */
ssize_t _write(int file, const void* ptr, size_t len)
{

  char* serial = (char*)STDOUTREG_ADDRESS;
  for (unsigned int oneChar = 0; oneChar < len; oneChar++)
    *serial = ((char*)ptr)[oneChar];

  return len;
}

/* Exit a program without cleaning up files.  */
void _exit(int exit_status)
{
  int* statusRegister = (int*)STATUSREG_ADDRESS;
  // set the exit flag
  *statusRegister |= (1 << EXITFLAG);
  while (1)
    ;
}
