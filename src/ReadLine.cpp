#include "readline.h"

ReadLine::ReadLine()
   : isExternalStream(false)
{
  FStream = new std::ifstream ;

  init();
}

ReadLine::ReadLine( std::string file )
   : isExternalStream(false)
{
  inFile = file ;

  FStream = new std::ifstream ;
  FStream->open( file.c_str());

  init();
}

ReadLine::ReadLine( std::ifstream &is )
   : isExternalStream(true)
{
  FStream = &is ;

  init();
}

ReadLine::~ReadLine()
{
  disconnect_cin();  // save, even if not existing

  if( stream )
    delete stream;

  if( ! isExternalStream )
  {
    close() ; // closes the ifstream obj, if existing
    delete FStream ;
  }

  return;
}

void
ReadLine::init()
{
  rangeFirst = 0. ;
  rangeLast = MAXDOUBLE ;
  rangeCol = 0  ;

  isBreakPoint=false;
  isClearSurroundingSpaces = false;
  isEof=false;
  isPutBackLine=false;
  isSkipBashComment = false;
  isSkipCharacter=false;
  isSkipWhiteLine = false;
  isRange = false ;
  isReadFloat=false;
  stream=0;

  if( FStream->is_open() )
  {
    save_sbuf_fin = FStream->rdbuf();

    stream = new std::istream( 0 );
    save_sbuf_stream = stream->rdbuf();
    stream->rdbuf( save_sbuf_fin );
  }
}

void
ReadLine::close()
{
  if( FStream->is_open() )
    FStream->close();
}

void
ReadLine::connect_cin(void)
{
  save_sbuf_cin = std::cin.rdbuf();
//  std::streamsize sz=save_sbuf_cin->in_avail();
//  int n=save_sbuf_cin->sbumpc();

  stream = new std::istream( save_sbuf_cin);
  return ;
}

void
ReadLine::disconnect_cin(void)
{
  if( stream )
  {
    std::cin.rdbuf(save_sbuf_cin);
    stream->rdbuf(save_sbuf_stream);
    delete stream;
    stream=0;
  }

  return ;
}

void
ReadLine::dismember(void)
{
  split = line ;

  isSplit=true;
}

bool
ReadLine::eof()
{
  // only if no line is currently put back is active
  if( ! isPutBackLine )
    isEof = stream->eof() ;

  return isEof ;
}

bool
ReadLine::findLine( std::string &rline, std::string &str, int opos )
{
  bool is;

  size_t i=0;
  if( opos > -1 )
    i = static_cast<size_t>( opos );

  size_t sz=str.size();

  while( ! (is=readLine()) )
  {
    if( !isSplit )
      dismember();

     if( opos < 0 )
     {
       if( line.find(str) < std::string::npos )
       {
          rline=line;
          return is;
       }
     }
     else
     {
       if( line.substr(i, sz) == str )
       {
          rline=line;
          return is;
       }
     }
  }

  return is;
}

std::string
ReadLine::getItem( size_t index)
{
   if( !isSplit )
      dismember();

   return split[index] ;
}

std::vector<std::string>
ReadLine::getItems( void)
{
   if( !isSplit )
      dismember();

   return split.getItems() ;
}

std::vector<std::string>
ReadLine::getItems( size_t beg, size_t end )
{
   if( !isSplit )
      dismember();

   if( end == 0 )
     end = split.size();

   std::vector<std::string> tmp ;
   for( size_t i=beg ; i < end ; ++i )
     tmp.push_back( split[i]) ;

   return tmp ;
}

bool
ReadLine::getLine( std::string &str0 )
{
  bool bret = readLine();

  if( !isSplit )
    dismember();

  if( isSkipCharacter )
  {
     str0.clear();
     for(size_t i=0 ; i < line.size() ; ++i )
     {
       bool is=true;
       for( size_t j=0 ; j < vSkipChars.size() ; ++j )
         if( line[i] == vSkipChars[j] )
           is=false;

       if( is )
         str0 += line[i] ;
     }

     line = str0;
  }

  if( isSkipWhiteLine )
    if( hdhC::isWhiteString( line ) )
      return getLine(str0) ; // safe for empty str0

  str0=line;

  if( isClearSurroundingSpaces )
     str0 = hdhC::stripSurrounding( str0 );

  return bret;
}

std::string
ReadLine::getPreviousLine( void )
{
   return prevLine ;
}

std::vector<double>
ReadLine::getValues( void )
{
   if( ! isReadFloat )
   {
     val.clear();

     for( size_t i=0 ; i < split.size() ; i++ )
       val.push_back( split.toDouble( i ) );
   }

   return val ;
}

double
ReadLine::getValue( size_t index )
{
   if( !isSplit )
      dismember();

   if( isReadFloat )
     return val[index] ;
   else
     return split.toDouble( index ) ;
}

bool
ReadLine::getValue( size_t index, double &v )
{
   if( !isSplit )
      dismember();

   if( isReadFloat )
     return val[index] ;
   else
     return split.toDouble( index, v ) ;
}

bool
ReadLine::open( void )
{
  if( inFile.size() > 0 )
  {
    FStream = new std::ifstream(inFile.c_str()) ;

    if( FStream->is_open() )
    {
      stream = new std::istream( FStream->rdbuf() );
      isReadFloat=false;
      val.clear();
    }
  }
  else
    stream = new std::istream( std::cin.rdbuf() );

  //method open():  returns true for failure
  return ! FStream->is_open() ;
}

bool
ReadLine::open( std::string f )
{
  // setFilename() calls open(). Returns true for failure
  return setFilename(f) ;
}

int
ReadLine::peek(void)
{
  if( isPutBackLine )
    return line[0];
  else
    return stream->peek();
}

void
ReadLine::putBackLine(void)
{
   swapLine();

   isPutBackLine=true;
   isSwappedEof=isEof;
   isEof=false;
}

bool
ReadLine::readLine(void)
{
  // return: true for inStrem->eof()

  if( isPutBackLine )
  {
    swapLine();  // de-swaps line an prevLine
    isPutBackLine=false;
    isEof=isSwappedEof;
    return false;
  }

  char cbuf ;
  isSplit=false;

  prevLine = line;
  line.erase();

  bool skip=false;

  // variable number of columns
  while( !(isEof = stream->eof()) )
  {
    cbuf = stream->get();

    // skip fom #-char to the end of tzhe line
    if( isSkipBashComment && cbuf == '#' )
      skip=true;
    if( skip  && ( cbuf == '\n' || cbuf == '\r' )
    {
      if( (cbuf = stream->peek()) == '\n' )
        cbuf = stream->get();

      return false;
    }

    if( breakPoint )
    {
      // read across lines to the breakpoint
      if( cbuf == '\n' || cbuf == '\r' )
       continue;

      if( cbuf == breakPoint )
        return false ;  // Zeile erfolgreich gelesen
    }
    else if( cbuf == '\n' || cbuf == '\r' )
    {
      if( (cbuf = stream->peek()) == '\n' )
        cbuf = stream->get();

      return false ;  // Zeile erfolgreich gelesen
    }

    line += cbuf ;  // line enthaelt alle SPACE-Chars
  }

  if( (isEof=stream->eof()) )
    return true ;

  return false ;
}

bool
ReadLine::readFloat( void )
{
  // Initialisation
  if( ! isReadFloat )
  {
    // if the begin of a range was searched for, then there was already a readLine
    if( readLine() )
      return true ;
    else if( !isSplit )
      dismember();

    for( int i=0 ; i < noOfCols ; ++i )
      val.push_back( getValue(i) ) ;

    while( isRange && getValue(rangeCol) < rangeFirst   )
    {
      for( int i=0 ; i < noOfCols ; ++i )
        *stream >> val[i] ;

      if( (isEof=stream->eof()) )
          return true ;

    }

    isReadFloat = true ;  //must be after the for-loop !!

    return false ;
  }

  // return: true for stream->eof()

  for( int i=0 ; i < noOfCols ; ++i )
    *stream >> val[i] ;

  if( (isEof=stream->eof()) )
      return true ;

  // user defined stop of reading
  if( isRange && rangeLast < getValue(rangeCol) )
    return true  ;

  return false ;
}

void
ReadLine::rewind( void )
{
  stream->clear();
  stream->seekg(std::ifstream::beg);
  return;
}

bool
ReadLine::setFilename( std::string f )
{
  if( f.size() > 0 )
    inFile=f;

  //method open() --> setFilename()  return true for failure
  return open() ;
}

bool
ReadLine::setRange( double wave0, double wave1, size_t col )
{
  rangeFirst = wave0 ;
  rangeLast  = wave1 ;
  rangeCol   = col   ;
  isRange    = true;

  return true ;
}

bool
ReadLine::setRange( std::string & s )
{
   isRange = true;

   rangeFirst = hdhC::string2Double(s,1) ;

   double aa;

   if( hdhC::string2Double(s, 2, &aa) ) // returned bool
     rangeLast = aa ;

   if( hdhC::string2Double(s, 3, &aa) )  // returned bool
     rangeCol = (int)aa - 1 ;

   return true ;
}

bool
ReadLine::skipLines( int count )
{
   for( int i=0 ; i < count ; i++)
   {
     if( readLine() )
       return true;  // EOF
     else if( !isSplit )
       dismember();
   }

   return false ;
}

void
ReadLine::swapLine(void)
{
   line.swap(prevLine);

   split = line ;
   return;
}
