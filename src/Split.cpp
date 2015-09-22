#include "split.h"

Split::Split()
{
  init();
  sep.push_back(std::string(" "));  // by default
  sep.push_back(std::string("\t"));  // by default
}

Split::Split(std::string s) : str(s)
{
  init();
  sep.push_back(std::string(" "));  // by default
  sep.push_back(std::string("\t"));  // by default
}

Split::Split(const char* s) : str(s)
{
  init();
  sep.push_back(std::string(" "));  // by default
  sep.push_back(std::string("\t"));  // by default
}

Split::Split(std::string s, std::string sp, bool isStr) : str(s)
{
  init();
  addSeparator(sp, isStr);
}

Split::Split(std::string s, char sp) : str(s)
{
  init();
  s=sp;
  sep.push_back(s);
}

Split::Split(const Split &t)
{
  xcptn.ofsError=0;
  str = t.str ;
  sep=t.sep;
  isDecomposed = t.isDecomposed;
  isFixedWidth=t.isFixedWidth;
  isEmptyItemsEnabled=t.isEmptyItemsEnabled;
  items = t.items ;
  itemPos = t.itemPos ;
  is_valid=t.is_valid;
}

Split::~Split()
{
  if( xcptn.ofsError )
    delete xcptn.ofsError ;
}

Split&
Split::operator=(  const char *s )
{
  std::string sstr(s);
  isDecomposed=false;
  items.clear();
  itemPos.clear();
  return *this=sstr;
}

Split&
Split::operator=( std::string s )
{
  str=s;
  isDecomposed=false;
  items.clear();
  itemPos.clear();
  return *this;
}

Split&
Split::operator+=( std::string s )
{
  str+=s;
  isDecomposed=false;
  return *this;
}

std::string&
Split::operator[](size_t i)
{
  if( ! isDecomposed )
    decompose();

  if( i >= items.size() )
    return empty;

  return items[i];
}

void
Split::addIgnore( std::string s, bool isStr)
{
  if( isStr )
    ignore.push_back(s);
  else
  {
    std::string t;
    for( size_t i=0 ; i < s.size() ; ++i)
    {
      t=s[i];
      ignore.push_back(t);
    }
  }

  return ;
}

void
Split::addSeparator( std::string s, bool isStr)
{
  if( isStr )
    sep.push_back(s);
  else
  {
    std::string t;
    for( size_t i=0 ; i < s.size() ; ++i)
    {
      t=s[i];
      sep.push_back(t);
    }
  }

  return ;
}

void
Split::alternateSplitting(std::string s)
{
  items.clear();
  itemPos.clear();

  size_t pos0=0;  // where a number begins
  size_t pos1=0;    // one index behind the end

  while ( toDouble( s, pos0, pos1) < MAXDOUBLE )
  {
     if( pos0 > 0 )
     {
       // string begins with a substring
       items.push_back( s.substr(0, pos0) );
       itemPos.push_back(0);
     }

     // string begins with a number
     items.push_back( s.substr(pos0, pos1 - pos0) );
     itemPos.push_back(pos0);

     s = s.substr(pos1);
     pos0=pos1=0;
  }

  // Is the last part of the string a sub-string?
  // Remember: pos0 is advancing until it points to the begin
  //           of the next number
  if( pos0 == s.size() )
  {
    items.push_back( s );
    itemPos.push_back(pos0);
  }

  isDecomposed=true;

  return;
}

void
Split::clear(void)
{
  str = "";
  items.clear();
  itemPos.clear();

  return ;
}

const char*
Split::c_str(size_t i)
{
  if( ! isDecomposed )
  {
    if( str.size() == 0 )
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "Split::c_str()"
           <<"\nno string specified.";
      exceptionError( ostr.str() );
    }

    decompose();
  }

  if( i>=items.size() )
  {
      std::ostringstream ostr(std::ios::app);
      ostr << "Split::c_str()"
           << "\nno such high token index.";
      exceptionError( ostr.str() );
  }

  return items[i].c_str();
}

void
Split::decompose(void)
{
  items.clear();
  itemPos.clear();

  if( ignore.size() )
    doIgnore( str ) ;  // will perhaps change object wide 'str'

  if( isFixedWidth )
     getFixedFormat();
  else
  {
    std::string tmp;

    std::string::size_type pos, pos0, pos1  ;
    int sepIndex=-1;
    pos0=0;

    do
    {
      pos1 = std::string::npos ; // starting value

      // Works for multiple separators.
      // Find the lowest next pos1 out of all separators
      for( std::string::size_type i=0 ; i < sep.size() ; ++i)
      {
        if( ( pos = str.find(sep[i], pos0)) < pos1)
        {
          pos1 = pos ;
          sepIndex= static_cast<int>(i);
        }
      }

      if( pos1 < std::string::npos)
      {
        tmp = str.substr( pos0, pos1-pos0) ;
        if( isEmptyItemsEnabled || tmp.size() > 0 )
        {
          items.push_back( tmp );
          itemPos.push_back(pos0);
        }

        pos0 = pos1 + sep[sepIndex].size() ;
      }
    } while( pos1 < std::string::npos ) ;

    // the last token remained
    tmp = str.substr( pos0) ;
    if( isEmptyItemsEnabled || tmp.size() > 0 )
    {
      items.push_back( tmp );
      itemPos.push_back(pos0);
    }
  }

  if( items.size() == 0 )
  {
    items.push_back("");
    itemPos.push_back(0);
  }

  isDecomposed=true;

  return ;
}

void
Split::doIgnore( std::string x )
{
  std::string xx;

  for( size_t j=0 ; j < ignore.size() ; ++j )
  {
    xx.clear();
    size_t sz = ignore[j].size() ;

    if( sz == 1 )
    {
      // ignore plain character
      for( size_t i=0 ; i < x.size() ; ++i )
      {
         if( x[i] == ignore[j][0] )
           continue;
         xx += x[i];
      }
    }
    else
    {
      // ignore strings
      for( size_t i=0 ; i < x.size() ; ++i )
      {
         if( x[i] == ignore[j][0] )
         {
           if( x.substr(i,sz) == ignore[j] )
           {
             i += (sz - 1) ;
             continue;
           }
           else
             xx += x[i];
         }
         else
          xx += x[i];
      }
    }

    x = xx ;
  }

  str = x ;
}

void
Split::exceptionError(std::string str)
{
  // Occurrence of an error stops the run immediately.
  xcptn.strError = "error_SPLIT.txt" ;

  // open file for appending data
  if( ! xcptn.ofsError )
    xcptn.ofsError
       = new std::ofstream(xcptn.strError.c_str(), std::ios::app);

  *xcptn.ofsError << str << std::endl;

  exit(1);
  return ;
}

std::vector<std::string>
Split::getItems(void)
{
  if( ! isDecomposed )
    decompose();

  return items;
}

std::string
Split::getLine(void)
{
  std::string s;
  for( size_t i=0 ; i < items.size() ; ++i)
    s += items[i] ;
  return s;
}

void
Split::init(void)
{
  xcptn.ofsError=0;

  isDecomposed=false;
  isFixedWidth=false;
  isEmptyItemsEnabled=false;
  is_valid=false;

  return;
}

bool
Split::isDigit(unsigned char c)
{
//  if(  c == '.' || c == '-' || c == '+'  )
//    return true;

  int i = c ;

  i -= 47 ;

  if( i > 0 && i < 11 )
    return true;
  else
    return false;
}

bool
Split::isNumber( size_t index )
{
  if ( index >= size() )
    return false;

  double val;

  return toDouble( index, val);
}

void
Split::getFixedFormat(void)
{
  size_t pos=0;

  while( pos < str.size() )
  {
    items.push_back( str.substr(pos,fixedWidth) );
    itemPos.push_back(pos);
    pos += fixedWidth;
  }

  return ;
}

std::string
Split::range(size_t i0, size_t i1 )
{
  // Get the substring between indicated separator positions.
  // The default for the second position is the end of the input string.
  // This holds also for a too high value of i1.
  if( i0 >= itemPos.size() )
    return "";
  else if( i1 == static_cast<size_t>(-1) || itemPos[ itemPos.size() - 1 ] < (i1+1) )
    return str.substr(itemPos[i0]);
  else
    return str.substr(itemPos[i0], itemPos[i1+1]);
}

void
Split::setIgnore( std::string s, bool isStr)
{
  isDecomposed=false;
  ignore.clear();

  addIgnore(s, isStr);

  return ;
}

void
Split::setSeparator( std::string s, bool isStr)
{
  isDecomposed=false;
  sep.clear();

  addSeparator(s, isStr);

  return ;
}

size_t
Split::size(void)
{
  if( str.size() > 0 && ! isDecomposed )
    decompose();

  return items.size();
}

bool
Split::toDouble( size_t i, double &retVal, size_t nr)
{
  if( i >= size() )
    return false;

  size_t pos0, pos1;
  pos0 = pos1 = 0;
  return toDouble( items[i], pos0, pos1, retVal, nr) ;
}

double
Split::toDouble( size_t i, size_t nr, bool isRetZero)
{
  if( i >= size() )
    return isRetZero ? 0. : MAXDOUBLE ;

  size_t pos0, pos1;
  pos0 = pos1 = 0;
  return toDouble( items[i], pos0, pos1, nr, isRetZero);
}

bool
Split::toDouble( std::string s,
     size_t &pos0, size_t &pos1, double &retVal, size_t nr)
{
  /* s   input string
    val  number (float or int); to be found and to be returned
     nr  index of the scanned number, counting from position 1.

  If no number was found:
    isReturnValNull:      true  ==> return 0.
                Default:  false ==> return MAXDOUBLE */

  double val=0.  ;

  for( size_t n=0 ; n<nr ; n++ )
  {
    //Code is based on a snippet from K&R: The C Programming Language

    for( ; ! ( isDigit(s[pos0])  ||  s[pos0] == '-'  || s[pos0] == '+'
            ||  s[pos0] == '.' ) ; pos0++  )
    {
      if( pos0 == s.size() )
        return false ;    // nothing found
    }

    pos1=pos0;

    // where the work is done
    val = toDoubleFct( s, pos1 );
  }

  retVal = val ;

  return true ;
}
double
Split::toDouble( std::string s,
  size_t &pos0, size_t &pos1, size_t nr, bool isReturnValNull)
{
  /*  s  input string
    val  number (float or int); to be found and to be returned
     nr  index of the scanned number, counting from position 1.

  If no number was found:
    isReturnValNull:      true  ==> return 0.
                Default:  false ==> return MAXDOUBLE */

  double val=0. ;

  for( size_t n=0 ; n<nr ; n++ )
  {
    for( ; ! ( isDigit(s[pos0])  ||  s[pos0] == '-'  || s[pos0] == '+'
            ||  s[pos0] == '.' ) ; pos0++  )
    {
      if( pos0 == s.size() )
        return isReturnValNull ? 0. : MAXDOUBLE ;  // not found
    }

    // where the work is done
    pos1=pos0;
    val = toDoubleFct( s, pos1 );
  }

  return val ;
}

double
Split::toDoubleFct( std::string s, size_t &pos )
{
  int sign ,power_count=0 ;
  double power , val;

  sign = (s[pos] == '-') ? -1 : 1 ;

  if( s[pos] == '+' || s[pos] == '-')
    ++pos ;

  for( val=0.0; isDigit(s[pos]); ++pos )
    val = 10.0 * val + (s[pos] - '0');

  if( s[pos] == '.' )
    ++pos ;

  for( power = 1. ; isDigit( s[pos] ); ++pos )
  {
    val = 10.0 * val + (s[pos] - '0' );
    power *= 10.0;
    power_count++;
  }

  // any E format?
  if( s[pos]=='e' || s[pos]=='E' )
  {
    ++pos;
    val = sign * val / power ;

    bool isSign = s[pos] == '+' || s[pos] == '-';

    if( ((pos+1) == s.size()) || (isSign && !isDigit(s[pos+1])) )
    {
      --pos ; // the detected 'e' or 'E' is not part of the number
              // although a sign follows, but without any digit
      return val;
    }

    if( ! ( isSign || isDigit(s[pos]) ) )
    {
      --pos ; // the detected 'e' or 'E' is not part of the number
      return val;
    }

    std::string t(s.substr(pos));

    // get power by a new query
    size_t hpos=0;
    val *= pow( 10. , toDoubleFct(t, hpos) );

    pos += hpos ;
/*
    ++pos1;
    while( (s[pos1] == '+' || s[pos1] == '-' || isDigit(s[pos1]) ) )
      ++pos1 ;   // clear the positions of th E format
*/

    return val ;
  }

  val = sign * val / power ;

  return val ;
}

bool
Split::toInt( size_t i,  int &retVal, size_t nr)
{
  double x;
  bool b=toDouble(i, x, nr);
  if( b )
  {
     retVal = static_cast<int>(x);
     return true;
  }
  else
    return false;
}

int
Split::toInt( size_t i, bool isRetZero, size_t nr)
{
//  double x=toDouble( i, isRetZero, nr=1);
  return static_cast<int>( toDouble( i, nr, isRetZero) ) ;
}
