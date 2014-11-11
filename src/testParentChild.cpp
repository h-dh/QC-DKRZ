#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "hdhC.h"
#include "date.h"
#include "getopt_hdh.h"
#include "matrix_array.h"
#include "nc_api.h"
#include "annotation.h"
#include "readline.h"
#include "split.h"

#include "hdhC.cpp"
#include "Date.cpp"
#include "GetOpt_hdh.cpp"
#include "Annotation.cpp"
#include "NcAPI.cpp"
#include "ReadLine.cpp"
#include "Split.cpp"


/*! \file testParentChild.cpp
 \brief Check time lag between a parent and child experiment.

 Values of time steps and time bounds are checked. Consistancy
 is checked between the last step of the parent file, the first step
 of the child, and the lag across the two. Additionally,
 non-overlapping of the corresponding time bounds is testet.\n
 Options: \n
  -c netCDF data file of the child experiment.\n
  -p netCDF data file of the parent experiment.
*/

struct Obj
{
  Obj() : nc_c(), nc_p(), refDate_c(), refDate_p(), isOpen(false) {;}

  std::string file_c;
  std::string file_p;

  NcAPI nc_c;
  NcAPI nc_p;
  bool isOpen;

  Date refDate_c;
  Date refDate_p;

  std::string units_c;
  std::string units_p;

  double refTimeDelay;

  size_t index_p;
  double t0_c;
  double t1_p ;
} ;

void    checkTime(Obj &);
void    description(void);
void    exceptionError(std::string );
void    exceptionWarning(std::string );
void    finally(Obj &o, std::string &flag, int eCode);
void    getTimeProperties(Obj &);
void    printFlag(std::string&);
void    sync(Obj &);

std::ofstream *ofsError=0;
std::ofstream *ofsWarning=0;

std::string strError = "qc_parent_error_";
std::string strWarning = "qc_parent_warning_";
std::string filename;

/*! \file testParentChild.cpp
\brief Test proper continuation across successive experiments.
 
Times and time boundaries (the latter if available) towards the end \n
of the parent file and those at the beginning of the experiment \n
of the child are compared. Different reference dates by the unit \n
attributes of time in each file are possible. \n
The files are synchronised in a way that the child may start from \n any time value of the parent (e.g. forking from a control run).\n
Requirements (to be given by options):\n
  -c netCDF data file of the child experiment.\n
  -p netCDF data file of the parent experiment.*/

int main(int argc, char *argv[])
{
  Obj obj;

  std::string flag; //is used in any case; at least empty

  if( argc == 1 )
  {
    description();
    return 3;
  }

  GetOpt opt;
  opt.opterr=0;
  int copt;
  std::string str0;
  bool not_c=true;
  bool not_p=true;

  while( (copt = opt.getopt(argc, argv, "c:hp:<--help>")) != -1 )
  {
    if( opt.longOption > 0 )
      str0=opt.longOption;

    if( opt.longOption && str0 == "--help" )
    {
      description();
      return 0;
    }

    switch ( copt )
    {
      case 'c':
        obj.file_c = opt.optarg ;
        filename = obj.file_c ; //for error messaging
        not_c=false;
        break;
      case 'h':
        description();
        return 0;
        break;
      case 'p':
        obj.file_p = opt.optarg ;
        not_p=false;
        break;
      default:
        std::ostringstream ostr(std::ios::app);
        ostr << "testExpCont:getopt() unknown option -" ;
        ostr << copt;
        ostr << "\nreturned option -" << copt ;
        exceptionError( ostr.str() ); 
        break;
    }
  }

#ifndef RAISE_ERRORS
  if( not_c )
#endif
  {
    std::string note("W30_5: Missing command-line parameter for the child experiment: -c file.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::main()" ;
    exceptionError( ostr.str() );
    flag=note ;
  }

#ifndef RAISE_ERRORS
  if( not_p )
#endif
  {
    std::string note("W30_6: Missing command-line parameter for the parent experiment: -p file.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::main()" ;
    if( flag.size() )
      flag += ";";
    flag += note ;
    exceptionError( ostr.str() );  
  }

#ifndef RAISE_ERRORS
  if( not_c || not_p )
#endif
    finally(obj, flag, 30);

#ifndef RAISE_ERRORS
  if( !
#endif
obj.nc_c.open( obj.file_c )
#ifdef RAISE_ERRORS
;
#endif 
#ifndef RAISE_ERRORS
    )
#endif
  {
    std::string note("W31_1: Could not open the netCDF file of the child experiment");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::main()" ;
    ostr << "\nFile: "  << obj.file_c ;
    exceptionError( ostr.str() );  
    finally(obj, note, 31);
  }

#ifndef RAISE_ERRORS
  if( !
#endif
obj.nc_p.open( obj.file_p )
#ifdef RAISE_ERRORS
;
#endif 
#ifndef RAISE_ERRORS
    )
#endif
  {
    std::string note("W31_2: Could not open the netCDF file of the parent experiment");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::main()" ;
    ostr << "\nFile: "  << obj.file_p ;
    exceptionError( ostr.str() );  
    finally(obj, note, 31);
  }

  obj.isOpen=true;

  // time delay between the two reference dates; store in obj
  getTimeProperties(obj);

  // synchronise two time step streams to continuation/fork
  // positions
  sync(obj);

  // eventually check times across the files
  checkTime(obj);

  finally(obj, flag, 0);

}

void
checkTime(Obj &obj)
{
  // Check the lag across the files
  MtrxArr<double> mv;

  // The last time step of the parent before the child begins.
  double lag_p = obj.t1_p
                   - obj.nc_p.getData(mv, "time", obj.index_p -1);
  double lag_pc = obj.t0_c - obj.t1_p ;

  double epsilon=1.e-10;

#ifndef RAISE_ERRORS
  if( ! hdhC::compare(lag_p, lag_pc, "==", epsilon) )
#endif
  {
    std::string mess("The lag across files differs from the last lag in the parent.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\nW36: " << mess ;
    ostr << "\ntestExpCont::checkTime" ;
    ostr << "\nParent:"     ;
    ostr << "\n   File:          " << obj.file_p ;
    ostr << "\n   Rec#:          " << obj.index_p ;
    ostr << "\n   Time:          " << obj.t1_p ;
    ostr << "\n   Lag before:    " << lag_p ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:          " << obj.file_c ;
    ostr << "\n   First time:    " << obj.t0_c ;
    ostr << "\nLag across files: " << lag_pc ;
    exceptionError( ostr.str() );  

    std::string note("W36: ");
    note += "rec# (parent)";
    note += hdhC::double2String(obj.index_p) + ": ";
    note += mess ;
    std::cout << note << ";" << std::endl;
#ifndef RAISE_ERRORS
    finally(obj, note, 36);
#endif
  }

  // check the stepping across the files
  double lag_c = obj.nc_c.getData(mv, "time", 1) - obj.t0_c ;

#ifndef RAISE_ERRORS
  if( ! hdhC::compare(lag_p, lag_c, "==", epsilon) )
#endif
  {
    std::string mess("The last lag of the parent differs from the first lag of the child.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\nW37: " << mess ;
    ostr << "\ntestExpCont::checkTime" ;
    ostr << "\nParent:" ;
    ostr << "\n   File:       " << obj.file_p ;
    ostr << "\n   Rec#:       " << obj.index_p ;
    ostr << "\n   Time:       " << obj.t1_p ;
    ostr << "\n   Lag before: " << lag_p ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:       " << obj.file_c ;
    ostr << "\n   First time: " << obj.t0_c ;
    ostr << "\n   First lag:  " << lag_c ;
    exceptionError( ostr.str() );  

    std::string note("W37: ");
    note += "rec# (parent)";
    note += hdhC::double2String(obj.index_p) + ": ";
    note += mess ;
    std::cout << note << ";" << std::endl;
#ifndef RAISE_ERRORS
    finally(obj, note, 37);
#endif
  }

  // overlapping time bounds across experiments

  // get the name of the time bounds variable
  std::string bounds( obj.nc_p.getAttString("bounds", "time") );
#ifndef RAISE_ERRORS
  if( bounds.size() == 0 )
#endif
  {
    std::string note("W38_1: No bounds attribute of time of the parent.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note;
    ostr << "\ntestExpCont::checkTime()";
    ostr << "\nFile (parent):" << obj.file_p ;
    exceptionWarning( ostr.str() );
    std::cout << note << ";" << std::endl;
#ifndef RAISE_ERRORS
    return;
#endif
  }

  obj.nc_p.getData(mv, bounds, obj.index_p);

  double tB_p[2];
  tB_p[0]=mv[0] ;
  tB_p[1]=mv[1] ;

  // same for the child
  bounds.clear();
  bounds = obj.nc_c.getAttString("bounds", "time");
#ifndef RAISE_ERRORS
  if( bounds.size() == 0 )
#endif
  {
    std::string note("W38_2: No bounds attribute of time of the child.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note;
    ostr << "\ntestExpCont::checkTime()";
    ostr << "\nFile (child):" << obj.file_c ;
    exceptionWarning( ostr.str() );
    std::cout << note << ";" << std::endl;
#ifndef RAISE_ERRORS
    return;
#endif
  }

  obj.nc_c.getData(mv, bounds, 0);

  double tB_c[2];
  tB_c[0]=mv[0] + obj.refTimeDelay;
  tB_c[1]=mv[1] + obj.refTimeDelay;

#ifndef RAISE_ERRORS
  if( tB_c[0] < tB_p[1] )
#endif
  {
    std::string mess("Time bound lags of the last parent lag and the first lag of the child overlap.");

    std::ostringstream ostr(std::ios::app);
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr << std::setprecision(0) ;

    ostr << "\nW38_3: " << mess;
    ostr << "\ntestExpCont::checkTime()";
    ostr << "\nParent:" ;
    ostr << "\n   File:        " << obj.file_p ;
    ostr << "\n   Units:       " << obj.units_p ;
    ostr << "\n   Rec#:        " << obj.index_p ;
    ostr << "\n   Time Bounds: " << tB_p[0] << " - " << tB_p[1] ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:        " << obj.file_c ;
    ostr << "\n   Units:       " << obj.units_c ;
    ostr << "\n   Rec#:        " << 0 ;
    ostr << "\n   Time Bounds: " << tB_c[0] << " - " << tB_c[1] ;

    std::string note("W38_3: ");
    note = "Rec# (parent)";
    note += hdhC::double2String(obj.index_p);
    note += ": ";
    note += mess;
    exceptionWarning( ostr.str() );
    std::cout << note << ";" << std::endl;
  }

  return;
}

void description(void)
{
  std::cout << "usage: testExpCont.x -c file -l file -p file\n";
  std::cout << "Test proper continuation across successive CMIP5\n";
  std::cout << "Only time values are compared. Improper file\n";
  std::cout << "types, i.e. frequency, ensemble member etc., are\n";
  std::cout << "not checked. The files are synchronised in a way that\n";
  std::cout << "the child may start from any time value of the\n";
  std::cout << "parent (e.g. forking from a control run).\n";
  std::cout << "-c      netCDF data file of the successive experiment.\n";
  std::cout << "-h      This text.\n";
  std::cout << "--help  This text.\n";
  std::cout << "-p      netCDF data file of the parent experiment.";
  std::cout << std::endl;
}

void
exceptionError(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is due to exit.
  static bool doInit=true;
  if( doInit )
  {
     // base name if available, i.e. after init of the InFile obj
    if( filename.size() > 0 )
      strError += hdhC::getBasename(filename) ;
    strError += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! ofsError )
    ofsError = new std::ofstream(strError.c_str());

  *ofsError << str << std::endl;

  return;
}

void
exceptionWarning(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is due to exit.
  static bool doInit=true;
  if( doInit )
  {
     // base name if available, i.e. after init of the InFile obj
    if( filename.size() > 0 )
      strWarning += hdhC::getBasename(filename) ;
    strWarning += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! ofsWarning )
    ofsWarning = new std::ofstream(strWarning.c_str());

  *ofsWarning << str << std::endl;

  return;
}

void
finally(Obj &obj, std::string &flag, int eCode)
{
#ifndef RAISE_ERRORS
  if( obj.isOpen )
  {
    obj.nc_c.close();
    obj.nc_p.close();
  }
#endif

  if( eCode  )
  {  // forced exit
    std::cout << "abortedExecution;" ;

    // print error flag a mark the end of the output line
    std::cout << flag << ";" << std::endl;
#ifndef RAISE_ERRORS
    exit(eCode);  // short-cut
#endif
  }

  std::cout << "normalExecution;" ;

#ifndef RAISE_ERRORS
  exit(0);
#endif

  return;
}

void
getTimeProperties(Obj &obj)
{
  // time properties.
  // Also the delay, in terms of time unit measure of the first file,
  // e.g. day, between the reference dates given by the time unit
  // attributes.

  obj.refTimeDelay=0.; // pre-set

  // Gregorian, no-leap, 360_day
  std::string calendar_c( obj.nc_c.getAttString("calendar", "time") );
  std::string calendar_p( obj.nc_p.getAttString("calendar", "time") );

#ifndef RAISE_ERRORS
  if( calendar_p != calendar_c )
#endif
  {
    std::string note("W32: Different calendar types in parent and child.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::getTimeProperties()" ;
    ostr << "\nParent:" ;
    ostr << "\n   File:     "  << obj.file_p ;
    ostr << "\n   Calendar: "  << calendar_p ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:     "  << obj.file_c ;
    ostr << "\n   Calendar: "  << calendar_c ;
    ostr << "\nNote: this is adjusted." ;
    exceptionWarning( ostr.str() );
    std::cout << note << ";" << std::endl;
  }

  obj.units_p = obj.nc_p.getAttString("units", "time") ;
  obj.units_c = obj.nc_c.getAttString("units", "time") ;

#ifndef RAISE_ERRORS
  if( obj.units_c.size() == 0 )
#endif
  {
    std::string note("W33_1: No time units attribute in the child file.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::getTimeProperties()" ;
    ostr << "\nFile (parent): "  << obj.file_c ;
    exceptionError( ostr.str() );  
    std::cout << note << ";" << std::endl;
    finally(obj, note, 33);
  }

#ifndef RAISE_ERRORS
  if( obj.units_p.size() == 0 )
#endif
  {
    std::string note("W33_2: No time units attribute in the parent file.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::getTimeProperties()" ;
    ostr << "\nFile (parent): "  << obj.file_p ;
    exceptionError( ostr.str() );  
    std::cout << note << ";" << std::endl;
    finally(obj, note, 33);
  }

  // check the measure of the units
  std::vector<std::string> measure;
  measure.push_back( "year" );
  measure.push_back( "month" );
  measure.push_back( "day" );
  measure.push_back( "hour" );
  measure.push_back( "minute" );
  measure.push_back( "second" );
  measure.push_back( "none" ); // index == 6

  size_t i_c ;
  size_t pos_c;
  for( i_c=0 ; i_c < measure.size() ; ++i_c  )
    if( (pos_c=obj.units_p.find(measure[i_c])) < std::string::npos )
      break;

  size_t i_p ;
  size_t pos_p;
  for( i_p=0 ; i_p < measure.size() ; ++i_p  )
    if( (pos_p=obj.units_p.find(measure[i_p])) < std::string::npos )
      break;

#ifndef RAISE_ERRORS
  if( i_c < 6 && i_c != i_p )
#endif
  {
    std::string note ("W34_1: Time units attribute: different measuring in child and parent.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::getTimeProperties()" ;
    ostr << "\nParent:" ;
    ostr << "\n   File:  "  << obj.file_p ;
    ostr << "\n   Units: "  << obj.units_p ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:  "  << obj.file_c ;
    ostr << "\n   Units: "  << obj.units_c ;
    std::cout << note << ";" << std::endl;
//     exceptionError( ostr.str() );

    finally(obj, note, 33);
  }

  // check reference dates
  std::string s_p( obj.units_p.substr( pos_p +measure[i_p].size() ) );
  std::string s_c( obj.units_c.substr( pos_c +measure[i_c].size() ) );

#ifndef RAISE_ERRORS
  if( s_p != s_c )
#endif
  {
    std::string note ("W34_2: Time units attribute: different reference dates in child and parent.");

    std::ostringstream ostr(std::ios::app);
    ostr << "\n" << note ;
    ostr << "\ntestExpCont::getTimeProperties()" ;
    ostr << "\nParent:" ;
    ostr << "\n   File:  "  << obj.file_p ;
    ostr << "\n   Units: "  << obj.units_p ;
    ostr << "\nChild:"  ;
    ostr << "\n   File:  "  << obj.file_c ;
    ostr << "\n   Units: "  << obj.units_c ;
    ostr << "\nNote: this is adjusted." ;
    exceptionWarning( ostr.str() );
    std::cout << note << ";" << std::endl;
  }

  // reference dates
  obj.refDate_c.setDate( obj.units_c, calendar_c );
  obj.refDate_c.setDate( obj.units_p, calendar_p );

  // the time delay; identity means no delay.
  // This will adjust all time values of the child to
  // the reference date of the parent
  if( obj.units_c != obj.units_p )
     obj.refTimeDelay= obj.refDate_p.getSince( obj.refDate_c );

  return;
}

void
sync(Obj &obj)
{
  // Synchronise two time step streams to their continuation/fork
  // positions. The time the child is beginning will follow
  // immediately after the value of the parent time with closest
  // (from the end), but not identical, time.

  // number of time steps in the parent.
  size_t recNum_p = obj.nc_p.getNumOfRecords() ;

  // apply begin and end time of the child and parent , respectively,
  // for some checks.
  MtrxArr<double> mv;

  double tOrig_c   = obj.nc_c.getData(mv, "time", 0)  ;
  double t0_c = tOrig_c + obj.refTimeDelay ;
  double t1_p = obj.nc_p.getData(mv, "time", 0);

  obj.t0_c = t0_c ;
  
#ifndef RAISE_ERRORS
  if( t0_c <= t1_p )
#endif
  {
    std::string note("W35: Child begins earlier than the parent.");

    std::ostringstream ostr(std::ios::app);

    ostr << "\n" << note ;
    ostr << "\ntestExpCont::sync()" ;
    ostr << "\nParent:                "     ;
    ostr << "\n   File:                  "  << obj.file_p ;
    ostr << "\n   Units:                 "  << obj.units_p;
    ostr << "\n   Time at the beginning: "  << t1_p ;
    ostr << "\nChild: " ;
    ostr << "\n   File:                  "  << obj.file_c ;
    ostr << "\n   Units:                 "  << obj.units_c;
    ostr << "\n   Time at the beginning: "  << tOrig_c ;
    exceptionError( ostr.str() );
    std::cout << note << ";" << std::endl;
    finally(obj, note, 35);
  }

  // no overlap?
  t1_p = obj.nc_p.getData(mv, "time", recNum_p - 1);

  if( t1_p < t0_c )
  {
     obj.t1_p = t1_p ;
     obj.index_p = recNum_p -1 ;
     return;
  }

  size_t rec_p;
  double epsilon=1.e-10;

  // loop reverse
  for( rec_p=recNum_p-1 ; rec_p > 1 ; --rec_p )
  {
    t1_p = obj.nc_p.getData(mv, "time", rec_p);

    // time_c > time_p + epsilon
    if( hdhC::compare(t0_c, t1_p, ">", epsilon) )
      continue;

    if( hdhC::compare(t0_c, t1_p, "==", epsilon) )
    {
      obj.t1_p = t1_p ;
      obj.index_p = rec_p +1 ;
      return;
    }

    obj.t1_p = t1_p ;
    obj.index_p = rec_p ;
    return;
  }

  return;
}
