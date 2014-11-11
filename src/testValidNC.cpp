#include "netcdf.h"  // local instance

/*! \file testValidNC.cpp
\brief Check whether the supplied file is in valid netCDF fomat. */
 
int main(int argc, char *argv[])
{
  if( argc == 0 )
  {
    return 1;
  }

  int ncid;

  if( nc_open( argv[1], 0, &ncid) )
    return 1 ;

  nc_close(ncid);
  return 0;
}
