#ifndef _CF_14_H
#define _CF_14_H

#include "hdhC.h"
#include "iobj.h"
#include "annotation.h"
#include "variable.h"
#include "udunits2.h"
#include "converter.h"

class CF : public IObj
{
  public:

  std::map<std::string, int> vIx;

  struct SN_Entry
  {
     SN_Entry(std::string &sn){name = sn; found=false;}

     std::string name;
     std::string std_name;
     std::string remainder;
     bool        found;
     std::string alias;
     std::string canonical_units;
     std::string amip;
     std::string grib;
  };

  CF();
  ~CF(){;}

  //! coresponding to virtual methods in IObj
  bool         closeEntryTime(void){return false;}
  bool         entry(void);
  bool         init(void) ;
  void         linkObject(IObj *);
  void         setFilename(std::string s){;}
  void         setFilePath(std::string p){;}
  void         setTablePath(std::string p){ tablePath=p; }

  void   chap(void);

  void   chap2(void);       // names
  void   chap2_1(void);     // filename
  void   chap2_2(void);     // dimensions
  void   chap2_3(void);     // names
  void   chap2_4(void);     // dimensions
  void   chap2_5_1(void);   // missing data
  void   chap2_6(void);     // attributes
  bool   chap2_6_1(void);   // convention

  void   chap3(void);                  //
  bool   chap3_1_inqDeprecatedUnits(Variable&, std::string &u);
  void   chap3_3(void);     // standard_name etc.
  void   chap3_4(void);     // ancillary_variables
  void   chap3_5(void);     // flags

  void   chap4(void);                  // Coordinate Types
  void   chap4(Variable&);      // Coordinate Types
  bool   chap4_1(Variable&);    // lat/lon coordinate
  bool   chap4_3(Variable&);    // vertical coordinate
  bool   chap4_3_1(Variable&);  // vertical dimensional coord
  void   chap4_3_2(void);       // vertical dimensionless coord
  bool   chap4_3_2(Variable&, std::vector<std::string>&, std::vector<std::string>& );
  bool   chap4_4(Variable&);    // time
  bool   chap4_4_1(Variable&);  // calendar

  void   chap5(void);    // Coordinate Systems
  void   chap5_0(void);  // dim-list of variables; post-poned
  void   chap5_1(Variable&);  // coordinate variables
  void   chap5_2(void);  // coordinate attribute(s)
  void   chap5_3(void);  // reduced horizontal grid:
  void   chap5_4(void);  // time series of station data
//  void   chap5_5(void);  // trajectories: checked implicitely
  void   chap5_6(void);  // grid mapping
  int    chap5_6_gridMappingVar(Variable& dv, std::string &, std::string);
  void   chap5_6_attProps(Variable& dataVar,
                         std::string mCV[]);       // map coordinates

  void   chap6(void);    // labels

  void   chap7(void);    // cells
  void   chap7_1(void);  // cell boundaries
  void   chap7_2(void);  // cell measure
  void   chap7_3(void);  // cell methods
  bool   chap7_3_4(Variable&, std::string&) ;
  void   chap7_4(void);  // climatological statistics

  void   chap8(void);    // reduction of data size
  void   chap8_1(Variable&);  // packed data
  void   chap8_2(Variable&);  // compression by gathering (scal_factor, offset)

  void   chap9(void);    // discrete sampling geometries (CF-1.6)
  void   chap9_featureType(std::vector<std::string> &validFeatureType,
                           std::vector<std::string> &featureType) ;
  bool   chap9_point(std::vector<std::string> &);
  bool   chap9_timeSeries(std::vector<std::string> &);
  bool   chap9_profile(std::vector<std::string> &);
  bool   chap9_trajectory(std::vector<std::string> &);
  bool   chap9_timeSeriesProfile(std::vector<std::string> &);
  bool   chap9_trajectoryProfile(std::vector<std::string> &);

  // recommendations
  void   rc_chap(void);
  void   rc_chap2(void);      // NetCDF files and components
  void   rc_chap2_3(void);    // names
  void   rc_chap2_4(void);    // dimensions of a variable
  void   rc_chap2_6_2(void);  // title and history
  void   rc_chap3(void);      // description of data
  void   rc_chap3_5(void);    // description of data
  void   rc_chap4(void);      // coordinate types
  void   rc_chap4_4a(Variable&);   // ref time 0
  void   rc_chap4_4b(void);   // year and month
  // void rc_chap4_4_1(void) ;  // in chap4_4_1()
  void   rc_chap5(void);      // coordinate types
  void   rc_chap7_1(Variable&);   // cell boundaries
  void   rc_chap7_3b(Variable&, std::vector<std::string> &dim );
  void   rc_chap7_3c(Variable&, std::vector<std::string> &dim,
                                std::vector<std::string> &method);

  void   applyOptions(void);
  void   cellMethods_Climatology(std::vector<std::string> &name,
                                 std::vector<std::string> &method,
                                 Variable&) ;
  void   cellMethods_Comment(std::string&, Variable&) ;
  void   cellMethods_Method(std::string&, Variable&) ;
  void   cellMethods_Name(std::string&, Variable&) ;
  void   cellMethods_WhereOver(std::string&, Variable&, std::string mode) ;
  void   checkAuxCoordData(Variable&) ;
  void   checkCoordVarValues(Variable&) ;
  void   checkCoordVarFillValueAtt(Variable&) ;
  void   checkGroupRelation(void);
  void   checkRelation(void);
  // variables are dimension of another variable
  bool   checkRelationAsDimension(std::vector<bool>&);
  // a dimension of a variable is shared by another variable
  bool   checkRelationByDimension(std::vector<bool>&);
  bool   checkRelationCF16(std::vector<bool>&);
  bool   checkRelationScalar(std::vector<bool>&);
  void   checkSN_Modifier(std::string &vn, std::string &sn, std::string &mod);
  bool   cmpUnits( std::string s, std::string ref);
  void   enableCheck(void){isCheck=true;}
  int    finally(int eCode=0);
  void   finalAtt(void) ;
  void   finalAtt_axis(void);
  void   finalAtt_coordinates(void);
  void   finalAtt_positive(void);
  void   final_dataVar(void);
  void   findAmbiguousCoords(void);
  bool   findLabel(Variable&);
  bool   findCellMeasures(Variable&);
  void   findTimeVariable(void);
  std::vector<std::string>  // mode: "key", "arg", "1st", "2nd". tail=last char of key
         getKeyWordArgs(std::string&, std::string mode="arg", char tail='\0');
  bool   getPureDimension(void);
  void   hasBounds(Variable&);
  void   initDefaults(void);
  void   inqAuxVersusPureCoord(void);
  bool   isBounds(Variable&);
  bool   isCompressEvidence(Variable&, bool*) ; // bool* for int-type
//  bool   isCompliant(void){ return isCF;}
template <typename T>
  bool   is_FillValue(Variable&, T val);
  bool   parseUnits( std::string s);
  bool   run(void);
  void   setCheck(std::string&);
  void   setFollowRecommendations(bool b){followRecommendations=b;}
  void   setTable(std::string p){ std_name_table=p; }
  bool   scanStdNameTable(std::vector<struct SN_Entry>&);
  bool   scanStdNameTable(ReadLine&, struct SN_Entry &);
  bool   timeUnitsFormat(Variable&, bool annot=true);
  std::string
         units_lon_lat(Variable&, std::string units="");
  void   verify_formula_terms(Variable&, int, std::string &requestedFormTerms,
              std::vector<std::string> &fTerms,
              std::vector<std::pair<std::string, std::string> >& p_found_ft);
  // cross-check of standard_name vs. formula_terms (case: dimless vertical coord)
  bool   xCheckSNvsFT( Variable& var,
              std::vector<std::string>& valid_sn,
              std::vector<std::string>& valid_ft,
              int& valid_sn_ix, int& valid_ft_ix,
              std::vector<std::pair<std::string, std::string> >& p_found_ft);

  static std::vector<std::string> attName;
  static std::vector<char>        attType;  // S: string, N: numeric, D: data type

  bool isCF14_timeSeries;

  bool isCheck;  // true: perform checks, false: only set variable values
  size_t cFVal;  // e.g. CF-1.6 --> cFVal=16
  bool followRecommendations; // false by default
  std::string cFVersion;

  bool isPureDimInit;
  std::vector<std::string> pureDimension;

  ut_system*   unitSystem;

  std::string std_name_table;
  std::string region_table;
  std::string tablePath;

  std::string timeName;  // the name of the unlimited/time variable
  int         timeIx;
  int         compressIx;

  // a few names of attributes used throughout the checks
  std::string n_axis;
  std::string n_bounds;
  std::string n_cell_measures;
  std::string n_cell_methods;
  std::string n_cf_role;
  std::string n_climatology;
  std::string n_coordinates;
  std::string n_FillValue;
  std::string n_formula_terms;
  std::string n_long_name;
  std::string n_latitude;
  std::string n_longitude;
  std::string n_missing_value;
  std::string n_positive;
  std::string n_standard_name;
  std::string n_units;
  std::string n_valid_max;
  std::string n_valid_min;
  std::string n_valid_range;

  static const bool lowerCase = true;
  std::string bKey;
  std::string fail;
};

#endif
