#include <sys/statvfs.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

/*! \file diskUsage.c
 \brief Print disk usage information.

 See statvfs man-page.
 The fs-name has to be provided as single argument on the command-line.
 Output: available_space used_space relative_avail_space[%].\n
 -DSTSTVFS during compilation: some machines use a statvfs struct,
  others statvfs64.
*/

/* description of sttvfs

 struct statvfs {
 unsigned long  f_bsize;        file system block size
 unsigned long  f_frsize;       fragment size
 fsblkcnt_t     f_blocks;       size of fs in f_frsize units
 fsblkcnt_t     f_bfree;        free blocks
 fsblkcnt_t     f_bavail;       free blocks for non-root
 fsfilcnt_t     f_files;        inodes
 fsfilcnt_t     f_ffree;        free inodes
 fsfilcnt_t     f_favail;       free inodes for non-root
 unsigned long  f_fsid;         file system ID 
 unsigned long  f_flag;         mount flags 
 unsigned long  f_namemax;      maximum filename length };

 int statvfs(const char *filename, struct statvfs buf);

 return 0 on success and -1 on failure
*/

int main(int argc, const char* argv[])
{
#ifdef STATVFS
  struct statvfs buf;
#else
  struct statvfs64 buf;
#endif

 int status;

 long double total, used, free;
 double prcnt;

#ifdef STATVFS
  status = statvfs(argv[1], &buf);
#else
  status = statvfs64(argv[1], &buf);
#endif

 if( status )
 {
   fprintf(stderr, "error inquiring disk usage %s\n", argv[1]);
   return 1 ;
 }

 total = (long double) buf.f_blocks;
/* f_bsize  = (long double) buf.f_bsize;*/
 free  = (long double) buf.f_bfree;

 used  = total - free;
 prcnt = (double)( used/total ) * 100.;

 /* something went wrong; perhaps the numbers were to large
    for type 'long double'*/
 if( prcnt < 0. || prcnt > 100. )
   return 1;

/*
f_blocks=1024.;
for( i=0 ; i < 3 ; ++i )
{
 total /= f_blocks ;
 free  /= f_blocks ;
 used  /= f_blocks ;
}

 fprintf(stdout,"%Lf", total );
 fprintf(stdout,"    %Lf", used );
 fprintf(stdout,"   %Lf", free );
*/

 fprintf(stdout,"   %f\n", prcnt );

 return 0 ;
}
