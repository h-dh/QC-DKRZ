#include "freqDist.h"

template<typename T>
FreqDist<T>::FreqDist()
{
  init();
  reset();
}

template<typename T>
FreqDist<T>::FreqDist(double dx, const bool isCent, T val)
{
  init();

  isCentric=isCent;
  binWidth=dx;

  reset();
  install(val);
}

template<typename T>
void
FreqDist<T>::addBuild(std::string rFile)
{
   // Combine input from rFile with *this.
   // If rFile contains only the class properties, then a new build
   // is created. If rFile also contains data, then the data is read
   // for continuation.

  ReadLine file;

  if( file.open(rFile.c_str()) )
    return ;  //could not open file

  // Read the properties at the first time, i.e. the first input file
  if( isFirstReadBuild )
  {
    reInstall(file);  // install freqDist from file
    isFirstReadBuild=false;
    return ;
  }
  else
  {
    if (readBuild(file) )  // check properties and adjust, respectively.
    {
      file.close();
      return ;  // file is empty or sym., widths, fillValue differ
    }
  }

  // merging starts here

  // first value of the data section
  ensembleCount += hdhC::string2Double(file.getLine(),1) ;

  file.readLine();

  // Get queues. Note: result of the last read attempt is
  //still valid
  int numClasses=static_cast<int>(file.getValue(0)) - 2;

  // Note: The binBorder queue[index] is the left border of a class.
  //       The right side of a class is queue[index] + binWidth.
  //       This has changed since the previous version.

  for( int i=0 ; i <  numClasses; ++i )
  {
    file.readLine();

    // borders with index 0 and 1 (not used)
    // calculate a value
    push_val( file.getValue(4)/file.getValue(3),  // simul. value
              file.getValue(2),        // frequeny
              file.getValue(3),        // counts
              file.getValue(4)) ;      // accum. value
  }

  if ( file.readLine())
  {
    file.close();
    return;
  }

  file.close();

  return ;
}

template<typename T>
void
FreqDist<T>::clear(void)
{
  binBorder.clear() ;
  binVal.clear();
  binFreq.clear();
  binCount.clear();

  lastUsedIndex=0;
}

template<typename T>
void
FreqDist<T>::exceptionError(std::string str)
{
  // Occurrence of an error stops the run immediately.
  xcptn.ofsError=0;
  xcptn.strError = "error_FREQ_DIST.txt" ;

  // open file for appending data
  if( ! xcptn.ofsError )
    xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

  *xcptn.ofsError << str << std::endl;

  exit(1);
  return ;
}

template<typename T>
double
FreqDist<T>::getAverage( int i )
{
  if( binFreq[i] <= 0. )
    return 0.;

  return binVal[i] / binCount[i]  ;
}

template<typename T>
double
FreqDist<T>::getAverage( int i, double v )
{
  if( binFreq[i] <= 0. )
    return 0.;

  return v / binCount[i] ;
}

template<typename T>
std::vector<std::pair<double,double> >
FreqDist<T>::getHistBorder(void)
{
  std::vector<std::pair<double,double> > v;

  for ( int i= 0; i < binBorder.size() ; i++)
     if( binFreq[i] > 0. )
       v.push_back( std::pair<double,double>(binBorder[i]
                    ,binBorder[i] + binWidth ) ) ;

  return v;
}

template<typename T>
std::vector<double>
FreqDist<T>::getHistFreq(void)
{
  std::vector<double> v;

  for ( int i= 0; i < binFreq.size() ; i++)
    if( binFreq[i] > 0. )
      v.push_back( binFreq[i] ) ;

  return v;
}

template<typename T>
std::vector<double>
FreqDist<T>::getHistVal(void)
{
  std::vector<double> v;

  for ( int i= 0; i < binVal.size() ; i++)
    if( binFreq[i] > 0. )
      v.push_back( getAverage(i) ) ;

  return v;
}

template<typename T>
void
FreqDist<T>::init( void )
{
  isPrintPerCent=false;
  isFirstReadBuild=true;
  isReadOnlyProperties=false;

  isAutomResizeEnabled=false;
  automResizeFactor=2.;
  automResizeThreshold=50;

  // default for properties
  isCentric=true;
  binWidth=1.e-08;
  rangeMin=0.;
  rangeMax=0.;
  isFillingValueEnabled=false;
  numFV=0;
  ensembleCount=0.;
  lastUsedIndex=0;

  isPrintHistogram=false;

  NUMERICAL_TOLERANCE=1.E-5;

  return;
}

template<typename T>
void
FreqDist<T>::install(T val )
{
  // initVal==0 by default
  if( binWidth == 0. && ! isAutomResizeEnabled )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "FreqDist::install()";
    ostr << "\nbin width equals zero";

    exceptionError( ostr.str() ); // exits
  }

  // also for disabled width adjustment
  double vLeft = updateBinWidth(val) ;

  binBorder.insert(binBorder.begin()+1, vLeft);
  binFreq.insert(binFreq.begin()+1, 0.);
  binVal.insert(binVal.begin()+1, 0.);
  binCount.insert(binCount.begin()+1, 0.);

  return ;
}

template<typename T>
void
FreqDist<T>::print(std::string file,
   int formatWidth, int formatPrecis )
{
  std::streambuf* cout_sbuf = std::cout.rdbuf();
  std::ofstream *out = 0 ;

  if( ! isPrintHistogram )
  {
    if( file.size() )
      outputFilename=file;
    else if( outputFilenameBase.size() )
      outputFilename=outputFilenameBase + ".hist";
  }

  if ( outputFilename.size() > 0 )
  {
    out = new std::ofstream ;
    out->open( outputFilename.c_str() );
    std::cout.rdbuf(out->rdbuf());
  }

  std::cout.setf( std::ios::scientific, std::ios::floatfield);

  double inverseEnsembleCount;

  // when invoked by post-processing then there is no explicit
  // knowledge about weighting. But, in case of area weighted
  // freq dist, the sum of binFreq equals the ensembleCount
  double sumOfBinFreq=0;
  for( size_t k=1 ; k < binFreq.size() -1 ; ++k)
     sumOfBinFreq += binFreq[k] ;
  sumOfBinFreq=static_cast<double>(
      static_cast<long>(sumOfBinFreq + 0.5) ) ;

  ensembleCount=static_cast<double>(
      static_cast<long>(ensembleCount) ) ;

  if( sumOfBinFreq == ensembleCount )
    inverseEnsembleCount
             =( ensembleCount>0 ) ? (1./ensembleCount) : 1.;
  else
  {
    // total sum of events
    inverseEnsembleCount=0.;
    for( size_t k=0 ; k < binCount.size() ; ++k)
      inverseEnsembleCount += binCount[k] ;
    if( inverseEnsembleCount > 0. )
      inverseEnsembleCount = 1./inverseEnsembleCount ;
    else
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "FreqDist::print()";
      ostr << "\nempty frequency distribution";

      exceptionError( ostr.str() ); // exits
    }
  }

  double a;
  if( isPrintPerCent )
    inverseEnsembleCount *= 100.;

  // print contour of histogram bars
  std::cout << std::setprecision(formatPrecis);

  for ( size_t i= 0; i < binBorder.size() ; i++)
  {
    if( binFreq[i] == 0. )
      continue;

    a=getAverage(i) ;

    if( isPrintHistogram )
    {
      std::cout << std::setw(formatWidth)
                << binBorder[i] ;
      std::cout << std::setw(formatWidth) << 0 ;
      std::cout << std::setw(formatWidth)
                << a <<"\n" ;
    }

    // print the table line
    std::cout << std::setw(formatWidth)
              << binBorder[i] ;

    // second border only for the table format
    if( ! isPrintHistogram )
    {
      std::cout << std::setw(formatWidth)
                << binBorder[i]+binWidth ;
    }

    std::cout << std::setw(formatWidth)
              << ( binFreq[i] * inverseEnsembleCount ) ;
    std::cout << std::setw(formatWidth)
              << a <<"\n" ;

    if( isPrintHistogram )
    {
      std::cout << std::setw(formatWidth)
                << (binBorder[i]+binWidth) ;
      std::cout << std::setw(formatWidth)
                << ( binFreq[i] * inverseEnsembleCount ) ;
      std::cout << std::setw(formatWidth)
                << a <<"\n" ;


      // Close the histogram bar to the baseline.
      // Skip this, if the next up-draw is identical
      if( fabs( binBorder[i+1] -binBorder[i] -binWidth )
           > binWidth*1.E-10 )
      {
        std::cout << std::setw(formatWidth)
                  << (binBorder[i]+binWidth) ;

        std::cout << std::setw(formatWidth) << 0 ;
        std::cout << std::setw(formatWidth)
                << a << std::endl ;
      }
    }
  }

  if ( outputFilename.size() > 0 )
  {
    std::cout.rdbuf(cout_sbuf); // restore the original stream buffer
    delete out;
  }

  return ;
}

template<typename T>
void
FreqDist<T>::printHist(std::string file,
     int formWidth, int formPrecis )
{
  isPrintHistogram=true;

  if( file.size() )
    outputFilename=file;
  else if( outputFilenameBase.size() )
    outputFilename=outputFilenameBase + ".Hist";

  print(file, formWidth, formPrecis);

  return ;
}

template<typename T>
void
FreqDist<T>::push_val( double val, double weight,
  double counts, double accum_val )
{
 // The only push_val function assigning values
 // to the frequency distribution

  // procedure for merging pre-calculated class properties
  double eff_val=val;
  if( counts > 1. )
    eff_val = accum_val;

  if( val == (val+binWidth) ||
      ( val > 0. && static_cast<int>(val) < 0) )
  {  // incredible huge. Thus, perform a plain scan through the total range
    size_t pos;
    for( pos=0 ; pos < binBorder.size() ; ++pos )
    {
      if( val > binBorder[pos] )
        continue;
      if( val == binBorder[pos] )
        break;

       binBorder.insert(binBorder.begin()+pos, val);
       binFreq.insert(binFreq.begin()+pos, 0.);
       binVal.insert(binVal.begin()+pos, 0.);
       binCount.insert(binCount.begin()+pos, 0.);
       break;
    }

    binFreq[pos]  += weight ;
    binCount[pos] += counts ;
    binVal[pos]   +=  eff_val ;

    lastUsedIndex = pos ; //for both: left an right side
    return;
  }

  int left, right;

  // Within the current freqDist range.
  // Initialise the nested interval method.
  if( val < binBorder[lastUsedIndex] )
  {
     left = 0 ;
     right = lastUsedIndex ;
  }
  else
  {
     left = lastUsedIndex ;
     right = binFreq.size() -1 ;
  }

  // Scan through intervals.
  do
  {
    if( (right - left ) == 1 )  // found
      break;

    int i = (left + right ) / 2 ;

    if( val >= binBorder[i] )
    {
       left = i ;
       if( val < (binBorder[i] + binWidth) )
       {
         // found the interval
         right = i+1 ;
         break;
       }
    }
    else
    {
       right = i ;
       if( val > (binBorder[i] - binWidth) )
       {
         // found the interval
         left = i-1 ;
         break;
       }
    }

    // adjust class properties, if the number of classes
    // exceeds given threshold (default: 50).
    if( isAutomResizeEnabled
        && binFreq.size() > automResizeThreshold )
    {
      resize();
      push_val(val, weight, counts, accum_val);  // recursion
      return;
    }
  } while (true) ;

  size_t pos;

  // Did we found a class or a gap?
  if( val >= (binBorder[left] + binWidth) )
  {
     // A gap: new class in front of right.
     // Find the leftmost boundary.

     // even if class width adjustment is disabled, the
     // method gives the new border to be inserted
     double vLeft=updateBinWidth(val);

     pos=static_cast<size_t>(right);
     binBorder.insert(binBorder.begin()+pos, vLeft);
     binFreq.insert(binFreq.begin()+pos, 0.);
     binVal.insert(binVal.begin()+pos, 0.);
     binCount.insert(binCount.begin()+pos, 0.);
  }
  else
     pos=static_cast<size_t>(left);

  binFreq[pos]  += weight ;
  binCount[pos] += counts ;
  binVal[pos]   += eff_val;

  lastUsedIndex = pos ; //for both: left an right side

  return;
}

template<typename T>
void
FreqDist<T>::push_val(std::valarray<T> &v)
{
   push_val(v.size(), &v[0]);
   return;
}

template<typename T>
void
FreqDist<T>::push_val(std::valarray<T> &v, std::valarray<double> &w)
{
   push_val(v.size(), &v[0], &w[0]);
   return;
}

template<typename T>
void
FreqDist<T>::push_val(const MtrxArr<T> &v)
{
  if( isFillingValueEnabled )
  {
    for ( size_t i=0 ; i < v.size() ; i++)
    {
       if( testEquality( v[i], fillingValue ) )
       {
         ++numFV;
         continue;
       }

       push_val( v[i] ) ;
    }
  }
  else
    for ( size_t i=0 ; i < v.size() ; i++)
       push_val( v[i] ) ;

  return;
}

template<typename T>
void
FreqDist<T>::push_val(const MtrxArr<T> &v, const MtrxArr<double> &w)
{
  if( isFillingValueEnabled )
  {
    for ( size_t i=0 ; i < v.size() ; i++)
    {
       if( testEquality( v[i], fillingValue ) )
       {
         ++numFV;
         continue;
       }

       push_val( v[i], w[i] ) ;
    }
  }
  else
    for ( size_t i=0 ; i < v.size() ; i++)
       push_val( v[i], w[i] ) ;

  return;
}

template<typename T>
void
FreqDist<T>::push_val(size_t size, const T *v, const double *w)
{
  if( isFillingValueEnabled )
  {
    for ( size_t i=0 ; i < size ; i++)
    {
       if( testEquality( v[i], fillingValue ) )
       {
         ++numFV;
         continue;
       }

       push_val( v[i], (w==0)?1. : w[i] ) ;
    }
  }
  else
    for ( int i=0 ; i < size ; i++)
       push_val( v[i], (w==0)?1. : w[i] ) ;

}

template<typename T>
void
FreqDist<T>::push_val(std::vector<T> &v)
{
  if( isFillingValueEnabled )
  {
    for ( size_t i=0 ; i < v.size() ; i++)
    {
       if( testEquality( v[i], fillingValue ) )
       {
         ++numFV;
         continue;
       }

       push_val( v[i] ) ;
    }
  }
  else
    for ( int i=0 ; i < v.size() ; i++)
       push_val( v[i] ) ;

  return;
}

template<typename T>
void
FreqDist<T>::push_val(std::vector<T> &v, std::vector<double> &w)
{
  if( isFillingValueEnabled )
  {
    for ( size_t i=0 ; i < v.size() ; i++)
    {
       if( testEquality( v[i], fillingValue ) )
       {
         ++numFV;
         continue;
       }

       push_val( v[i], w[i] ) ;
    }
  }
  else
    for ( int i=0 ; i < v.size() ; i++)
       push_val( v[i], w[i] ) ;

  return;
}

template<typename T>
bool
FreqDist<T>::readBuild( ReadLine &file)
{
  // read properties of a freqDist histogram

  if( file.readLine() )
  {
    file.close();
    return true;  // file is empty
  }

  std::string str;
  double val;

  do
  {
    str=file.getLine();
    val=hdhC::string2Double(str,1);

    // Properties form a header of a data file.

    if( str.find("Symmetry about value=") < std::string::npos )
    {
       if( isCentric == (str[str.size()-1] == 'T' ? true : false ) )
         continue;
       goto ERROR;
    }
    if( str.find("Class width=") < std::string::npos )
    {
       if( testEquality(binWidth,  val) )
         continue;
       goto ERROR;
     }
    if( str.find("Filling value=") < std::string::npos )
    {
       if( testEquality(static_cast<T>(val), fillingValue ) )
         continue;
       goto ERROR;
    }
    if( str[0] == '#' || str.substr(0,2) == "//" )
    {
       setInfo(str);
       continue;
    }

    // Note: The ensemble count starts the data section.
    if( str.find("Number of ensembles=") < std::string::npos )
       return false;

  } while ( ! file.readLine() ) ;

  return true;

ERROR:
  std::ostringstream ostr(std::ios::app);
  ostr << "FreqDist::readBuild()";
  ostr << "\nproperties of build files differ";

  exceptionError( ostr.str() ); // exits

  return true ;  // never reach
}

template<typename T>
void
FreqDist<T>::reInstall(std::string rFile)
{
  // read properties of a freqDist histogram
  ReadLine file;

  if( file.open(rFile.c_str()) )
    return;

  reInstall( file ) ;

  file.close();

  return ;
}

template<typename T>
void
FreqDist<T>::reInstall( ReadLine &file)
{
  // read properties of a freqDist histogram

  if( file.readLine() )
    return ;  // file is empty

  std::string str;
  double val;

  do
  {
    // properties form a header of a data file
    str=file.getLine();

    // Start of the data section with initialisations of the freqDist
    if( str.find("Number of ensembles=") < std::string::npos )
    {
      if( isReadOnlyProperties )
        return;  // ignore the rest

      ensembleCount = hdhC::string2Double(str,1) ;
      file.readLine();

      int numClasses=static_cast<int>(file.getValue(0));

      clear(); // do not use reset

      // -infinity
      binBorder.push_back( -MAXDOUBLE );
      binVal.push_back( 0. );
      binFreq.push_back( 0. );
      binCount.push_back( 0. );

      for( int i=0 ; i <  numClasses; ++i )
      {
        file.readLine();
        binBorder.push_back(file.getValue(0));
        binFreq.push_back(file.getValue(2));
        binCount.push_back(file.getValue(3));
        binVal.push_back(file.getValue(4));
      }

      // +infinity
      binBorder.push_back( MAXDOUBLE );
      binVal.push_back( 0. );
      binFreq.push_back( 0. );
      binCount.push_back( 0. );

      return;
    }

    if( str.find("Symmetry about value=") < std::string::npos )
    {
       isCentric = str[str.size()-1] == 'T' ? true : false;
       continue;
    }
    if( str.find("Class width=") < std::string::npos )
    {
       binWidth = hdhC::string2Double(str,1) ;
       continue;
    }
    if( str.find("Range minimum=") < std::string::npos )
    {
       val=hdhC::string2Double(str,1);
       rangeMin = static_cast<T>(val) ;
       continue;
    }
    if( str.find("Range maximum=") < std::string::npos )
    {
       val=hdhC::string2Double(str,1);
       rangeMax = static_cast<T>(val) ;
       continue;
    }
    if( str.find("Filling value=") < std::string::npos )
    {
       // do not overwrite a mV already set
       if( ! isFillingValueEnabled )
       {
         val=hdhC::string2Double(str,1);
         fillingValue = static_cast<T>(val) ;
         isFillingValueEnabled=true;
       }
       continue;
    }
    if( str.find("Number of filling values=") < std::string::npos )
    {
       val=hdhC::string2Double(str,1);
       numFV = static_cast<size_t>(val) ;
       continue;
    }
    if( str[0] == '#' || str.substr(0,2) == "//" )
       setInfo(str);

  } while ( ! file.readLine() ) ;

  return ;
}

template<typename T>
void
FreqDist<T>::reset(void)
{
  clear();
  ensembleCount=0.;
  numFV=0;

  binBorder.push_back( -MAXDOUBLE );
  binVal.push_back( 0. );
  binFreq.push_back( 0. );
  binCount.push_back( 0. );

  binBorder.push_back( MAXDOUBLE );
  binVal.push_back( 0. );
  binFreq.push_back( 0. );
  binCount.push_back( 0. );

  lastUsedIndex=0;

  return;
}

template<typename T>
void
FreqDist<T>::resize(void)
{
  // adjust class properties, if the number of classes
  // exceeds given threshold (default: 50).
  std::vector<double> tmpBorder( binBorder );
  std::vector<double> tmpVal( binVal );
  std::vector<double> tmpFreq( binFreq );
  std::vector<double> tmpCount( binCount );

  // clears all vectors
  // and set +/- infinity boundaries

  double saveEC=ensembleCount;
  size_t save_numFV=numFV;

  reset();

  ensembleCount=saveEC ;
  numFV=save_numFV ;

  // re-init
  if( binWidth < .1 )
    binWidth *= 10.;  // going up the powers
  else if( binWidth < .5 )
    binWidth=.5;
  else
    binWidth *= automResizeFactor ;

  // feed the temporary stored distribution. Pay attention
  // to the two infinity classes at both ends of the vectors.
  size_t sz=0;
  if( tmpFreq.size() > 0 )
    sz = tmpFreq.size() -1 ;

  for( size_t i=1 ; i < sz ; ++i)
  {
    if( tmpCount[i] == 0. )
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "FreqDist::resize()";
      ostr << "\nunexpected zero-count for index=" << i ;

      exceptionError( ostr.str() ); // exits
    }

    // Build new freq dist the usual way. But we only want to
    // determine the borders and the corresponding index, so
    // we have effectiveley a merging case.
    push_val( tmpVal[i] / tmpCount[i] , tmpFreq[i],
                  tmpCount[i], tmpVal[i] );
  }

  return;
}

template<typename T>
void
FreqDist<T>::saveBuild( std::string sFile )
{
  // save an ASCII freqDist histogram
  // in the build-format:
  //     bound_0, bound_1, freq, count, value

  // sFile: the name of the file for saving. If empty,
  //   then: use internal name for the output.
  //   else: std:: cout

  if( binFreq.size() == 2 )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "FreqDist::saveBuild()";
    ostr << "\nno freqDist to save. Sorry";

    exceptionError( ostr.str() ); // exits
  }

  // if not provided, test for a previously set filename
  std::string myFile;
  std::streambuf* cout_sbuf = 0;
  bool isStdOut = false;

  if( sFile.size() > 0 )
    myFile=sFile;
  else if ( outputFilenameBase.size() > 0 )
    myFile=outputFilenameBase + ".build";
  else
    isStdOut = true;

  // open file for saving all and redirect to std::cout
  std::ofstream *out = 0;
  if( ! isStdOut )
  {
    out = new std::ofstream() ;
    out->open(myFile.c_str(), std::ios::out);
    cout_sbuf = std::cout.rdbuf() ;  // save original buffer
    std::cout.rdbuf(out->rdbuf()); // connect out with std::cout
  }

  // save the properties of the freqDist, but only if no file exists
  saveProperties();

  std::cout << std::endl;  // of no importance

  // Save the number of ensembles. First line of the data section.
  std::cout << "Number of ensembles=" << std::setw(16)<< std::setprecision(9)
            << ensembleCount << std::endl;

  // save size of of classes (reduced by 2 due to infinity classes)
  std::cout << (binFreq.size()-2) << "\n" ;

  // save all. index 0 and size()-1 indicate infinity
  if( binFreq.size() > 0 )
  {
    for( size_t i=1 ; i < binFreq.size() -1 ; ++i )
    {
       std::cout
         << std::setw(16) << std::setprecision(9) << binBorder[i] << " "
         << std::setw(16) << std::setprecision(9)
         << (binBorder[i] + binWidth) << " "
         << std::setw(22) << std::setprecision(15) << binFreq[i] << " "
         << std::setw(22) << std::setprecision(15) << binCount[i] << " "
         << std::setw(22) << std::setprecision(15) << binVal[i]
         << std::endl ;
    }
  }

  if( ! isStdOut )
  {
    std::cout.rdbuf(cout_sbuf);  // restore original stream buffer

    delete out;
  }

  return;
}

template<typename T>
void
FreqDist<T>::saveProperties(std::string sFile )
{
  // save the layout of a freqDist histogram

  // open file for saving the configuration
  std::ofstream *file=0;
  std::streambuf* cout_sbuf=0;

  // by default, this method  writes to std::cout, may-be
  // already connected to a file.

  if( sFile.size() > 0 )
  {
    file = new std::ofstream() ;
    file->open(sFile.c_str(), std::ios::out);
    cout_sbuf = std::cout.rdbuf() ;  // save original buffer
    std::cout.rdbuf(file->rdbuf()); // connect out with std::cout
  }

  std::cout.setf( std::ios::scientific, std::ios::floatfield);

  // At first, save properties. Remember: binBorder: index ==0
  // and index == size()-1  indicate infinity
  std::cout << "Symmetry about value="
            << static_cast<char>(isCentric ? 'T' : 'F') << std::endl;
  std::cout << "Class width=" << std::setw(22)<< std::setprecision(15)
            << binWidth << std::endl;
  std::cout << "Range minimum=" << std::setw(22)<< std::setprecision(15)
            << binBorder[1] << std::endl;
  std::cout << "Range maximum=" << std::setw(22)<< std::setprecision(15)
            << (binBorder[ binBorder.size()-2]+binWidth) << std::endl;
  if( isFillingValueEnabled )
  {
    std::cout << "Filling value=" << std::setw(15)
              << std::setprecision(5) << fillingValue << std::endl;
    std::cout << "Number of filling values="
              << numFV << std::endl;
  }

  for( size_t j=0 ; j < infoLine.size() ; ++j )
    std::cout << infoLine[j] << std::endl;

  if( sFile.size() > 0 )
  {
    std::cout.rdbuf(cout_sbuf);  // restore original stream buffer

    delete file;
  }

  return;
}

template<typename T>
void
FreqDist<T>::setInfo(std::string str)
{
   // add some info to the header of a save-file
   // make sure that the string is not contained, yet
   bool is=true ;
   for( size_t j=0 ; j < infoLine.size() ; ++j )
   {
      if( str == infoLine[j] )
        is=false;
   }

   if( is )
   {
     if( str[0] != '#' && str.substr(0,2) != "//" )
       str.insert(0,"#");

     infoLine.push_back( str );
   }
  return;
}

template<typename T>
void
FreqDist<T>::setOutputFilename(std::string f)
{
  if( f.size() > 6 )
  {
    if( f.substr(f.size()-5) == ".hist" )
      f=f.substr(0, f.size()-5);
    else if( f.substr(f.size()-5) == ".Hist" )
      f=f.substr(0, f.size()-5);
    else if( f.substr(f.size()-6) == ".build" )
      f=f.substr(0, f.size()-6);
    else if( f.substr(f.size()-5) == ".prop" )
      f=f.substr(0, f.size()-5);
  }

  outputFilenameBase=f;
  return;
}

template<typename T>
void
FreqDist<T>::setProperties(double dx, const bool isCent)
{
  isCentric=isCent;
  binWidth=dx;

  return;
}

template<typename T>
void
FreqDist<T>::setProperties(double dx, T val, const bool isCent)
{
  isCentric=isCent;
  binWidth=dx;

  install(val) ;
  return;
}

//inline
template<typename T>
template<typename TE>
bool
FreqDist<T>::testEquality(TE val, TE comp)
{
   // return true, if val is in +- epsilon around comp
   TE resid = (val < comp) ? comp-val : val-comp ;
   return resid < NUMERICAL_TOLERANCE ? true : false ;
}

template<typename T>
double
FreqDist<T>::updateBinWidth(T val)
{
     // Return the next border of a new bin.
     // Designed for centred bins.
     //  Non-centred is taken into account in install().
     double shift=0.;

     if( val < 0. )
       shift=-1.*static_cast<double>( static_cast<long>(val) )+1. ;

     double vLeft = (val + shift ) / binWidth ;
     long j=static_cast<long>( vLeft) ;

     if( isAutomResizeEnabled && j < 0L )
     {
       if( binWidth < .1 )
         binWidth *= 10.;  // going up the powers
       else if( binWidth < .5 )
         binWidth=.5;
       else
         binWidth *= automResizeFactor ;

       vLeft=updateBinWidth(val);  // recursively
     }
     else
       vLeft=static_cast<double>(j) * binWidth - shift ;

     return vLeft ;
}
