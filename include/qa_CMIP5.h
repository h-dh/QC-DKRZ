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

class QA_Exp;

struct CMOR
{
  CMOR(QA*);

  void   applyOptions(std::vector<std::string>&);

  //! Comparison of dimensions between file and the var-requirements table
  /*! Cross-checks with the standard table are performed only once for
   each variable at first time encounter in the CMIP Project ensuring
   conformance.*/

  // the next one is applied to several checks
  void   checkStringValues(
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD,
            std::string&, size_t maxSz=0 );

  void   checkDimSheet(std::vector<std::string>&, InFile& in,
            VariableMetaData&,
            std::vector<struct DimensionMetaData>&,
            std::map<std::string, size_t>& col,
            std::string dName, size_t colMax);

  //! Check dimensions
  void   checkDimSheetEntries(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  //! Compare dimensional properties from the table to the file
  void   checkDimSheet_axis(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& f_DMD,
           struct DimensionMetaData& t_DMD);

  void   checkDimSheet_boundsQuest(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_boundsRequested(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_boundsValues(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_coordsAtt(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_indexAxis(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_longName(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  bool   checkDimSheet_outname(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& f_DMD,
           struct DimensionMetaData& t_DMD);

  void   checkDimSheet_positive(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& f_DMD,
           struct DimensionMetaData& t_DMD);

  void   checkDimSheet_requested(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_stdName(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& f_DMD,
           struct DimensionMetaData& t_DMD);

  void   checkDimSheet_type(InFile& in,
           VariableMetaData& vMD,
           struct DimensionMetaData& f_DMD,
           struct DimensionMetaData& t_DMD);

  //! Check the units of a dimension's variable representation.
  void   checkDimSheet_units(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD);

  void   checkDimSheet_validMax(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_validMin(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkDimSheet_value(InFile& in,
            VariableMetaData& var,
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD) ;

  void   checkRequestedAttributes(void);
  void   checkReqAtt_global(void);
  void   checkReqAtt_variable(Variable&);
/*
  // requested by the CMOR table, thus checked in that context
  void   checkReqVariableType(void);
*/

  /*! Prepare the check for dimensions and variable.*/
  bool   checkVarReqTable(InFile& in,
            VariableMetaData& var,
            std::vector<struct DimensionMetaData>& );

  //! Apply the cross-check for the variable.
  void   checkVarReqTableEntry(
             VariableMetaData& ,
             VariableMetaData& tbl_entry);

  // the next one is applied to several checks
  void   checkWithTolerance(
            struct DimensionMetaData& f_DMD,
            struct DimensionMetaData& t_DMD,
            std::string&, size_t maxSz=0 );

  //! Find occurrence of a heading line in string str0.
  /*! Returns true, if not found or the maximum number of columns needed.
      col maps required column titles to their index.*/
  //! Find entry of a requested variable in the standard table.
  bool   findVarReqTableEntry(ReadLine& , std::string& ,
            VariableMetaData& var,
            std::map<std::string, size_t>& col, size_t col_max );

  //! Find the name of requested sub-table in string str0.
  bool   findVarReqTableSheet(ReadLine& , std::string& str0,
            VariableMetaData& var);

  void   findVarReqTableSubSheet(std::string& str0, VariableMetaData& vMD );

  //! Store properties of a dimension in the struct.
  /*! Note: the name of the dimension is passed by the struct.
      dim-name is returned; accounts for a switch to a label-name*/
  std::string
         getDimMetaData(InFile& in,
             VariableMetaData& ,
             struct DimensionMetaData& , std::string dName);

  void   initDefaults(void);

  //! Read the headlines for dimensions and variable.
  /*! Read from the standard table; used to identify columns.*/
  bool   readHeadline(ReadLine&,
            VariableMetaData&,
            std::vector<std::string>&,
            std::map<std::string, size_t>& v_col,
            std::map<std::string, size_t>& d_col,
            size_t& v_colMax, size_t& d_colMax);


  //! Starting function for all table cross-checks.
  void   run(InFile&, VariableMetaData&);

  std::string parentExpID;
  std::string parentExpRIP;

  static std::string n_activity;
  static std::string n_axis;
  static std::string n_bounds_quest;
  static std::string n_bounds_requested;
  static std::string n_bounds_values;
  static std::string n_cell_measures;
  static std::string n_cell_methods;
  static std::string n_CMOR_dimension;
  static std::string n_CMOR_name;
  static std::string n_CMOR_tables;
  static std::string n_CMOR_variable_name;
  static std::string n_coordinates;
  static std::string n_index_axis;
  static std::string n_long_name;
  static std::string n_name_alt;
  static std::string n_outname;
  static std::string n_output;
  static std::string n_output_dim_name;
  static std::string n_output_var_name;
  static std::string n_positive;
  static std::string n_priority;
  static std::string n_product;
  static std::string n_requested;
  static std::string n_standard_name;
  static std::string n_tol_on_requests;
  static std::string n_type;
  static std::string n_unformatted_units;
  static std::string n_units;
  static std::string n_valid_max;
  static std::string n_valid_min;
  static std::string n_value;

  static std::string tableSheet;
  static std::string tableSheetAlt;  // special case, cf. Omon, cf3hr, and cfSites
  static std::string tableSheetSub;

  Annotation* notes;
  QA*         pQA;
  QA_Exp*     pExp;
};

std::string CMOR::n_activity            ="activity";
std::string CMOR::n_axis                ="axis";
std::string CMOR::n_bounds_quest        ="bnds_quest";
std::string CMOR::n_bounds_requested    ="bounds_requested";
std::string CMOR::n_bounds_values       ="bounds_values";
std::string CMOR::n_cell_measures       ="cell_measures";
std::string CMOR::n_cell_methods        ="cell_methods";
std::string CMOR::n_CMOR_dimension      ="CMOR_dimension";
std::string CMOR::n_CMOR_name           ="CMOR_name";
std::string CMOR::n_CMOR_tables         ="CMOR_tables";
std::string CMOR::n_CMOR_variable_name  ="CMOR_variable_name";
std::string CMOR::n_coordinates         ="coordinates";
std::string CMOR::n_index_axis          ="index_axis";
std::string CMOR::n_long_name           ="long_name";
std::string CMOR::n_name_alt            ="name_alt";
std::string CMOR::n_output              ="output";
std::string CMOR::n_output_dim_name     ="output_dimension_name";
std::string CMOR::n_output_var_name     ="output_variable_name";
std::string CMOR::n_positive            ="positive";
std::string CMOR::n_priority            ="priority";
std::string CMOR::n_product             ="product";
std::string CMOR::n_requested           ="requested";
std::string CMOR::n_standard_name       ="standard_name";
std::string CMOR::n_tol_on_requests     ="tol_on_requests";
std::string CMOR::n_type                ="type";
std::string CMOR::n_unformatted_units   ="unformatted_units";
std::string CMOR::n_units               ="units";
std::string CMOR::n_valid_max           ="valid_max";
std::string CMOR::n_valid_min           ="valid_min";
std::string CMOR::n_value               ="value";

std::string CMOR::tableSheet=hdhC::empty;
std::string CMOR::tableSheetAlt=hdhC::empty;  // special case, cf. Omon, cf3hr, and cfSites
std::string CMOR::tableSheetSub=hdhC::empty;

struct DRS_CV
{
  DRS_CV(QA*);

  void   applyOptions(std::vector<std::string>&);
  void   checkFilename(std::string& fName, struct DRS_CV_Table&);
  void   checkFilenameEncoding(Split&, struct DRS_CV_Table&);
  void   checkFilenameGeographic(Split&);
  void   checkMIP_tableName(Split&);
  void   checkPath(std::string&, struct DRS_CV_Table&);
  void   findFN_faults(Split&, Split&,
                   std::map<std::string, std::string>&,
                   std::string& text);
  void   findPath_faults(Split&, Split&,
                   std::map<std::string, std::string>&,
                   std::string& text);
  void   checkProductName(std::string& drs_product,
                   std::string prod_choice,
                   std::map<std::string, std::string>& gM);
  void   checkVariableName(std::string& f_vName);

  std::string
         getEnsembleMember(void);
  int    getPathBegIndex( Split& drs, Split& x_e,
            std::map<std::string, std::string>& gM );
  void   run(void);

    //! Test the time-period of the input file.
  /*! If the end-date in the filename and the last time value
      match within the uncertainty of 0.75% of the time-step, then
      the file is assumed to be completely qa-processed.
      Syntax of date ranges as given in CORDEX  DRS Syntax.*/
  bool   testPeriod(Split&);
  bool   testPeriodAlignment(std::vector<std::string> &sd, Date** pDates, bool b[])  ;
  void   testPeriodPrecision(std::vector<std::string> &sd,
              std::vector<std::string>& text);
  bool   testPeriodDatesFormat(std::vector<std::string> &sd) ;
  bool   testPeriodFormat(Split&, std::vector<std::string> &sd) ;

  bool enabledCompletenessCheck;

  std::string ensembleMember;

  Annotation* notes;
  QA*         pQA;
};

//! Struct containing dimensional properties to cross-check with table information.
struct DimensionMetaData
{
  DimensionMetaData(Variable* p=0){var=p;}

  std::map<std::string, std::string> attMap;
  Variable*    var;
};

//! Properties of the variable to cross-check with table information.
class VariableMetaData
{
  public:
  VariableMetaData(QA*, Variable *v=0);
  ~VariableMetaData();

  int  finally(int errCode=0);
  void forkAnnotation(Annotation *p);
  void setAnnotation(Annotation *p);
  void setParent(QA* p){pQA=p;}

  //! Verify units % or 1 by data range
  void verifyPercent(void);

  // buffer results
  DataOutputBuffer dataOutputBuffer;

  std::map<std::string, std::string> attMap;
  size_t priority;
  bool isNewVar;

  Annotation* notes;
  Variable*   var;
  QA*         pQA ;
  QA_Data     qaData;

};

class QA_Exp
{
public:
  //! Default constructor.
  QA_Exp();

  void   applyOptions(std::vector<std::string>&);

  //! Only a single data variable is permitted
  void   checkDataVarNum(void);

  //! Checks meta-data
  void   checkMetaData(InFile& ) ;

  //! Is it NetCDF-4, is it compressed?
  void   checkNetCDF(InFile& );

  void   checkVariableType(void);

//! Variable objects for netcdf meta-data
  /*! Only the objects are created.*/
  void   createVarMetaData(void);

  //! Get global attribute 'frequency'
  std::string
         getFrequency(void);

  //! Get the MIP table name from the global attributes
  std::string
         getTableSheet(void);

  //! get and check MIP table name; s would indicate a name to be tried
  std::string
         getMIP_tableName(std::string s="");

  std::string
         getTableEntryID(std::string vN="");

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

  void   run(std::vector<std::string>&);

  void setParent(QA*);

  //! Set properties of variable from netcdf file
  void   setVarMetaData(VariableMetaData& );

  //! Name of the netCDF file with results of the quality control
  struct hdhC::FileSplit varReqTable;

  std::vector<VariableMetaData> varMeDa;

  NcAPI* nc;
  QA*    pQA;

  MtrxArr<double> tmp_mv;

  // the same buf-size for all buffer is required for testing replicated records
  size_t bufferSize;

  // init for test about times
  bool isUseStrict;
  bool isClearBits;

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;

  std::string cfStndNames;
  std::string currMIP_tableName;
  std::string frequency;
  static std::vector<std::string> MIP_tableNames;

  std::string experiment_id;
  std::string fVarname;

  static std::string getCaptionIntroDim(
                   struct DimensionMetaData& f_entry,
                   struct DimensionMetaData& t_entry, std::string att="");
  static std::string getCaptionIntroVar(
                   std::string table,
                   struct VariableMetaData& f_entry, std::string att="");
  std::string getVarnameFromFilename(void);
  std::string getVarnameFromFilename(std::string str);
  bool        not_equal(double x1, double x2, double epsilon);
  void        pushBackVarMeDa(Variable*);
};

const char* CMIP5_MIPS[] = {
  "fx",       "Oyr",      "Oclim",    "Amon",      "Omon",     "Lmon",
  "LImon",    "OImon",    "aero",     "day",       "6hrLev",   "6hrPlev",
  "3hr",      "cfMon",    "cfDay",    "cf3hr",     "cfSites",  "cfOff"
};

std::vector<std::string> QA_Exp::MIP_tableNames(CMIP5_MIPS, CMIP5_MIPS + 18);

#endif

