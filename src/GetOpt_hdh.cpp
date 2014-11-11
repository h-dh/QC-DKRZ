#include "getopt_hdh.h"

class GetOpt_Err
{
public:
  GetOpt_Err( std::string t){ s=t ;}

  void print_message( )
    { std::cerr << s.c_str() << std::endl; exit(1); }

private:
  std::string s ;
};

GetOpt::GetOpt()
{
  priv_optind=1;
  priv_optopt=0;

  optind=1; // points to the next
  longOption=0;
  optarg=0;
  opterr=1;

  isParsed=false;
  isFinal=false;
}

GetOpt::~GetOpt()
{
}

int
GetOpt::getopt( int argc, char *argv[], const char *optstr)
{
  if( ! isParsed )
  {
    parseOptString( optstr);
    effArgc = argc ;
  }

  longOption = 0 ;  //default for no long option
  int tmp =  parseParameterList( argc, argv, optstr ) ;

  optind = priv_optind ;
  optopt = priv_optopt ;

  return tmp ;
}

void
GetOpt::parseOptString(const char * optstr )
{
  std::string t( optstr ) ;

  for( size_t i=0 ; i < t.size() ; i++)
  {
    if( t.at(i) == ':' ) 
    {
      try
      {
        std::string st("GetOpt::parseOptString()\n" );
        st += "Argument without option in option string" ;
        throw GetOpt_Err(st ) ;
      }
      catch( GetOpt_Err e)
      {
         e.print_message();
      }
    }
    else if( t.at(i) == '<' )   // long option identification
    {
      size_t pos0=i+1, pos1;

      // long option
      if( t.at(i+1) == '-' )
        pos0 = i+2;
      if( t.at(i+2) == '-' )
        pos0 = i+3;
      pos1=t.find('>',pos0);
      if( pos1 >= t.size() )
      {
        try
        {
          std::string st("GetOpt::parseOptString()\n");
          st += "long options in option string must be given in <>" ;
          throw GetOpt_Err( st );
        }
        catch( GetOpt_Err e)
        {
          e.print_message();
        }
      }

      //vector with long options
      std::string u("--");
      u += t.substr(pos0,pos1-pos0) ;
      legalOpt.push_back( u );
      isLegalLongOpt.push_back(true);

      // long option argument required?
      if( (pos1+1)<t.size() && t.at(pos1+1) == ':' )  // at pos1+1 we find '>'
      {
        i = pos1+1 ; //update of loop count.
        isLegalOptArg.push_back( true );
      }
      else
      {
        i = pos1 ; //update of loop count
        isLegalOptArg.push_back( false );
      }
    }
    else  // short option identification
    {
      //vector with short options
      legalOpt.push_back( t.substr(i,1) );
      isLegalLongOpt.push_back( false );

      // short option argument required?
      if( i<(t.size()-1) && t.at(i+1) == ':' )
      {
        ++i  ; //update of loop count
        isLegalOptArg.push_back( true );
      }
      else
        isLegalOptArg.push_back( false );
    }
  }

  isParsed = true ;

  return ;
}

int
GetOpt::parseParameterList(int argc, char *argv[], const char * optstr )
{
  if( isFinal )
    return -1 ;

  if( priv_optind == effArgc )
  {
    // restore the nonOptArgvs
    restoreNonOptArg(argc, argv);
    
    isFinal = true ;
    return -1 ;
  }

  bool isNonOpt;

  do
  {
    isNonOpt = false;
    std::string t(argv[priv_optind]);
    thisArgv = argv[priv_optind];

    if( priv_optind == effArgc-1 )
      nextArgv = 0 ;
    else
      nextArgv = argv[priv_optind+1] ;

    if( t == "-" )  // ToDo
    {  
      ++priv_optind ;
      restoreNonOptArg(argc, argv);
      isFinal=true;
      return -1 ;
    }

    if( t == "--" )  // forced end of option scanning
    {
      ++priv_optind ;

      // push all remaining argv's into the nonOptArg
      for( int j=priv_optind ; j<effArgc ;j++)
        nonOptArg.push_back( argv[j]);  //collect non-option argv's

      restoreNonOptArg(argc, argv);

      isFinal=true;
      return -1 ;
    }

    // investigate the next option.
    // update of priv_optind in the methods.
    if( t.at(0) == '-' )
    {
       if( scanLongOpt( t ) )
         return 0 ;
       else if( scanShortOpt(t) )
         return thisShortOpt ;
       else
         return '?';  
    }
    else
    {
      shiftArgv( argc, argv ) ;

      isNonOpt = true ;
    }
  } while( isNonOpt && priv_optind < effArgc ) ;

  return -1; 
}

bool
GetOpt::scanLongOpt( std::string& t )
{
  if( t.at(1) != '-' )  // not a long option
    return false ;

  // scan through and compare with legal long option arguments
  size_t myIndex=0;
  for( ; myIndex < legalOpt.size() ; myIndex++ )
  {
    if( isLegalLongOpt[myIndex] && t.substr(0,t.find('=')) == legalOpt[myIndex] )
      break;   // legal long option found
  }

  if( myIndex == legalOpt.size() )
  { // error: no long option
     if( opterr != 0 )
     {
       try
       {
         std::string st("GetOpt::scanLongOpt(): long option ");
         st += t ;
         st += " is not declared in option string" ;

         throw  GetOpt_Err( st );
       }
       catch( GetOpt_Err e)
       {
         e.print_message();
       }
     }

     priv_optopt='?';
     optarg=0;
     longOption=thisArgv ;
     return true;
  }

  ++priv_optind;

  // no arg for long option required
  if( ! isLegalOptArg[myIndex] )
  {
    // error, if an argument is attached
    if( t.size() > legalOpt[myIndex].size() )
    {
      try
      {
        std::string st("GetOpt::scanLongOpt(): long option ");
        st += legalOpt[myIndex] ;
        st += " is declared to have no option, but " ;
        st += t ;

        throw  GetOpt_Err( st );
      }
      catch( GetOpt_Err e)
      {
        e.print_message();
      }
    }

    optarg = 0;
    longOption = thisArgv ;
    return true  ;
  }

  // look for '='. If the last character is '=', then arg is empty.
  // If '=' is not last character, the the argument is attached to the long option.
  // if '=' was not found, then the nextArgv is an argument

  size_t pos = t.find('=') ;
  if( pos < t.size() )
  {
    optarg = &thisArgv[pos+1] ; 
    attachedLongOpt=thisArgv;
    attachedLongOpt = attachedLongOpt.substr(0,pos);
    longOption=attachedLongOpt.c_str() ;
  }
  else 
  {
    ++priv_optind;
    longOption=thisArgv;
    optarg =  nextArgv ; 

    if( optarg == 0 )
    {
      try
      {
        std::string st("GetOpt::scanLongOpt():\n long option ");
        st += t ;
        st += " is dedclared to have an argument." ;

        throw  GetOpt_Err( st );
      }
      catch( GetOpt_Err e)
      {
        e.print_message();
      }
    }
  }

  return true ;
}

bool
GetOpt::scanShortOpt( std::string& t)
{
  // scan through and compare with legal long option arguments

  size_t myIndex=0;
  for( ; myIndex < legalOpt.size() ; myIndex++ )
  {
    if( t.substr(1,1) == legalOpt[myIndex] )
      break;   // legal option found
  }

  if( myIndex == legalOpt.size() )
  { // error: no short option
     if( opterr != 0 )
     {
       try
       {
         std::string st("GetOpt::scanShortOpt(): option ");
         st += t ;
         st += " is not declared in option string" ;

         throw  GetOpt_Err( st );
       }
       catch( GetOpt_Err e)
       {
         e.print_message();
       }
     }

     priv_optopt= t[1];
     optarg=0;
     return false;
  }

  ++priv_optind;
  thisShortOpt=t[1];

  // no arg for option required
  if( ! isLegalOptArg[myIndex] )
  {
     optarg = 0;
     return true  ;
  }

  // the argument may be attached to the option or may be the next argument
  // e.g. -cArg or -c Arg

  if( t.size() > 2 )
  {
    optarg = &thisArgv[2] ; 
  }
  else 
  {
    try
    {
      optarg = nextArgv ; 
      ++priv_optind;

      if( nextArgv == 0 )
      {
        std::string st("GetOpt::scanShortOption(): option ") ;
        st += t;
        st += " is declared to have an argument" ;
        throw GetOpt_Err( st );
      }
    }
    catch( GetOpt_Err e)
    {
      e.print_message();
    } 
  }

  return true ;
}

void
GetOpt::shiftArgv( int argc, char *argv[] )
{
  nonOptArg.push_back(thisArgv);  //collect non-option argv's
  --effArgc ;  //first decrement then use in the for-loop

  // remove non-optArgv from the list and close the gap
  for( int j=priv_optind ; j<effArgc ; ++j)
    argv[j] = argv[j+1] ;
    
  // restore the nonOptArgvs
  if( priv_optind == effArgc )
    restoreNonOptArg(argc, argv);

  return;
}

void
GetOpt::restoreNonOptArg( int argc, char *argv[] )
{
  // restore the nonOptArgvs
  int i=0;
  for( int j=priv_optind ; j<argc ; ++j, ++i)
    argv[j] = nonOptArg[i] ;
    
  return;
}
