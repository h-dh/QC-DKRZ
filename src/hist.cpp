/*! \file hist.cpp
 \brief Frequency Distribution

Eventually convert frequency distributions from the storage-formatted build-file of to the final table.

 Options:\n
    -a       Merge  histogramms from file1, ..., fileN.\n
    -B       Save fD as build.\n
    -D w.pF  Format fuer Output: w=Breite, p=Genauigkeit.\n
             F=d: Dezimal, Default: exponentiell.\n
    -d       Difference of histograms in bar-printing format:\n
             file1 minus file2. Result is in bar-printing format. \n
    -o str   Output file. \n
    -p       Save histogram as text.\n
    -P       Save histogram for bar-printing.\n
    -Q       Save properties of the fD.\n
    -z       rel. Freq. in per cent.
*/

#include <iostream>
#include <iomanip>

#include <string>
#include <vector>

#include "hdhC.h"
#include "readline.h"
#include "freqDist.h"
#include "getopt_hdh.h"

// class template
#include "FreqDist.cpp"

void descript(void);
void difference(std::string &, int optind, int argc , char *argv[]);
void print(double bound, double v, double p, 
  int formatWidth, int formatPrecis);

int formatWidth;
int formatPrecis;
bool isScientific;

int main(int argc , char *argv[])
{
  if( argc == 1 )
  {
     descript();
     exit(1);
  }

  std::string outFileName("fD_defaultPrintName");

  bool isAdd=false;
  bool isDiff=false;
  bool isPerCent=false;
  bool isPrintBarContour=false;
  bool isPrintText=false;
  bool isSaveBuild=false;
  bool isSaveProperties=false;

  GetOpt opt;

  formatWidth=17;
  formatPrecis=6;
  isScientific=true;

  opt.opterr = 0;
  int c ;

  while( (c = opt.getopt(argc,argv,"aBD:do:pPQz")) != -1 )
  {
    switch ( c )
    {
    case 'a':
      isAdd=true;
      break;
    case 'B':
      isSaveBuild=true;
      break;
    case 'd':
      isDiff=true;
      break;
    case 'D':
      {
      std::string tmp ;
      if( tmp[tmp.size()-1] == 'd' || tmp[tmp.size()-1] == 'D' )
          isScientific = false;

      c=tmp.find('.') ;
      formatWidth = (int)hdhC::string2Double( tmp.substr(0, c).c_str() ,1 );
      formatPrecis= (int)hdhC::string2Double( tmp.substr(c+1,tmp.size()-c-1).c_str() ,1 );
      }

      break;

    case 'o':
      outFileName=(opt.optarg);
      if( outFileName.size() > 6
        && outFileName.substr(outFileName.size()-6) == ".build" )
           outFileName.erase(outFileName.size()-6) ;
      else if( outFileName.size() > 5
         && outFileName.substr(outFileName.size()-5) == ".hist" )
           outFileName.erase(outFileName.size()-5) ;

      break;
    case 'p':
      isPrintText=true;
      break;
    case 'P':
      isPrintBarContour=true;
      break;
    case 'Q':
      isSaveProperties=true;
      break;
    case 'z':
      isPerCent=true;
      break;
    default:
      std::cerr << "getopt() returned option -" << (char)c << std::endl ;
      exit(1);
      break;
    }
  }

  if( argc == opt.optind )
  {
     descript();
     exit(0);
  }

  
  if( isDiff )
  {
    std::string t(outFileName);
    t += ".hist-diff";
    difference(t, opt.optind, argc, argv);
    return 0;
  }

  FreqDist<float> fD;

  for(int i=opt.optind ; i < argc ; ++i )
    fD.addBuild( argv[i] );

  if( isSaveBuild )
  {
    fD.setOutputFilename( outFileName + ".build" ) ;
    fD.saveBuild();  // std::cout
//    fD.saveBuild();  // std::cout
//    fD.saveProperties();  // std::cout
//    fD.saveProperties(outFileName);
  }

  if( isSaveProperties )
  {
    fD.setOutputFilename( outFileName + ".build" ) ;
//    fD.saveProperties();  // std::cout
    fD.saveProperties(outFileName);
  }

  if( isPrintBarContour)
  {
    fD.setOutputFilename( outFileName + ".hist" ) ;
    if( isPerCent )
      fD.setPrintPerCent();
    fD.printHist();
  }

  if( isPrintText)
  {
    fD.setOutputFilename( outFileName + ".hist" ) ;
    if( isPerCent )
      fD.setPrintPerCent();
    fD.print();
  }

  return 0;
}

void descript(void)
{
    std::cout << "Usage:  Options file1 file2 [ ... fileN]\n";
    std::cout << "  Combine histogramms.\n";
    std::cout << "  -a       Merge  histogramms from file1, ..., fileN.\n";
    std::cout << "  -B       Save fD as build.\n";
    std::cout << "  -D w.pF  Format fuer Output: w=Breite, p=Genauigkeit.\n" ;
    std::cout << "           F=d: Dezimal, Default: exponentiell.\n" ;

    std::cout << "  -d       Differenz of histograms in bar-printing format:\n";
    std::cout << "           file1 minus file2. Result is in bar-printing format\n";
    std::cout << "  -o str   Output\n";
    std::cout << "  -p       Save histogram as text.\n"
              << std::endl;
    std::cout << "  -P       Save histogram for bar-printing.\n" ;
    std::cout << "  -Q       Save properties of the fD.\n"
              << std::endl;
    std::cout << "  -z       rel. Freq. in per cent.\n"
              << std::endl;
    return;
}

void difference(std::string &outFileName,
        int optind, int argc , char *argv[])
{
  // der std.cout Buffer wird am Ende von main wiederhergestellt
  std::streambuf* cout_sbuf = std::cout.rdbuf();

  std::ofstream *out ;

  if( outFileName.size() > 0 )
  {
      out = new std::ofstream ;
      out->open( outFileName.c_str() );
      std::cout.rdbuf(out->rdbuf());
  }

  if( (argc - optind ) != 2 )
  {
    std::cout << "hist.x: option -d requires exactly two *.hist files" << std::endl;
    exit(1);
  }

  // Init
  ReadLine file1(argv[optind]), file2(argv[optind + 1]);

  std::vector<double> bound1;
  std::vector<double> bound2;
  std::vector<double> val1;
  std::vector<double> val2;
  std::vector<double> bound;
  std::vector<double> dif, prz;

  double  bndLast, bndLast1, bndLast2; 
 
  while( !file1.readLine() )
  {
    bndLast1=file1.getValue(0);
    if( file1.readLine())
      break;
    bndLast1=file1.getValue(0);
           
    bound1.push_back( bndLast1 );
    val1.push_back(file1.getValue(1));
    
    if( file1.readLine())
      break;
    
    bndLast1=file1.getValue(0);
  }
  
  while( !file2.readLine() )
  {
    bndLast2=file2.getValue(0);
    if( file2.readLine())
      break;
    bndLast2=file2.getValue(0);
           
    bound2.push_back( bndLast2 );
    val2.push_back(file2.getValue(1));
    
    if( file2.readLine())
      break;
    
    bndLast2=file2.getValue(0);
  }
  
  size_t i1=0, i2=0;
  double p,d;
  
  while( true )
  {
    if( i1 < bound1.size() && i2 < bound2.size()
          && bound1[i1] == bound2[i2] )
    {
       bound.push_back( bound1[i1] );
       d=val1[i1] - val2[i2] ;    
       dif.push_back( d );
       if( val2[i2] == 0. )
         p=0.;
       else
         p=d*100./val2[i2] ;
       prz.push_back( p );
       ++i1;
       ++i2;
       continue;
    }
    
    if( i1 < bound1.size() && i2 < bound2.size()
          && bound1[i1] < bound2[i2] )
    {
       bound.push_back( bound1[i1] );
       dif.push_back( val1[i1] );
       prz.push_back( MAXDOUBLE );
       ++i1;
       continue;
    }
    
    if( i1 < bound1.size() && i2 < bound2.size()
          && bound1[i1] > bound2[i2] )
    {
       bound.push_back( bound2[i2] );
       dif.push_back( -val2[i2] );
       prz.push_back( -100. );
       ++i2;
       continue;
    }

    if( i1 < bound1.size() && i2 == bound2.size() )
    {
       bound.push_back( bound1[i1] );
       dif.push_back( val1[i1] );
       prz.push_back( MAXDOUBLE );
       ++i1;
       continue;
    }
    
    if( i1 == bound1.size() && i2 < bound2.size() )
    {
       bound.push_back( bound2[i2] );
       dif.push_back( -val2[i2] );
       prz.push_back( -100. );
       ++i2;
       continue;
    }
    
    break;
  }
  
  bndLast=( bndLast1 >= bndLast2 ) ?  bndLast1 : bndLast2,
  bound.push_back( bndLast ) ;
  
  for( i1=0 ; i1 < dif.size() ; ++i1 )
  {
      print(bound[i1], 0., 0., formatWidth, formatPrecis);
      print(bound[i1], dif[i1], prz[i1], formatWidth, formatPrecis);
      print(bound[i1+1], dif[i1], prz[i1], formatWidth, formatPrecis);
  }
  print(bound[i1], 0., 0., formatWidth, formatPrecis);

  // restore the original stream buffer
  std::cout.rdbuf(cout_sbuf);
  
  return;
}

void print(double bound, double dif, double prz, 
  int formatWidth, int formatPrecis)
{
    std::cout.setf( std::ios::scientific, std::ios::floatfield);
    std::cout << std::setprecision(formatPrecis);
    
    std::cout << std::setw(formatWidth)
              << bound ;
    std::cout << std::setw(formatWidth)
              << dif ;
    std::cout << std::setw(formatWidth)
              << prz << std::endl ;
  return;
}
