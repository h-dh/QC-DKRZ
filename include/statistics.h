#ifndef _STATISTICS_H
#define _STATISTICS_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <time.h>
#include <values.h>

#include "matrix_array.h"

#ifndef UINT64_MAX
#  define UINT64_MAX ULONG_LONG_MAX
#endif
#ifndef INT64_MAX
#  define INT64_MAX LONG_LONG_MAX
#endif
#ifndef INT64_MIN
#  define INT64_MIN LONG_LONG_MIN
#endif

/*! \file statistics.h
 \brief Conduction of simple statistics.
*/

//! Simple statistics.
/*! Only min- and maximum, average (weighted or not)
and standard deviation are calculated. Input values matching
a specified filling value are ignored.*/

template <typename T>
class Statistics
{
public:

  //! Deprecated error/warning procedure.
  /*! Structure for error messaging. Obsolete by the Annotation procedure.*/
  struct ExceptionStruct
  {
     std::ofstream *ofsError;
     std::string strError;
  } ;

  Statistics();
  Statistics(size_t);
  Statistics(const Statistics<T>&);
  ~Statistics();

  Statistics<T>& operator=(const Statistics<T>&);
  
//  void   addMultiDim(std::vector<const double>);

//! Add unweighted values to the statistics.
  void   add(T);
  void   add(T, T); //2-dim spec.
  void   add(T const*, size_t);
  void   add(T const**, size_t, size_t);
  void   add(MtrxArr<T>&);
  void   add(const std::vector<T>&);

//! Increase the number of ensembles in the statistics.
/*! Used for normalisation of the total weight of the statistics.*/
  void   addEnsembleCount( size_t c=1)
             { ensembleCount += c;}

//! Add values and corresponding weights to the statistics.
template <typename W>
  void   addWeighted(T, W weight);
template <typename W>
  void   addWeighted(size_t, const T*, const W*);
template <typename W>
  void   addWeighted(MtrxArr<T> &val, MtrxArr<W> &weight);
template <typename W>
  void   addWeighted(const std::vector<T>&, const std::vector<W>&);
  void   clear( void ) ;
  void   copy(const Statistics&);

//! Dis- and enable filling value test.
/*! If enabled by mistake, then average and standard deviation are wrong, too.*/
  void   disableFillingValueTest(void)
             { isEnableFillingValue=false;}
  void   enableFillingValueTest( T fV )
             { setFillingValue(fV);}
  void   enableFillingValueTest( bool);
  bool   getFillingValue( T* );
  bool   getSampleAverage( double*) ;
  size_t getSampleEnsembleCount(void)
           { return ensembleCount == 0 ? 1 : ensembleCount; }
//! get counts for user defined exceptions as well as inf and nan
/*! string: all, inf, nan, user. ix: only for user defined exceptions, i.e. 0 from the start; -1 by default, summing all counts for user defined vals.*/
  size_t getSampleFillingValueNum( std::string, int ix=-1);
  bool   getSampleMax( T*);
  T      getSampleMax(void)
           { return static_cast<T>(sampleMax) ; }
  bool   getSampleMin( T*);
  T      getSampleMin(void)
           { return static_cast<T>(sampleMin) ; }
  bool   getSampleSize( size_t *);
  size_t getSampleSize(void)
           { return sampleSize; }
  bool   getSampleStdDev( double* ) ;
  bool   getSampleStdErr( double* ) ;
  bool   getSampleSum( double* );
  double getSampleSum(void){ return sampleSum ;}
  bool   getSampleSumOfSquares( double* );
  double getSampleSumOfSquares(void)
            { return sampleSumOfSquares ;}
  bool   getSampleTotalSquareWeight( double* );
  double getSampleTotalSquareWeight(void)
            { return sampleTotalSquareWeight ;}
  bool   getSampleTotalWeight( double* );
  double getSampleTotalWeight(void)
            { return sampleTotalWeight ;}
  bool   getSampleVariance( double* ) ;

  std::string
         getSampleProperties(void);
  void   setSampleProperties(std::string);
  void   setEqualityTolerance( double t) { EQUALITY_TOLERANCE = t ;}

//! Set values to be ecxluded from the statistics.
/*! The number of occurrences of FillingValue is counted.*/
  void   setFillingValue( T *e, size_t sz=1);
  void   setFillingValue( T e);

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;

  size_t              ensembleCount;
  std::vector<size_t> exceptionCount;
  size_t              countInf;
  size_t              countNaN;

private:
  double EQUALITY_TOLERANCE ;
  
  std::vector<T>      exceptionValue;
  double  sampleAverage;
  double  sampleMax;
  double  sampleMin;
  double  sampleStdDev;
  double  sampleStdErr;
  double  sampleVariance;
  double  sampleSumOfSquares;
  double  sampleSum;
  double  multiSumCrossProduct;
  long    sampleSize;

  bool                checkMA_FillValue;

  double sampleTotalWeight;
  double sampleTotalSquareWeight;

  void   exceptionError(std::string);
  bool   getSampleVarianceEqual( double* ) ;
  bool   getSampleVarianceWeight( double* ) ;
  void   init(void);
  void   initMinMaxTest(T x);
  bool   isEnableFillingValue;
  bool   testFillingValue(T);
};

#endif
