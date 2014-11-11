#include <iostream>
#include <fstream>
#include <string>

#include "hdhC.h"
#include "split.h"
#include "readline.h"

#include "hdhC.cpp"
#include "Split.cpp"
#include "ReadLine.cpp"

/*! \file check_CORDEX-standard-table.cpp
 \brief Convert CF xml-table containing standard names and units.

  Utilises cf-standard-name-table.xml contents converted into
  CSV format line-by-line.
  Param 1: CF standard name table 
  Param 2: CORDEX_variables_requirement_table_130104.csv
*/

int main(int argc, char *argv[])
{
  if( argc == 1 )
  {
    std::cout << "usage: check_CORDEX-standard-table.x file_1 file_2\n" ;
    std::cout << "  file_1   cf-standard-name-table.csv\n" ;
    std::cout << "  file_2   CORDEX_variables_requirement_table.csv" << std::endl ;
    return 0;
  }

  std::string file_cf( argv[1] ) ;
  std::string file_cordex( argv[2] ) ;

  if( file_cf.substr( file_cf.size() - 4) != ".csv" )
     // wrong format
     return 1 ;

  if( file_cordex.substr( file_cordex.size() - 4) != ".csv" )
     // wrong format
     return 2 ;

  ReadLine ifs_cf( file_cf ) ;

  if( ! ifs_cf.isOpen() )
    // could not open
    return 3 ;

  std::vector<std::string> name_cf;
  std::vector<std::string> units_cf;

  std::string line;

  Split splt_cf ;
  splt_cf.setSeparator(",");
  splt_cf.enableEmptyItems();

  Split splt_cordex ;
  splt_cordex.setSeparator(",");
  splt_cordex.enableEmptyItems();
  
  while( ! ifs_cf.getLine(line) )
  {
    splt_cf = line ;
    name_cf.push_back(splt_cf[0]);
    units_cf.push_back(splt_cf[2]);
    // index 1: entry id for alias, else empty
  }

  size_t sz_cf=name_cf.size();
  ifs_cf.close();

  ReadLine ifs_cordex( file_cordex ) ;
  if( ! ifs_cordex.isOpen() )
    // could not open
    return 4 ;

  bool is;
  
  while( ! ifs_cordex.getLine(line) )
  {
     if( line[0] == '#' )
        continue;

     splt_cordex = line ;
     is=true;
     
     for( size_t i=0 ; i < sz_cf ; ++i )
     {
       if( splt_cordex[3] == name_cf[i] )
       {
	  if( splt_cordex[4] != units_cf[i] )
	  {
	     std::cout << "Std.Name=" << name_cf[i] 
	               << ": CF units=" << units_cf[i] 
	               << ", CORDEX units:" << splt_cordex[4] << std::endl ;
	  }

	  is=false;
	  break;
       }
     }

     if( is )
     {
	std::cout << "CORDEX Name=" << splt_cordex[3]
	          << " is not a CF standard name" << std::endl;
     }
  }
  
  ifs_cordex.close();

  return 0;
}
