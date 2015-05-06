#ifndef _OUT_FILE_H
#define _OUT_FILE_H

#include "base.h"

/*! \file out_file.h
 \brief Output of data (not for QA).
*/

//! Writing results to netCDF file(s) (derived from class Base).
/*! Not used for the quality control.*/

class Annotation;

class OutFile : public Base
{
public:
  OutFile();
  ~OutFile(){;}

  //! coresponding to virtual methods in IObj
  bool         entry(void){return false;}
  bool         init(void) ;
  void         linkObject(IObj *);
  void         setTablePath(std::string p){ ; }

  void  initDefaults(void);
};

#endif
