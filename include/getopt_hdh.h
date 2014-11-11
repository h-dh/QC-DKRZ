#ifndef _GETOPT_HDH_H
#define _GETOPT_HDH_H

#include <iostream>
#include <string>
#include <vector>

/*! \file getopt_hdh.h
 \brief Parsing of optional parameters.
*/

//! Parse command-line options.

/*! Note: This was one the earliest approaches of an apprentice in C++.\n
 GetOpts is a kind of re-engeneering of the shells' getop(s) version.
 It is used the same way in a while loop.\n
 GetOpt excepts one-character (short) options as well as long options,
 e.g. --long. Long, short and non-option arguments may be mixed
 in the parameter list argv.\n
 Short options are returned by getopt() as int, whereas long options
 are stored in const char* longOptarg (null is
 returned, if not avaialable).
 The char* optarg points to the argument of both short option and long
 option. The value of the index to the next argv element is stored in
 optind. After processing all options, optind indexes the first
 non-option argument if any.\n
 The option string identifies (short) options by the option character,
 long options by the full option name in <>, and arguments of (long and
 short) options by a colon. Long option parameters may be given by a
 separate token or attached by '='.\n
 Example:   char *optionestring="ab:<--get>:<set>" is for options\n
    -a\n
    -b   bArg\n
   --get getArg   or as well  --get=getArg\n
   --set\n
 A '--' in the argv list stops option parsing. The index of the next
 argv following '--' is stored in optind and all remaining argv's are
 treated as non-options.\n
 The '-' feature as well as POSIX extensions are not implemented.\n
 Return values: If there are no more options in argv, optind returns
 -1. If getopt() does not recognize a short option character, it prints
 an error message to stderr, stores the character in optopt, and
 returns `?'. If getopt() does not recognize a long option,
 an error message is printed to stderr, longOption points to the
 unknown option, and optarg points to null.\n
 The calling program may inhibit error messages by setting opterr to 0.
*/

class GetOpt
{
public:
  //! Default constructor
  GetOpt();
  //! Destructor
  ~GetOpt();

  //! Analyse provided options argv on the basis of the options string s.
  int   getopt( int argc, char *argv[], const char *s);

  //! Pointer to the argument of the current option (null for NONE).
  char* optarg;

  //! longOptarg points to the long option inclusively leading '--'.
  const char* longOption ;
  /*! Long options may be given on the command-line as e.g. --myOption.
      Long options with argument may be given as '--myOption
      theArgument' or as '--myOption=theArgument'.*/

  //! index of the next argv element.
  int   optind;

  //! Set to 0 suppresses error messages to stderr. Def.: 1
  int   opterr;

  //! Contains the option character, if an unknown option was found
  int   optopt;

private:
  int   priv_optind;        // index to the next argv element.
  int   priv_opterr;        // if set to 0 suppresses error messages to stderr. Def.: 1
  int   priv_optopt;        // contains the option character, if an unknown option was found

  void                     parseOptString( const char *);
  std::vector<std::string> legalOpt;
  std::vector<bool>        isLegalOptArg;
  std::vector<bool>        isLegalLongOpt;
  std::vector<bool>        isLegalOptionalOpt;

  std::vector<char*>       nonOptArg;    // collects nonOptArgv's
  int                      effArgc     ; // argc minus number of nonOptArgvs
  std::string              attachedLongOpt;  // place holder for option attached as 
                                             // in e.g. --abc=arg

  int                      parseParameterList(int argc, char *argv[], const char *);
  bool                     scanLongOpt( std::string& );
  bool                     scanShortOpt( std::string&);

  bool                     isParsed ;
  bool                     isFinal ;
  int                      thisShortOpt;
  char*                    thisArgv; // a pointer to the actual option in argv 
  char*                    nextArgv; // a pointer to the actual arg of option in argv item
  void                     restoreNonOptArg(int argc, char *argv[]);
  void                     shiftArgv( int argc, char *argv[] );
};

#endif
