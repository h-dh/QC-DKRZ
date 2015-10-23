#ifndef _QA_NONE_H
#define _QA_NONE_H

#include "hdhC.h"
#include "date.h"
#include "annotation.h"
#include "qa.h"
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

  //! Verify units % or 1 by data range
  void verifyPercent(void);
};

class QA_Exp
{
  public:

   //! Default constructor.
  QA_Exp();

  void   applyOptions(std::vector<std::string>&);

  //! Make VarMetaData objects.
  void   createVarMetaData(void);

  std::string
         getAttValue(size_t v_ix, size_t a_ix);

  std::string
         getFrequency(void);

  std::string
         getTableEntryID(std::string vName);

  //! Initialisation of flushing gathered results to netCDF file.
  /*! Parameter indicates the number of variables. */
  void   initDataOutputBuffer(void);

  //! Set default values.
  void   initDefaults(void);

  //! Initialisiation of a resumed session.
  /*! Happens for non-atomic data sets and those that are yet incomplete. */
  void   initResumeSession(std::vector<std::string>& prevTargets);

  //! Check the path to the tables;
  void   inqTables(void){return;}

  void   run(std::vector<std::string>&);

  void   setParent(QA*);

  bool   testPeriod(void); //{return false;}

  std::vector<VariableMetaData> varMeDa;

  NcAPI *nc;
  QA* pQA;

  // the same buf-size for all buffer is required for testing replicated records
  size_t bufferSize;

  // init for test about times
  bool isCaseInsensitiveVarName;
  bool isClearBits;
  bool isUseStrict;  // dummy

  std::vector<std::string> excludedAttribute;
  std::vector<std::string> overruleAllFlagsOption;

  std::vector<std::string> constValueOption;
  std::vector<std::string> fillValueOption;
  std::vector<std::string> outlierOpts;
  std::vector<std::string> replicationOpts;

  std::string frequency;
  std::string fVarname;

  void  pushBackVarMeDa(Variable*);
};

#endif
