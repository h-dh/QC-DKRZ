#ifndef _QA_TIME_H
#define _QA_TIME_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"

//! Quality Control Program Unit for time management and checks.

class QA;
class SharedRecordFlag;

class TimeInputBuffer
{
   public:

//   TimeInputBuffer();

   std::string name;
   std::string name_bnds;

   int    remainingRecs;

   int sz ;
   int sz_chunk;
   size_t sz_passed;

   bool hasNoTime;
   bool hasBnds;
   bool isValid;

   size_t *start_b;
   size_t *count_b;
   size_t *start;
   size_t *count;

   double *pTime;
   double *pBnds;
   NcAPI  *pInNc;

   void   getTimeBnds(double *pair, size_t currRec, double offset=0.) ;
   double getTimeValue(size_t currRec, double offset=0.);
   void   init(InFile*);
   void   update(double offset=0.);
} ;

class TimeOutputBuffer
{
  public:
  TimeOutputBuffer();
  ~TimeOutputBuffer();

  void    clear(void);
  void    flush(void);

  //! Change the flush counter; 1500 by default
  void    initBuffer(NcAPI *n, size_t next=0, size_t max=1500);
  void    setName(std::string n);
  void    setNextFlushBeg(size_t n){nextFlushBeg=n;};
  void    store(double val, double step);

  std::string name;
  std::string name_step;

  size_t  bufferCount;
  size_t  maxBufferSize;
  size_t  nextFlushBeg;

  NcAPI  *nc;

  double *buffer;
  double *buffer_step;
};


enum TimeTableMode
{
    NONE, REGULAR, ORPHAN, DISABLE, CYCLE
};
TimeTableMode timeTableMode;

class QA_Time
{
  public:

  //! Default constructor.
  QA_Time();
//  ~QA_Time();

  //! get only options from QA options relevant to class QA_Time
  void   applyOptions(std::vector<std::string> &os);

  /*! Close records for time only.*/
  bool   closeEntry(void);
  bool   entry(void);
  bool   init(void);

  bool isCheckTime;
  bool isFormattedDate;
  bool isMaxDateRange;
  bool isNoCalendar;
  bool isRegularTimeSteps;
  bool isPrintTimeBoundDates;
  bool isReferenceDate;
  bool isReferenceDateAcrossExp;
  bool isSingleTimeValue;
  bool isTimeBounds;

  std::string time;  // the name
  double      lastTimeStep;  // is set in flush()
  double      refTimeDelay;

  Date refDate;

//  std::string cTime;
  Date prevDate ;
  Date firstDate;
  Date currDate;
  Date lastDate;

  double prevTimeValue ;
  double firstTimeValue;
  double currTimeValue;
  double lastTimeValue;
  double referenceTimeStep, currTimeStep ;

  std::string timeBoundsName;

  double prevTimeBoundsValue[2];
  double firstTimeBoundsValue[2];
  double currTimeBoundsValue[2];
  double lastTimeBoundsValue[2];

  Date prevTimeBoundsDate[2];
  Date firstTimeBoundsDate[2];
  Date currTimeBoundsDate[2];
  Date lastTimeBoundsDate[2];

  // proleptic Gregorian, no_leap, 30day-month
  std::string calendar;  //default is empty for Gregorian
  std::string currDateStr;
  std::string dateFormat ; // only for absolute dates as time values
  std::string maxDateRange;
  std::string timeTable;
  std::string tablePath;

  std::string fail;
  std::string notAvailable;

  void   clear(void);

  //! Check time properties.
  void   disableTimeBoundsTest(void){isTimeBounds=false;}
  void   enableTimeBoundsTest(void){isTimeBounds=true;}

  void   finally(NcAPI *);

  bool   getTimeBounds(double *pair, size_t curr);

  //! Return a date
  /*!  string1: "first", "curr", "last"; string2 (time-bounds): "left", "right",
       with "" by default for time-value*/
  Date  getDate(std::string, std::string s="");
  void   getDRSformattedDateRange(std::vector<Date> &,
                   std::vector<std::string> &);

  void   init(InFile*, Annotation*, QA*);
  void   initDefaults(void);

  //! Initialisiation of a resumed session.
  void   initResumeSession(void);

  /*! Absolute date encoded by a format given in time:units */
  bool   initTimeAbsolute(std::string &units);

  /*! Time relative to a reference date specified by time attributes */
  bool   initTimeRelative(std::string &units);

  void   initTimeTable(std::string id_1st, std::string id_2nd="");

  void   openQcNcContrib(NcAPI*);

  //! Parse the time_table file
  bool   parseTimeTable(size_t rec);

  void   setInFile(InFile *p){ pIn=p;}
  void   setNotes(Annotation *p){ notes=p;}
  void   setNextFlushBeg(size_t);

  //! Set the path to and the name of the time table.
  /*! When the name is supplied by option, then use the default.*/
  void   setTable(std::string &p, std::string t="");

  //! Synchronise the in-file and the qa-netCDF file.
  /*! Return value==true for isNoProgress.*/
  bool   sync(bool checkData, bool postProc);

  //! Test time value.
  /*! This member is just the entry to different tests.*/
  void   testDate(NcAPI &);

  //! Check for valid time bounds
  /*! No overlap with previous time bounds, reasonable span etc.. */
  void   testTimeBounds(NcAPI &);

  //! Check time steps
  bool   testTimeStep(void);

  MtrxArr<double> tmp_mv;
  std::string name;

  // Time Table: hold the state over a record
  size_t tt_block_rec ;
  bool   tt_isBlock;
  size_t tt_index ;
  size_t tt_count_recs;
  std::string tt_id;

  Split  tt_xmode;
  std::vector<Date> tt_dateRange;

  size_t bufferCount;
  size_t maxBufferSize;
  size_t nextFlushBeg;

  SharedRecordFlag sharedRecordFlag;
  TimeInputBuffer timeInputBuffer ;
  TimeOutputBuffer timeOutputBuffer;

  Annotation *notes;
  InFile *pIn;
  QA *pQA;
};

#endif
