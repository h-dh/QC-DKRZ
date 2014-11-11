#ifndef _MATRIX_ARRAY_H
#define _MATRIX_ARRAY_H

#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>

#include "hdhC.h"

/*! \file matrix_array.h
 \brief Interface to memory data: array <--> matrix format.
*/

//! Handling of data exceptions within the MtrxArr class.
/*! User provided exception value(s), a.k.a. _FillValue, Inf and NaN.*/
template<typename T>
class ValueException
{
public:
  ValueException();
  ~ValueException();
  void clearExceptionValue(void);
  void copy( const ValueException<T> &g, bool clearCounts=false);
  void disableValueExceptionTest(std::string s);
  void enableValueExceptionTest(std::string s);
  void enableExceptionValue(T *argv, size_t argc, std::string s="");

template<typename T_in>
  void import( const ValueException<T_in> &g);

  void isInfNaN(bool *is, size_t isSz, size_t isBeg, T* arr, size_t arr_sz);
  void isInf(bool *is, size_t isBeg, size_t isEnd, T* arr, size_t arr_sz);
  void isNaN(bool *is, size_t isBeg, size_t isEnd, T* arr, size_t arr_sz);
  void resetExceptionCounts(void);
  void setExceptionValue(T *argv, size_t argc, std::string &s);
  void testValueException( T* arr, size_t arr_sz,
         std::vector<size_t> &validRangeBegin ,
         std::vector<size_t> &validRangeEnd );
  void updateValueException(bool b)
          { update = b; }

  // telling events. Note: inf and NaN are counted separately.
  std::vector<T> exceptionValue;  // user provided exception values
  std::vector<size_t> exceptionCount;
  size_t countInf;
  size_t countNaN;

  // permitted values: 'inf', 'NaN', or 'user'
  std::map<std::string, bool> isValueExceptionTest;

  bool   update;
private:      // prevent copying
  ValueException(const ValueException&);
  ValueException& operator=(const ValueException&);
};

template<typename T>
ValueException<T>::ValueException()
  {
    // init value exceptions
    isValueExceptionTest["inf"]=false;
    isValueExceptionTest["nan"]=false;
    isValueExceptionTest["user"]=false;
    isValueExceptionTest["none"]=true;

    update = true;
  }

template<typename T>
ValueException<T>::~ValueException()
{
    clearExceptionValue();
}

template<typename T>
void
ValueException<T>::clearExceptionValue(void)
{
    exceptionValue.clear();
    exceptionCount.clear();

    return;
}

template<typename T>
void
ValueException<T>::copy( const ValueException<T> &g, bool is)
{
    exceptionValue = g.exceptionValue ;
    exceptionCount = g.exceptionCount ;
    countInf  = g.countInf;
    countNaN  = g.countNaN;

    if( is )
    {
      for( size_t i=0 ; i < exceptionCount.size() ; ++i )
        exceptionCount[i] = 0 ;
      countInf  = 0;
      countNaN  = 0;
    }

    return;
}

template<typename T>
void
ValueException<T>::disableValueExceptionTest(std::string s)
{
    hdhC::Lower()(s);

    size_t c=0;
    if( s.find("inf") < std::string::npos )
    {
      isValueExceptionTest["inf"]=false;
      ++c;
    }
    if( s.find("nan") < std::string::npos )
    {
      isValueExceptionTest["nan"]=false;
      ++c;
    }
    if( s.find("user") < std::string::npos )
    {
      isValueExceptionTest["user"]=false;
      ++c;
    }

    if( c == 3 )
      isValueExceptionTest["none"]=true;
    else
      isValueExceptionTest["none"]=false;

    return;
}

template<typename T>
void
ValueException<T>::enableValueExceptionTest(std::string s)
{
    if( s.size() )
      hdhC::Lower()(s);
    else
      s="inf nan user"; //enable all

    if( s.find("inf") < std::string::npos )
    {
      isValueExceptionTest["inf"]=true;
      isValueExceptionTest["none"]=false;
    }
    if( s.find("nan") < std::string::npos )
    {
      isValueExceptionTest["nan"]=true;
      isValueExceptionTest["none"]=false;
    }
    if( s.find("user") < std::string::npos )
    {
      isValueExceptionTest["user"]=true;
      isValueExceptionTest["none"]=false;
    }

    return;
}

template<typename T>
void
ValueException<T>::enableExceptionValue(T *argv, size_t argc, std::string s)
{
    clearExceptionValue();

    // init value exceptions
    if( argc )
    {
      for( size_t i=0 ; i < argc ; ++i )
      {
        exceptionValue.push_back( argv[i] );
        exceptionCount.push_back( 0 );
      }
    }

    s += " user" ;
    enableValueExceptionTest(s);

    return;
}

template<typename T>
template<typename T_in>
void
ValueException<T>::import( const ValueException<T_in> &g)
{
    exceptionValue.clear();
    for( size_t i=0 ; i < g.exceptionValue.size() ; ++i )
      exceptionValue.push_back(static_cast<T>(g.exceptionValue[i]) ) ;

    exceptionCount.clear();
    for( size_t i=0 ; i < g.exceptionCount.size() ; ++i )
      exceptionCount.push_back(g.exceptionCount[i]) ;

    countInf  = g.countInf;
    countNaN  = g.countNaN;

    update = g.update;
    return;
}

template<typename T>
void
ValueException<T>::isInfNaN(bool *is, size_t isSz, size_t isBeg, T* arr, size_t arr_sz)
{
     size_t isEnd= arr_sz - isBeg ;
     if( isEnd > isSz )
       isEnd=isSz ;

     for( size_t i=0 ; i < isEnd ; ++i )
       is[i] = false;  // default for is not INF and not NaN

     if( isValueExceptionTest["inf"] )
       isInf(is, isBeg, isEnd, arr, arr_sz);

     if( isValueExceptionTest["nan"] )
       isNaN(is, isBeg, isEnd, arr, arr_sz);

     return;
}

template<typename T>
void
ValueException<T>::isInf(bool *is, size_t isBeg, size_t isEnd, T* arr, size_t arr_sz)
{
     for( size_t i=0 ; i < isEnd ; ++i )
     {
#ifndef XLC
       if( std::isinf( arr[isBeg+i] ) )
#else
       if( isinf( arr[isBeg+i] ) )
#endif
       {
         is[i] = true ;
         ++countInf;
       }
     }

     return;
}

template<typename T>
void
ValueException<T>::isNaN(bool *is, size_t isBeg, size_t isEnd, T* arr, size_t arr_sz)
{
     // NaN compared to itself equals to false

     // Hope this is sufficient to prevent aggressive
     // optimisation; volatile should be avoided, because
     // this would take values from the central storage.
//     T *p;

     for( size_t i=0 ; i < isEnd ; ++i )
     {
#ifndef XLC
       if( std::isnan( arr[isBeg + i]) )
#else
       if( isnan( arr[isBeg + i]) )
#endif
       {
         is[i] = true ;
         ++countNaN;
       }
     }

     return;
}

template<typename T>
void
ValueException<T>::resetExceptionCounts(void)
{
     countInf=0;
     countNaN=0;

    for( size_t i=0 ; i < exceptionCount.size(); ++i)
       exceptionCount[i]=0;

     return;
}
   
template<typename T>
void
ValueException<T>::setExceptionValue(T *argv, size_t argc, std::string &s)
{
    // init value exceptions
    if( argc )
    {
      exceptionValue.clear() ;
      exceptionCount.clear();

      for( size_t i=0 ; i < argc ; ++i )
      {
        exceptionValue.push_back( argv[i] ) ;
        exceptionCount.push_back( 0 );
      }

      s += " user";
    }

    enableValueExceptionTest(s);

    return;
}

template<typename T>
void
ValueException<T>::testValueException( T* arr, size_t arr_sz,
         std::vector<size_t> &validRangeBegin ,
         std::vector<size_t> &validRangeEnd )
{
   // Test values in arr against inf and NaN.
   // The result of the test is stored in array 'is'
   // which must be allocated with the same size of 'arr'

   // compose pairs of indexes to be used as ranges
   // for valid values in a for-loop.

   // reset
   resetExceptionCounts();

   validRangeBegin.clear();
   validRangeEnd.clear();

   if( isValueExceptionTest["none"] )
   {
     validRangeBegin.push_back(0);
     validRangeEnd.push_back( arr_sz );
     return;
   }

   bool is_valid=true;
   bool notValid=false;
   size_t isArrSz=1000;
   bool *isArr = new bool [isArrSz];
   size_t i=0;
   size_t count=0; 

   // test first block
   isInfNaN(isArr, isArrSz, i, arr, arr_sz);

   size_t lim=exceptionValue.size();

   // the algorithm must not start with an invalid value
   for( ; i < arr_sz ; ++i, ++count)
   {
     is_valid=true;

     // test block by block
     if( count == isArrSz )
     {
       count=0;
       isInfNaN(isArr, isArrSz, i, arr, arr_sz);
     }

     if( isArr[count] )
       is_valid=false;
     else
     {
       for( size_t j=0 ; j < lim ; ++j )
       {
         if( arr[i] == exceptionValue[j] )
         {
            ++exceptionCount[j];
            is_valid=false ;
         }
       }
     }

     if( is_valid )
       break;
   }

   if( ! is_valid )
   {
      // not a single valid value was found
      // this range will not start a loop
      validRangeBegin.push_back( 1 );
      validRangeEnd.push_back( 0 );
      return;
   }
     
   // regular search
   for( ; i < arr_sz ; )
   {
     // Part I: search the begin of a valid range.
     for( ; i < arr_sz ; ++i, ++count)
     {
       is_valid=true;

       // test block by block
       if( count == isArrSz )
       {
         count=0;
         isInfNaN(isArr, isArrSz, i, arr, arr_sz);
       }

       if( isArr[count] )
         is_valid=false;
       else
       {
         for( size_t j=0 ; j < lim ; ++j )
         {
           if( arr[i] == exceptionValue[j] )
           {
             ++exceptionCount[j];
             is_valid=false ;
           }
         }
       }

       if( is_valid )
       {
         validRangeBegin.push_back(i);
         ++i;
         ++count;
         break;
       }
     }

     // Part II: search the begin of an invalid range.
     for( ; i < arr_sz ; ++i, ++count)
     {
       // test block by block
       if( count == isArrSz )
       {
         count=0;
         isInfNaN(isArr, isArrSz, i, arr, arr_sz);
       }

       notValid=false;

       if( isArr[count] )
         notValid=true;
       else
       {
         for( size_t j=0 ; j < lim ; ++j )
         {
           if( arr[i] == exceptionValue[j] )
           {
              ++exceptionCount[j];
              notValid=true ;
           }
         }
       }

       if( notValid )
       {
         validRangeEnd.push_back(i);
         ++count;
         ++i;
         break;
       }
     }
   }

   delete [] isArr ;

   // Special case: arr[last] is valid and arr[last-1] is not.
   // The 'close' condition below would be true, thus leading to a fault.
   if( validRangeBegin.size() == validRangeEnd.size() )
     return;

   // close the last range
   if( (is_valid && ! notValid) || (is_valid && notValid) )
      validRangeEnd.push_back( arr_sz );

    return;
}

// the MArep class
//! The core of the MtrtAtt class.
template<typename T>
class MArep
{
public:
  struct ExceptionStruct
  {
     std::ofstream *ofsError;
     std::string strError;
  } ;

  MArep(const T* p, std::vector<size_t> &d );
  ~MArep();

  void assign( const T *p, std::vector<size_t> *d=0);
  void clearM(void);
  void exceptionError(std::string str);
  void getDim(std::vector<size_t> &d_in);
  T** const  getM(void);
  size_t getSize(void);
  MArep* makeRoot(void);
  void resize(size_t sz);
  void setM(void);

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;

  T* arr;       // pointer to data
  size_t counter; // number of references
  std::vector<size_t> dim; // dimensions' sizes
  size_t arr_sz;

  T** m2D;    //matrix access; initialised at 1st call

private:      // prevent copying
  MArep(const MArep&);
  MArep& operator=(const MArep&);
};


template<typename T>
MArep<T>::MArep(const T* p, std::vector<size_t> &d )
{
  // constructor with assignment for p>0
    m2D=0;
    counter=1;
    getDim(d);
    arr=0 ;
    arr_sz=getSize();

    resize(arr_sz);

    if( p )
      for(size_t i=0 ; i < arr_sz ; ++i)
        arr[i]=p[i];
    else
    {
      T zero = static_cast<T>( 0 ) ;
      for(size_t i=0 ; i < arr_sz ; ++i)
        arr[i]=zero;
    }

    // Note: there is no value exception test at construction time
//    validRangeBegin.push_back(0);
//    validRangeEnd.push_back(arr_sz);
  }

template<typename T>
MArep<T>::~MArep()
  {
    clearM();
    if( static_cast<bool>(arr) )
      delete [] arr;
  }

template<typename T>
void
MArep<T>::assign( const T *p, std::vector<size_t> *d)
{
    // this method imports plain arrays pointed to.
    if( ! p && ! d )
    {
      resize(0) ;  // reset all
      return;
    }

    if( d )  // dimensions are passed
    {
      getDim(*d);
      size_t tmp_sz=getSize();

      // previous layout doesn't hold anymore
      if( arr_sz != tmp_sz)
      {
        arr_sz = tmp_sz;
        resize(arr_sz);
        clearM();
      }

    }
//    else:  dimensions of *this must fit

    if( p )
      for(size_t i=0 ; i < arr_sz ; ++i)
        arr[i]=p[i];

    return ;
}

template<typename T>
void
MArep<T>::clearM(void)
{
    if ( static_cast<bool>(m2D) )
    {
      delete [] m2D;
      m2D=0;
     }

    return;
}

template<typename T>
void
MArep<T>::exceptionError(std::string str)
{
    // Occurrence of an error stops the run immediately.
    xcptn.ofsError=0;
    xcptn.strError = "error_MtrxArr_MArep.txt" ;

    // open file for appending data
    if( ! xcptn.ofsError )
      xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

    *xcptn.ofsError << str << std::endl;

//    exit(1);
    return ;
}

template<typename T>
void
MArep<T>::getDim(std::vector<size_t> &d_in)
{ 
    if( d_in == dim )
      return  ;

    // Test dimensions.
    dim.clear();

    // test, whether any dim size > 0
    bool is0=true;
    for(size_t i=0 ; i < d_in.size() ; ++i)
    {
      if( d_in[i] )
        is0=false ;
    }
    if( is0 )
      d_in[0]=1;

    // test, whether any dim size > 1
    bool isNGT1=true;
    for(size_t i=0 ; i < d_in.size() ; ++i)
      if( d_in[i] > 1 )
        isNGT1=false;

    if( isNGT1 )
    { // this is in fact not a field, but a point
      dim.push_back(1);
      return;
    }

    // Dimension size(s) of 1 is/are discarded, if
    // any other is > 1.
    for(size_t i=0 ; i < d_in.size() ; ++i)
      if( d_in[i] > 1 )
        dim.push_back(d_in[i]);

    return ;
}

template<typename T>
T** const
MArep<T>::getM(void)
{
     if( m2D == 0 )
       setM();
     return m2D;
}

template<typename T>
size_t
MArep<T>::getSize(void)
{
    if( dim.size() == 0 )
      return 0;

    size_t sz=1;
    for(size_t i=0 ; i < dim.size() ; ++i)
      sz *= dim[i];

    return sz;
}

template<typename T>
MArep<T>*
MArep<T>::makeRoot(void)
{
    // This method checks whether the MArep obj allocated by
    // *this will be re-used or whether a new obj has to be
    // created due to changing data.
    // This method does not change exception properties.
    // Even  exception properties are passed to a new MArep obj,
    // because changing data is going to be an the basis
    // of arithmetric operations (but, counting is reset).

    // *this obj is owner of MArep, i.e. allocated memory.
    if( counter == 1 )
      return this;

    --counter;

    // *this is not owner, thus it must allocate a new
    // MArep obj to store modified data.
    MArep *nrep = new MArep(arr, dim) ;

    return  nrep;
}

template<typename T>
void
MArep<T>::resize(size_t sz)
{
    if( static_cast<bool>(arr) )
    {
      delete [] arr;
      arr=0;
    }

    if( sz )
      arr = new T [sz] ;

    return;
}

template<typename T>
void
MArep<T>::setM(void)
{
    if( m2D > 0 )
      return;

    // m2D is freed at a different location

    // allocate new mem for the 2D shape
    if( dim.size() == 2 )
    { // m x n matrix
      m2D = new T* [dim[0]];

      for(size_t i0=0; i0 < dim[0]; ++i0)
        m2D[i0] = arr + i0 * dim[1] ;
    }

    return;
}

//! Abstract class for the MtrxArr class.
/*! Permit run-time decisions for the various MtrxArr templates.*/
class MtrxArrB
{
public:
  virtual ~MtrxArrB(){}

  virtual void addCounter(int) =0;

  virtual void disableValueExceptionUpdate(bool b=false) =0;
  virtual void enableValueExceptionUpdate(bool b=true) =0;

  virtual size_t  getExceptionCount(size_t) =0;
  virtual size_t  getExceptionCount(std::string="all") =0;

  virtual void getExceptionValue(void* p, size_t i=0) =0;

  virtual bool isValid(void) =0;  // for the entire data set
  virtual bool isValid(size_t i0) =0;
  virtual bool isValid(size_t i0, size_t i1) =0;
  virtual bool isValid(size_t i0, size_t i1, size_t i2) =0;

  virtual void setExceptionValue(void* p=0, size_t sz=0, std::string s="") =0;

};

//! Matrix-alike (2D, 3D) and/or array acccess of data of type T.

/*! Memory is only allocated by a constructor. Reference counting
 takes place and manages also modification of data by operators.
 If assignment methods are called, then allocated memory is
 re-used when the size of the array is unchanged. Otherwise the
 assigned pointer overwrites former values.
 Memory may also be allocated by multiple dimensional notation
 for a given shape (1D, 2D, or 3D).\n
 Special: storage of 'foreign' data may be conducted by the 'link()'
 method with no allocation, but full access.\n
 Read access is always given by the array 'arr'.\n
 Access methods: arr[index], members m[][], m3D[][][], and
 generally by get(i,[j],[k]), where j and/or k may be omitted.
 Checking of data for 'Inf', 'NaN', and user-defined exception values
 may be enabled. If so, then found value exceptions can be eliminated
 by using vectors validRangeBegin and validRangeEnd indicating ranges
 of valid data. Notice the difference between 'exception value'
 and 'value exception'. If a figure matches an 'exception value'
 (a.k.a. FillValue or missing_value), then a 'value exception'
 is given.
*/

template<typename T>
class MtrxArr : public MtrxArrB
{
  // representation
  MArep<T> *rep;


  struct ExceptionStruct
  {
     std::ofstream *ofsError;
     std::string strError;
  } ;

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;

public:
//! Default constructor.
/*! Note: no test for exception values in any constructor. */
  MtrxArr();

//! Copy constructor.
  MtrxArr(const MtrxArr<T>&);

  MtrxArr(size_t nrow, size_t ncol=0, size_t nz=0);
  MtrxArr(const T*, size_t nrow, size_t ncol=0, size_t nz=0);
  MtrxArr(std::vector<size_t> &dim);
  MtrxArr(const T*, std::vector<size_t> dim);
  ~MtrxArr() {clear();}

//  struct ValueException;  // check validity of data
  ValueException<T> *valExp;

//! Subscription operator.
/*! Note: This is for a 1D array.*/
  T& operator[](size_t i) { return arr[i] ;}
  T const &operator[](size_t i) const { return arr[i] ;}

  //! overloaded math operators
  /*! special feature: operation with an invalid value does
      not change the validity of this object; invalid
      array values are skipped.*/
  MtrxArr<T>& operator=( const T *p);
  MtrxArr<T>& operator=( const MtrxArr<T> &);
  MtrxArr<T>& operator=( const T );

  template<typename T_in> // special:
  MtrxArr<T>& operator=( const MtrxArr<T_in> &);

  void       operator+( const T );
  void       operator-( const T );
  void       operator*( const T );
  void       operator/( const T );
  MtrxArr<T>& operator+=( const T );
  MtrxArr<T>& operator-=( const T );
  MtrxArr<T>& operator*=( const T );
  MtrxArr<T>& operator/=( const T );

  void       operator+( MtrxArr<T> &g);
  void       operator-( MtrxArr<T> &g);
  void       operator*( MtrxArr<T> &g);
  void       operator/( MtrxArr<T> &g);
  MtrxArr<T>& operator+=( MtrxArr<T> &g);
  MtrxArr<T>& operator-=( MtrxArr<T> &g);
  MtrxArr<T>& operator*=( MtrxArr<T> &g);
  MtrxArr<T>& operator/=( MtrxArr<T> &g);

  //! Indirect indexing.
  MtrxArr<T> operator[]( const MtrxArr<size_t> &p );

  void        addCounter(int i)
                 { rep->counter += i; }

  //! Assignment of an array.
  template<typename T_in> // with type conversion
  void    assign( const T_in *p, std::vector<size_t> dim);
  void    assign( const T *p,  std::vector<size_t> dim);
  void    assign( const T *p,  size_t nx=0, size_t ny=0, size_t nz=0);
  template<typename T_in> // with type conversion
  void    assign( const T_in *p, size_t nx=0, size_t ny=0, size_t nz=0);

  //! get the starting address [with off set].
  T*      begin(size_t offSet=0)
        { return arr + offSet; }

  //! clear local allocations (not the for the reference).
  void    clear(void);

  //! Disable/enable testing on Inf, NaN, and user-defined values.
  /*! If parameter s is empty, then all types of exceptions will
      be tested. Values of s are "inf", "NaN, or "user" and any
      combination. Testing is enabled by default for all.*/
  void    disableValueExceptionTest(std::string s="");
  void    disableValueExceptionUpdate(bool b=false)
             { valExp->update = b; }

  void    enableValueExceptionTest(std::string s="", std::string add="")
             { valExp->enableValueExceptionTest(s); }

  void    enableValueExceptionUpdate(bool b=true)
             { valExp->update = b; }

  void    exceptionError(std::string str);

  void    frame( const MtrxArr<T> &g, size_t beg, size_t sz=0);

  //! Get value at given index(es).
  /*! For valarray, external arry, and matrix notation (2D, and 3-D).*/
  T       get(size_t i0, size_t i1, size_t i2)
            {return *(arr +(i0 * rep->dim[1]+i1) * rep->dim[2] + i2 ) ;}
  T       get(size_t i0, size_t i1)
            {return *(arr + i0 * rep->dim[1] + i1);}
  T       get(size_t i0)
            {return arr[i0];}

  //! Get number of columns.
  size_t  getColSize(void)
         { return rep->dim.size() > 1 ? rep->dim[1] : 0 ;}

  std::vector<size_t>
          getDimensions(void)const { return rep->dim;}

  size_t  getExceptionCount(size_t i);
  size_t  getExceptionCount(std::string s="all");

  void    getExceptionValue(void* p, size_t i=0);
  void    getExceptionValue(std::vector<T> &, size_t);

  T ** const
          getM(void)
         { return rep->getM(); }

  //! Get number of rows.
  size_t  getRowSize(void)
         { return rep->dim.size() > 0 ? rep->dim[0] : 0 ;}

  //! Get value at given index(es).
  /*! For valarray, external arry, and matrix notation (2D, and 3-D).*/
  size_t index(size_t i0, size_t i1, size_t i2)
            {return (i0*rep->dim[1]+i1) * rep->dim[2] + i2  ;}
  size_t index(size_t i0, size_t i1)
            {return i0 * rep->dim[1] + i1;}
  size_t index(size_t i0)
            {return i0;}

  //! Get the state of the data representation
  /*! Return value is false if a data value pointed to by index(es)
      is Inf, NaN, or a user-defined exception value (filling data).
      The method called without parameters returns false, if at least
      one value of the array is invalid.*/
  bool   isValid(void);  // for the entire data set
  bool   isValid(size_t i0);
  bool   isValid(size_t i0, size_t i1)
            {return isValid(i0 * rep->dim[1] + i1) ;}
  bool   isValid(size_t i0, size_t i1, size_t i2)
            {return isValid((i0*rep->dim[1]+i1) * rep->dim[2] + i2); }

  //! Link a pointer to data while keeping the shape.
  /*! Purpose: Layering of 3D data*/
  void    link( const T *p, bool keepShape=false);

  //! Put value at given index(es).
  /*! For valarray, external arry, and matrix notation (2D, and 3-D).*/
  void     put(T val, size_t i0,
               size_t i1=UINT_MAX, size_t i2=UINT_MAX);

  //! Put value at given index(es).
  /*! For valarray, external arry, and matrix notation (2D, and 3-D).
      No testing of inf etc. .*/
  void    put(bool b, T val, size_t i0,
              size_t i1=UINT_MAX, size_t i2=UINT_MAX);

  //! Resize valarray and matrix.
  void    resize(size_t nx, size_t ny=0, size_t nz=0);
  void    resize(std::vector<size_t>&d);

  //! Set exception values.
  /*! String enable value exception test for "inf" and/or "NaN",
      e.g. string="inf NaN" enables both. Number sz user-defined
      exception values (a.k.a. missingValue, _FillValue)
      are stored in array vE.*/
  void    setExceptionValue(void* p=0, size_t sz=0, std::string s="")
            { valExp->setExceptionValue(
                  reinterpret_cast<T*>(p), sz, s);}

  void    setExceptionValue(T *vE, size_t sz, std::string s="")
            { valExp->setExceptionValue(vE, sz, s);}

  //! Get size of data.
  size_t  size(void) const { return rep->arr_sz;}

  void    testValueException(void)
             { valExp->testValueException(rep->arr, rep->arr_sz,
               validRangeBegin, validRangeEnd); }

  T *arr ;  // points to &rep->(*pva)[0]
  T zero ;  // nomen est omen

  // combine to index ranges of valid values
  std::vector<size_t> validRangeBegin ;
  std::vector<size_t> validRangeEnd ;
};

// Default constructor.
template<typename T>
MtrxArr<T>::MtrxArr()
{
  std::vector<size_t> t;
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;

  rep = new MArep<T>(0, t );  // t with size()==0

  if( rep->arr )
    arr=rep->arr;
  else
    arr=0;
}

// Copy constructor.
template<typename T>
MtrxArr<T>::MtrxArr(const MtrxArr<T> &ma)
{
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;

  ma.rep->counter++;
  rep=ma.rep;

  rep->dim=ma.rep->dim;
  arr=rep->arr;

  valExp->copy( *ma.valExp );
}

// Construct for a given shape without assignment
template<typename T>
MtrxArr<T>::MtrxArr(size_t nx, size_t ny, size_t nz)
{
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;
  rep=0;

  std::vector<size_t> t;
  t.push_back(nx);
  t.push_back(ny);
  t.push_back(nz);

  rep = new MArep<T>(0, t );
  arr=rep->arr;
}

// Construct for a given shape without assignment
template<typename T>
MtrxArr<T>::MtrxArr(const T *p, size_t nx, size_t ny, size_t nz)
{
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;
  rep=0;

  std::vector<size_t> t;
  t.push_back(nx);
  t.push_back(ny);
  t.push_back(nz);

  rep = new MArep<T>(p, t );
  arr=rep->arr;

  // test and get exception values
  testValueException();
}

// Construct for a given shape without assignment
template<typename T>
MtrxArr<T>::MtrxArr(std::vector<size_t> &d)
{
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;
  rep=0;
  rep = new MArep<T>(0, d );
  arr=rep->arr;
}

template<typename T>
MtrxArr<T>::MtrxArr(const T *p, std::vector<size_t> t)
{
  zero = static_cast<T>( 0 );
  valExp = new ValueException<T> ;
  rep=0;
  rep = new MArep<T>(p, t );
  arr=rep->arr;

  // test and get exception values
  testValueException();
}

//! Assignment operator of const value to each array element.
template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator=( const T *p)
{
  // Sizes of p and rep must be identical.
  // This cannot be checked.

  if( rep->counter == 1)  // reuse the current MArep obj
    rep->assign(p) ;
  else
  {
    rep->counter--;
    rep=new MArep<T>(p, rep->dim);
  }

  arr=rep->arr;

  // test and get exception values
  testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator=( const MtrxArr<T> &g)
{
  if( this == &g )
    return *this;

  // update counter
  g.rep->counter++;

  if( (--rep->counter) == 0 )
    delete rep;  //delete the obsolete instance, if owner

  rep=g.rep;
  arr=rep->arr;

  // get exception values
  if( valExp->update )
  {
    valExp->copy( *g.valExp );

    validRangeBegin=g.validRangeBegin;
    validRangeEnd=g.validRangeEnd;
  }
//  else
//    testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator=(const T v)
{
  // Note: no check for exception values
  if( v == zero )
    return *this;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i]=v;

  testValueException();

  validRangeBegin.clear();
  validRangeEnd.clear();

  validRangeBegin.push_back(0);
  validRangeEnd.push_back( rep->arr_sz );

  return *this;
}

// assign a MtrxArr obj of different type
template<typename T>
template<typename T_in>
MtrxArr<T>&
MtrxArr<T>::operator=( const MtrxArr<T_in> &g)
{
  if( reinterpret_cast<void*>(this)
       == reinterpret_cast<void*>(const_cast<MtrxArr<T_in>*> (&g)) )
    return *this;

  assign(g.arr, g.getDimensions() );

  if( valExp && valExp->update )
  {
    valExp->import( *g.valExp );

    validRangeBegin=g.validRangeBegin;
    validRangeEnd=g.validRangeEnd;
  }

  testValueException();

  return *this;
}

//! const value to each element of array
template<typename T>
void
MtrxArr<T>::operator+(const T v)
{
  // Note: no check for exception values
  if( v == zero )
    return;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] += v;

  // Note: testValueException(): nothing has changed

  return ;
}

//! const MtrxArr object is added
template<typename T>
void
MtrxArr<T>::operator+(MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator+(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] += g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return ;
}

template<typename T>
void
MtrxArr<T>::operator-(const T v)
{
  // Note: no check for exception values

  if( v == zero )
    return;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] -= v ;

  // Note: testValueException(): nothing has changed

  return ;
}

//! const MtrxArr object is added
template<typename T>
void
MtrxArr<T>::operator-( MtrxArr<T> &g)
{
  // Note: no check for exception values

  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator-(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] -= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return ;
}

template<typename T>
void
MtrxArr<T>::operator*(const T v)
{
  // Note: no check for exception values

  if( v == static_cast<T>( 1 ) )
    return;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] *= v ;

  // Note: testValueException(): nothing has changed

  return ;
}

//! const MtrxArr object is added
template<typename T>
void
MtrxArr<T>::operator*( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator*(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] *= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return ;
}

template<typename T>
void
MtrxArr<T>::operator/(const T v)
{
  // Note: no check for exception values

  // ignore division by zero
  if( v == static_cast<T>(1) )
    return; 

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] /= v ;

  if( v == zero )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator/=(): division by zero.";
    exceptionError( ostr.str() ); // exits

    testValueException();
  }
  // else: nothing has changed

  return ;
}

//! const MtrxArr object is added
template<typename T>
void
MtrxArr<T>::operator/( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator/(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] /= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return ;
}

//! const MtrxArr object is added
template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator+=( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator+=(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] += g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator+=(const T v)
{
  // Note: no check for exception values

  if( v == zero )
    return *this;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] += v ;

  // Note: testValueException(): nothing has changed

  return *this;
}

//! const MtrxArr object is subtracted
template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator-=( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator-=(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] -= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator-=(const T v)
{
  // Note: no check for exception values

  if( v == zero )
    return *this;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] -= v ;

  // Note: testValueException(): nothing has changed

  return *this;
}

//! const MtrxArr object is multiplicated
template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator*=( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator*=(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] *= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator*=(const T v)
{
  // Note: no check for exception values

  if( v == static_cast<T>( 1 ) )
    return *this;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] *= v ;

  // Note: testValueException(): nothing has changed

  return *this;
}

//! const MtrxArr object is divided
template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator/=( MtrxArr<T> &g)
{
  if( size() != g.size() )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator/=(): MtrxArr objects of different size.";
    exceptionError( ostr.str() ); // exits

    return *this;
  }

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] /= g.arr[i] ;

  if( ! g.isValid )
    testValueException();

  return *this;
}

template<typename T>
MtrxArr<T>&
MtrxArr<T>::operator/=(const T v)
{
  // Note: no check for exception values

  if( v == static_cast<T>( 1 ) )
    return *this;

  rep=rep->makeRoot();
  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i)
     arr[i] /= v ;

  if( v == zero )
  {
    std::ostringstream ostr(std::ios::app);
    ostr << "\nMtrxArr<T>::operator/=(): division by zero.";
    exceptionError( ostr.str() ); // exits

    testValueException();
  }
  // else: nothing has changed

  return *this;
}

//! Assignment of a MtrxArr obj for indirect indexing.
template<typename T>
MtrxArr<T>
MtrxArr<T>::operator[]( const MtrxArr<size_t> &in )
{
  MtrxArr<T> tma(in.size());

  for(size_t i=0 ; i<in.size() ; ++i)
    tma.arr[i]=arr[ in.arr[i] ];

  if( !isValid() )
    tma.testValueException();

  return tma;
}

//! Assignment of an array.
template<typename T>
template<typename T_in>
void
MtrxArr<T>::assign( const T_in *p, std::vector<size_t> d )
{
  if( rep->counter == 1)  //reuse the current MArep obj
    rep->assign(0, &d) ;
  else
  {
    rep->counter--;
    rep=new MArep<T>(0, d);
  }

  arr=rep->arr;

  for( size_t i=0 ; i < rep->arr_sz ; ++i )
    arr[i] = static_cast<T>(p[i]);

  testValueException();

  return;
}

template<typename T>
void
MtrxArr<T>::assign( const T *p, std::vector<size_t> d )
{
  if( rep->counter == 1)  //reuse own array
    rep->assign(p, &d ) ;
  else
  {
    rep->counter--;
    rep=new MArep<T>(p, d );
  }

  arr=rep->arr;

  testValueException();

  return;
}

template<typename T>
void
MtrxArr<T>::assign( const T *p, size_t nx, size_t ny, size_t nz )
{
  std::vector<size_t> t;
  if( nx )
    t.push_back(nx);
  if( ny )
    t.push_back(ny);
  if( nz )
    t.push_back(nz);

  assign(p, t );
  return;
}

template<typename T>
template<typename T_in>
void
MtrxArr<T>::assign( const T_in *p, size_t nx, size_t ny, size_t nz )
{
  std::vector<size_t> t;
  if( nx )
    t.push_back(nx);
  if( ny )
    t.push_back(ny);
  if( nz )
    t.push_back(nz);

  assign(p, t);
  return;
}

template<typename T>
void
MtrxArr<T>::clear(void)
{
  if( (--rep->counter) == 0 )
    delete rep; // clears if no longer referenced

  if( static_cast<bool>(valExp) )
    delete valExp ;
}

template<typename T>
void
MtrxArr<T>::exceptionError(std::string str)
{
  // Occurrence of an error stops the run immediately.
  xcptn.ofsError=0;
  xcptn.strError = "error_MtrxArr.txt" ;

  // open file for appending data
  if( ! xcptn.ofsError )
    xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

  *xcptn.ofsError << str << std::endl;

//  exit(1);
  return ;
}

template<typename T>
void
MtrxArr<T>::frame( const MtrxArr<T> &g, size_t beg, size_t sz)
{
  if( sz )
  {  // check available space
     if( rep->arr_sz != sz )
       ;
  }

  // note: validRangeEnd points to the last index+1
  size_t end=beg+sz;

  rep->assign(g.arr + beg) ;

  arr=rep->arr;

  validRangeBegin.clear();
  validRangeEnd.clear();

  size_t lim=g.validRangeBegin.size();
  size_t k=0;

  // find start
  for( ; k < lim ; k++ )
     if( beg < g.validRangeEnd[k] )
       break;

  for( ; k < lim ; k++ )
  {

     if( end <= g.validRangeBegin[k] )
       break;

     // somewhere in between
     if( beg <= g.validRangeBegin[k] && end > g.validRangeEnd[k])
     {
       validRangeBegin.push_back( g.validRangeBegin[k] - beg ) ;
       validRangeEnd.push_back( g.validRangeEnd[k] - beg ) ;
       continue;
     }

     // the first interval from the left
     if( beg >= g.validRangeBegin[k] && beg < g.validRangeEnd[k])
     {
       validRangeBegin.push_back( 0) ;
       if( end > g.validRangeEnd[k] )
         validRangeEnd.push_back( g.validRangeEnd[k] - beg ) ;
       else
         validRangeEnd.push_back( sz ) ;
       continue;
     }

     validRangeBegin.push_back( g.validRangeBegin[k] - beg ) ;
     validRangeEnd.push_back( sz ) ;
     break;
  }

  // determine exception value counts of the frame
  if( g.valExp  )
  {
    // no exception value provided
    if( ! g.valExp->exceptionValue.size() )
      return;

    lim=validRangeBegin.size();
    for( k=0 ; k < lim ; k++ )
      sz -= (validRangeEnd[k] - validRangeBegin[k]) ;

    if( sz )
    {
      if( ! valExp->exceptionCount.size() )
      {
         if( g.valExp->exceptionCount.size() )
            valExp->copy( *g.valExp, true );
      }

      valExp->exceptionCount[0] = sz ;
    }
    else
      valExp->exceptionCount[0] = 0 ;
  }

  return ;
}

template<typename T>
size_t
MtrxArr<T>::getExceptionCount(size_t i)
{
  // special: indicates the index of user exception counts

  if( valExp->exceptionCount.size() <= i )
    return 0; 

  return valExp->exceptionCount[i] ;
}

template<typename T>
size_t
MtrxArr<T>::getExceptionCount(std::string s)
{
  if( valExp->exceptionCount.size() == 0 )
    return 0;

  size_t count=0;

  if( s == "all" )
  {
    for( size_t i=0 ; i < valExp->exceptionCount.size() ; ++i )
      count += valExp->exceptionCount[i];

    count += valExp->countInf;
    count += valExp->countNaN;

    return count;
  }

  if( s == "user" )
  {
    for( size_t i=0 ; i < valExp->exceptionCount.size() ; ++i )
      count += valExp->exceptionCount[i];
    return count;
  }

  if( s == "inf" )
    return valExp->countInf ;

  if( s == "nan" )
    return valExp->countNaN ;

  return count;
}

template<typename T>
void
MtrxArr<T>::getExceptionValue(void* p, size_t i)
{
  if( i )
  {
    getExceptionValue(p, i);
    return;
  }

  for( i=0 ; i < valExp->exceptionValue.size() ; ++i )
    getExceptionValue(p, i);

  return ;
}

template<typename T>
void
MtrxArr<T>::getExceptionValue(std::vector<T> &g, size_t i)
{
  g[i] = valExp->exceptionValue[i];

  return ;
}

template<typename T>
bool
MtrxArr<T>::isValid(void)
{
  // Note: the entire data set is invalid if vRE[0] == 0.
  if( validRangeBegin.size() == 1 && validRangeEnd[0] )
    return true;
  else
    return false;
}

template<typename T>
bool
MtrxArr<T>::isValid(size_t i0)
{
  for( size_t j=0 ; j < validRangeBegin.size() ; ++j )
    if( i0 >= validRangeBegin[j] && i0 < validRangeEnd[j] )
      return true;

  return false;
}

template<typename T>
void
MtrxArr<T>::link( const T *p, bool keepShape)
{
  // *this with incomplete initialisation
  if( ! arr )
    keepShape=false;

  if( keepShape )
  {
    // Only the adress rep.arr is pointing to has changed.
    // So, the shape must fit.

    rep->assign(p) ;
  }
  else
  {
    // sizes of p and rep must be identical
    if( rep->counter == 1)  //reuse own array
      rep->assign(p,&rep->dim) ;
    else
    {
      rep->counter--;
      rep=new MArep<T>(p, rep->dim);
    }
  }

  arr=rep->arr;
  testValueException();

  return ;
}

template<typename T>
void
MtrxArr<T>::put(T val, size_t i0, size_t i1, size_t i2)
{
  // Put value at given index(es).
  // For internal and external array and matrix notation (2D, and 3-D).

  if( ! arr )  // no data
    return ;  // ToDo: error handling

  size_t ii;

  if( i1 == UINT_MAX  )        // 1-D
    ii=i0;
  else if( i2 == UINT_MAX )   // 2-D
    ii=i0*rep->dim[1] + i1;  
  else                        // 3-D
    ii=(i0*rep->dim[1]+i1) * rep->dim[2] + i2 ;

  bool is=false;

  // test previous value
  if( arr[ii] == std::numeric_limits<T>::infinity()
      || arr[ii] != arr[ii] )
    is = true;

  // test next value
  if( val == std::numeric_limits<T>::infinity() || val != val )
    is = true;

  // now, set the value
  arr[ii]=val;

  if( is )
    testValueException();

  return;
}

template<typename T>
void
MtrxArr<T>::put(bool b, T val, size_t i0, size_t i1, size_t i2)
{
  // Put value at given index(es).
  // For internal and external array and matrix notation (2D, and 3-D).

  // Omit exception tests

  if( ! arr )  // no data
    return ; 

  size_t ii;

  if( i1 == UINT_MAX  )        // 1-D
    ii=i0;
  else if( i2 == UINT_MAX )   // 2-D
    ii=i0*rep->dim[1] + i1;  
  else                        // 3-D
    ii=(i0*rep->dim[1]+i1) * rep->dim[2] + i2 ;

  // now, set the value
  arr[ii]=val;

  return;
}

template<typename T>
void
MtrxArr<T>::resize(size_t nx, size_t ny, size_t nz)
{
  std::vector<size_t> t;
  t.push_back(nx);
  t.push_back(ny);
  t.push_back(nz);

  resize(t);
  return;
}

template<typename T>
void
MtrxArr<T>::resize(std::vector<size_t> &d)
{
  // resize
  rep->assign(0, &d );
  arr=rep->arr;

  return;
}

#endif
