#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "hdhC.h"

#include "iobj.h"
#include "brace_op.h"
//#include "date.h"
//#include "matrix_array.h"
#include "nc_api.h"
#include "annotation.h"
#include "split.h"
#include "readline.h"

#include "hdhC.cpp"
#include "BraceOP.cpp"
#include "NcAPI.cpp"
#include "Annotation.cpp"
#include "Split.cpp"
#include "ReadLine.cpp"

/*! \file getNC_att.cpp
 \brief Get value of global attributes.

 Purpose: get values of global attributes.\n
 A netCDF file is mandatory as first argument.
 The next arguments select global attributes.\n
 Output format (on separate lines): 'attribute=value'.
 Additionally, the key-word 'record_num' may be given as parameter.
 When an argument is not a valid attribute (or empty), then
 'attribute=' appears in the output.*/

void  exceptionError(std::string );

int main(int argc,char *argv[])
{
  NcAPI nc;

  if( ! nc.open( argv[1], "", false) )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "getNC_att: netCDF NcAPI::open()\n" ;
    ostr << "Could not open file " << argv[1] ;
    exceptionError( ostr.str() );  // exits
  }

  std::string s;
  std::string str;

  for( int i=2 ; i < argc ; ++i )
  {
    s=argv[i];

    if( ! nc.isAttValid(s) )
    {
      std::cout << s << "= " ;
      continue;
    }

    if( s == "record_num" )
    {
      size_t sz = nc.getNumOfRecords() ;

      std::cout << "record_num=" << sz << " " ;
      continue;
    }

    // is it a number or a char?
    if( nc.layout.globalAttType[nc.layout.globalAttMap[s]] == NC_CHAR )
    {
      str.clear();
      str=nc.getAttString(s);
      if( str.size() == 0 )
        std::cout << s << "= " ;
      else
        std::cout << s << '=' << str << " " ;
    }
    else
    {
       // is a number
       std::vector<double> v;
       nc.getAttValues(v, s);
       if( v.size() )
       {
         std::cout << s << "=" << v[0] ;
         for( size_t j=1 ; j < v.size() ; ++j )
           std::cout << "," << v[j] ;
         std::cout << " " ;
       }
       else
         std::cout << s << "= " ;
    }
  }

  return 0;
}

void
exceptionError(std::string str)
{
  std::string name = "error_getNC_att.txt";

  // open file for writing
  std::ofstream ofsError(name.c_str());

  ofsError << str << std::endl;

  exit(1);
  return ;
}
