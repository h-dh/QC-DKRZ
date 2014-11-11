#include "statistics.h"

template <typename T>
Statistics<T>::Statistics()
{
  init();
}

template <typename T>
Statistics<T>::Statistics(const Statistics<T> &s)
{
  init();
  copy(s);
}

template <typename T>
Statistics<T>::Statistics(size_t d)
{
  init();
}

template <typename T>
Statistics<T>::~Statistics()
{
  if( xcptn.ofsError )
    delete xcptn.ofsError ;
}


template <typename T>
Statistics<T>&
Statistics<T>::operator=(const Statistics<T> &s)
{
  if( this == &s )
    return *this ;

  copy(s);

  return *this ;
}

template <typename T>
void
Statistics<T>::add(T val)
{
  if( isEnableFillingValue && val == exceptionValue[0] )
  {
    ++exceptionCount[0];
    return;
  }

  // take into account constant values
  double dVal = static_cast<double>(val);

  sampleSum += dVal ;
  sampleSumOfSquares += dVal * dVal ;
  ++sampleSize ;

  if( dVal < sampleMin )
     sampleMin = dVal;
  if( dVal > sampleMax )
     sampleMax = dVal;

  return;
}

/*
template <typename T>
void
Statistics<T>::add(T val_1, T val_2)
{
  // specialisation: 2 dim
  if( dim != 2 )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "Statistics::add()"
         << " dim=2 required, but dim = " << dim ;
    exceptionError( ostr.str() );
    exit(1); 
  }
    
  T val[2];
  val[0]=val_1;
  val[1]=val_2;

  for( size_t idx=0 ; idx < dim ; ++idx)
  {
    T *x = &val;
   
    if( isEnableFillingValue && testFillingValue(*x, idx) )
    {
      ++exceptionCount;
      return;
    }

    double dVal=static_cast<double>( *x );

    sampleSum += dVal ;
    sampleSumOfSquares += dVal * dVal ;
    ++sampleSize ;

    if( dVal < sampleMin )
       sampleMin = dVal;
    if( dVal > sampleMax )
       sampleMax = dVal ;
  }
    
  multiSumCrossProduct[0] += val_1 * val_2  ;
  
  return;
}
*/

template <typename T>
void
Statistics<T>::add(T const *v, size_t count)
{
  // test for filling value in called method
  for( size_t i=0 ; i < count ; i++ )
  {
     // take into account constant values
     double dVal = static_cast<double>(v[i]);

     sampleSum += dVal ;
     sampleSumOfSquares += dVal * dVal ;
     ++sampleSize ;

     if( dVal < sampleMin )
        sampleMin = dVal;
     if( dVal > sampleMax )
        sampleMax = dVal;
   }

  return;
}

template <typename T>
void
Statistics<T>::add(T const **val, size_t nRow, size_t nCol )
{
  // test for filling value in called method 
  for( size_t k=0 ; k < nRow ; k++ )
    add(val[k], nCol);
  
  return;
}

template <typename T>
void
Statistics<T>::add(const std::vector<T>& val )
{
  size_t sz=val.size();

  // test for filling value in called method 
  for( size_t k=0 ; k < sz ; k++ )
    add(val[k]) ;

  return;
}

template <typename T>
void
Statistics<T>::add(MtrxArr<T> &v)
{
  if( checkMA_FillValue )
  {
     // init value exception testing
     for( size_t i=0 ; i < v.valExp->exceptionValue.size() ; ++i )
       exceptionValue.push_back( v.valExp->exceptionValue[i] ) ;

     //note: get counts only one by one
     exceptionCount.clear(); // was set in init()

     for( size_t i=0 ; i < exceptionValue.size() ; ++i )
       exceptionCount.push_back( v.getExceptionCount(i) ) ;
     countInf = v.getExceptionCount("inf");
     countNaN = v.getExceptionCount("nan");

     checkMA_FillValue=false;
  }
  else
  {
     for(size_t i=0 ; i < exceptionValue.size() ; ++i)
        exceptionCount[i] += v.getExceptionCount(i) ;

     countInf = v.getExceptionCount("inf");
     countNaN = v.getExceptionCount("nan");
  }

  size_t lim=v.validRangeBegin.size();
  for( size_t k=0 ; k < lim ; k++ )
  {
     for(size_t i=v.validRangeBegin[k] ; i < v.validRangeEnd[k] ; ++i)
     {
       // take into account constant values
       double dVal = static_cast<double>(v[i]);

       sampleSum += dVal ;
       sampleSumOfSquares += dVal * dVal ;
       ++sampleSize ;

       if( dVal < sampleMin )
          sampleMin = dVal;
       if( dVal > sampleMax )
          sampleMax = dVal;
     }
  }

  return;
}

template <typename T>
template <typename W>
void
Statistics<T>::addWeighted(T val, W weight)
{
  if( weight == 0 )
  {
    ++exceptionCount[0];
    return;
  }
  else
  {
    for( size_t i=0 ; i < exceptionValue.size() ; ++i )
    {
       if( val == exceptionValue[i] )
       {
         ++exceptionCount[0];
         break;
       }
    }

    return;
  }

  double x = val;
  x *= weight;
   
  ++sampleSize ;
  sampleSum += x ;
  sampleSumOfSquares += x * val ;
  sampleTotalWeight += weight;
  sampleTotalSquareWeight += weight*weight;  

  double dVal = static_cast<double>( val );

  if( dVal < sampleMin )
     sampleMin = dVal;
  if( dVal > sampleMax )
     sampleMax = dVal;

  return;
}

template <typename T>
template <typename W>
void
Statistics<T>::addWeighted(size_t sz, const T *v, const W *w)
{
  // test for filling value in called method 
  for( size_t k=0 ; k < sz ; k++ )
    addWeighted(v[k], w[k]);
  
  return;
}

template <typename T>
template <typename W>
void
Statistics<T>::addWeighted(MtrxArr<T> &v, MtrxArr<W> &w)
{
  if( checkMA_FillValue )
  {
     // init value exception testing
     for( size_t i=0 ; i < v.valExp->exceptionValue.size() ; ++i )
       exceptionValue.push_back( v.valExp->exceptionValue[i] ) ;

     //note: get counts only one by one
     exceptionCount.clear(); // was set in init()

     for( size_t i=0 ; i < exceptionValue.size() ; ++i )
       exceptionCount.push_back( v.getExceptionCount(i) ) ;
     countInf = v.getExceptionCount("inf");
     countNaN = v.getExceptionCount("nan");

     checkMA_FillValue=false;
  }
  else
  {
     for(size_t i=0 ; i < exceptionValue.size() ; ++i)
        exceptionCount[i] += v.getExceptionCount(i) ;

     countInf = v.getExceptionCount("inf");
     countNaN = v.getExceptionCount("nan");
  }

  for( size_t k=0 ; k < v.validRangeBegin.size() ; k++ )
  {
     for(size_t i=v.validRangeBegin[k] ; i < v.validRangeEnd[k] ; ++i)
     {
       double val = static_cast<double>( v[i] );
       double weight = static_cast<double>( w[i] );

       double x = val * weight;
   
       ++sampleSize ;
       sampleSum += x ;
       sampleSumOfSquares += x * val ;
       sampleTotalWeight += weight;
       sampleTotalSquareWeight += weight*weight;  

       if( val < sampleMin )
          sampleMin = val;
       if( val > sampleMax )
          sampleMax = val;
     }
  }

  return;
}

template <typename T>
template <typename W>
void
Statistics<T>::addWeighted(const std::vector<T>& v,const std::vector<W>& w)
{
  size_t sz=v.size();

  // test for filling value in called method 
  for( size_t k=0 ; k < sz ; k++ )
    addWeighted(v[k], w[k]);
  
  return;
}

template <typename T>
void
Statistics<T>::clear(void)
{
  sampleSize=0 ;
  sampleAverage=0. ;
  sampleStdDev=0. ;
  sampleStdErr=0. ;
  sampleVariance=0. ;
  sampleSumOfSquares=0. ;
  sampleSum=0. ;
  
  initMinMaxTest( static_cast<T> (0));
  
  sampleTotalWeight=0.;
  sampleTotalSquareWeight=0.;

  for( size_t i=0 ; i < exceptionCount.size() ; ++i)
    exceptionCount[i]=0;

  countInf=0;
  countNaN=0;

  ensembleCount=0;
  checkMA_FillValue=true;

  return ;
}

template <typename T>
void
Statistics<T>::copy( const Statistics<T> &s)
{
  EQUALITY_TOLERANCE=s.EQUALITY_TOLERANCE ;

  for( size_t i=0 ; i < s.exceptionValue.size() ; ++i )
    exceptionValue.push_back(s.exceptionValue[i]);
  for( size_t i=0 ; i < s.exceptionCount.size() ; ++i )
    exceptionCount.push_back(s.exceptionCount[i]);

  sampleAverage = s.sampleAverage ;
  sampleMax = s.sampleMax ;
  sampleMin = s.sampleMin ;
  sampleVariance = s.sampleVariance ;
  sampleSize = s.sampleSize ;
  sampleStdDev = s.sampleStdDev ;
  sampleStdErr = s.sampleStdErr ;
  sampleSumOfSquares = s.sampleSumOfSquares ;
  sampleSum = s.sampleSum ;

  sampleTotalWeight=s.sampleTotalWeight;
  sampleTotalSquareWeight=s.sampleTotalSquareWeight;

  isEnableFillingValue = s.isEnableFillingValue ;

  countInf = s.countInf;
  countNaN = s.countNaN;

  ensembleCount = s.ensembleCount;
  return;
}

template <typename T>
void
Statistics<T>::enableFillingValueTest( bool b)
{
  // dimension of exceptionValue is already right by constructor
  isEnableFillingValue=b;

  if( exceptionCount.size() == 0 )
    exceptionCount.push_back(0);
  else
  {
    for( size_t i=0 ; i < exceptionCount.size() ; ++i)
      exceptionCount[i]=0;
  }

  return;
}

template <typename T>
void
Statistics<T>::exceptionError(std::string str)
{
  // Occurrence of an error stops the run immediately.
  xcptn.strError = "error_STATISTICS.txt" ;

  // open file for appending data
  if( ! xcptn.ofsError )
    xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

  *xcptn.ofsError << str << std::endl;

  return ;
}

template <typename T>
bool
Statistics<T>::getFillingValue(T *v)
{
  if( exceptionValue.size() )
  {
    *v = exceptionValue[0];
    return true;
  }

  return false;
}

template <typename T>
bool
Statistics<T>::getSampleAverage( double *a )
{
  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  double sTW=sampleTotalWeight ;

  if( sampleTotalWeight == 0. )
    sTW =  static_cast<double>(sampleSize) ;
  
  *a = sampleAverage = sampleSum / sTW ;
    
  return true;
}

template <typename T>
size_t
Statistics<T>::getSampleFillingValueNum( std::string s, int ix )
{
  // ix: only for user defined exceptions, i.e. 0 from the start.
  //     -1 by default, summing all counts for user defined vals.

  if( s == "all" )
  {
    size_t count=0;
    for( size_t i=0 ; i < exceptionCount.size() ; ++i )
      count += exceptionCount[i];
    count += countInf;
    count += countNaN;

    return count;
  }

  if( s == "inf" )
    return countInf ;

  if( s == "nan" )
    return countNaN ;

  if( s == "user" )
  {
    size_t count=0;
    for( size_t i=0 ; i < exceptionCount.size() ; ++i )
        count += exceptionCount[i];
    return count;
  }

  return 0;  // xlc expects here a return
}


template <typename T>
bool
Statistics<T>::getSampleMax( T *a )
{
  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = static_cast<T>(sampleMax);
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleMin( T *a )
{
  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = static_cast<T>(sampleMin);
  return true;
}

template <typename T>
std::string
Statistics<T>::getSampleProperties(void)
{
  std::string s;

  s  ="sampleMin=" ;
  s += hdhC::double2String(
                   static_cast<double>(sampleMin) );

  s +=", sampleMax=" ;
  s += hdhC::double2String(
                   static_cast<double>(sampleMax) );

  s +=", sampleSize=" ;
  s += hdhC::double2String(
                   static_cast<double>(sampleSize) );

  s += ", sampleSum=" ;
  s += hdhC::double2String(
                   static_cast<double>(sampleSum ) );

  s += ", sampleSumOfSquares=" ;
  s += hdhC::double2String(
                   static_cast<double>(sampleSumOfSquares) );

  T zero = 0;
  if( sampleTotalWeight > zero )
  {
    s += ", sampleTotalWeight=" ;
    s += hdhC::double2String(
                   static_cast<double>(sampleTotalWeight ) );

    s += ", sampleTotalSquareWeight=" ;
    s += hdhC::double2String(
                   static_cast<double>(sampleTotalSquareWeight) );
  }

  return s;
}

template <typename T>
bool
Statistics<T>::getSampleSize( size_t *a )
{
  if( sampleSize == 0L )
    return false;

  *a = sampleSize ;
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleStdDev( double *s )
{
  double v;
  if( ! getSampleVariance( &v ) )
  {
    if( exceptionValue.size() )
      *s = exceptionValue[0] ;
    return false;
  }
    
  *s = sampleStdDev=sqrt(v);
  
  return true ;
}

template <typename T>
bool
Statistics<T>::getSampleStdErr( double *e )
{
  double v;
  
  if( ! getSampleVariance( &v ) )
  {
    if( exceptionValue.size() )
      *e = exceptionValue[0] ;
    return false;
  }
    
  sampleStdErr = v / sqrt(static_cast<double>(sampleSize) ) ;
      
  return true ;
}

template <typename T>
bool
Statistics<T>::getSampleSum( double *a )
{

  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = sampleSum;
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleSumOfSquares( double *a )
{

  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = sampleSumOfSquares;
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleTotalSquareWeight( double *a )
{

  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = sampleTotalSquareWeight;
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleTotalWeight( double *a )
{

  if( sampleSize == 0L )
  {
    if( exceptionValue.size() )
      *a = exceptionValue[0] ;
    return false;
  }

  *a = sampleTotalWeight;
  return true;
}

template <typename T>
bool
Statistics<T>::getSampleVariance( double *v )
{
  if ( sampleTotalWeight == 0. )
    return getSampleVarianceEqual( v );
  else
    return getSampleVarianceWeight( v );
}

template <typename T>
bool
Statistics<T>::getSampleVarianceEqual( double *v )
{
  if( sampleSize < 2L )
  {
    if( exceptionValue.size() )
      *v = exceptionValue[0] ;
    return false;
  }
  
  double x = static_cast<double>(sampleSize) ;

  sampleVariance = 
         (sampleSumOfSquares - sampleSum*sampleSum/x)/(x-1.) ;
  
  
  if( sampleVariance < 0. && fabs(sampleVariance) < EQUALITY_TOLERANCE )
    sampleVariance = 0.;  // tolerates numerical noise
  
  *v = sampleVariance ;

  return true ;
}

template <typename T>
bool
Statistics<T>::getSampleVarianceWeight( double *v )
{
  if( sampleSize == 1 )
    return false;

  sampleVariance = 
         ( sampleSumOfSquares * sampleTotalWeight
          - sampleSum*sampleSum )
         / (sampleTotalWeight*sampleTotalWeight -sampleTotalSquareWeight); 
    
  if( sampleVariance < 0. && fabs(sampleVariance) < EQUALITY_TOLERANCE )
    sampleVariance = 0.;  // tolerates numerical noise
  
  *v = sampleVariance ;

  return true ;
}

template <typename T>
void
Statistics<T>::init(void)
{
  xcptn.ofsError=0;

  EQUALITY_TOLERANCE=1.E-10;
  clear();

  checkMA_FillValue=true;

  isEnableFillingValue = false ;
  exceptionCount.push_back( 0 );
    
  return ;
}

template <typename T>
void
Statistics<T>::initMinMaxTest( T x)
{
  std::string s( typeid(x).name() );

  if( s[0] == 'f' )
  {
     sampleMin=MAXFLOAT;
     sampleMax=-MAXFLOAT;
  }
  else if( s[0] == 'd' )
  {
     sampleMin=MAXDOUBLE;
     sampleMax=-MAXDOUBLE;
  }
  else if( s[0] == 'i' )
  {
     sampleMin=INT_MAX;
     sampleMax=INT_MIN;
  }
  else if( s[0] == 'a' )
  {
     sampleMin=SCHAR_MAX;
     sampleMax=-SCHAR_MAX;
  }
  else if( s[0] == 'h' )
  {
     sampleMin=UCHAR_MAX;
     sampleMax=0;
  }
  else if( s[0] == 's' )
  {
     sampleMin=SHRT_MAX;
     sampleMax=SHRT_MIN;
  }
  else if( s[0] == 't' )
  {
     sampleMin=USHRT_MAX;
     sampleMax=0;
  }
  else if( s[0] == 'j' )
  {
     sampleMin=UINT_MAX;
     sampleMax=0;
  }
  else if( s[0] == 'y' )
  {
     sampleMin=UINT64_MAX;
     sampleMax=0;
  }
  else if( s[0] == 'x' )
  {
     sampleMin=INT64_MAX;
     sampleMax=INT64_MIN;
  }
 
  return;
}


template <typename T>
void
Statistics<T>::setFillingValue( T *v, size_t sz)
{
  // dimension of exceptionValue is already adjusted at construction
  exceptionValue.clear();
  isEnableFillingValue=true;

  for( size_t i=0 ; i < sz ; ++i)
  {
    exceptionValue.push_back(v[i]);
  }
  return;
}

template <typename T>
void
Statistics<T>::setFillingValue(T v)
{
  isEnableFillingValue=true;
  exceptionValue.push_back(v);
  return;
}

template <typename T>
void
Statistics<T>::setSampleProperties(std::string s)
{
  Split splt( s, "," );
  Split pair;
  pair.setSeparator('=');

  std::string s0;

  for( size_t i=0 ; i < splt.size() ; ++i )
  {
    pair = splt[i] ;

    s0 = hdhC::stripSurrounding(pair[0] );

    if( s0 == "sampleMax" )
      sampleMax = static_cast<T>( pair.toDouble(1) );

    else if( s0 == "sampleMin" )
      sampleMin = static_cast<T>( pair.toDouble(1) );

    else if( s0 == "sampleSize" )
      sampleSize = static_cast<long>( pair.toDouble(1) );

    else if( s0 == "sampleSum" )
      sampleSum = static_cast<T>( pair.toDouble(1) );

    else if( s0 == "sampleSumOfSquares" )
      sampleSumOfSquares = static_cast<T>(  pair.toDouble(1) );

    else if( s0 == "sampleTotalWeight" )
      sampleTotalWeight = static_cast<T>( pair.toDouble(1) );

    else if( s0 == "sampleTotalSquareWeight" )
      sampleTotalSquareWeight = static_cast<T>(  pair.toDouble(1) );
  }

  return;
}

template <typename T>
bool
Statistics<T>::testFillingValue(T v)
{
  if( fabs( v - exceptionValue[0] ) < EQUALITY_TOLERANCE )
    return true ;
  else
    return false ;
}
