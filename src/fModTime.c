/*! \file fModTime.c
 \brief Print modification time of a file

 The filename has to be provided as single argument on the command-line.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, const char* argv[])
{
  struct stat buffer;
  int         status;
  long mTime;

  status = stat(argv[1], &buffer);

  if( status == 0 )
  {
    time_t t=buffer.st_mtime ;
    mTime = t ;

    fprintf(stdout,"%li", mTime );
  }
  else
  {
    fprintf(stdout,"%li", (long)0 );
    fprintf(stderr, "error getting stat of file %s\n", argv[1]);
  }

  return status;
}

