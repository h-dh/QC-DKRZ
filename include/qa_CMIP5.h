#ifndef _QA_CMIP5_H
#define _QA_CMIP5_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "qa.h"
#include "qa_data.h"
#include "qa_time.h"
#include "qa_PT.h"

//! Quality Control Program Unit for CMIP5.
/*! All the QA considerations are covered by this class.\n
Properties specified in standard_output.xlsx document
from http://cmip-pcmdi.llnl.gov/cmip5/docs
are checked as well as the Data Reference Syntax given in cmip5_data_reference_syntax.pdf.\n
The netCDF data-file is linked by a pointer to the InFile class
instance. Results of the QA are written to a netCDF file
(to the directory where the main program was started) with filename qa_<data-filename>.nc. Outlier test and such for replicated records
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
  std::string  bnds_name;
  uint32_t     checksum;  //fletcher32
  size_t       size;
  std::string  value;
  std::string  requested ;
};

//! Properties of the variable to cross-check with table information.
class VariableMetaData
{
  public:
  VariableMetaData(QA*, Variable *v=0);
  ~VariableMetaData();

  std::string varReqTableSheet;
  std::string varReqTableSheetAlt;  // special case, cf. Omon, cf3hr, and cfSites
  std::string varReqTableSheetSub;

  bool isOwnVar;
  bool isForkedAnnotation;

  // these have no instance in the Variable class
  std::string name_alt;  // Table column: output variable name
  std::string longName;
  std::string cellMethods;
  std::string cellMeasures;
  std::string typeStr;

  size_t priority;

  // buffer results
  DataOutputBuffer dataOutputBuffer;

  Annotation* notes;
  Variable*   var;
  QA*         pQA ;
  QA_Data     qaData;

  //! Verify units % or 1 by data range
  void verifyPercent(void);

  int  finally(int errCode=0);
  void forkAnnotation(Annotation *p);
  void setAnnotation(Annotation *p);
  void setParent(QA *p){pQA=p;}
};

class QA_Exp
{
public:
  //! Default constructor.
  QA_Exp();

  void   applyOptions(std::vector<std::string>&);

  //! Only a single data variable is permitted
  void   checkDataVarNum(void);

  //! Check of dimensionless variables
  /*! These are represented by a single-value-dimension in the
      standard table and by a dimensionless variable in the file.*/
  bool   checkDimlessVar(InFile& in, Split& splt_line,
            VariableMetaData& var,
            struct DimensionMetaData *&p_dimFE,
            struct DimensionMetaData& dimFE_altern,
            struct DimensionMetaData& dimTE,
            std::map<std::string, size_t>& col);

  bool   checkDimSpecialValue(InFile& in, VariableMetaData& vMD,
            struct DimensionMetaData& , std::string& dimName) ;

  //! Comparison of dimensions between file and the var-requirements table
  /*! Cross-checks with the standard table are performed only once for
   each variable at first time encounter in the CMIP Project ensuring
   conformance.*/
  void   checkDimVarReqTable(ReadLine& tbl, InFile& in,
            VariableMetaData& var,
            std::vector<struct DimensionMetaData>&,
            std::map<std::string, size_t>& col,
            std::string dName, size_t colMax);

  //! Check dimensions
  void   checkDimVarReqTableEntry(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& nc,
            struct DimensionMetaData& tbl) ;

  void   checkDimAxis(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  void   checkDimBndsName(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  void   checkDimChecksum(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  void   checkDimLongName(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  bool   checkLonLatParamRep(InFile& in,
           VariableMetaData& vMD,
           std::string& nc_entry_name,
           std::string& tbl_entry_name);

  void   checkDimOutName(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  void   checkDimSize(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  void   checkDimStndName(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& nc_entry,
           struct DimensionMetaData& tbl_entry);

  //! Check the (transient) time dimension.
  void   checkDimULD(
            VariableMetaData& var,
            struct DimensionMetaData& nc,
            struct DimensionMetaData& tbl);

  //! Check the units of a dimension's variable representation.
  void   checkDimUnits(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& nc,
            struct DimensionMetaData& tbl);

  //! Match filename components and global attributes of the file.
  void   checkFilename(std::vector<std::string>&,
            std::string& stdSubTables_table);

  //! Checks meta-data
  void   checkMetaData(InFile& ) ;

  //! Is it NetCDF-4, is it compressed?
  void   checkNetCDF(InFile& );

  //! Starting function for all table cross-checks.
  void   checkTables(InFile& in, VariableMetaData& v);

  //! Cross-check with standard table.
  /*! Prepare the check for dimensions and variable.*/
  bool   checkVarReqTable(InFile& in,
            VariableMetaData& var,
            std::vector<struct DimensionMetaData>& );

  //! Check dimensional bounds: layout and size
  /*! Number of values and checksum of the bounds*/
  void   checkVarReqTableDimBounds(InFile& in, Split& splt_line,
            VariableMetaData& var,
            struct DimensionMetaData& dimFE,
            struct DimensionMetaData& dimTE,
            std::map<std::string, size_t>& col) ;

  //! Check dimensional values: layout and size
  /*! Number of values and checksum*/
  void   checkVarReqTableDimValues(InFile& in, Split& splt_line,
            VariableMetaData& var,
            struct DimensionMetaData& file,
            struct DimensionMetaData& table,
            std::map<std::string, size_t>& col) ;

  //! Apply the cross-check for the variable.
  void   checkVarReqTableEntry(
             VariableMetaData& ,
             VariableMetaData& tbl_entry);

  //! Variable objects for netcdf meta-data
  /*! Only the objects are created.*/
  void   createVarMetaData(void);

  //! Find occurrence of a heading line in string str0.
  /*! Returns true, if not found or the maximum number of columns needed.
      col maps required column titles to their index.*/
  bool   findNextVariableHeadline(ReadLine& , std::string& str0,
            VariableMetaData& var, std::vector<std::string>& );

  //! Find entry of a requested variable in the standard table.
  bool   findVarReqTableEntry(ReadLine& , std::string& ,
            VariableMetaData& var,
            std::map<std::string, size_t>& col, size_t col_max,
            std::vector<std::string>&  );

  //! Find the name of requested sub-table in string str0.
  bool   findVarReqTableSheet(ReadLine& , std::string& str0,
            VariableMetaData& var);

  void   findVarReqTableSheetSub(std::string& str0,
            VariableMetaData& vMD, std::vector<std::string>& );

  //! Store properties of a dimension in the struct.
  /*! Note: the name of the dimension is passed by the struct.*/
  void   getDimMetaData(InFile& in,
             VariableMetaData& ,
             struct DimensionMetaData& , std::string dName);

  std::string
         getTableEntryID(std::string vN="");

         //! Get global attribute 'frequency'
  std::string
         getFrequency(void);

  //! Get the MIP table name from the global attributes
  std::string
         getTableSheet(std::vector<std::string>&);

  //! get and check MIP table name
  void
         getTableSheet(VariableMetaData& );

//  std::string
//         getVarReqTable(void){ return varReqTable.file ; }

//  std::string
//         getTablePath(void){ return tablePath; }

  void   init(QA*, std::vector<std::string>&);

  //! Check the path to the tables;
  bool   inqTables(void);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Initialisiation of a resumed session.
  /*! Happens for non-atomic data sets and those that are yet incomplete. */
  void   initResumeSession(std::vector<std::string>& prevTargets);

  //! Read the headlines for dimensions and variable.
  /*! Read from the standard table; used to identify columns.*/
  bool   readHeadline(ReadLine&,
            VariableMetaData&,
            std::map<std::string, size_t>& v_col,
            std::map<std::string, size_t>& d_col,
            size_t& v_colMax, size_t& d_colMax);

  //! Connect this with the object to be checked
//  void   setInFilePointer(InFile *p) { pIn = p; }

  //! Set properties of variable from netcdf file
  void   setVarMetaData(VariableMetaData& );

  //! Test the time-stamp of the input file.
  /*! If the end-date in the filename and the last time value
      match within the uncertainty of 0.75% of the time-step, then
      the file is assumed to be completely qa-processed.
      Syntax of date ranges as given in CMIP% DRS Syntax.*/
  bool   testPeriod(void);
  bool   testPeriodAlignment(std::vector<std::string>& sd, Date** pDates, bool b[])  ;
  void   testPeriodCut(std::vector<std::string>& sd) ;
  bool   testPeriodCut_CMOR_isGOD(std::vector<std::string>& sd, Date**);
  void   testPeriodCutRegular(std::vector<std::string>& sd,
              std::vector<std::string>& text);
  bool   testPeriodFormat(std::vector<std::string>& sd) ;

  //! Name of the netCDF file with results of the quality control
  struct hdhC::FileSplit varReqTable;
  struct hdhC::FileSplit table_DRS_CV;

  std::vector<VariableMetaData> varMeDa;

  NcAPI* nc;
  QA*    pQA;

  MtrxArr<double> tmp_mv;

  // the same buf-size for all buffer is required for testing replicated records
  size_t bufferSize;

  // init for test about times
  bool enabledCompletenessCheck;
  bool isUseStrict;
  bool isCaseInsensitiveVarName;
  bool isCheckParentExpID;
  bool isCheckParentExpRIP;
  bool isClearBits;

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;

  std::string cfStndNames;
  std::string MIP_tableName;
  std::string frequency;
  std::string parentExpID;
  std::string parentExpRIP;

  std::string experiment_id;

  std::string fVarname;

  std::string getSubjectsIntroDim(VariableMetaData& vMD,
                   struct DimensionMetaData& nc_entry,
                   struct DimensionMetaData& tbl_entry);
  std::string getVarnameFromFilename(std::string str);
  bool        not_equal(double x1, double x2, double epsilon);
  void        pushBackVarMeDa(Variable*);
};

#endif
