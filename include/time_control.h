#ifndef _TIME_CONTROL_H
#define _TIME_CONTROL_H

#include "hdhC.h"
#include "iobj.h"

/*! \file time_control.h
 \brief Time range window for reading of records (not for QA).
*/

//! Record range constraint for file processing.
/*! Find start and end point of the unlimited (record) variable.
    Usually, time, but also for any kind of
    numerical value. Generate optionally filenames of sequences
    of files by pattern matching related to time.*/

class TimeControl : public IObj
{
  public:
  // exception messaging
  struct ExceptionStruct
  {
     std::ofstream *ofsError;
     std::ofstream *ofsWarning;

     std::string strError;
     std::string strWarning;
  } ;

   TimeControl();
   TimeControl( const TimeControl& );
   ~TimeControl();

   TimeControl& operator=( const TimeControl&);

  //! coresponding to virtual methods in IObj
    bool         closeEntryTime(void){return false;}
    bool         entry(void){return false;}

/*! Get units from the in-file and synchronise to the beginning of
    the time window, if supplied. The end-record of the in-file is
    then set according to the time-window's end.*/
    bool         init(void);
    void         linkObject(IObj *);
    void         setFilename(std::string s){filename=s;}
    void         setFilePath(std::string p){path=p;}
    void         setTablePath(std::string p){ ; }

   void   applyOptions(void);
   void   copy( const TimeControl&);
   void   exceptionError(std::string );
   void   exceptionWarning(std::string );
   void   finally(int errCode=0);

//! Find the record number matching the sides of the time-window.
/*! Character 'side' indicates whether it is the left side by 'l'
    or any other for the right side. rVName indictes the name of
    the variable representing the unlimited dimension (record).*/
   size_t findRecAtTime(std::string rVName, double tLim, char side);
   std::string
          getAbsoluteFilename(void);
   void   getCurrDate(NcAPI &nc, size_t rec);
   Date& getCurrDate(void){return currDate;}
   Date  getEndDate(void){ return endDate;}
   std::string
          getFilename(void);

   int    getSourceID(void){return identNum;}
   Date  getStartDate(void){ return beginDate;}
   static void
          help(void);

   void   initDefaults(void);
   bool   isBuildFilename(void){return isBuildFileName;}
   void   resetFilename(void){filename="";}

//! Set the begin/end of the time window.
/*! If string s begins with letter 't' or 'T', then s indicates
    a time value corresponding to the refernce date in the file.*/
   void   setBeginDate(std::string);
   void   setEndDate(std::string);

   void   setFilenamePattern(std::string s){filenamePattern=s;}

   void   setIncrement(std::string);

   //! Connect this with the object to be controlled
   void   setInFilePointer(InFile *p)
              { pIn = p; }
   void   setReferenceDate(std::string s)
             {refDate.setDate(s) ;}
   void   setRec(size_t rec)
             {currRec=rec;}
   void   setSourceID(int i){identNum=i;}
   void   setSrcStr(std::string s)
             {srcStr.push_back(s); return;}

//! Stop at reaching a given time limit.
/*! Return true, if the limit is exceeded.*/
   bool   syncEnd(double);
   bool   syncEnd(Date&);

   bool isBuildFileName;
   bool isTimeFrame;

   Date  refDate;

   bool   isBegin, isEnd;
   size_t beginRec, currRec, endRec;
   double beginValue, currValue, endValue;
   Date  beginDate, currDate, endDate ;

   std::string recVarName;

   bool isTime;
   MtrxArr<double> tmp_mv;

   int identNum;
   int argCount;

   std::string incrementPattern;
   double incrementValue;
   std::string filenamePattern;
   std::string filename;
   std::string path;
   std::vector<std::string> srcStr;

   // messaging in case of exceptions.
   struct ExceptionStruct xcptn;
};

#endif
