#ifndef _QA_PT_H
#define _QA_PT_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"

class QA;
class InFile;

class ProjectTable
{
  public:
  ProjectTable(QA *, InFile*, struct hdhC::FileSplit& );

  //! Prepare the comparison of dimensions between file and project table.
  /*! This is checked for each chunk or atomic data set in each
  experiment ensuring conformance.*/
  void   check(void);
  bool   check(Variable&, std::string entryID);

  //! Check the type of variables
  //! The target variable must be of FLOAT.
  void   checkType(Variable&, std::string &);

  //! Put Attributes of given variable to string
  void   getAtts(Variable&, std::string &) ;

  //! Put all meta-data to string.
  void   getMetaData(Variable&, std::string& entryID, std::string &md);

  //! Get the checksum of all non-unlimited variables
  void   getValues(Variable&, std::string &) ;

  bool   lockFile(std::string &fName);

  void   setAnnotation(Annotation *p){ notes=p;}
  void   setExcludedAttributes(std::vector<std::string> &);
  void   setTable(std::string &p, std::string &t);

  bool   unlockFile(std::string &fName);

  //! Add meta data of each variable to the project table.
  /*! Meta-data of the data variable and the auxiliaries.*/
  void   write(Variable&, std::string& entryID);

  std::vector<std::string> excludedAttributes;

  struct hdhC::FileSplit projectTableFile;

  Annotation *notes;
  InFile     *pIn;
  QA         *qa;  //the parent
} ;

#endif
