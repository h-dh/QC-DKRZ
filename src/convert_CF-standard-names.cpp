#include <iostream>
#include <fstream>
#include <string>

#include "hdhC.h"
#include "split.h"
#include "readline.h"

#include "hdhC.cpp"
#include "Split.cpp"
#include "ReadLine.cpp"

/*! \file convert_CF-standard-names.cpp
 \brief Convert CF xml-table containing standard names and units.

  Standard name and units are extracted and written in
  CSV format line-by-line.
*/

int main(int argc, char *argv[])
{
  if( argc == 1 )
  {
    std::cout << "usage: convert_CF-standard-names.x cf-standard-name-table.xml" << std::endl;
    return 0;
  }
  
  std::string filename( argv[1] ) ;

  if( filename.substr( filename.size() - 4) != ".xml" )
     // wrong format
     return 1 ;

  std::string tmpFile(filename.substr(0,filename.size() - 4) );
  tmpFile += ".tmp" ;

  std::string outFile(filename.substr(0,filename.size() - 4) );
  outFile += ".csv" ;

  ReadLine ifs( filename ) ;
  ifs.clearSurroundingSpaces();

  std::fstream ofs( tmpFile.c_str(), std::ios::out );

  if( ! ifs.isOpen() )
    // could not open
    return 2 ;

  if( ! ofs.is_open() )
    // could not open
    return 3 ;

  std::string line;
  std::string str;
  std::string alias;
  size_t p0, p1;

  // Find entries, aliases and units.
  // Units are only avaialble for entries.
  // Two types of output lines of a temp files:
  //     entry,entry,units
  //     alias, entry
  while( ! ifs.getLine(line) )
  {
     // standard name
     if( (p0=line.find("<entry id=")) < std::string::npos )
     {
       p0 += 10;
       p1=line.find(">", p0) ;

       str = line.substr(p0+1, p1-p0-2);
       ofs << str << "," << str ;

       while( ! ifs.getLine(line) )
       {
         // units
         if( (p0=line.find("<canonical_units>")) == std::string::npos )
            continue;
         p0 += 17 ;
         p1=line.find("</canonical_units>", p0) ;

         ofs << "," << line.substr(p0, p1-p0) << "\n";
         break;
       }

       continue;
     }

     // looking for aliases
     if( (p0=line.find("<alias id=")) < std::string::npos )
     {
       p0 += 10;
       p1=line.find(">", p0) ;

       alias = line.substr(p0+1, p1-p0-2);
       ifs.getLine(line) ;

       p0=line.find("<entry_id>") + 10;
       p1=line.find("<", p0) ;

       str = line.substr(p0, p1-p0);

       ofs << str << "," << alias << std::endl ;
     }
  }

  ifs.close();
  ofs.close();

  // sort file.tmp to file.tmp.tmp
  str =  "sort " ;
  str += tmpFile ;
  str += " > " ;
  str += tmpFile ;
  str += ".tmp" ;

  if( system( str.c_str()) )
    return 4 ;

  // Read again, swap first and second column, thus with leading alias.
  // Add units to alias lines. remove second column from entry lines.
  str=tmpFile;
  str += ".tmp";

  ifs.open( str );
  ofs.open(outFile.c_str(), std::ios::out);

  if( ! ifs.isOpen() )
    // could not open
    return 12 ;

  if( ! ofs.is_open() )
    // could not open
    return 13 ;

  std::vector<std::string> c0;
  std::vector<std::string> c1;
  std::string units;

  Split splt;
  splt.setSeparator(",");

  ifs.getLine(line) ;
  splt = line ;
  c0.push_back( splt[0]);
  c1.push_back( splt[1]);
  if( splt.size() > 2 )
    units=splt[2];

  while( ! ifs.getLine(line) )
  {
     splt = line ;
     if( splt[0] == c0[0] )
     {
       c0.push_back( splt[0]);
       c1.push_back( splt[1]);
       if( splt.size() > 2 )
         units=splt[2];
     }
     else
     {
       for( size_t i=0 ; i < c0.size() ; ++i )
       {
         if( c0[i] == c1[i] )
           ofs << c0[i] << ",," << units << std::endl;
         else
           ofs << c1[i] << ","
               << c0[i] << ","
               << units << std::endl;
       }

       c0.clear();
       c1.clear();
       units.clear();

       c0.push_back( splt[0]);
       c1.push_back( splt[1]);
       if( splt.size() > 2 )
         units=splt[2];
     }
  }


  // last flush
  for( size_t i=0 ; i < c0.size() ; ++i )
  {
    if( c0[i] == c1[i] )
      ofs << c0[i] << ",," << units << std::endl;
    else
      ofs << c1[i] << ","
          << c0[i] << ","
          << units << std::endl;
  }

  ifs.close();
  ofs.close();

  str = "rm ";
  str += tmpFile;
  system( str.c_str()) ;

  str += ".tmp";
  system( str.c_str()) ;

  return 0;
}
