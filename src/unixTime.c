#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>

/*! \file unixTime.c
 \brief Output time in seconds.

 See man-page for struct timeval time. Any paramter adds
 fraction of a second to the output.
*/

int main(int argc, char *argv[])
{
  /* a struct timeval to hold the current time. */
  /* timeval is defined in sys/time.h */
  struct timeval time;

  double myTime;

  /* get the current time and store value in 'time' */
  gettimeofday(&time, NULL);

  /* convert the time to a fraction */

/*
  myTime = time.tv_sec + (time.tv_usec/1000000.0);
*/

  if( argc == 2 )
  {
/*    printf("%lf", (time.tv_sec+time.tv_usec));*/
    myTime = time.tv_sec + (time.tv_usec/1000000.0);
    printf("%f", myTime);
    return 0;
  }

  printf("%li", time.tv_sec);
  return 0;
}

