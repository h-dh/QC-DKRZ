#ifndef _INTERFACE_OBJECT_H
#define _INTERFACE_OBJECT_H

/*! \file iobj.h
 \brief Abstract class interface to class Base and its derivations.
*/

//! Interface for all classes implementing shared methods

/*! Purpose: application of C++ Abstract Class properties.*/

class Annotation;
class CF;
class FD_interface;
class InFile;
class Oper;
class OutFile;
class QA;
class TimeControl;

class IObj{
public:
    virtual ~IObj(){}

    virtual bool         entry(void) = 0;
    virtual bool         init(void) = 0;
    virtual void         linkObject(IObj *) = 0;
    virtual void         setFilename(std::string ) = 0 ;
    virtual void         setFilePath(std::string ) = 0 ;
    virtual void         setTablePath(std::string ) = 0 ;

    //! Pointer to a member function in the polymorph context.
    /*! This pointer contains initially the pointer to the
        the member init() and is switched there to point to entry().*/
    bool (IObj::*execPtr)(void);

    int          getObjID(void);
    std::string  getObjName(void);

    void         setObjID(int);
    void         setObjName(std::string);

    void         setOptions( std::string);
    void         setOptions( std::vector<std::string> &);

    int thisID;
    std::string objName;
    std::vector<std::string> optStr;

    Annotation   *notes;
    CF           *cF;
    InFile       *pIn;
    FD_interface *fDI;
    Oper         *pOper;
    OutFile      *pOut;
    QA           *qA;
    TimeControl  *tC;
};

inline int          IObj::getObjID(void)            { return thisID ; }
inline std::string  IObj::getObjName(void)          { return objName; }

inline void         IObj::setObjID(int i)           { thisID=i; }
inline void         IObj::setObjName(std::string n) { objName=n; }

inline void         IObj::setOptions( std::string s) {optStr.push_back(s);}
inline void         IObj::setOptions( std::vector<std::string> &o)
{
  for( size_t  i=0 ; i < o.size() ; ++i )
    optStr.push_back(o[i]);

  return;
}

#endif
