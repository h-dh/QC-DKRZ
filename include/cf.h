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
//     SN_Entry(std::string &sn){name = sn; found=false;}

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
  bool   closeEntryTime(void){return false;}
  bool   entry(void);
  bool   init(void) ;
  void   linkObject(IObj *);
  void   setFilename(std::string s){;}
  void   setFilePath(std::string p){;}
  void   setTablePath(std::string p){ tablePath=p; }


  void   chap(void);
  void   chap_reco(void);

  void   chap2(void);       // names
  void   chap2_reco(void);      // NetCDF files and components
  void   chap2_1(void);     // filename
  void   chap2_2(void);     // dimensions
  void   chap2_3(void);     // names
  void   chap2_3_reco(void);    // names
  void   chap2_4(void);     // dimensions
  void   chap2_4_reco(void);    // dimensions of a variable
  void   chap2_5_1(void);   // missing data
  void   chap2_6(void);     // attributes
  bool   chap2_6_1(void);   // convention
  void   chap2_6_2_reco(void);  // title and history

  void   chap3(void);                  //
  void   chap3_reco(void);      // description of data
  void   chap3_3(void);     // standard_name etc.
  void   chap3_4(void);     // ancillary_variables
  void   chap3_5(void);     // flags
  void   chap3_5_reco(void);    // description of data

  void   chap4(void);                  // Coordinate Types
  bool   chap4(Variable&);      // Coordinate Types
  bool   chap4_1(Variable&);    // lat/lon coordinate
  bool   chap4_3(Variable&);    // vertical coordinate
  bool   chap4_3_1(Variable&);  // vertical dimensional coord
  void   chap4_3_2(void);       // vertical dimensionless coord
  bool   chap4_3_2(Variable&, std::vector<std::string>&, std::vector<std::string>&,
                   int ft_jx, int sn_jx);
  // check of standard_name vs. formula_terms (case: dimless vertical coord)
  bool   chap4_3_2_checkSNvsFT( Variable& var,
            std::vector<std::string>& valid_sn,
            std::vector<std::string>& valid_ft,
            int& valid_sn_ix,
            int& ft_ix, int& sn_ix, std::string& units );
  void   chap4_3_2_deprecatedUnits(Variable&, std::string &units);
  void   chap4_3_2_getParamVars( Variable&,
            std::vector<std::string>& valid_sn,
            std::vector<std::string>& valid_ft,
            int& valid_ft_ix, int& valid_sn_ix, int att_ft_ix,
            std::vector<std::pair<std::string, std::string> >& att_ft_pv) ;
  void   chap4_3_2_verify_FT(Variable&, int, std::string &reqFormTerms,
            int att_ft_ix, std::vector<std::string> &fTerms,
            std::vector<std::pair<std::string, std::string> >& p_found_ft);
  bool   chap4_4(Variable&);    // time
  void   chap4_4a_reco(Variable&);   // ref time 0
  bool   chap4_4_1(Variable&);  // calendar

  void   chap5(void);    // Coordinate Systems
  void   chap5_reco(void);      // coordinate types
  void   chap5_0(void);  // dim-list of variables; post-poned
  void   chap5_1(Variable&);  // coordinate variables
  void   chap5_2(void);  // coordinate attribute(s)
  void   chap5_3(void);  // reduced horizontal grid:
//  void   chap5_4(void);  // time series of station data
//  void   chap5_5(void);  // trajectories: checked implicitely
  void   chap5_6(void);  // grid mapping
  int    chap5_6_gridMappingVar(Variable& dv, std::string &, std::string);
  void   chap5_6_attProps(Variable& dataVar,
            std::string mCV[]);       // map coordinates

  void   chap6(void);    // labels

  void   chap7(void);    // cells
  void   chap7_3_inqBounds(Variable&,  std::vector<std::string>& name,
            std::vector<std::string>& method, bool );
  void   chap7_1(void);  // pertains to both boundaries and climatologies
  void   chap7_1_reco(Variable&);   // cell boundaries
  void   chap7_2(void);  // cell measure
  void   chap7_3(void);  // cell methods
  bool   chap7_3_cellMethods_Comment(std::string&, Variable&) ;
  bool   chap7_3_cellMethods_Method(std::string&, Variable&) ;
  bool   chap7_3_cellMethods_Name(std::string&, Variable&) ;
  void   chap7_3b_reco(Variable&, std::vector<std::string> &dim );
  bool   chap7_3_3(std::string& method, Variable&, std::string mode) ;
  bool   chap7_3_4a(std::string&) ;
  void   chap7_3_4b(Variable&, std::vector<std::string> &dim, std::vector<std::string> &method);
  void   chap7_4a(void);  // climatological statistics
  bool   chap7_4b(Variable&, std::vector<std::string> &name, std::vector<std::string> &method) ;

  void   chap8(void);    // reduction of data size
  void   chap8_1(Variable&);  // packed data
  void   chap8_2(Variable&);  // compression by gathering (scal_factor, offset)

  void   chap9(void);    // discrete sampling geometries (CF-1.6)
  void   chap9_featureType(std::vector<std::string> &validFeatureType,
            std::vector<std::string> &featureType) ;
  void   chap9_getSepVars(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  std::vector<std::string>
         chap9_guessFeatureType(std::vector<std::string> &featureType,
           std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix) ;
  bool   chap9_horizontal(std::vector<int>& xyzt_ix);
  void   chap9_MV(std::vector<size_t>& dv_ix);
  bool   chap9_orthoMultDimArray(Variable&, std::vector<int>& xyzt_ix);
  bool   chap9_point(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  bool   chap9_profile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  void   chap9_sample_dimension(std::vector<size_t>& dv_ix);
  bool   chap9_timeSeries(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  bool   chap9_timeSeriesProfile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  bool   chap9_trajectory(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);
  bool   chap9_trajectoryProfile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix);

  void   applyOptions(void);
  void   attributeSpellCheck(void);

  std::string
         captAtt(std::string a);
  std::string
         captAtt(std::string v, std::string a);
  std::string
         captAtt(std::string v, std::string a, std::string&);
  std::string
         captAtt(std::string v, std::string a, std::string&, std::string&);
  std::string
         captAtt(std::string v, std::string a,
                 std::string&, std::string&, std::string&);
  std::string
         captAtt(std::string& v, std::string& a, bool colon, bool blank, bool isUpper);

  std::string
         captVal(std::string v, bool trailingBlank=true);

  std::string
         captVar(std::string v, bool is_colon=true, bool is_blank=true);
  std::string
         captVar(std::string v, std::string&);
  std::string
         captVar(std::string v, std::string&, std::string&);

  void   checkCoordinateValues(Variable&, bool isFormTermAux=false) ;
template <typename T>
  void   checkCoordinateValues(Variable&, bool, T);
  void   checkCoordinateFillValueAtt(Variable&) ;
  void   checkGroupRelation(void);

  // variables are dimension of another variable
  bool   checkRelationAsDimension(std::vector<bool>&);
  // a dimension of a variable is shared by another variable
  bool   checkRelationByDimension(std::vector<bool>&);
  bool   checkRelationCF16(std::vector<bool>&);
  bool   checkRelationScalar(std::vector<bool>&);
  void   checkSN_Modifier(Variable &);
  bool   cmpUnits( std::string s, std::string ref);
  void   enableCheck(void){isCheck=true;}
  int    finally(int eCode=0);
  void   finalAtt(void) ;
  void   finalAtt_axis(void);
  void   finalAtt_coordinates(void);
  void   finalAtt_coordinates_A(std::vector<size_t>& dv_ix,
            std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix );
  void   finalAtt_coordinates_B(std::vector<size_t>& dv_ix,
            std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix );
  void   finalAtt_coordinates_C(std::vector<size_t>& dv_ix,
            std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix );
  void   finalAtt_positive(void);
  void   final_dataVar(void);
  void   findAmbiguousCoords(void);
  bool   findLabel(Variable&);
  void   findCellMeasures(Variable&);
  void   getAssociatedGroups(void);
  void   getDims(void);
  std::vector<std::string>
         // mode: "key", "arg", "1st", "2nd". tail=last char of key
         getKeyWordArgs(std::string&, std::string mode="arg", char tail='\0');
  void   getSN_TableEntry(void);
  void   getVarStateEvidences(Variable&);
  void   hasBounds(Variable&);
  void   initDefaults(void);
  void   inqAuxVersusPureCoord(void);
  bool   isBounds(Variable&);
  bool   isChap6_labelSubstAuxCoord(Variable& coord_aux, std::vector<std::string>& ca);
  bool   isChap9_specialLabel(Variable& label, Variable& var);
  bool   isCompressAux(Variable&);
  bool   isCompressEvidence(Variable&, bool*) ; // bool* for int-type
//  bool   isCompliant(void){ return isCF;}
  bool   isLatitude(void);
  bool   isLongitude(void);
  bool   isXYZinCoordAtt(bool withT=false);
  bool   parseUnits( std::string s);
  void   postAnnotations(void);

  bool   run(void);
  bool   scanStdNameTable(std::vector<int>& zx);
  bool   scanStdNameTable(ReadLine&, Variable&, std::string);
  void   setCheck(std::string&);
  void   setFollowRecommendations(bool b){followRecommendations=b;}
  void   setTable(std::string p){ std_name_table=p; }
  bool   timeUnitsFormat(Variable&, bool annot=true);
  std::string
         units_lon_lat(Variable&, std::string units="");
  double wagnerFischerAlgo(std::string&, std::string&);

  bool isCF14_timeSeries;

  bool isCheck;  // true: perform checks, false: only set variable values
  size_t cFVal;  // e.g. CF-1.6 --> cFVal=16
  bool followRecommendations; // false by default
  bool isFeatureType;
  std::string cFVersion;

  ut_system*   unitSystem;

  std::string std_name_table;
  std::string region_table;
  std::string tablePath;

  std::string timeName;  // the name of the unlimited/time variable
  int         timeIx;
  int         compressIx;

  // a few names of attributes used throughout the checks
  std::string n_ancillary_variables;
  std::string n_area;
  std::string n_attribute;
  std::string n_axis;
  std::string n_bounds;
  std::string n_calendar;
  std::string n_cell_measures;
  std::string n_cell_methods;
  std::string n_cf_role;
  std::string n_climatology;
  std::string n_compress;
  std::string n_Conventions;
  std::string n_coordinates;
  std::string n_dimension;
  std::string n_featureType;
  std::string n_FillValue;
  std::string n_flag_masks;
  std::string n_flag_values;
  std::string n_formula_terms;
  std::string n_global;
  std::string n_grid_latitude;
  std::string n_grid_longitude;
  std::string n_grid_mapping;
  std::string n_instance_dimension;
  std::string n_long_name;
  std::string n_latitude;
  std::string n_leap_month;
  std::string n_leap_year;
  std::string n_longitude;
  std::string n_missing_value;
  std::string n_month_lengths;
  std::string n_NC_GLOBAL;
  std::string n_number_of_observations;
  std::string n_positive;
  std::string n_standard_name;
  std::string n_sample_dimension;
  std::string n_time;
  std::string n_units;
  std::string n_valid_max;
  std::string n_valid_min;
  std::string n_valid_range;
  std::string n_variable;

  std::string blank;
  std::string no_blank;
  std::string no_colon;
  std::string s_empty;
  std::string s_upper;
  std::string s_lower;

  static const bool lowerCase = true;
  std::string bKey;
  std::string fail;
  std::string NO_MT;

  std::vector<std::string>        associatedGroups;
  static std::vector<std::string> attName;
  static std::vector<char>        attType;  // S: string, N: numeric, D: data type
  std::vector<std::string>        dimensions;
  std::vector<size_t>             effDims_ix;
  std::vector<size_t>             varRepDims_ix;

  // properties of coordinates attributes
  std::vector<std::pair<int, int> > ca_pij;
  std::vector<int> ca_ix; // ca_vx[i] points to var_ix, def.: -1
  std::vector<std::vector<std::string> > ca_vvs ;
};

#endif
