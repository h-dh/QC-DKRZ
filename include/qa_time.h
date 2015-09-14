#ifndef _QA_TIME_H
#define _QA_TIME_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"

//! Quality Control Program Unit for time management and checks.

class QA;
class SharedRecordFlag;

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
    UNDEF, REGULAR, ORPHAN, DISABLE, CYCLE
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

  bool isFormattedDate;
  bool isMaxDateRange;
  bool isNoCalendar;
  bool isNoProgress;
  bool isRegularTimeSteps;
  bool isPrintTimeBoundDates;
  bool isReferenceDate;
  bool isReferenceDateAcrossExp;
  bool isSingleTimeValue;
  bool isTimeBounds;

  std::string name;  // the name
  std::string boundsName;
  int time_ix;    // the var-index
  int timeBounds_ix;
  bool isTime;

  double lastTimeStep;  // is set in flush()
  double refTimeOffset;
  double refTimeStep;

  Date refDate;

  double prevTimeValue ;
  double firstTimeValue;
  double currTimeValue;
  double lastTimeValue;
  double currTimeStep ;

  double prevTimeBoundsValue[2];
  double firstTimeBoundsValue[2];
  double currTimeBoundsValue[2];
  double lastTimeBoundsValue[2];

//  Date prevTimeBoundsDate[2];
//  Date firstTimeBoundsDate[2];
//  Date currTimeBoundsDate[2];
//  Date lastTimeBoundsDate[2];

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

  void   getDate(Date& , double t);
  void   getDRSformattedDateRange(std::vector<Date> &,
                   std::vector<std::string> &);
  void   getTimeBoundsValues(double *pair, size_t rec, double offset=0.);

  bool   init(InFile*, Annotation*, QA*);

  /*! Absolute date encoded by a format given in time:units */
  bool   initAbsoluteTime(std::string &units);

  void   initDefaults(void);

  /*! Time relative to a reference date specified by time attributes */
  bool   initRelativeTime(std::string &units);

  //! Initialisiation of a resumed session.
  void   initResumeSession(void);

  bool   initTimeBounds(double offset=0.) ;
  void   initTimeTable(std::string id_1st, std::string id_2nd="");

  void   openQA_NcContrib(NcAPI*);

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

  MtrxArr<double> mv_tb;
  MtrxArr<double> mv_t;

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
  TimeOutputBuffer timeOutputBuffer;

  Annotation *notes;
  InFile *pIn;
  QA *pQA;
};

#endif
