#ifndef _READLINE_H
#define _READLINE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <stddef.h>
#include <math.h>
#include <values.h>

#include "hdhC.h"
#include "split.h"

/*! \file readline.h
 \brief Reading of data: file and standard input.
*/

//! Access to std::cin and reading of files.
/*! std::cin and files, respectively, are read line-by-line. Both
 unix-like line-end styles and that of MS are accepted. Reading
 from std::cin may be swichted on and off.
 After reading a line, various properties may be retrieved, also
 the previous line. Putting back a line is possible.*/

class ReadLine
{
private:
  // Input streams
  std::streambuf *save_sbuf_cin;
  std::streambuf *save_sbuf_fin;
  std::streambuf *save_sbuf_stream;

  std::istream   *stream ;


public:
//! Default contructor.
  ReadLine();  // use ReadLine.setFilename() to open ifstream

//! Construct and open file.
  ReadLine( std::string filename, bool testExistence=true );

//! Construct and connect to an in-stream.
  ReadLine( std::ifstream & );

//! Destructor-
  ~ReadLine();

//! The currently activated stream
  std::ifstream
    *FStream ;

//! Get char at i-th position from last line read.
  char&
    at(size_t i){ return line.at(i); }

//! Discard white characters in front and behind a line.
  void
    clearSurroundingSpaces( void ){ isClearSurroundingSpaces=true; }


//! Close an open stream.
/*!Safe if not open.*/
  void
    close(void);

//! Connect to std::cin.
  void
    connect_cin(void);

//! Disconnect from std::cin
  void
    disconnect_cin(void);

//! Return eof state from stream.
  bool
    eof(void);

  void
    feedStream(void);

//! Find a string in the line.
/*! The file is sequentielly searched forward line by line.
    A negative opos indicates a search anywhere in the line.*/
  bool
    findLine( std::string &line, std::string &str, int opos=0 );

//! Get the name of the opened file.
  std::string
    getFilename(void){return inFile; }

//! Get i-th item from line.
  std::string
    getItem(size_t i) { return split[i]; }

//! Get a vector filled with the line items.
  std::vector<std::string>
    getItems(void) { return split.getItems(); }

//! Get range of items filled into a vector.
/*! By default, range starts at index 'beg' to the end of the line.*/
  std::vector<std::string>
    getItems( size_t beg, size_t end=0); // end=0:

//! Get the last read line.
  std::string
    getLine( void ){return line;}

  bool
    getLine( std::string& );

//! Get the previous line.
/*! Between a 'putBackLine' directive and the next reading of a line,
 this method will get the next line, because the next line
 and the previous one are simply swapped.*/
  std::string
    getPreviousLine( void );

//! Get all items from the last line converted to double.
  std::vector<double>
    getValues( void );

//! Get the i-th line item converted to double.
  double
    getValue( size_t i );

//! Put the i-th line value to variable pointed to by 'p'.
/*! If the item could be converted successfully into a double, then
 true is returned, and the number is stored in *p.*/
  bool
    getValue( size_t i, double &p  );

//! Full access to the ifstream object.
  std::ifstream   *ifstream ;

//! Is stream open?
  bool
    isOpen(void) { return is_fopen; }

//! Open file or stream.
/*! Only if a filename was set previously, the file is opened.*/
  bool
    open( void ) ;

//! Open file.
  bool
    open( std::string f );

 ////! Open file.
//  bool
//    open( const char *); //{ setFilename(c); return open();}

//! Show the next character in the stream.
  int
    peek(void);
//  void                     readBinaryLine(void);

//! Put back last read line.
/*! After a line was put back, line and previous line are
 identical until the next read directive. Eof value and
 peeking the next charcter is taken into account.*/
  void
    putBackLine();

//! Read next line (external call).
  bool
    readLine(void);

//! Read next line (internal call).
  bool
    readLine(bool doSplit);

//! Read line consisting of numbers only.
/*! Only floats/double must be in the file.
    Uses std::cin assignments.*/
  bool
    readFloat(void);

  bool
    readFloat(bool);

//! Set stream to the beginning.
  void
    rewind(void);

//! Set a breakpoint within a line
  void
    setBreakpoint(char b){ breakpoint=b; isBreakpoint=true;}

//! Set filename.
  bool
    setFilename( std::string="", bool testExistence=true );

//! Set a range of line numbers for reading.
/*! The range may be time or wavenumber. All lines outside
 the range are discarded. Parameter 'col' is the column
 number, where to find the range forming values.*/
  bool
    setRange( double wave0, double wave1, size_t col );

//! Set range.
/*! The string contains begin, end and column of a range, each
 separated by any character(s) not part of a scientific number.*/
  bool
    setRange( std::string & );

//! Get number of items in the line.
  size_t
    size(void){ return split.size(); }

//! Discard Bash comments.
/*! From the first appearance to the end of the line.*/
    void
    skipBashComment( void ){ isSkipBashComment=true; }

    //! Discard characters.
/*! From the first appearance to the end of the line.*/
    void
    skipCharacter( char c){ vSkipChars.push_back(c); isSkipCharacter=true; }

/*! Note that each character is discrded, not just the string */
    void
    skipCharacter(std::string);

//! Skip the first int lines.
  bool
    skipLines( int );  // skip the first int lines

//! Skip white lines.
/*! A white line may consist only of space[s] and tab[s].\n
    Returns an empty string. */
  void
    skipWhiteLines( void ){ isSkipWhiteLine=true; }

//! Full access to the Split class.
  Split split;

//! Get substring of original line.
  std::string
    subLine(size_t pos){ return line.substr(pos);}

//! Get substring of original line.
  std::string
    subLine(size_t pos, size_t n){ return line.substr(pos, n);}

//! Unset a breakpoint within a line
  void
    unsetBreakpoint(void){ isBreakpoint=false;}

private:
  // Input filenames
  std::string inFile ;
  std::string line ;
  std::string prevLine ;
  char breakpoint;
  std::vector<char> vSkipChars;
  std::vector<double> val ; //line: converted values

  double rangeFirst;      // Beginn des Spektrums ...
  double rangeLast ;      // Ende des Spektrums ...
  int    rangeCol ;       // Spalte fuer range Bestimmung
  bool   isRange ;        // true: range wird abgefragt
  bool   is_fopen;

  bool isBreakpoint;
  bool isClearSurroundingSpaces;
  bool isEof;
  bool isExternalStream;
  bool isPutBackLine;
  bool isReadFloat ;
  bool isSkipBashComment;
  bool isSkipCharacter;
  bool isSkipWhiteLine;
  bool isSwappedEof;

  int  noOfCols;

  void init(void);
  void swapLine(void); // swaps line and prevLine
};

#endif
