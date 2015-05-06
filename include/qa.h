#ifndef _QA_H
#define _QA_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "qa_data.h"
#include "qa_time.h"
#include "qa_PT.h"

//! Quality Control Program Unit for project NONE.
/*! All the QA considerations are covered by this class.\n
The QA_NONE.cpp and qa_NONE.h files have to be linked to
QA.cpp and qa.h, respectively.\n
The netCDF data-file is linked by a pointer to the InFile class
instance. Results of the QA are written to a netCDF file
(to the directory where the main program was started) with filename
qa_<data-filename>.nc. Outlier test and such for replicated records
are performed. Annotations are supplied via the Annotation class
linked by a pointer.
*/

//! Struct containing dimensional properties to cross-check with table information.
struct DimensionMetaData
{
  // first item is set by 'time'
  std::string  cmor_name;
  std::string  outname;
  std::string  stndname;
  std::string  longname;
  std::string  type;
  std::string  units;
  bool         isUnitsDefined;
  std::string  index_axis;
  std::string  axis;
  std::string  coordsAtt;
  std::string  bounds;
  uint32_t     checksum;  //fletcher32
  size_t       size;
};

  //! QA related variable information.
class VariableMetaData
{
  public:
  VariableMetaData(QA*, Variable *var=0);
  ~VariableMetaData();

  // index of variable obj
  std::vector<size_t>  dimVarRep;

  std::string standardName;
  std::string longName;
  std::string units;
  std::string cellMethods;
  std::string cellMethodsOpt;
  std::string positive;

  bool        isUnitsDefined;
  bool        isForkedAnnotation;

  std::string name;
  std::string stdTableFreq;
  std::string stdSubTable;
  std::string type;
  std::string dims;
  std::string time_units;
  std::string unlimitedDim;

  Annotation     *notes;
  Variable *var;
  QA             *pQA ;
  QA_Data         qaData;

  int  finally(int errCode=0);
  void forkAnnotation(Annotation *p);
  void setAnnotation(Annotation *p);
  void setParent(QA *p){pQA=p;}
};

class QA : public IObj
{
  public:

   //! Default constructor.
  QA();
  ~QA();

  //! coresponding to virtual methods in IObj

  //! Check field properties of the variable.
  /*! Designed for multiple usage for sub-layers of a data block.
      Each multiple-set must be invoked by calling method 'clearStatistics'.*/
  bool   entry(void);

  //! Initialisation of the QA object.
  /*! Open the qa-result.nc file, when available or create
   it from scratch. Meta data checks are performed.
   Initialise time testing, time boundary testing, and cycles
   within a time step. At the end  entry() is called to test
   the data of fields.*/
  bool   init(void) ;
  void   linkObject(IObj *);
  void   setFilename(std::string name);
  void   setFilePath(std::string s){;}
  void   setTablePath(std::string p){ tablePath=p; }

  void   applyOptions(bool isPost=false);

//! Checks meta-data
  void   checkMetaData(InFile &) ;

  /*! Close records for time and data.*/
  void   closeEntry(void);

  //! Make VarMetaData objects.
  void   createVarMetaData(void);

  //! The final operations.
  /*! An exit code is returned.*/
  int    finally(int errCode=0);

  //! The final qa data operations.
  /*! Called from finall(). An exit code is returned.*/
  int    finally_data(int errCode=0);

  std::string
         getAttValue(size_t v_ix, size_t a_ix);

  std::string
         getCurrentTable(void){ return currTable ; }

  int    getExitCode(void){return exitCode;}

  std::string
         getFrequency(void);

  std::string
         getTablePath(void){ return tablePath; }

  //! Brief description of options
  static void
         help(void);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Global attributes of the qa-netCDF file.
  /*! Partly reflecting global attributes from the sources. */
  void   initGlobalAtts(InFile &);

  //! Initialisiation of a resumed session.
  /*! Happens for non-atomic data sets and those that are yet incomplete. */
  void   initResumeSession(void);

  //! Check the path to the tables;
  void   inqTables(void);

  bool   isProgress(void){ return ! isNoProgress ; }

  //! Open a qa_result file for creation or appending data.
  /*! CopY time variable from input-nc file.
   Collect some properties of the in-netcdf-file in
   struct varMeDa. Also check properties against tables.
  */
  void   openQcNc(InFile&);

  //! Perform only post-processing
  bool   postProc(void);
  bool   postProc_outlierTest(void);

  //! Connect this with the object to be checked
  void   setInFilePointer(InFile *p) { pIn = p; }

  //! Unused.
  /*! Needed to be conform to a specific Base class functionality */
  void   setSrcStr(std::string s)
             {srcStr.push_back(s); return;}

  //! Store results in the internal buffer
  /*! The buffer is flushed to file every 'flushCountMax' time steps.*/
  void   store(std::vector<hdhC::FieldData> &fA);
  void   storeData(std::vector<hdhC::FieldData> &fA);
  void   storeTime(void);

  //! Test the time-period of the input file.
  /*! If the end-date in the filename and the last time value
      match within the uncertainty of 0.75% of the time-step, then
      the file is assumed to be completely qa-processed. */
  bool   testPeriod(void);

  //! Name of the netCDF file with results of the quality control
  std::string qaFilename;
  std::string qaNcfileFlags;

  int exitCode;
  bool isExit;

  std::vector<VariableMetaData> varMeDa;

  NcAPI *nc;
  QA_Time qaTime;

//private:
  int thisId;

  size_t currQcRec;
  size_t importedRecFromPrevQA; // initial num of recs in the write-to-nc-file
  MtrxArr<double> tmp_mv;

  // init for test about times
  bool enablePostProc;
  bool enableVersionInHistory;
  bool isCaseInsensitiveVarName;
  bool isClearBits;
  bool isFileComplete;
  bool isInstantaneous;
  bool isNoProgress;
  bool isNotFirstRecord;
  bool isResumeSession;

  size_t nextRecords;

  bool isCheckMeta;
  bool isCheckTime;
  bool isCheckData;

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;
  std::vector<std::string> outlierOpts;
  std::vector<std::string> replicationOpts;

  std::string currTable;
  std::string projectTableName;
  std::string tablePath;

  std::string frequency;

  int identNum;
  std::string dataFile;
  std::string dataPath;
  std::string dataFilename;
  std::string fVarname;
  char        fileSequenceState;
  std::string project;
  std::string project_as;
  std::string prevVersionFile;
  std::vector<std::string> srcStr;
  std::string svnVersion;

  std::string fail;
  std::string notAvailable;
  std::string fileStr;
  std::string fileTableMismatch;

  void        appendToHistory(size_t);
  void        pushBackVarMeDa(Variable*);
  void        setExit(int);
  void        setCheckMode(std::string);
  void        setTable(std::string, std::string acronym="");
};

#endif
