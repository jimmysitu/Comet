#include <machine/syscall.h>
#include <sys/types.h>

#define STATUSREG_ADDRESS 0x300004
#define STDOUTREG_ADDRESS 0x300000

#define EXITFLAG 0

/* Write to a file.  */
ssize_t _write(int file, char* ptr, size_t len)
{

  volatile char* serial = (char*)STDOUTREG_ADDRESS;
  for (unsigned int oneChar = 0; oneChar < len; oneChar++)
    *serial = ptr[oneChar];

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

extern char _end[]; /* _end is set in the linker command file */
char* heap_ptr;

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
char* _sbrk(nbytes) int nbytes;
{
  char* base;

  if (!heap_ptr)
    heap_ptr = (char*)&_end;
  base = heap_ptr;
  heap_ptr += nbytes;

  return base;
}
