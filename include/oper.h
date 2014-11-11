#ifndef _OPER_H
#define _OPER_H

#include "base.h"

/*! \file oper.h
 \brief Operation of data (not for QC).
*/

class Annotation;

//! Operation on data stored in a GeoData object (derived from class Base).
/*! Not used for the quality control.*/

class Oper : public Base
{
public:
  Oper();
  ~Oper(){;}

  //! coresponding to virtual methods in IObj
  bool         entry(void){return false;}
  bool         init(void) ;
  void         linkObject(IObj *);
  void         setTablePath(std::string p){ ; }

  void applyOptions(void);
  static void
       help(void);

  void initDefaults(void);
  void parseOperation(void);

  std::string mathExpr;
};

#endif
