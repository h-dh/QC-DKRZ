#include "hdhC.h"

namespace hdhC
{

template<typename T>
T clearLeastBits( T v, size_t shft)
{
  switch( sizeof( T ) )
  {
    case 8:
    {
      void *p = &v;
      uint64_t *r=reinterpret_cast<uint64_t*>(p);
      *r >>= shft;
      *r <<= shft;
    }
    break;
    case 4:
    {
      void *p = &v;
      uint32_t *r=reinterpret_cast<uint32_t*>(p);
      *r >>= shft;
      *r <<= shft;
    }
    break;
    case 2:
    {
      void *p = &v;
      uint16_t *r=reinterpret_cast<uint16_t*>(p);
      *r >>= shft;
      *r <<= shft;
    }
    break;
    case 1:
    {
      void *p = &v;
      uint8_t *r=reinterpret_cast<uint8_t*>(p);
      *r >>= shft;
      *r <<= shft;
    }
    break;
  }

  return v;
}

bool compare(double x, double y, std::string op, double epsilon)
{
  // compare x and y within uncertainty ranges e
  // A factor of 2 is substituted in epsilon
  // modes: op: "==" --> x == y
  //        op: "<"  --> x < y
  //        op: ">"  --> x > y

  if( op == "==" )
  {
    if( fabs(x - y) < epsilon )
      return true;
    else
      return false;
  }

  if( op == "<" )
  {
    if( (x + epsilon) < y )
      return true;
    else
      return false;
  }

  if( op == ">" )
  {
    if( x > (y + epsilon) )
      return true;
    else
      return false;
  }

  return false;
}

bool
compare(double x, double y, std::string op, int decimals)
{
  // fabs( (x + x*10^-decimals) op (y + y*10^-decimals )

  // epsilon is 10^-DECS of the smaller value of x and y*/

// compare x and y within uncertainty ranges e
  // A factor of 2 is substituted in epsilon
  // modes: op: "==" --> x == y
  //        op: "<"  --> x < y
  //        op: ">"  --> x > y
  double delta_x = x;
  double delta_y = y;

  for( int i=0 ; i < decimals ; ++i )
  {
     delta_x /= 10.;
     delta_y /= 10.;
  }

  double epsilon = fabs(delta_y - delta_x);
  double val = fabs( x - y );

  if( op == ">" )
  {
    if( val > epsilon )
      return true;
  }
  else if( op == "<" )
  {
    if( val < epsilon )
      return true;
  }
  else if( val <= epsilon )
     return true;

  return false;
}

double convertTime(std::string targetUnit, std::string time,
            std::string unit)
{
  // Conversion between different time formats.
  // Note: time-unit appended at string 'time' overrules unit.
  // Time-units: y,mo,d,h,mi,s .
  // Note: This function does not work, if the conversion
  // crosses the border between year and months on the one hand
  // and days or smaller units on the other. In such a case, please,
  // use combination of class Date.addTime() and Date.get..., where
  // ... denotes the target unit.

  // If there is a mix of digits and non-digits, size is >= 2:
  // If isNonDigit && size == 1, then there is an error.
  // If size == 0, error
  // But, for !isNonDigit && size == 1 we got the default

  // It is save to append units (even if empty) to time
  // Note: time-unit overrules parameter unit.
  time += unit;

  Split splt;
  splt.alternateSplitting( time );

  bool isNon = isNonDigit(time);
  if( (splt.size() == 1 && isNon) || (splt.size() == 0) )
  {
     std::cerr << "convertTime(): error in 2nd parameter (time="
               << time << ")" << std::endl;
     exit(1);
  }

  char c ;
  std::string sc;

  if( isNon )
  {
    c = tolower(splt[1][0]) ;
    sc = Lower()(splt[1]);

    if( c == 'y' )
       unit="y";
    else if( sc.substr(0,2) == "mo" )
       unit="mo";
    else if( c == 'd' )
       unit="d";
    else if( c == 'h' )
       unit="h";
    else if( sc.substr(0,2) == "mi" )
       unit="mi";
    else if( c == 's' )
       unit="s";
  }
  else if( unit.size() == 0 )
    return splt.toDouble(0) ;

  c = tolower(targetUnit[0]) ;
  sc = Lower()(targetUnit);

  if( unit[0] == 'y' )
  {
    if( c == 'y')
      return splt.toDouble(0);
    else if( sc.substr(0,2) == "mo" )
      return 12. * splt.toDouble(0);

    return 0. ;
  }

  if( unit.substr(0,2) == "mo" )
  {
    if( c == 'y')
      return splt.toDouble(0) / 12.;
    else if( sc.substr(0,2) == "mo" )
      return splt.toDouble(0);

    return 0. ;
  }

  if( unit[0] == 'd' )
  {
    if( c == 'd')
      return  splt.toDouble(0);
    else if( c == 'h')
      return  24. * splt.toDouble(0);
    else if( sc.substr(0,2) == "mi")
      return  1440. * splt.toDouble(0);
    else if( c == 's')
      return  86400. * splt.toDouble(0);

    return 0. ;
  }

  if( unit[0] == 'h' )
  {
    if( c == 'd')
      return  splt.toDouble(0) / 24. ;
    else if( c == 'h')
      return  splt.toDouble(0);
    else if( sc.substr(0,2) == "mi")
      return  60. * splt.toDouble(0);
    else if( c == 's')
      return  3600. * splt.toDouble(0);

    return 0. ;
  }

  if( unit.substr(0,2) == "mi" )
  {
    if( c == 'd')
      return  splt.toDouble(0) / 1440. ;
    else if( c == 'h')
      return  splt.toDouble(0) / 60.;
    else if( sc.substr(0,2) == "mi")
      return  splt.toDouble(0);
    else if( c == 's')
      return  60. * splt.toDouble(0);

    return 0. ;
  }

  if( unit[0] == 's' )
  {
    if( c == 'd')
      return  splt.toDouble(0) / 86400. ;
    else if( c == 'h')
      return  splt.toDouble(0) / 3600.;
    else if( sc.substr(0,2) == "mi")
      return  splt.toDouble(0) / 60. ;
    else if( c == 's')
      return  splt.toDouble(0);
  }

  // crosses border
  return 0. ;
}

double deci2dgr(double grad, double min, double sec)
{
  double sign=1.;

  if( grad <0. )
    sign = -1. ;
  else if( grad == 0. && min  < 0. )
    sign = -1. ;
  else if( grad == 0. && min == 0. && sec < 0. )
    sign = -1. ;

  return sign * (fabs(grad) + fabs(min)/60. + fabs(sec)/3600.) ;
}

double dgr2deci(std::string s)
{
  double sign=1.;
  double konvFakt=0.;
  char c='\0';
  double grad=0.;
  double min=0.;
  double sec=0.;

  if( s.empty() )
  {
    std::cerr << "dgr2deci: string is empty" << std::endl ;
    exit(1);
  }

  for( size_t i=0 ; i < s.size() ; ++i )
  {
    /*Konvertion: Stunde in Grad: suche h,H bzw.  g,G,n,N,s,S,O,o,e,E,w,W */
    c = s[i] ;

    if( c == 'h' || c == 'H' )
    {
      konvFakt = 360./24. ;
      break;
    }
    else if( c == 'g' || c == 'G' || c == 'o' || c == 'O'
               || c == 'e' || c == 'E' || c == 'n' || c == 'N'  )
    {
      konvFakt = 1. ;
      break;
    }
    else if( c == 'w' || c == 'W' || c == 's' || c == 'S' )
    {
      konvFakt = -1. ;
      break;
    }
    else if( i == ( s.size() - 1 ) )
      return string2Double(s,1);
  }

  grad  = string2Double(s,1);
  if ( ! string2Double(s,2, &min) )
    min  = 0. ;
  if ( ! string2Double(s,3, &sec) )
    sec  = 0. ;

  if( grad < 0. )
    sign = -1. ;
  else if( grad == 0. && min  < 0. )
    sign = -1. ;
  else if( grad == 0. && min == 0. && sec < 0. )
    sign = -1. ;

  return sign * (fabs(grad) + fabs(min)/60. + fabs(sec)/3600.)*konvFakt ;
}

// ----------------------------------------------------

std::string double2String( double z, int d, std::string flag)
{
  // Note: for downward compatibility.


  /* a floating number is converted to a string. The number of
     decimals is prescribed by 'd'. Rounding takes place.
     Format: 'flag'="n_X" with width 'n'
     and the char 'X' used to fill left-side.
     If '_' is omitted, then the width is filled with blanks.
     Default no 'lead': no left-side filling.
     Negative value of d indicate rounding with additional
     elimination of trailing zeros and a trailing decimal point.*/

  std::string str;
  size_t pos;

  if( isDigit(flag) )
  {
    str  = "w=";
    str += flag;
  }
  else if( (pos=flag.find('_')) < std::string::npos )
  {
    str  = "w=";
    str += flag.substr(0,pos) ;
    str += ",f=";
    str += flag[flag.size()-1];
  }
  else if( flag.size() )
    str = flag;

  if( str.size() )
    str += ",";

  str  += "p=";
  if( d < 0 )
  {
    str += itoa( -1*d );
    str += "|adjusted";
  }
  else
    str += itoa(d);

  return double2String(z, str);

}

std::string double2String( double z, std::string flag)
{
  /* a floating number is converted to a string.
     Format: [??][w=int?][p=int[|adj]?][f=int?][s[ci]]

     Separator ?: by default ',', The pre- and post-fix may
     be omitted for the default. If changed, then the prefix
     has to be formed by twice a character (not: w, p,f,s).
     w: width of the field, p: precision, f: filling character,
     sci: scientific notation.
     If the value of precision contains substring '|adj', then
     trailing zeros and decimal point are discarded..
  */

  bool isSci=false;
  bool isAdjusted=false;
  std::string c;
  int width = -1;
  int precision = -5;

  Split splt;

  if( flag.size() > 2 && ! isDigit(flag) && flag[0] == flag[1] )
  {
    splt.setSeparator(flag[0]);
    flag=flag.substr(2,flag.size()-2);
  }
  else
    splt.setSeparator(',');

  splt = flag;
  for(size_t i=0 ; i < splt.size() ; ++i )
  {
     if( splt[i][0] == 's' )
     {
       isSci=true;
       continue;
     }

     if( splt[i][0] == 'w' )
     {
       width = static_cast<int>( hdhC::string2Double(splt[i]) ) ;
       continue;
     }
     if( splt[i][0] == 'f' )
     {
       c = splt[i][splt[i].size()-1];
       continue;
     }
     if( splt[i][0] == 'p' )
     {
       Split splt_p(splt[i],"|");
       for( size_t i=0 ; i < splt_p.size() ; ++i )
       {
         if( splt_p[i].substr(0,3) == "adj" )
           isAdjusted=true;
         else
           precision = static_cast<int>( hdhC::string2Double(splt_p[i]) ) ;
       }
       continue;
     }
  }

  std::ostringstream ostr(std::ios::app);

  double z_abs(fabs(z));

  if( isSci || z_abs > 100000. || (z_abs > 0.0 && z_abs < 0.000001) )
    ostr.setf(std::ios::scientific, std::ios::floatfield);
  else
    ostr.setf(std::ios::fixed, std::ios::floatfield);

  if( flag.size() )
  {
    ostr.setf(std::ios::adjustfield);
    if( c.size() )
      ostr.fill(c[0]);

    if( width > -1 )
    {
      if( precision )
        ostr << std::setw(width+precision+1);
      else
        ostr << std::setw(width);
    }
  }

  ostr.precision( precision );

  ostr << z ;

  if( isAdjusted )
  {
    std::string str( ostr.str() );
    std::string sci_rep;

    if( isSci )
    {
       size_t pos;
       if( (pos=str.find('e')) < std::string::npos )
          ;
       else if( (pos=str.find('E')) < std::string::npos )
         ;

       if( pos < std::string::npos )
       {
          sci_rep = str.substr(pos);
          str = str.substr(0, pos);
       }
    }

    int pos=str.size()-1;
    for(  ; pos > -1 ; --pos)
      if( str[pos] != '0' )
        break ;

    if( str[pos] == '.' )
      --pos;

    str = str.substr(0, ++pos);
    str += sci_rep;

    return str;
  }

  return ostr.str();
}

// ----------------------------------------------------

void envelope( std::vector<double> &x, std::vector<double> &y,
        std::vector<double> &wEnv, std::vector<double> &env, bool upper )
{
  int index;
  bool isTst;

  env.clear();
  wEnv.clear();

  if( upper )  // upper envelope of the curve
    index = 1;
  else
    index = 0;

  for( size_t i=1 ; i<x.size()-1 ; i++)
  {
    if( index == 0 )  //Minimum
      isTst = ( y[i-1] > y[i] && y[i+1] > y[i]  ) ;
    else              //Maximum
      isTst = ( y[i-1] < y[i] && y[i+1] < y[i]  ) ;

    if( isTst )
    {
      // Extremum was found
      Point scheitel = parabolaSummit(x[i-1], x[i], x[i+1], y[i-1],
                       y[i], y[i+1]);

      env.push_back(scheitel.y);
      wEnv.push_back(scheitel.x);
    }
  }

  return ;
}

// ----------------------------------------------------

template<typename T>
bool equal(T x1, T x2, double epsilon)
{
  // Is (x1-x2) == 0 under the constraint of comuter inaccuracies?
  // Get the magnitude and compare values to an adjusted epsilon.

  // return: true if fabs(x1-x2) < magnitude*epsilon
  //         where magnitude is the one of the larger absolute value

  // simple equality
  if( x1 == x2 )
    return true;

  // Note: it is assumed the typenames are restricted to
  // a kind of float

  //take the type into account
  std::string ti( typeid(x1).name() );

  if( ti == "d" )
    epsilon *= epsilon ;

  // test for the same sign
  if( x1 > 0. )
  {
    if( x2 < 0. )
      return false;
  }
  else
  {
    if( x2 > 0. )
      return false;
  }

  x1=fabs(x1);
  x2=fabs(x2);

  // get the magnitude of the larger value
  double x = (x1 < x2) ? x2 : x1 ;
  double scale;

  if( x > 1. )
  {
    // find the magnitude; large value case
    scale=0.1;
    do
    {
       scale *= 10.;  // increase
       x /= 10.;      // decrease
    } while( x > 1. );
  }
  else
  {
    // find the magnitude; small value case
    scale=10.;
    do
    {
       scale /= 10.;  // increase
       x *= 10.;      // decrease
    } while( x < 1. );
  }

  if( fabs( x1-x2 ) < (scale*epsilon ) )
    return true;  //
  else
    return false;
}

// ----------------------------------------------------
// Note: *_cmip5 versions do not calculate the Fletcher32
// checksum as expected for data arrays that are
// sub-divided and processed in sequential calls,
// because the second reduction step is done at the
// end of each call.
// The versions are kept for CMIP5 application in order
// to get backward compatibility.

uint32_t fletcher32_cmip5( std::string &str, bool *reset)
{
   return fletcher32_cmip5(str.c_str(), str.size(), reset);
}

uint32_t fletcher32_cmip5( double data, bool *reset)
{
   std::string str = double2String(data) ;

   return fletcher32_cmip5( str.c_str(), str.size(), reset) ;
}

uint32_t fletcher32_cmip5( double *data, size_t sz, bool *reset)
{
   std::string str ;

   for( size_t i=0 ; i < sz ; ++i )
     str += double2String(data[i]) ;

   return fletcher32_cmip5( str.c_str(), str.size(), reset) ;
}

uint32_t fletcher32_cmip5( const char *data, size_t byte_sz, bool *reset )
{
  bool thisReset=true;
  if( reset )
    thisReset = *reset;

  // adapted from:
  // http://en.wikipedia.org/wiki/Fletcher%27s_checksum
  // Ref.: Fletcher, J. G., An Arithmetic Checksum for Serial
  // Transmissions, IEEE Trans. on Comm., Vol. COM-30, pp 247-252.

   // Note: this version does not calculate the Fletcher32
   // checksum as expected for data arrays that are
   // sub-divided and processed in sequential calls,
   // because the second reduction step is done at the
   // end of each call.
   // The version is kept for CMIP5 application in order
   // to get backward compatibility.

   // Length in 16-bit words; even
   size_t len = byte_sz / 2;

   static uint32_t sum1 = 0xffff;
   static uint32_t sum2 = 0xffff;

   if( thisReset )
   {
     sum1 = 0xffff;
     sum2 = 0xffff;

     // the boolean is set to false after the first call. It
     // keeps that value until changed outside
     thisReset = false;
   }

   while (len)
   {
     size_t tlen = len > 360 ? 360 : len;
     len -= tlen;

     do
     {
        // invariant against big or little endianess
        sum1 += static_cast<uint32_t>(
          (static_cast<uint16_t>(data[0]) << 8)
            | static_cast<uint16_t>(data[1])  );

        data += 2; //shift data by two bytes
        sum2 += sum1;
     } while (--tlen);

     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // last (odd) byte
   if(byte_sz % 2)
   {
     sum1 += (uint32_t)(((uint16_t)*data) << 8);
     sum2 += sum1;
     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // Second reduction step to reduce sums to 16 bits
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);

   if( reset )
     *reset = thisReset ;


   return sum2 << 16 | sum1;
}

// ----------------------------------------------------

uint32_t fletcher32( std::string &str, bool *reset)
{
  size_t sz = str.size();
  return fletcher32(str.c_str(), sz, reset);
}

template<typename T>
uint32_t fletcher32( std::vector<T> &v, bool *reset, size_t nShift)
{
  uint32_t cs;
  for( size_t i=0 ; i < v.size() ; ++i )
    cs = fletcher32(v[i], reset, nShift);
  return cs ;
}

template<typename T>
uint32_t fletcher32( T data, bool *reset, size_t nShift)
{
  return fletcher32( &data, 1, reset, nShift) ;
}

template<typename T>
uint32_t fletcher32( T *data, size_t sz, bool *reset, size_t nShift)
{
  int i = 1;
  char *p = (char *)&i;

  size_t nBytes=sizeof(T);

  if( nShift )
  {
    if(nBytes > 1 && p[0] == 1)
    {
      // little endian
      return fletcher32_LE_clear( data, sz, reset, nShift) ;
    }
    else
    {
      // big endian
      uint8_t *c=reinterpret_cast<uint8_t*>(data);
      return fletcher32_BE_clear( c, sz*nBytes, reset, nShift) ;
    }
  }
  else
  {
    if(nBytes > 1 && p[0] == 1)
    {
      // little endian
      return fletcher32_LE( data, sz, reset) ;
    }
    else
    {
      // big endian
      uint8_t *c=reinterpret_cast<uint8_t*>(data);
      return fletcher32_BE(c, sz*nBytes, reset) ;
    }
  }
}

template<typename T>
uint32_t fletcher32( const T *data, size_t sz, bool *reset, size_t nShift)
{
  T *c;
  c=const_cast<T*>(data);

  int i = 1;
  char *p = (char *)&i;

  size_t nBytes=sizeof(T);

  if( nShift )
  {
    if( nBytes > 1 && p[0] == 1)
    {
      // little endian
      return fletcher32_LE_clear( c, sz, reset, nShift) ;
    }
    else
    {
      // big endian
      uint8_t *d=reinterpret_cast<uint8_t*>(c);
      return fletcher32_BE_clear( d, sz*nBytes, reset, nShift) ;
    }
  }
  else
  {
    if( nBytes > 1 && p[0] == 1)
    {
      // little endian
      return fletcher32_LE( c, sz, reset) ;
    }
    else
    {
      // big endian
      uint8_t *d=reinterpret_cast<uint8_t*>(c);
      return fletcher32_BE( d, sz*nBytes, reset) ;
    }
  }

}

uint32_t fletcher32_BE( uint8_t *data, size_t len, bool *reset)
{
  bool thisReset=true;
  if( reset )
    thisReset = *reset;

  // adapted from:
  // http://en.wikipedia.org/wiki/Fletcher%27s_checksum
  // Ref.: Fletcher, J. G., An Arithmetic Checksum for Serial
  // Transmissions, IEEE Trans. on Comm., Vol. COM-30, pp 247-252.

  // first call: reset==true
  // following calls: reset==false from this function
  // last call: reset==true externally; finalisation

  // len: the number of bytes

   static uint32_t sum1     = 0xffff;
   static uint32_t sum2     = 0xffff;

   if( thisReset )
   {
     sum1 = 0xffff;
     sum2 = 0xffff;

     // the boolean is set to false after the first call. It
     // keeps that value until changed outside
     thisReset = false;
   }

   while (len)
   {
     size_t llen = len > 5056 ? 5056 : len;
     len -= llen;

     do
     {
        sum1 += *data++;
        sum2 += sum1;
     } while (--llen);

     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // Second reduction step to reduce sums to 32 bits
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);

  if( reset )
    *reset = thisReset;

   return sum2 << 16 | sum1;
}

template<typename T>
uint32_t fletcher32_LE( T *data, size_t len, bool *reset )
{
  bool thisReset=true;
  if( reset )
    thisReset = *reset;

  // adapted from:
  // http://en.wikipedia.org/wiki/Fletcher%27s_checksum
  // Ref.: Fletcher, J. G., An Arithmetic Checksum for Serial
  // Transmissions, IEEE Trans. on Comm., Vol. COM-30, pp 247-252.

  // first call: reset==true
  // following calls: reset==false from this function
  // last call: reset==true externally; finalisation

   static uint32_t sum1     = 0xffff;
   static uint32_t sum2     = 0xffff;

   if( thisReset )
   {
     sum1 = 0xffff;
     sum2 = 0xffff;

     // the boolean is set to false after the first call. It
     // keeps that value until changed outside
     thisReset = false;
   }

   size_t numBytes = sizeof(T);
   size_t threshold = static_cast<size_t>(5056)/numBytes ;

   while (len)
   {
     size_t llen = len > threshold ? threshold : len;
     len -= llen;

     do
     {
       size_t j=numBytes-1;
       uint8_t *d=reinterpret_cast<uint8_t*>(data);
       do
       {
          sum1 += d[j];
          sum2 += sum1;
       } while ( j-- > 0 ) ;
       data++ ;
     } while (--llen);

     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // Second reduction step to reduce sums to 32 bits
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);

  if( reset )
    *reset = thisReset;

   return sum2 << 16 | sum1;
}

template<typename T>
uint32_t fletcher32_BE_clear( T *data, size_t len, bool *reset, size_t nShift )
{
  bool thisReset=true;
  if( reset )
    thisReset = *reset;

  // adapted from:
  // http://en.wikipedia.org/wiki/Fletcher%27s_checksum
  // Ref.: Fletcher, J. G., An Arithmetic Checksum for Serial
  // Transmissions, IEEE Trans. on Comm., Vol. COM-30, pp 247-252.

  // first call: reset==true
  // following calls: reset==false from this function
  // last call: reset==true externally; finalisation

   static uint32_t sum1     = 0xffff;
   static uint32_t sum2     = 0xffff;

   if( thisReset )
   {
     sum1 = 0xffff;
     sum2 = 0xffff;

     // the boolean is set to false after the first call. It
     // keeps that value until changed outside
     thisReset = false;
   }

   size_t numBytes = sizeof(T);
   size_t threshold = static_cast<size_t>(5056)/numBytes ;

   while (len)
   {
     size_t llen = len > threshold ? threshold : len;
     len -= llen;

     do
     {
       size_t j=0;

       T tmp = clearLeastBits( *data, nShift );
       uint8_t *d=reinterpret_cast<uint8_t*>(&tmp);
//printBits(*data);
//printBits(tmp);
//std::cout << std::endl;
//exit(0);
       do
       {
          sum1 += d[j];
          sum2 += sum1;
       } while ( j++ < numBytes ) ;
       data++ ;
     } while (--llen);

     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // Second reduction step to reduce sums to 32 bits
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);

  if( reset )
    *reset = thisReset;

   return sum2 << 16 | sum1;
}

template<typename T>
uint32_t fletcher32_LE_clear( T *data, size_t len, bool *reset, size_t nShift )
{
  bool thisReset=true;
  if( reset )
    thisReset = *reset;

  // adapted from:
  // http://en.wikipedia.org/wiki/Fletcher%27s_checksum
  // Ref.: Fletcher, J. G., An Arithmetic Checksum for Serial
  // Transmissions, IEEE Trans. on Comm., Vol. COM-30, pp 247-252.

  // first call: reset==true
  // following calls: reset==false from this function
  // last call: reset==true externally; finalisation

   static uint32_t sum1     = 0xffff;
   static uint32_t sum2     = 0xffff;

   if( thisReset )
   {
     sum1 = 0xffff;
     sum2 = 0xffff;

     // the boolean is set to false after the first call. It
     // keeps that value until changed outside
     thisReset = false;
   }

   size_t numBytes = sizeof(T);
   size_t threshold = static_cast<size_t>(5056)/numBytes ;

   while (len)
   {
     size_t llen = len > threshold ? threshold : len;
     len -= llen;

     do
     {
       size_t j=numBytes-1;

       T tmp = clearLeastBits( *data, nShift );
       uint8_t *d=reinterpret_cast<uint8_t*>(&tmp);
//printBits(*data);
//printBits(tmp);
//std::cout << std::endl;
//exit(0);
       do
       {
          sum1 += d[j];
          sum2 += sum1;
       } while ( j-- > 0 ) ;
       data++ ;
     } while (--llen);

     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   // Second reduction step to reduce sums to 32 bits
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);

  if( reset )
    *reset = thisReset;

   return sum2 << 16 | sum1;
}

// ----------------------------------------------------

std::string getBasename(std::string &str)
{
  std::string path, filename, basename, extension ;

  getPathComponents(str, path, filename, basename, extension);

  return basename;
}

std::string getExtension(std::string &str)
{
  std::string path, filename, basename, extension ;

  getPathComponents(str, path, filename, basename, extension );

  return extension;
}

std::string getFilename(std::string &str)
{
  std::string path, filename, basename, extension ;

  getPathComponents(str, path, filename, basename, extension);

  return filename;
}

std::string getPath(std::string &str, bool isWithSlash)
{
  std::string path, filename, basename, extension ;

  getPathComponents(str, path, filename, basename, extension);

  if( isWithSlash )
    path += '/' ;

  return path;
}

void getPathComponents(std::string &str,
             std::string &path,   //path no trailing '/', except if root.
                                  //if no path, then '.'
             std::string &fName,  //complete filename
             std::string &base,   //filename without extension
             std::string &ext)    //the extension with leading '.'
{
  std::string::size_type pos0, pos1 ;

  // path
  if( (pos0 = str.rfind('/') ) == std::string::npos )
    path.clear() ;  // empty path
  else
    if( pos0 == 0 )
      path = "/";
    else
      path = str.substr( 0, pos0 ) ;

  // filename
  if( pos0  == std::string::npos )
    fName = str ;                 // str is filename
  else if( pos0 == str.size() )
    fName = "" ;                  // str is just a path
  else
    fName = str.substr( pos0+1) ;

  // basename
  if( (pos1 = fName.rfind('.')) == std::string::npos )
    base = fName ;
  else
    base = fName.substr( 0 , pos1  ) ;

  // extension
  if( pos1 >= fName.size() )
    ext = "" ;
  else
    ext = fName.substr( pos1+1, fName.size() - pos1 -1 ) ;

  return;
}

// ----------------------------------------------------

template <typename T>
T invertBits ( T val )
{
  // invert the ordering of bits

  size_t sz = sizeof(val);
  uint8_t *r=reinterpret_cast<uint8_t*>(&val);

  T inverse = 0;
  uint8_t *inv=reinterpret_cast<uint8_t*>(&inverse);

  for ( size_t j=0; j < sz; ++j )
  {
    for ( size_t i=0; i < 8; ++i )
    {
      // shift left and then append the last bit from val
      inv[7-i] = ( inv[7-i] << 1 ) | ( r[j] & 1 );
      r[j] >>= 1;
    }
  }

  return inverse;
}

bool isWhiteString( std::string &str )
{
  // a string only of spaces and tabs
  size_t pE=str.size();

  if( pE == 0 )
    return false;

  for( size_t p=0 ; p < pE ; ++p )
  {
    if( str[p] == ' ' || str[p] == '\t' )
      continue ;
    else
      return false ;
  }

  return true ;
}

template <typename T>
void printBits ( T val )
{
  // note: the ordering of printed bits is inversed!
  T ival = invertBits(val);

  size_t sz = sizeof(val);
  uint8_t *r=reinterpret_cast<uint8_t*>(&ival);

  for ( size_t j=0; j < sz; ++j )
  {
    for ( size_t i=0; i < 8; ++i )
    {
      std::cout << ( r[i] & 1 );
      r[i] >>= 1;
    }
    std::cout << " " ;
  }

  std::cout << std::endl;
  return;
}

// ----------------------------------------------------
std::string
getNonAlpha(std::string s)
{
  std::string t;

  for( size_t i=0; i < s.size(); ++i )
  {
    if( ! isAlpha(s[i]) )
    {
      size_t j=0;
      for( ; j < t.size() ; ++j )
         if( t[j] == s[i] )
            break;

      if( j == t.size() )
        t += s[i];
    }
  }

  return t;
}

std::string
getNonAlphaNum(std::string s)
{
  std::string t;

  for( size_t i=0; i < s.size(); ++i )
  {
    if( ! isAlphaNum(s[i]) )
    {
      size_t j=0;
      for( ; j < t.size() ; ++j )
         if( t[j] == s[i] )
            break;

      if( j == t.size() )
        t += s[i];
    }
  }

  return t;
}

std::string
getUniqueString(std::string& s)
{
  std::vector<std::string> vs;

  Split x_s(s) ;

  for( size_t j=0 ; j < x_s.size() ; ++j )
    vs.push_back(x_s[j]) ;

  std::vector<std::string>vs2(getUniqueVector(vs));

  std::string us;
  for( size_t j=0 ; j < vs2.size() ; ++j)
  {
    if(j)
      us += " " ;
    us += vs2[j];
  }

  return us;
}

std::string
getUniqueString(std::vector<std::string>& vs)
{
  std::string s;

  for( size_t j=0 ; j < vs.size() ; ++j )
     if(j)
       s += vs[j] ;

  return getUniqueString(s);
}

std::vector<std::string>
getUniqueVector(std::string& s)
{
  std::vector<std::string> vs;

  Split x_s(s) ;

  for( size_t j=0 ; j < x_s.size() ; ++j )
    vs.push_back(x_s[j]) ;

  return getUniqueVector(vs);
}

std::vector<std::string>
getUniqueVector(std::vector<std::string>& vs)
{
  std::vector<std::string> uni;

  for( size_t j=0 ; j < vs.size() ; ++j )
  {
     size_t u;
     for( u=0 ; u < uni.size() ; ++u )
       if( uni[u] == vs[j] )
          break;

     if( u == uni.size() )
       uni.push_back(vs[j]);
   }

  return uni;
}

bool isAlpha(unsigned char c)
{
  int i = c ;

  bool is=false;

  if( ((i-64) > 0 && (i-64) < 27 ) || ((i-96) > 0 && (i-96) < 27 ) )
    is=true;

  return is;
}

bool isAlpha(std::string s, bool isContains)
{
  if( ! s.size() )
    return false;

  if( isContains )
  {
    for(size_t k=0; k < s.size(); ++k)
      if( isAlpha(s[k]) )
        return true;
  }
  else
  {
    for(size_t k=0; k < s.size(); ++k)
      if( !isAlpha(s[k]) )
        return false;
  }

  return true;
}

bool isAlphaNum(unsigned char c)
{
  int i = c ;

  bool is=false;

  if( (i-64) > 0 && (i-64) < 27 )
    is=true;
  else if( (i-96) > 0 && (i-96) < 27 )
    is=true;
  else if( (i-47) > 0 && (i-47) < 11 )
    is=true;

  return is;
}

template <typename T>
bool
isAmong(T& item, std::vector<T>& set)
{
  for(size_t i=0 ; i < set.size() ; ++i )
    if( item == set[i] )
      return true ;

  return false;
}


template <typename T>
bool
isAmong(T item, std::vector<T>& set, bool is)
{
  // note that the bool ist just for the compiler
  for(size_t i=0 ; i < set.size() ; ++i )
    if( item == set[i] )
      return true ;

  return false;
}

template <typename T>
bool
isAmong(std::vector<T>& sub, std::vector<T>& set, bool all)
{
  if( set.size() == 0 )
     return false;

  size_t count=0;
  for( size_t i=0 ; i < sub.size() ; ++i )
  {
    if( isAmong(sub[i], set) )
    {
      if( !all )
        return true;

      ++count;
    }
    else if( all )
      return false;
  }

  if( count == sub.size() )
    return true;  // for all==true

  return false ;
}

bool isDigit(unsigned char c, bool isNumber)
{
  if( isNumber )
    if(  c == '.' || c == '-' || c == '+'  )
      return true;

  int i = c ;

  i -= 47 ;

  if( i > 0 && i < 11 )
    return true;
  else
    return false;
}

bool isDigit(std::string s, bool isContains)
{
  if( ! s.size() )
    return false;

  if( isContains )
  {
    for(size_t k=0; k < s.size(); ++k)
      if( isdigit(s[k]) )
        return true;
  }
  else
  {
    for(size_t k=0; k < s.size(); ++k)
      if( !isdigit(s[k]) )
        return false;
  }

  return true;
}

bool isNonDigit(std::string s)
{
  for(size_t k=0; k < s.size(); ++k)
    if( !isdigit(s[k]) )
      return true;

  return false;
}

bool isNumber( std::string &str )
{
  const char *s = str.c_str();

  /*  s  ist der gesamte String
   return: true, falls string nur Zahl, sonst false */

  /* der folgende Teil entspricht im wesentlichen der Fkt 'atof' */
  for( ; ( isdigit(*s)  ||  *s == '-'  || *s == '+'
          ||  *s == '.' ||  *s == 'E' ||  *s == 'e' ) ; s++ )
  {
    if( *s== 'E' ||  *s == 'e' )
    {
      if( *(s+1) == '+' ||  *(s+1) == '-' || isdigit(*(s+1)) )
        continue;
    }
  }

  if( *s == '\0' )
    return true ;    /* nur Zahl */

  return false ;
}

// ----------------------------------------------------

std::string itoa(int n )
{
  //for downward compability only
  int sign=1;
  if( n < 0 )
  {
    n *= -1 ;
    sign=-1;
  }

  return itoa( static_cast<long> (n), sign ) ;
}

std::string itoa(unsigned long long int n, int sign )
{
  // returns string representation of n

  std::string t;

  // the inverse of n is stored in t
  do
  {
    t += n % 10 + '0';
  } while((n/=10)>0);

  if(sign < 0)
    t += '-';

  // reverse
  for( size_t j=0 ; j < t.size()/2 ; ++j)
  {
    char c = t[j];
    size_t l= t.size() - j - 1 ;
    t[j] = t[l] ;
    t[l] = c    ;
  }

  return t ;
}

// ----------------------------------------------------

Point parabolaSummit(double xm, double x, double xp, double ym,
                        double y, double yp)
{
  Point punkt;
  double epsilon, a1,a2 ;

  a1 = (yp-y)*(xm*xm - x*x) -(ym - y)*(xp*xp -x*x) ;
  a2 = (ym-y)*(x-xp) - (yp-y)*(x-xm) ;

  if( fabs(a2) > 0. )
    epsilon = a1 / (2.*a2) ;
  else  // ist eine Gerade
    epsilon = x ;

  punkt.x = epsilon ;

  a2 = ( (x-epsilon)*(x-epsilon) - (xm-epsilon)*(xm-epsilon) );

  if( fabs(a2) > 0. )
    punkt.y = yp - (xp-epsilon)*(xp-epsilon) *(y - ym) / a2 ;
  else  // entartet zum Pkt
    punkt.y = yp ;

  return punkt ;
}

// ----------------------------------------------------

double planck(double anue,double t)
{
  double a,b,pl,aa;
  double nk_c ,nk_h ,nk_k;

/*
  c=2.9979 * 1.E+08 M/S
  h=6.6256 * 1.E-34 J*S
  k=1.3805 * 1.E-23 J/K
  WELLENZAHL:ANUE=FREQ. * C ; [CM-1] =: 100 [M-1]
  SIGMA=5.6693*1.E-08 [W M-2 K-4]
*/

  if( t == 0. )
    return 0.;

  nk_c=2.99792458 ;
  nk_h=6.62606876 ;
  nk_k=1.3806503 ;
  a=2.*nk_h*nk_c*nk_c*1.e-10;
  b=nk_h*nk_c/nk_k*.1 / t;

  aa=a*anue*anue*anue ;
  pl=aa/(exp( b*anue) - 1.);

  return  pl;
}

// ----------------------------------------------------

// private function
double string2DoubleFct( std::string &, size_t & );

//public access
bool string2Double( const char *s0, size_t nr, double* retVal)
{
  std::string t(s0) ;
  t += '\0';
  return string2Double( t, nr, retVal) ;
}

bool string2Double( std::string s0, size_t nr, double* retVal)
{
  /*  s  entrire string
    val  eventualy the found number
     nr  the number-position of anumber to be searched in the string;
         i.e. nr=2 looks for the second real number in the string.
   return: true, if number was detected, else: false */

  std::string s(s0) ;
  s += '\0';

  size_t index=0 ;
  double val=0.  ;

  for( size_t n=0 ; n<nr ; n++ )
  {
    /* mainly function 'atof' */
    for( ; ! ( isDigit(s[index])  ||  s[index] == '-'  || s[index] == '+'
            ||  s[index] == '.' ) ; index++  )
    {
      if( s[index] == '\0' )
        return false ;    // no number found
    }

    // where all the work is done
    val = string2DoubleFct( s, index );
   }

  *retVal = val ;

  return true ;
}
double string2Double( const char *s0, size_t nr, bool isReturnValNull)
{
  std::string s(s0);
  s += '\0' ;
  return string2Double( s, nr, isReturnValNull) ;
}

double string2Double( std::string s0, size_t nr, bool isReturnValNull)
{
  /*  s  entrire string
    val  eventualy the found number
     nr  the number-position of anumber to be searched in the string;
         i.e. nr=2 looks for the second real number in the string.

    If isReturnValNull set true: return 0. in case of not found.
    Default:  false ==> return MAXDOUBLE  */

  double val=0.;

  std::string s( s0 );
  s += '\0' ;

  size_t index=0 ;

  for( size_t n=0 ; n<nr ; n++ )
  {
    //mainly fct 'atof'
    for( ; ! ( isDigit(s[index])  ||  s[index] == '-'  || s[index] == '+'
            ||  s[index] == '.' ) ; index++  )
    {
      if( s[index] == '\0' )
        return isReturnValNull ? 0. : MAXDOUBLE ; //no number in string
    }

    // where work takes place
    val = string2DoubleFct( s, index );
  }

  return val ;
}

double string2Double( const char *s0, size_t nr, int &width, int &precision, bool isReturnValNull)
{
  std::string s(s0) ;
  s += '\0' ;

  return string2Double( s, nr, width, precision, isReturnValNull) ;
}

double string2Double( std::string s0, size_t nr, int &width, int &precision, bool isReturnValNull)
{
  /*  s  entrire string
    val  eventualy the found number
     nr  the number-position of anumber to be searched in the string;
         i.e. nr=2 looks for the second real number in the string.
    width      return the length of the substring containing the number
    precision  number of decimals

    If isReturnValNull set true: return 0. in case of not found.
    Default:  false ==> return MAXDOUBLE  */

  std::string s( s0 );
  s += '\0' ;

  size_t index=0 ;
  size_t beg=0;
  double val=0. ;

  width = 0;
  precision=0;

  for( size_t n=0 ; n<nr ; n++ )
  {
    for( ; ! ( isDigit(s[index])  ||  s[index] == '-'  || s[index] == '+'
            ||  s[index] == '.' ) ; ++index )
    {
      if( s[index] == '\0' )
        return isReturnValNull ? 0. : MAXDOUBLE ;    // keine Zahl vorhanden
    }

    beg=index;

    val = string2DoubleFct( s, index );
  }

  // Bestimmung der Feldbreite und Praezision
  size_t front=s.find('.',beg);

  std::string frontStr(s.substr(beg, front-beg) );
  std::string decStr(s.substr(front+1, index-front-1) );

  precision = decStr.size();
  width     = frontStr.size() + precision +1 ;

  return val ;
}

double string2Double( const char *s0, size_t nr ,int *restIndex, bool isReturnValNull )
{
   std::string s(s0);
   s += '\0' ;

   return string2Double( s, nr, restIndex, isReturnValNull ) ;
}

double string2Double( std::string s0, size_t nr ,int *restIndex, bool isReturnValNull )
{
  //restIndex: index where the remmaining non-digit string continues

  std::string s( s0 );
  s += '\0' ;

  size_t index=0 ;
  double val=0.  ;

  for( size_t n=0 ; n<nr ; n++ )
  {
    for( ; ! ( isDigit(s[index])  ||  s[index] == '-'  || s[index] == '+'
            || s[index] == '.' ) ; ++index )
    {
      if( s[index] == '\0' )
      {
        *restIndex = index ;
        return isReturnValNull ? 0. : MAXDOUBLE ; //no number
      }
    }

    val = string2DoubleFct( s, index );
  }

  *restIndex = index ;

  return val ;
}

double string2DoubleFct( std::string &s, size_t &index )
{
  int sign ,power_count=0 ;
  double power, val ;

  sign = (s[index] == '-') ? -1 : 1 ;

  if( s[index] == '+' || s[index] == '-')
    ++index ;

  for( val=0.0; isDigit(s[index]); ++index )
    val = 10.0 * val + (s[index] - '0');

  if( s[index] == '.' )
    ++index ;

  for( power = 1.0; isDigit( s[index]); ++index )
  {
    val = 10.0 * val + (s[index] - '0' );
    power *= 10.0;
    power_count++;
  }

  // scientific notation?
  if( s[index]=='e' || s[index]=='E' )
  {
    ++index;
    val = sign * val / power ;

    if( ! (s[index] == '+' || s[index] == '-' || isDigit(s[index]) ) )
      return val ;

    bool isSign = s[index] == '+' || s[index] == '-';

    if( ((index+1) == s.size()) || (isSign && !isDigit(s[index+1])) )
    {
      // the detected 'e' or 'E' is not part of the number
      // although a sign follows, but without any digit
      return val;
    }

    if( ! ( isSign || isDigit(s[index]) ) )
    {
      // the detected 'e' or 'E' is not part of the number
      return val;
    }

    std::string t(s.substr(index));
    val *= pow( 10. , string2Double(t,1) );

    // index must be adjusted for the next looping
    while( (s[index] == '+' || s[index] == '-' || isDigit(s[index]) ) )
      ++index ;

    return val ;
  }

  val = sign * val / power ;

  return val ;
}

// ----------------------------------------------------

std::string
clearChars(std::string &str, std::string s, bool isStr )
{
  // Rermove all characters from a str.
  // By default, each character in 's' is removed.
  // If isStr==true, then only every occurrence of
  // the string 's' is removed.

 std::string t;

  if( isStr )
  {
     size_t pos0=0, pos1;
     while( (pos1=str.find(s,pos0)) < std::string::npos )
     {
       t += str.substr(pos0, pos1-pos0) ;
       pos0=pos1+s.size();
     };
     t += str.substr(pos0) ;
  }
  else
  {
    // each character
    for( size_t j=0 ; j < str.size() ; ++j)
    {
      bool is=true;
      for( size_t i=0 ; i < s.size() ; ++i)
      {
        if( str[j] == s[i] )
        {
          is=false;
          break;
        }
      }

      if( is )
        t += str[j] ;
    }
  }

  return t ;
}

std::string clearInternalMultipleSpaces(std::string &str )
{
  // replace multiple internal spaces by a single blank
  Split splt(str);

  std::string s(splt[0]);

  for( size_t p=1 ; p < splt.size() ; ++p )
  {
    s += " " ;
    s += splt[p] ;
  }

  return s ;
}

std::string clearSpaces(std::string &str )
{
  // clear all spaces from string
  std::string s;

  for( size_t p=0 ; p < str.size() ; ++p )
    if( str[p] != ' ' && str[p] != '\t' && str[p] != '\n')
      s += str[p] ;

  return s ;
}

std::string sAssign(std::string right, bool withSpaces)
{
  if( withSpaces )
    return "= <" + right + ">" ;
  else
    return "=<" + right + ">" ;
}

std::string sAssign(std::string left, std::string right, bool withSpaces)
{
  if( withSpaces )
    return left + " = <" + right + ">" ;
  else
    return left + "=<" + right + ">" ;
}

std::string stripSurrounding(std::string &str, std::string mode )
{
  // strip off leading and trailing blanks and tabs
  size_t p0=0;
  size_t p1=str.size()-1;
  bool isEmpty=true;

  std::string side;

  std::vector<char> token;
  token.push_back(' ');
  token.push_back('\t');
  for( size_t i=0 ; i < mode.size() ; ++i )
  {
    if( mode[i] == 'r' )
    {
      if( mode.substr(i, 5) == "right" )
      {
        side = "right";
        i += 4;
      }
    }
    else if( mode.substr(i, 4) == "left" )
    {
      side = "left";
      i += 3;
    }
    else
      token.push_back(mode[i]);
  }

  if( side != "right" )
  {
    for( size_t p=0 ; p < str.size() ; ++p )
    {
      bool isCont=false;
      for( size_t i=0 ; i < token.size() ; ++i )
        if( str[p] == token[i] )
          isCont=true ;

      if( isCont )
        continue;

      p0=p;
      isEmpty=false;

      break;
    }
  }

  if( side != "left" )
  {
    for( int p=str.size()-1 ; p > -1 ; --p )
    {
      bool isCont=false;
      for( size_t i=0 ; i < token.size() ; ++i )
        if( str[p] == token[i] )
          isCont=true ;

      if( isCont )
        continue;

      p1=p;
      isEmpty=false;
      break;
    }
  }

  if( isEmpty )
    return "" ;

  return str.substr(p0, p1 - p0 +1 ) ;
}

// ----------------------------------------------------

void upperCase(char &c)
{
  c = toupper(c);
  return;
}

void lowerCase(char &c)
{
  c = tolower(c);
  return;
}


// ----------------------------------------------------

} // end of namespace
