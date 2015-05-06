#ifndef _PARSE_H
#define _PARSE_H

#include "hdhC.h"

/*! \file parse.h
 \brief Parse parameters.
*/

//! Parse input-parameter specific to the qa_main frame program.

/*! The purpose is to set up a chain of Base-class derived objects properly.
    Options may be provided (and be mixed) from the command-line,
file or re-directed input (with --file=name
and/or --input on the comand-line).\n
For the Quality Control here, this means just the InFile object
(connected to the non-Base-derived QA and FD_interface objects).\n
Example of instructions in a file:\n \n
     -f path-to/hur_Amon_ECHAM6-MPIOM-LR_rcp45_r1_195501-199412.nc \n
     IN::0:QA0:FD0 \n
     QA::0:f=qa_hur_Amon_ECHAM6-MPIOM-LR_rcp45_r1.nc:returnDateRange:tablePath=...
        ...path-to/Project_tables:tableProject=CMIP5:tableStandard=...
        ...standard_output_11_26_09.csv\n
     FD::0:useAreaWeight:s=fd_hur_Amon_ECHAM6-MPIOM-LR_rcp45_r1.build \n \n
 This means for the InFile obj: use the default -f 'string'
 information, connect to the QA obj and to the FreqDist obj.
 The QA gets a filename for the results, will return a string
 indicating the time range, paths to the standard and project tables,
 respectively. The freqDist is built area-weighted and the result will
 be stored. For the particular purpose of Quality Control,
 the chain of objects has only a single member and
 both QA::0 and FD::0 are embedded in the InFile
 option-string (with appropriate syntax).
 Parameters are supplied by the script qaExecutor calling the program.\n
 Note that '/', '\', '{', and '}' are not accepted as separtors.

*/

class Parse
{
public:
  Parse();
  Parse(std::vector<std::string> &a);
  Parse(int argc, const char *argv[] );

  std::vector<std::vector<std::string> >
    getLinkList(void) { return linkList; }

  //! Get name and ID of a class identification string
  /*! Example: in==XX::0. Return string==XX::0, name==XX, id==0, sep=':'.*/
  static std::string
    getListObj( std::string &, std::string &name, int &id);
  static std::string
    getListObj( std::string &, std::string &name, int &id, std::string &param);
  std::string
    parseObj( std::string &, std::string &name, int &id, std::string &param);
  void
    parse(void) ;
  void
    printList(void);

  //! get access to the global exception and annotation handling
  void setNotes(Annotation *n) {notes = n; }

private:
  void   exceptionError(std::string );
  void   exceptionWarning(std::string );
  void   finally(int errCode=0, std::string s="");
  void   getRank(void);
  size_t nextFreeObjID(std::string oStr);
  void   mkLinkList(void);
  void   unique(void);

  std::vector<std::string> argv;
  std::vector<std::vector<std::string> > linkList;

  Annotation *notes;
};

#endif
