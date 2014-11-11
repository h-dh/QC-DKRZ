#ifndef _QC_DATA_H
#define _QC_DATA_H

#include "hdhC.h"
#include "annotation.h"

//! Quality Control Program Unit for checking the data section.
/*! Outlier test and such for replicated records
are performed. Annotations are supplied via the Annotation class
linked by a pointer.
*/

class QC;
class QC_Data;
class VariableMetaData ;

class DataOutputBuffer
{
  //! Buffering of record-based results.
  /*! Due to efficiency, chunks of records are investigated before
   writing any data. A vector of objects of collects the results.*/

   public:
   DataOutputBuffer();
   ~DataOutputBuffer(){ clear();}

   void clear(void);
   void    flush(void);

   //! Change the flush counter; 1500 by default
   void    initBuffer(NcAPI *n, size_t next=0, size_t max=1500);
   void    setName(std::string n){name=n;}
   void    setNextFlushBeg(size_t n){nextFlushBeg=n;};
   void    store(hdhC::FieldData &fA);

   std::string name;

   size_t  bufferCount;
   size_t  maxBufferSize;
   size_t  nextFlushBeg;

   double *min;
   double *max;
   double *ave;
   double *std_dev;
   int    *fill_count;
   uint32_t *checksum;

   NcAPI *nc;
};

class SharedRecordFlag
{
  public:
  SharedRecordFlag();
 ~SharedRecordFlag();

  void   adjustFlag(int num, size_t rec, int erase=0);
  void   flush(void);

  //! Change the flush counter; 1500 by default
  void   initBuffer(NcAPI *, size_t next=0, size_t max=1500);
  void   setAnnotation(Annotation *p){notes=p;}
  void   setName(std::string n){name=n;}
  void   setNextFlushBeg(size_t n){nextFlushBeg=n;};
  void   store(void);

  size_t bufferCount;
  size_t maxBufferSize;
  size_t nextFlushBeg;

  int   *buffer;
  int    currFlag;

  std::string name;

  Annotation *notes;
  NcAPI *nc;
};

class Outlier
{
  public:
  Outlier( QC *p, size_t vmdIX, std::string name);
  ~Outlier(){;}

  QC *pQC;

  static bool
        isSelected( std::vector<std::string> &opts,
                     std::string &vName,
                     bool isQC_enablePostProc,
                     int effDims );

  //! Evaluate outlier test
  /*! When of checking sub-temp files or in post-processing mode.
      Acknowledgement: Dr. Andreas Chlond, MPI-M Hamburg, suggested to
      exploit the function N(sigma) as criterion.*/
  void  parseOption(std::vector<std::string>&);
  void  setAnnotation(Annotation *p){notes=p;}
  bool  test(QC_Data*);

  std::vector<std::string> options;

  size_t vMDix;
  std::string name;

  Annotation *notes;
} ;

class ReplicatedRecord
{
  //! Test for replicated records.
  /*! Before an array of values (e.g. ave, max ,min)
      is flushed to the qc_<filename>.nc file, the values
      in the array are tested for replicated records in
      the priviously written qc_<filename>.nc as well as
      in the array itself.*/

  public:
  ReplicatedRecord( QC *, size_t vMDix, std::string name);
  ~ReplicatedRecord(){;}

  static bool
         isSelected( std::vector<std::string> &options,
                     std::string &vName,
                     bool isQC_enablePostProc,
                     int effDims );

  void   parseOption( std::vector<std::string> &opts ) ;
  void   report( std::vector<std::string> &note, bool isMultiple );
  void   setAnnotation(Annotation *p){notes=p;}
  void   setGroupSize(size_t i){groupSize=i;}
  void   test( int nRecs, size_t bufferCount, size_t nextFlushBeg,
               bool isMultiple, bool isFirst );

  bool   enableReplicationOnlyGroups;

  std::string name;
  std::vector<std::string> options;
  size_t groupSize;

  size_t vMDix;
  Annotation *notes;
  QC *pQC;
} ;

class QC_Data
{
  public:
  QC_Data();
  ~QC_Data();

  void   applyOptions(bool isPost=false);

  //! Collects the results of checked records.
  /*! Does not really print, but store in the 'qcNcOutData' object.*/
  void   checkFinally(Variable *);

  void   disableTests(std::string);

  //! The final operations.
  /*! An exit code is returned.*/
  int    finally(int errCode=0);

  void   forkAnnotation(Annotation *p);

  //! Flush results to the qc-netCDF file.
  void   flush(void);

  void   init(InFile*, QC*, std::string);

  //! Change the flush counter; 1500 by default
  void   initBuffer(NcAPI *, size_t next=0, size_t max=1500);

  //! Initialisiation of a resumed session.
  void   initResumeSession(void);

  void   openQcNcContrib(NcAPI*, Variable *var);

  void   setAnnotation(Annotation *p);
  void   setInFile(InFile *p){ pIn=p;}

  void   setName(std::string);
  void   setNextFlushBeg(size_t n);
  void   setStatisticsAttribute(NcAPI*);
  void   store(hdhC::FieldData &);

  void   test( int, hdhC::FieldData &);
  bool   testConst(hdhC::FieldData &);
  bool   testInfNaN(hdhC::FieldData &);
  bool   testStndDev(hdhC::FieldData &);
  bool   testValidity(hdhC::FieldData &);

  std::string name;

  size_t bufferCount;
  size_t maxBufferSize;
  size_t nextFlushBeg;

  double currMin;
  double currMax;

  Statistics<double> statAve;
  Statistics<double> statMin;
  Statistics<double> statMax;
  Statistics<double> statStdDev;

  bool   allRecordsAreIdentical;

  bool   enableConstValueTest;
  bool   enableFillValueTest;
  bool   enableOutlierTest;
  bool   enableStdDevTest;
  bool   enableReplicationTest;
  bool   enableReplicationOnlyGroups;

  bool   isEntirelyConst;
  bool   isEntirelyFillValue;
  bool   isForkedAnnotation;
  bool   isSingleValueField;

  std::string   ANNOT_NO_MT;
  size_t        numOfClearedBitsInChecksum;

  //! results to qc.nc
  DataOutputBuffer dataOutputBuffer;
  SharedRecordFlag sharedRecordFlag;

  Annotation *notes;
  InFile *pIn;
  Outlier *outlier;
  QC *pQC;
  ReplicatedRecord *replicated;
};

#endif
