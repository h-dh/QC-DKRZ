#ifndef _QA_CORDEX_H
#define _QA_CORDEX_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "qa.h"
#include "qa_data.h"
#include "qa_time.h"
#include "qa_PT.h"

//! Quality Control Program Unit for CORDEX.
/*! All the QA considerations specific to CORDEX are covered by this class.\n
Requested properties from the cordex_archive_specifications.pdf
from http://cordex.dmi.dk/joomla/images/CORDEX are checked as well
as specifications in CORDEX_variables_requirement_table.pdf.\n
Results of the QA are written to a netCDF file
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

//! Properties of the variable to cross-check with table information.
class VariableMetaData
{
  public:
  VariableMetaData(QA*, Variable *var=0);
  ~VariableMetaData();

  // only used for reading tables
  std::string cellMethods;
  std::string cellMethodsOpt;
  std::string standardName;
  std::string longName;
  std::string positive;
  std::string units;

  bool        isUnitsDefined;
  bool        isForkedAnnotation;

  std::string name;

  //! store results temporarily
  DataOutputBuffer dataOutputBuffer;

  Annotation*  notes;
  Variable*    var;
  QA*          pQA;
  QA_Data      qaData;

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

  void   checkDimStandardTable(ReadLine &tbl, InFile &in,
            VariableMetaData &var,
            std::vector<struct DimensionMetaData>&,
            std::map<std::string, size_t> &col,
            std::string dName, size_t colMax);

  //! Check dimensions
  void   checkDimTableEntry(InFile &in,
            VariableMetaData &var,
            struct DimensionMetaData &nc,
            struct DimensionMetaData &tbl) ;

  void   checkDimAxis(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  void   checkDimChecksum(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  void   checkDimLongName(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  void   checkDimOutName(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  void   checkDimSize(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  void   checkDimStndName(InFile &in,
           VariableMetaData &vMD,
           struct DimensionMetaData &nc_entry,
           struct DimensionMetaData &tbl_entry);

  //! Check the (transient) time dimension.
  void   checkDimULD(
            VariableMetaData &var,
            struct DimensionMetaData &nc,
            struct DimensionMetaData &tbl);

  //! Check the units of a dimension's variable representation.
  void   checkDimUnits(InFile &in,
            VariableMetaData &var,
            struct DimensionMetaData &nc,
            struct DimensionMetaData &tbl);

  void   checkDRS(InFile &);
  void   checkDRS_ModelName(InFile &, std::string &aN, std::string &aV,
            char model, std::string iN="", std::string iV="");

  //! Match filename components and global attributes of the file.
  void   checkFilename(InFile &) ;

  //! Test optional global attribute
  void   checkDrivingExperiment(InFile &);

  //! Check near-surface height value between 0 - 10m
  void   checkHeightValue(InFile &);

//! Checks meta-data
  void   checkMetaData(InFile &) ;

  //! Is it NetCDF-4, is it compressed?
  void   checkNetCDF(InFile &);

  //! Consistency between filename, plev variable and data value
  /*! For variables defined for a specific pressure level, eg.ta850*/
  void   checkPressureCoord(InFile&);

  void   checkVarTableEntry(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Consistency test between file and table: cell_methods.
  void   checkVarTableEntry_cell_methods(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Consistency test between file and table: long name.
  void   checkVarTableEntry_longName(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Consistency test between file and table: (output) name.
  void   checkVarTableEntry_name(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Consistency test between file and table: standard name.
  void   checkVarTableEntry_standardName(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Consistency test between file and table: units.
  /*! If one of table or CF standard units matches, then the check will pass.*/
  void   checkVarTableEntry_units(
             VariableMetaData &,
             VariableMetaData &tbl_entry);

  //! Make VarMetaData objects.
  void   createVarMetaData(void);

  //! Check time properties.
  void   domainCheck(ReadLine&);
  void   domainCheckData(std::string &var_lon, std::string &var_lat,
            std::vector<std::string>&, std::string tbl_id);
  void   domainCheckDims(std::string item, std::string &t_Nlon,
            std::string &f_Nlon, std::string tble_id);
  void   domainCheckPole(std::string item,
            std::string &t_Nlon, std::string &f_Nlon);
  void   domainFindTableType( std::vector<std::vector<std::string> > &tbl1,
            std::vector<std::vector<std::string> > &tbl2,
            int &ix1, int &ix2 );
  bool   domainFindTableTypeByRange(
            std::vector<std::vector<std::string> > &T1,
            std::vector<std::vector<std::string> > &T2,
           int &table_id, size_t &row );

  //! Find entry of a requested variable in the standard table.
  bool   findTableEntry(ReadLine &, std::string &,
            VariableMetaData &tble_entry);

  bool   findTableEntry(ReadLine &, std::string &name,
            size_t col_outName, std::string &str0);

  //! Find the name of requested sub-table in string str0.
//  bool   find_CORDEX_SubTable(ReadLine &, std::string &str0,
//            VariableMetaData &var);

  std::string
         getAttValue(size_t v_ix, size_t a_ix);

  //! Store properties of a dimension in the struct.
  /*! Note: the name of the dimension is passed by the struct.*/
  bool   getDimMetaData(InFile &in,
             VariableMetaData &,
             struct DimensionMetaData &, std::string &);

  std::string
         getFrequency(void);

//  std::string
//         getStandardTable(void){ return varReqTable.file ; }

  //! Get the Standard table name from the global attributes
  void   getSubTable(void);

  std::string
         getTableEntryID(std::string vN="");

  void   init(QA*, std::vector<std::string>&);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Initialisiation of a resumed session.
  /*! For non-atomic data sets and such being incomplete, yet. */
  void   initResumeSession(std::vector<std::string>& prevTargets);

  //! Check existence of tables
  bool   inqTables(void);

  //! Read the headlines for dimensions and variable.
  /*! Read from the standard table; used to identify columns.*/
  bool   readTableCaptions(ReadLine &, std::string freq,
            std::map<std::string, size_t> &col, std::string &str0);

  void   requiredAttributes_check(InFile &);
  void   requiredAttributes_checkCloudVariableValues(InFile &,
             std::string &aux, std::string &vals) ;
  void   requiredAttributes_checkGlobal(InFile &,
             std::vector<std::string> &atts);
  void   requiredAttributes_checkVariable(InFile &,
             Variable &, std::vector<std::string> &atts);
  void   requiredAttributes_readFile(
           std::vector<std::string> &,
           std::vector<std::vector<std::string> > &);

  //! Cross-check with standard table.
  /*! Prepare the check for dimensions and variable.*/
  void   varReqTableCheck(InFile &in,
            VariableMetaData &var,
            std::vector<struct DimensionMetaData>& );

  //! Check dimensional bounds: layout and size
  /*! Number of values and checksum of the bounds*/
/*
  void   varReqTableCheckDimBounds(InFile &in, Split &splt_line,
            VariableMetaData &var,
            struct DimensionMetaData &dimFE,
            struct DimensionMetaData &dimTE,
            std::map<std::string, size_t> &col) ;
*/
  //! Check dimensional values: layout and size
  /*! Number of values and checksum*/
/*
  void   varReqTableCheckDimValues(InFile &in, Split &splt_line,
            VariableMetaData &var,
            struct DimensionMetaData &file,
            struct DimensionMetaData &table,
            std::map<std::string, size_t> &col) ;
*/

  //! Test the time-period of the input file.
  /*! If the end-date in the filename and the last time value
      match within the uncertainty of 0.75% of the time-step, then
      the file is assumed to be completely qa-processed.
      Syntax of date ranges as given in CORDEX  DRS Syntax.*/
  bool   testPeriod(void);
  bool   testPeriodAlignment(std::vector<std::string> &sd, Date** pDates, bool b[])  ;
  void   testPeriodCut(std::vector<std::string> &sd) ;
  bool   testPeriodCut_CMOR_isGOD(std::vector<std::string> &sd, Date**);
  void   testPeriodCutRegular(std::vector<std::string> &sd,
              std::vector<std::string>& text);
  bool   testPeriodFormat(std::vector<std::string> &sd) ;

  //! Name of the netCDF file with results of the quality control
  struct hdhC::FileSplit GCM_ModelnameTable;
  struct hdhC::FileSplit RCM_ModelnameTable;
  struct hdhC::FileSplit table_DRS_CV;
  struct hdhC::FileSplit varReqTable;

  std::vector<VariableMetaData> varMeDa;

  NcAPI *nc;
  QA* pQA;

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
  bool isRotated;

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;
  std::vector<std::string> requiredAttributesOption;

  std::string cfStndNames;

  std::string frequency;
  std::string parentExpID;
  std::string parentExpRIP;
  std::string experiment_id;
  std::string subTable ;

  std::string fVarname;

  std::string getCaptIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry, std::string att="");
  std::string getSubjectsIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry, bool isColon=true);
  std::string getVarnameFromFilename(std::string str);
  void        pushBackVarMeDa(Variable*);
  void        setCheckMode(std::string);
};

#endif
