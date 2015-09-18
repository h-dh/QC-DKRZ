//#include "qa.h"

// Macro option to enable output of all messages.
// Please compile with '-D RAISE_ERRORS'

QA::QA()
{
  initDefaults();
}

QA::~QA(void)
{
  if( nc )
    delete nc ;
}

void
QA::appendToHistory(size_t xCode)
{
  // date and time at run time
  std::string today( Date::getTodayStr() );
  today += ":";

  std::string s;
  std::string hst;
  std::string hstVersion;

  s = nc->getAttString("history");
  if( s.size() )
  {
    // append to history

    // check for a changed path to data
    Split splt(s);
    if( splt.size() > 2 && splt[1] == "path_to_data:" )
    {
      std::string hstPath(splt[2]);
      size_t pos;
      if( (pos=hstPath.rfind("\n")) < std::string::npos )
         hstPath.erase(pos,1);
      if( qaFile.path != hstPath )
      {
        hst += "\n" ;
        hst += today;
        hst += " changed path to data: ";
        hst += qaFile.path + "\n" ;
      }
    }
  }
  else
  {
    // the root of the history string
    hst += today;
    hst += " path_to_data: ";
    hst += qaFile.path ;
  }

  // did the package version change? Yes? Then add to history
  if( s.size() && enableVersionInHistory )
  {
    // look for the last version file named in the historys string,
    // thus a reversed scan for the first version element
    Split splt(s);
    size_t index=splt.size()-1;
    //note: would be an infinite loop for index >= 0,
    //      because of size_t value is positively definit.
    for( ; index >= 1 ; --index )
      if( splt[index] == "revision:" )
        break;

    std::string tmp ;
    if( index > 0 )
    {
      ++index; // version number in the next position
      tmp = splt[index] ;
    }
    else
      // not in the history, so take the one from the version attribute
      tmp = nc->getAttString("QA revision");

    if( revision != tmp )
    {
      hst += "\n" ;
      hst += today;
      hst += " changed QA revision: " ;
      hst += revision ;
    }
  }

  if( hst.size() )
  {
    hst = s + hst ;
    nc->setGlobalAtt( "history", hst);
  }

  return;
}

void
QA::applyOptions(bool isPost)
{
  enablePostProc=isPost;

  // the first loop for items with higher precedence
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "pP" || split[0] == "postProc"
       || split[0] == "post_proc")
     {
       enablePostProc=true;
       break;
     }
  }

  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "cIVN"
            || split[0] == "CaseInsensitiveVariableName"
                 || split[0] == "case_insensitive_variable_name" )
     {
          isCaseInsensitiveVarName=true;
          continue;
     }

     if( split[0] == "cM"
       || split[0] == "checkMode"
         || split[0] == "check_mode" )
     {
       if( split.size() == 2 )
         setCheckMode(split[1]);

        continue;
     }

     if( split[0] == "dIP"
          || split[0] == "dataInProduction"
              || split[0] == "data_in_production" )
     {
        // effects completeness test in testPeriod()
        isFileComplete=false;
        continue;
     }

     if( split[0] == "dP"
          || split[0] == "dataPath" || split[0] == "data_path" )
     {
       if( split.size() == 2 )
          // path to the directory where the execution takes place
          qaFile.setPath(split[1]);
       continue;
     }

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);
       continue;
     }

     if( split[0] == "f" )
     {
       if( split.size() == 2 )
          qaFile.setFile(split[1]);
       continue;
     }

     if( split[0] == "fST" || split[0] == "ForceStndTable"
         || split[0] == "force_stnd_table" )
     {
          isForceStndTable=true ;
          continue;
     }

     if( split[0] == "nextRecords" )
     {
       if( split.size() == 2 )
          nextRecords=static_cast<size_t>(split.toDouble(1));
       continue;
     }

     if( split[0] == "oT"
           || split[0] == "outlierTest"
                || split[0] == "outlier_test" )
     {
       if( enablePostProc && split.size() == 2 )
          outlierOpts.push_back(split[1]);
       continue;
     }

     if( split[0] == "pEI" || split[0] == "parentExperimentID"
         || split[0] == "parent_experiment_id" )
     {
       if( split.size() == 2 )
       {
          parentExpID=split[1];
          if( parentExpID == "none" )
            isCheckParentExpID=false ;
       }
       continue;
     }

     if( split[0] == "pER" || split[0] == "parentExperimentRIP"
         || split[0] == "parent_experiment_rip" )
     {
       if( split.size() == 2 )
       {
          parentExpRIP=split[1];
          if( parentExpRIP == "none" )
            isCheckParentExpRIP=false ;
       }
       continue;
     }

     if( split[0] == "qNF" || split[0] == "qaNcfileFlags"
       || split[0] == "qa_ncfile_flags" )
     {
       if( split.size() == 2 )
          qaNcfileFlags=split[1];
       continue;
     }

     if( split[0] == "rR"
            || split[0] == "replicatedRecord"
                 || split[0] == "replicated_record" )
     {
       if( split.size() > 1 )
       {
          // input could be: ..., only_groups=num
          // then num would be in split[2]
          std::string tmp( split[1] );
          if( split.size() == 3 )
          {
            tmp += '=' ;
            tmp += split[2] ;
          }

          Split csv ;
          csv.setSeparator(",");
          BraceOP groups(tmp);
          while ( groups.next(tmp) )
          {
            csv = tmp;
            for( size_t i=0 ; i < csv.size() ; ++i )
              replicationOpts.push_back(csv[i]);
          }
       }
       continue;
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
          tablePath=split[1];
       continue;
     }

     if( split[0] == "totP"
          || split[0] == "totalPeriod" || split[0] == "total_period")
     {
       if( split.size() == 2 )
          totalPeriod = split[1] ;
       continue;
     }

     if( split[0] == "tPr"
          || split[0] == "tableProject" )  // dummy
     {
       if( split.size() == 2 )
          projectTableFile.setFile(split[1]);
       continue;
     }

     if( split[0] == "tStd"
          || split[0] == "tableStandard" )
     {
       if( split.size() == 2 )
          varReqTable.setFile(split[1]);
       continue;
     }

     if( split[0] == "uS" || split[0] == "useStrict"
            || split[0] == "use_strict" )
     {
          isUseStrict=true ;
          isForceStndTable=true;
          setCheckMode("meta");
     }
     continue;
   }

   // apply a general path which could have also been provided by setTablePath()
   if( projectTableFile.path.size() == 0 )
      projectTableFile.setPath(tablePath) ;

   if( varReqTable.path.size() == 0 )
      varReqTable.setPath(tablePath);

   return;
}

void
QA::checkDataVarNum(void)
{
  //each file must contain only a single output field fro ma single simulation
  if( pIn->dataVarIndex.size() == 1 )
    return;

  std::string key("9_1");
  if( notes->inq( key) )
  {
    std::string capt("multiple output fields in the file, found variables: ") ;

    for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
    {
      if(i)
        capt += ", ";

      capt += pIn->variable[pIn->dataVarIndex[i]].name;
    }
  }

  return;
}

bool
QA::checkDimlessVar(InFile &in, Split &splt_line,
   VariableMetaData &vMD,
   struct DimensionMetaData *&p_dimFE,
   struct DimensionMetaData &dimFE_altern,
   struct DimensionMetaData &dimTE,
   std::map<std::string, size_t> &col)
{
  // This 'swaps' properties from a dim-less variable to
  // the corresponding dimension in the table.

  // Dimensions of size==1 in the table are usually
  // not defined as dimension in the file, but only as variable
  // representation of a level. Detect this, switch to the
  // temporary dimFE and fill from variable attributes.

  // get settings for the respective column; default separator is ' '
  Split splt_value( splt_line[col["value"]] );

  if( splt_value.size() != 1 )
    return true;

  // switch
  p_dimFE = &dimFE_altern;

  dimFE_altern.cmor_name = dimTE.cmor_name ;
  dimFE_altern.outname    = dimTE.outname ;

  std::string vName(dimTE.outname) ;

  // toggle to false, if nothing was stored
  dimFE_altern.isUnitsDefined=true;
  dimFE_altern.units    = in.nc.getAttString("units", vName,
                          dimFE_altern.isUnitsDefined);
  dimFE_altern.units
     = hdhC::clearInternalMultipleSpaces(dimFE_altern.units);
  dimFE_altern.axis     = in.nc.getAttString("axis", vName);
  dimFE_altern.longname = in.nc.getAttString("long_name", vName);
  dimFE_altern.longname
     = hdhC::clearInternalMultipleSpaces(dimFE_altern.longname);
  dimFE_altern.stndname = in.nc.getAttString("standard_name", vName);
  dimFE_altern.type     = in.nc.getVarType(vName);
  dimFE_altern.size     = 1 ;
  dimFE_altern.bnds_name = in.nc.getAttString("bounds", vName);

  if( in.nc.getVarType(vName) == NC_NAT )  // NC_NAT == 0
  {
    // no dim and no variable in the file, but
    // the standard table has depth , height or
    // something like that in the dim-list of the
    // variable
    std::string key("5_5");
    if( notes->inq( key, vName) )
    {
      std::string capt("missing auxiliary in the file") ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Conflict for auxiliary between file and table." ;
      ostr << "\nStandard table: " << vMD.varReqTableSheet ;
      ostr << ", MIP-Table: " << vMD.varReqTableSheetSub ;
      ostr << ", Variable: " << vMD.var->name ;
      ostr << "\nExpected auxiliary: " << vName ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }

      return false;
    }
  }

  // get the value of the var-representation from the file
  if( in.nc.getVarType(vName) == NC_CHAR )
  {
    std::vector<std::string> vs;
    in.nc.getData(vs, vName);

    bool reset=true;  // is set to false during the first call
    for(size_t i=0 ; i < vs.size() ; ++i)
    {
      vs[i] = hdhC::stripSurrounding(vs[i]);
      dimFE_altern.checksum = hdhC::fletcher32_cmip5(vs[i], &reset) ;
    }
  }
  else
  {
    MtrxArr<double> mv;
    in.nc.getData(mv, vName);

    bool reset=true;
    for( size_t i=0 ; i < mv.size() ; ++i )
      dimFE_altern.checksum = hdhC::fletcher32_cmip5(mv[i], &reset) ;
  }

  return true;
}

void
QA::checkDimVarReqTable(ReadLine &ifs, InFile &in,
  VariableMetaData &vMD,
  std::vector<struct DimensionMetaData> &dimNcMeDa,
  std::map<std::string, size_t> &col,
  std::string dimName, size_t colMax )
{
  // dimName is a so-called CMOR-dimension in the standard table.
  // This method is called for each of such a dimension.

  // dimName is the only unique connection between the dimensional
  // table sheet and those table sheets for the variables.
  // Two kinds of names are given: the CMOR variant giving explicit
  // names for dedicated purposes and the 'output name'.
  // The 'output name' is ambivalent by subsuming
  // a CMOR dimension family  to a single type
  // (e.g. plev7, plev17, etc. --> plev)

  // Dimensions are described in the upper part of the standard table,
  // which was opened in the calling method/function.

   std::string str0;

   // back to the beginning.
   ifs.FStream->seekg(0, std::ios::beg) ;

   // This should give the heading for the dimension table sheet.
   // The table sheet is identified by the first column.
   while( !ifs.getLine(str0) )
   {
     if( str0.substr(0,13) == "CMOR table(s)" )
       break ;
   }

   struct DimensionMetaData dimTE ;

   // purpose: a dim in the table is only available as variable
   // in the file
   struct DimensionMetaData  tmp_dimFE ;
   VariableMetaData   tmp_vMD(this) ;

//   VariableMetaData &vMD = varMeDa[ivMD] ;

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   splt_line=str0;

   // look for a matching entry in the dimension table.
   while( ! ifs.getLine(str0) )
   {
     splt_line=str0;

     // end-of-table sheet
     if( ifs.eof()
         || splt_line[0].substr(0,10) == "CMOR Table" )
       break;

     // is it in the table?
     if( splt_line[col["cmorName"]] == dimName )
     {

       if( splt_line.size() < colMax )
       {
          std::string key("4_2");

          if( notes->inq( key) )
          {
            std::string capt("corrupt standard sub-table for dimensions: wrong number of columns.") ;

            std::ostringstream ostr(std::ios::app);
            ostr << "Standard table: " << varReqTable.filename;
            ostr << "\nCurrent line: " << str0 ;
            ostr << "\nRequired number of items of the table sheet for ";
            ostr << "dimensions is " << colMax << ", but found ";
            ostr << splt_line.size() << "." ;

            (void) notes->operate(capt, ostr.str()) ;
            {
              notes->setCheckDataStr(fail);
              setExit( notes->getExitValue() ) ;
            }
         }
       }

       dimTE.cmor_name=splt_line[col["cmorName"]];
       dimTE.outname=splt_line[col["outputName"]];
       dimTE.stndname=splt_line[col["standardName"]];
       dimTE.longname=splt_line[col["longName"]];
       dimTE.longname
         = hdhC::clearInternalMultipleSpaces(dimTE.longname);
       dimTE.units=splt_line[col["units"]];
       dimTE.units
         = hdhC::clearInternalMultipleSpaces(dimTE.units);
       if( dimTE.units.size() )
         dimTE.isUnitsDefined=true;
       else
         dimTE.isUnitsDefined=false;
       dimTE.type=splt_line[col["type"]];
       dimTE.coordsAtt=splt_line[col["coord"]];
       dimTE.index_axis=splt_line[col["index_axis"]];
       dimTE.axis=splt_line[col["axis"]];
       dimTE.bnds_name=splt_line[col["bounds?"]];

       dimTE.value=splt_line[col["value"]];
       dimTE.requested=splt_line[col["requested"]];

       size_t index;
       for( index=0 ; index < dimNcMeDa.size() ; ++index)
       {
          if( dimNcMeDa[index].outname == dimTE.outname )
            // regular grid:  var(...,dim,...) and dim(dim,...) available
            for( size_t i=0 ; i < in.variable.size() ; ++i )
              if( in.variable[i].name == dimNcMeDa[index].coordsAtt )
                goto BREAK2;

          // special:lon / lat parameterised by an index array
          if( dimTE.outname == "lon" || dimTE.outname == "lat" )
            if( checkLonLatParamRep(in, vMD, dimNcMeDa[index].outname, dimTE.outname) )
               return;  // index array has not the usual attributes
       }

BREAK2:
       // special: the standard tables contains also dimensions of size==1 or none in the
       // list of dimensions; this is often discarded from the list of dims of a
       // variable in the file
       if( index == dimNcMeDa.size() )
       {
          // the user could have defined a dimension of size==1 or,
          // as usual, discarded size and dimension.
          if(   in.nc.isDimValid(dimTE.outname) )
          {
             if( in.nc.isVariableValid(dimTE.outname) )
             {
               dimNcMeDa.push_back( DimensionMetaData() );
               getDimMetaData(in, vMD, dimNcMeDa.back(), dimTE.outname) ;
               checkDimVarReqTable(ifs, in, vMD, dimNcMeDa,
                  col, dimTE.cmor_name, colMax );
             }
          }
          else
          {
            if( in.nc.isVariableValid(dimTE.outname) )
            {
              dimNcMeDa.push_back( DimensionMetaData() );
              getDimMetaData(in, vMD, dimNcMeDa.back(), dimTE.outname) ;
              checkDimVarReqTable(ifs, in, vMD, dimNcMeDa,
                 col, dimTE.cmor_name, colMax );
            }
          }
       }

       // just the existance of a var-rep
       if( checkDimSpecialValue(in, vMD, dimTE, dimName) )
          return;

       if( dimTE.cmor_name == "olevel" || dimTE.cmor_name.substr(4) == "alev" )
         return;  // no 'values', no 'requested', but a single value in a var-rep in the file

       Split splt_value(splt_line[col["value"]]);  // to be used in the block

       // There could be layers additionally
       // to the 17 mandatory ones for the dimension 'plevs'.
       // Thus, we have to take into account the 17 mandatory
       // ones for the comparison with the standard table and
       // the real number from the file for the project table.
       size_t   plevs_file_size = dimNcMeDa[index].size ;
       uint32_t plevs_file_checksum = dimNcMeDa[index].checksum;

       // switch between the regular objects and the one defined above.
       struct DimensionMetaData *p_dimFE = &dimNcMeDa[index] ;
       VariableMetaData  *p_vMD   = &vMD;

       // Purpose: no straight definition of a dim from the table within
       // the file. but given as scalar variable in the file.
       // Switch to a pseudo dimFE object.
       if( !checkDimlessVar(in, splt_line,
              vMD, p_dimFE, tmp_dimFE, dimTE, col) )
         return; // aux is not in the file

       // size and checksum of dimensional values
       checkVarReqTableDimValues(in, splt_line,
              *p_vMD, *p_dimFE, dimTE, col);

       // size and checksum of dimensional bounds
       checkVarReqTableDimBounds(in, splt_line,
              *p_vMD, *p_dimFE, dimTE, col);

       // compare findings from the file with those from the table
       checkDimVarReqTableEntry(in, *p_vMD, *p_dimFE, dimTE) ;

       if( dimTE.cmor_name == "plevs" )
       {
         // restore for the project table
         dimNcMeDa[index].size     = plevs_file_size  ;
         dimNcMeDa[index].checksum = plevs_file_checksum ;
       }

       return;
    }
  }

  // error, because the dimensions looked for is
  // from a CMOR Table for variables, but was not found in the
  // table sheet for dimensions.

  std::string key("4_1");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt("dimension not found in the standard_ouput table.") ;

    std::ostringstream ostr(std::ios::app);
    ostr << "QA::checkDimVarReqTable()";
    ostr << "\nStandard_output table: " << varReqTable.filename;
    ostr << "\nDimension " <<  dimName;
    ostr << " not found in the table.";

    std::string t0(dimName);
    t0 += ": not found in the standard table";

    (void) notes->operate(capt, ostr.str()) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

bool
QA::checkDimSpecialValue(InFile &in, VariableMetaData &vMD,
      struct DimensionMetaData &dimTE, std::string &dimName)
{
   // this method is visited whenever a dimension in the var-requirement
   // table has no corresponding var-rep in the file of the same name.

   // required to enter the second if-clause in case the first one wasn't
   size_t ix;

   // the user could have defined a dimension of size==1, but the var-rep
   // is given by a different name. On the other hand, the dimension in the table
   // corresponds to a single value;

   if( in.nc.isDimValid(dimTE.outname) && in.nc.getDimSize( dimTE.outname ) == 1 )
   {
     // find var-rep with a single dim of size==1
     // possible that the var-rep is named slightly differently, e.g. height and height2m
     size_t sz=dimTE.outname.size() ;

     for( ix=0 ; ix < in.varSz ; ++ix )
     {
       if( in.variable[ix].dimName.size() == 1
              && in.variable[ix].dimName[0].substr(0,sz) == dimTE.outname )
       {
         // only a single value: postponed coding
         return true ;
       }
     }
   }

   // size and dimension is often discarded, then the var-rep must have no dimension.
   if( ix == in.varSz )
   {
     for( ix=0 ; ix < in.varSz ; ++ix )
     {
       if( in.variable[ix].dimName.size() == 0 )
       {
         // only a single value: postponed coding
         return true;
       }
     }
   }

   if( ix == in.varSz )
   {
     std::string key("4_3");

     if( notes->inq( key, vMD.var->name) )
     {
       std::string capt("dimension from the table not found in the file.") ;

       std::ostringstream ostr(std::ios::app);
       ostr << "Variable Requirements table: ";
       ostr << varReqTable.filename;
       ostr << ", variable: ";
       ostr << vMD.var->name;
       ostr << ", dimension: " <<  dimName;
       ostr << "\nFile: dimension not available ";

       (void) notes->operate(capt, ostr.str()) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
     }

     return true;
   }

   // no check of any values
/*
   // there are var-reps which bear the value they ought to have in the name,
   // e.g. height10m
   isAnnot=false;

   MtrxArr<double> mv;
   in.nc.getData(mv, in.variable[ix].name);
   std::string s_mv ;
   if( mv.size() )
     s_mv=hdhC::double2String( mv[0] );
   else
     isAnnot=true;

   if( in.variable[ix].unit.size() )
     s_mv += in.variable[ix].unit ;
   else
     isAnnot=true;

   size_t pos;
   // this is such a constructred name
   if( (pos=in.variable[ix].name.rfind(s_mv)) < std::string::npos )
   {
      if( (pos+s_mv.size()) != in.variable[ix].name.size() )
        isAnnot=true;
   }
   else
     isAnnot=true;
*/

   return false;
}

void
QA::checkDimVarReqTableEntry(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?
  // Failed checks against a project table result in issuing an
  // error message. Only warnings for a failed check against
  // the standard table.

  // special: variable 'lev' contains the added coefficients a and b
  // of the hybrid sigma pressure coordinates. Also, the size of
  // this dimension may vary.
  // Is done separately.
  if( currTable == varReqTable.filename && tbl_entry.outname == "lev" )
      return;

  if( tbl_entry.outname != nc_entry.outname )
     checkDimOutName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.stndname != nc_entry.stndname )
     checkDimStndName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.longname != nc_entry.longname )
     checkDimLongName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.axis != nc_entry.axis )
     checkDimAxis(in, vMD, nc_entry, tbl_entry);

  // the second condition takes especially into account
  // declaration of a variable across table sheets
  // (Omon, cf3hr, and cfSites)
  if( tbl_entry.bnds_name == "yes"
       &&  vMD.cellMethods != "time: point" )
     checkDimBndsName(in, vMD, nc_entry, tbl_entry);

  // special for the unlimited dimension
  if( tbl_entry.outname == "time" )
  {
    checkDimULD(vMD, nc_entry, tbl_entry) ;
  }
  else
  {
    // all the items in this else-block have a transient meaning
    // for the time variable

    // special: 'plevs' check only for the 17 mandatory levels.
    //          This in ensured by the calling method.

    if( tbl_entry.size != nc_entry.size )
      checkDimSize(in, vMD, nc_entry, tbl_entry);

    // note: an error in size skips this test
    else if( tbl_entry.checksum != nc_entry.checksum)
      checkDimChecksum(in, vMD, nc_entry, tbl_entry);

    // special: units
    checkDimUnits(in, vMD, nc_entry, tbl_entry);
  }

  return ;
}

void
QA::checkDimAxis(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("3_5");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() );
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);
    capt += " axis";

    if( tbl_entry.axis.size() && nc_entry.axis.size() )
    {
      capt += ": mismatch of values, found " ;
      capt += nc_entry.axis;
      capt += ", required " + tbl_entry.axis;
    }
    else if( tbl_entry.axis.size() )
    {
      capt += " in the file: N/A, required " ;
      capt += tbl_entry.axis;
    }
    else
    {
      capt += " in the table: N/A, found " ;
      capt += nc_entry.axis;
    }

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(fail);
    setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA::checkDimBndsName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // standard table
  if( nc_entry.bnds_name.size() )
    return ;

  std::string key("3_7");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() );
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;
    capt += " bounds not available in the table, requested: " ;
    capt += tbl_entry.bnds_name ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(fail);
    setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA::checkDimChecksum(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("3_6");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt = getCurrentTableSubst() ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;
    capt += " layout (checksum) changed, expected " ;
    capt += hdhC::double2String(tbl_entry.checksum) ;
    capt += ", found " + hdhC::double2String(nc_entry.checksum) ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(fail);
    setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
QA::checkLonLatParamRep(InFile &in,
           VariableMetaData &vMD,
           std::string &dimFE_name, std::string &dimTE_name)
{
   // The simple case, lon(lon) and var(...,lon,...), was done already; same for lat


   // special: lon/lat may be parameterised
   // parameter representation; there is a var(...,dimFE_name,...)
   // and variable lon/lat(dimFE_name,...) with
   // a) dimFE_name: variable
   // b) dimFE_name: type int
   // c) dimFE_name: units==1 or empty

   // find var-rep of dim and analyse var-reps's dimensions
   for( size_t i=0 ; i < in.variable.size() ; ++i )
   {
     struct Variable &vLonLat = in.variable[i];

     // find lon/lat variable
     if( vLonLat.name.substr(0,3) == dimTE_name.substr(0,3) )
     {
       // check that dimFE_name is dimensions of lon/lat,
       bool is=true;
       for( size_t j=0 ; j < vLonLat.dimName.size() ; ++j )
         if( vLonLat.dimName[j] == dimFE_name )
           is=false;

       if( is )
         goto BREAK ;

       // check that dimFE_name is a variable
       for( size_t k=0 ; k < in.variable.size() ; ++k )
       {

         if( in.variable[k].name == dimFE_name )
            // type of int
            if( in.nc.getVarTypeStr( in.variable[k].name ) == "int" )
               // must be unit==1 or undefined
               if( in.variable[k].units == "1" || in.variable[k].units.size() == 0 )
                  return true;
       }
     }
   }

BREAK:
   return false;
}

void
QA::checkDimLongName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("3_4");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() );
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

    if( tbl_entry.longname.size() && nc_entry.longname.size() )
      capt += " long name conflict." ;
    else if( tbl_entry.longname.size() )
      capt += " long name not available in the file." ;
    else
      capt += " long name not available in the table." ;

    std::string text("            long name:\t table: ") ;
    if( tbl_entry.longname.size() )
      text += tbl_entry.longname ;
    else
      text += notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.longname.size() )
      text += nc_entry.longname ;
    else
      text += notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA::checkDimOutName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("47_1");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() ) ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

    if( tbl_entry.outname.size() && nc_entry.outname.size() )
      capt += " output name conflict." ;
    else if( tbl_entry.outname.size() )
      capt += " output name not available in the file." ;
    else
      capt += " output name not available in the table." ;

    std::string text("          output name:\t table: ") ;
    if( tbl_entry.outname.size() )
      text += tbl_entry.outname ;
    else
      text += notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.outname.size() )
      text += nc_entry.outname ;
    else
      text += notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
QA::checkDimSize(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("3_8");
  key += nc_entry.outname;
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() ) ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);
    capt += " size conflict.";

    std::string text("             dim-size:\t table: ") ;
    text += hdhC::double2String(tbl_entry.size) ;
    text += "\n                      \tncfile: " ;
    text += hdhC::double2String(nc_entry.size) ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA::checkDimStndName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("3_3");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() ) ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

    if( tbl_entry.stndname.size() && nc_entry.stndname.size() )
      capt += " standard name conflict." ;
    else if( tbl_entry.stndname.size() )
      capt += " standard name not available in the file." ;
    else
      capt += " standard name not available in the table." ;

    std::string text("        standard name:\t table: ") ;
    if( tbl_entry.stndname.size() )
      text += tbl_entry.stndname ;
    else
      text += notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.stndname.size() )
      text += nc_entry.stndname ;
    else
      text += notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA::checkDimULD(
     VariableMetaData &vMD,
     struct DimensionMetaData &nc_entry,
     struct DimensionMetaData &tbl_entry)
{
  // Special for the unlimited dimension

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?
  // Checking ranges is senseless, thus discarded.


  if( tbl_entry.units.find('?') < std::string::npos )
  {
    // only standard table

    // Note: units of 'time' in standard table: 'days since ?',
    //       in the file: days since a specific date.
    //       Remember, the standard table is only visited once.
    //       Afterwards, checks are against the project table.

    // we accept one or more blanks
    if( ( tbl_entry.units.find("days") < std::string::npos
          && tbl_entry.units.find("since") < std::string::npos )
                       !=
        (  nc_entry.units.find("days") < std::string::npos
          &&  nc_entry.units.find("since") < std::string::npos ) )
    {
      std::string key("3_9");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCurrentTableSubst() ) ;
        capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

        if( tbl_entry.units.size() && nc_entry.units.size() )
          capt += " units conflict." ;
        else if( tbl_entry.units.size() )
          capt += " missing key string in units in the file: PERIOD since." ;
        else
          capt += " missing key string in units in the table: PERIOD since." ;

        std::string text("                units:\t table: ") ;
        if( tbl_entry.units.size() )
          text += tbl_entry.units ;
        else
          text += "missing key string: days since" ;

        text += "\n                      \tncfile: " ;
        if( nc_entry.units.size() )
          text += nc_entry.units ;
        else
          text += "missing key string: days since" ;

        (void) notes->operate(capt, text) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }
  }
  else
  {  // Compare against the project table.
    if( tbl_entry.units != nc_entry.units )
    {
      // Note: a different date between the two might not be a fault.
      // If keywords match, but the dates do not, then emit
      // only a warning.
      if( ( tbl_entry.units.find("days") < std::string::npos
            && tbl_entry.units.find("since") < std::string::npos )
                         !=
          (  nc_entry.units.find("days") < std::string::npos
            &&  nc_entry.units.find("since") < std::string::npos ) )
      {
        std::string key("3_9");
        if( notes->inq( key, vMD.var->name) )
        {
          std::string capt( getCurrentTableSubst() ) ;
          capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;

          if( tbl_entry.units.size() && nc_entry.units.size() )
            capt += " units conflict." ;
          else if( tbl_entry.units.size() )
            capt += " missing key string in units in the file: PERIOD  since." ;
          else
            capt += " missing key string in units in the table: PERIOD  since." ;

          std::string text("                units:\t table: ") ;
          if( tbl_entry.units.size() )
            text += tbl_entry.units ;
          else
            text += "missing key string: days since" ;

          text += "\n                      \tncfile: " ;
          if( nc_entry.units.size() )
            text += nc_entry.units ;
          else
            text += "missing key string: days since" ;

          (void) notes->operate(capt, text) ;
          {
            notes->setCheckMetaStr(fail);
            setExit( notes->getExitValue() ) ;
          }
        }
      }
      else // deviating reference dates
      {
        // There is nothing wrong with deviating references
        if( ! qaTime.isReferenceDate )
          return ;  // do not check at all

        // do not check at the beginning of a new experiment.
        if( ! qaTime.isReferenceDateAcrossExp && currQARec == 0 )
          return ;

        Date tbl_ref( tbl_entry.units, qaTime.calendar );
        Date nc_ref( nc_entry.units, qaTime.calendar );

        std::string key("3_10");
        if( notes->inq( key, vMD.var->name) )
        {
          std::string capt( getCurrentTableSubst() ) ;
          capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;

          if( tbl_entry.units.size() && nc_entry.units.size() )
            capt += " units: different reference dates." ;
          else if( tbl_entry.units.size() )
            capt += " units: reference date not available in the file." ;
          else
            capt += " units: reference date not available in the table." ;

          std::string text ;
          if( tbl_ref != nc_ref )
          {
            text += "                units:\t table: " ;
            if( tbl_entry.units.size() )
              text += tbl_entry.units ;
            else
              text += notAvailable ;

            text += "\n                      \tncfile: " ;
            if( nc_entry.units.size() )
              text += nc_entry.units ;
            else
              text += notAvailable ;
          }

          (void) notes->operate(capt, text) ;
          {
            notes->setCheckMetaStr(fail);
            setExit( notes->getExitValue() ) ;
          }
        }
      }
    }
  }

  return ;
}

void
QA::checkDimUnits(InFile &in,
     VariableMetaData &vMD,
     struct DimensionMetaData &nc_entry,
     struct DimensionMetaData &tbl_dim_entry)
{
  // Special: units related to any dimension but 'time'

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?

  // index axis has no units
  if( tbl_dim_entry.index_axis == "ok")
    return;

  if( tbl_dim_entry.units == nc_entry.units )
    return;

  // I) dim==lev: generic
  if( tbl_dim_entry.outname == "lev" )
    return;  // this may have units or not

  std::string tableType;
  if( currTable == varReqTable.filename )
    tableType = "standard table: ";
  else
    tableType = "project table: ";

  std::string t0;

  // I) dimension's var-rep without units in the standard table
  if( tbl_dim_entry.outname == "site")
  {
    std::string key("47_6");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt( getCurrentTableSubst() );
      capt += getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry) ;

      if( nc_entry.units.size() && nc_entry.units != "1" )
        capt += " dimension=site: has units." ;

      std::string text("                units:\t table: not defined") ;
      text += "\n                      \tncfile: " ;
      if( nc_entry.units.size() )
        text += nc_entry.units ;

      (void) notes->operate(capt, text) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }

      return;  // integer index of stations (sites)
    }
  }

  // II) dimensions without a literal var-rep
  std::vector<std::string> dimVar;
  dimVar.push_back("region");
  dimVar.push_back("passage");
  dimVar.push_back("type_description");

  std::vector<std::string> dim;
  dim.push_back("basin");
  dim.push_back("line");
  dim.push_back("type");

  // note: the loop is mostly passed without action
  for(size_t ix=0 ; ix < dimVar.size() ; ++ix )
  {
    if( tbl_dim_entry.outname != dim[ix] )
      continue;

    // Is coordinates-att set in the corresponding variable?
    bool isDefined=true;
    std::string tmp( in.nc.getAttString(
                    "coordinates",vMD.var->name, isDefined ) );

    if( tmp != dimVar[ix] )
    {
      std::string key("4_10");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCurrentTableSubst() );
        capt += getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry);

        if( isDefined )
          capt += " incorrect coordinate attribute." ;
        else
          capt += " coordinate attribute not defined." ;

        std::string text("   var coordinate-att:\t table: ") ;
        text += dimVar[ix] ;

        text += "\n                      \tncfile: " ;
        if( isDefined )
          text += tmp ;
        else
          text += "coordinate attribute not defined" ;

        (void) notes->operate(capt, text) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }

    // check unit attribute of the coord-att variable.
    isDefined=true;
    tmp = in.nc.getAttString("units", dimVar[ix], isDefined ) ;

    if( isDefined && tmp != "1" )
    {
      std::string key("3_13");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCurrentTableSubst() ) ;
        capt += getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry);
        capt += " incorrect units." ;

        t0 = dimVar[ix] + ": incorrect units";

        std::string text ;
        text = "\nFile: coordinate attribute variable: units:",
        text += " valid: empty, 1, or absent. But is: " ;
        text += tmp;

        (void) notes->operate(capt, text) ;
        {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
        }
      }
    }

    return;
  }

  // III) regular case
  std::string key("47_6");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCurrentTableSubst() ) ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry);

    if( tbl_dim_entry.units.size() && nc_entry.units.size() )
      capt += " units conflict." ;
    else if( tbl_dim_entry.isUnitsDefined )
      capt += " units not available in the file." ;
    else
      capt += " units not defined in the table." ;

    std::string text("                units:\t table: ") ;
    if( tbl_dim_entry.units.size() )
      text += tbl_dim_entry.units ;
    else
      text += "not defined" ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.units.size() )
      text += nc_entry.units ;
    else if( nc_entry.isUnitsDefined )
      text += notAvailable ;
    else
      text += "not defined" ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
QA::checkFilename(std::vector<std::string> &sTable,
  std::string &gaTableSheet)
{
  // The items of the filename must match corresponding global attributes.
  // Existence of the global attributes was checked elsewhere.
  // A comparison is not possible if at least one of the global atts
  // doesn't exist.

  size_t g_ix = pIn->varSz ;  // index to the global atts

  int table_id_ix;
  if( (table_id_ix=pIn->variable[g_ix].getAttIndex("table_id")) == -1 )
    return;

  int model_id_ix;
  if( (model_id_ix=pIn->variable[g_ix].getAttIndex("model_id")) == -1 )
    return;

  int experiment_id_ix;
  if( (experiment_id_ix=pIn->variable[g_ix].getAttIndex("experiment_id")) == -1 )
    return;

  int ix[3];
  if( (ix[0] = pIn->variable[g_ix].getAttIndex("realization")) == -1 )
    return;

  if( (ix[1] = pIn->variable[g_ix].getAttIndex("initialization_method")) == -1 )
    return;

  if( (ix[2] = pIn->variable[g_ix].getAttIndex("physics_version")) == -1 )
    return;

  std::string ga_ensmb("r");
  ga_ensmb += pIn->variable[g_ix].attValue[ix[0]][0] ;
  ga_ensmb += 'i' ;
  ga_ensmb += pIn->variable[g_ix].attValue[ix[1]][0] ;
  ga_ensmb += 'p' ;
  ga_ensmb += pIn->variable[g_ix].attValue[ix[2]][0] ;


  // CMIP5 filename encoding
  // <variable_name>_<varReqTableSheetSubs>_<model>_<experiment>_<ensemble_member>

  // assemble a filename-like string from global attributes.
  std::string a("<variable>_");
  a += pIn->variable[g_ix].attValue[table_id_ix][0] + "_" ;
  a += pIn->variable[g_ix].attValue[model_id_ix][0] + "_" ;
  a += pIn->variable[g_ix].attValue[experiment_id_ix][0] + "_" ;
  a += ga_ensmb;

  if( getFrequency() != "fx" )
    a += "[_<temporal subset>].nc" ;

  // comparable format of the real filename
  std::string f( pIn->file.basename);
  Split splt(f, "_");

  if( splt.size() > 4 )
  {
    // changed context for f
    f = ("<variable>_");
    f += splt[1] + "_" ;  // table sheet
    f += splt[2] + "_" ;  // model_id
    f += splt[3] + "_" ;  // experiment_id

    // needed later
    experiment_id = splt[3] ;

    if( splt[1] != "fx" )
      f += splt[4] + "[_<temporal subset>].nc" ;  // ensemble member
  }
  else
  {
     std::string key("1_4");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("filename structure with syntax error.");

       (void) notes->operate(capt) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
    }
  }

  if( a != f )
  {
    std::string key("1_5");
    if( notes->inq( key, fileStr))
    {
       std::string capt("filename does not match file attributes.") ;

       (void) notes->operate(capt) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
    }
  }

  return;
}

void
QA::checkMetaData(InFile &in)
{
  notes->setCheckMetaStr("PASS");

  inqTables();

  getFrequency();

  // is it NetCDF-3 classic?
  checkNetCDF(in);

  checkDataVarNum();

  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    if( varMeDa[i].var->isDATA )
    {
      getTableSheet(varMeDa[i]) ;
      setVarMetaData(varMeDa[i]);

      checkTables(in, varMeDa[i] );
    }
  }

  // eventually
  bool isNotInFName=true;
  for( size_t i=0 ; i < in.variable.size() ; ++i )
  {
     if( in.variable[i].name == fVarname )
     {
        isNotInFName=false;
        break;
     }
  }

  if( isNotInFName )
  {
     std::string key("1_3");

     if( notes->inq( key, fileStr) )
     {
       std::string capt("variable name in filename does not match any variable in the file.") ;

       std::string text("Variable name in the filename: ");
       text += fVarname ;
       text += "\nCandidates in the file: ";
       for( size_t j=0 ; j < varMeDa.size()-1 ; ++j)
         text += varMeDa[j].var->name + ", ";
       text += varMeDa[varMeDa.size()-1].var->name ;

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
       }
     }
  }

  // Read or write the project table.
  ProjectTable projectTable(this, &in, projectTableFile);
  projectTable.setAnnotation(notes);
  projectTable.setExcludedAttributes( excludedAttribute );

  // unsed set_1sr_ID(), because no appendix to the 1st item
  projectTable.set_2nd_ID( MIP_tableName );

  projectTable.check();

  // inquire passing the meta-data check
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    setExit( ev );

  return;
}

void
QA::checkNetCDF(InFile &in)
{
  // NC_FORMAT_CLASSIC (1)
  // NC_FORMAT_64BIT   (2)
  // NC_FORMAT_NETCDF4 (3)
  // NC_FORMAT_NETCDF4_CLASSIC  (4)

  int fm = in.nc.inqNetcdfFormat();

  if( fm == 1 || fm == 4 )
    return;

  std::string s("");

  if( fm == 2 )
  {
    s = "3, 64BIT formatted";
  }
  else if( fm == 3 )
  {
    s = "4, ";

    if( in.nc.inqDeflate() )
      s += "deflated (compressed)";
  }

  if( s.size() )
  {
    std::string key("12");
    if( notes->inq( key, fileStr ) )
    {
      std::string capt("format does not conform to netCDF classic") ;
      capt += ", found" + s;

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(fail);
    }
  }

  return;
}

void
QA::checkTables(InFile &in, VariableMetaData &vMD)
{
  // Matching the ncfile inherent meta-data against a pre-defined
  // so-called standard output table.

  // Meta data of variables from file or table are stored in struct varMeDa.
  // Similar for dimMeDa for the associated dimensions.

  std::vector<struct DimensionMetaData> dimNcMeDa;

  // Scan through the standard output table, respectivels variable requirements.
  // "Any" indicates that no valid table sheet was found
  if( vMD.varReqTableSheet != "Any" && ! vMD.var->isExcluded )
  {
    bool is;
    if( checkVarReqTable(in, vMD, dimNcMeDa) )
    {
      // very special: a tracer variable was not found
      // in table sheet Omon, because it is defined in Oyr
      if( vMD.varReqTableSheet == "Omon" )
      {
        vMD.varReqTableSheetAlt = "Omon" ;
        vMD.varReqTableSheet = "Oyr" ;

       is = checkVarReqTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.varReqTableSheet = "Omon" ;
        vMD.varReqTableSheetSub =  "Marine Bioge" ;
        vMD.varReqTableSheetAlt = "Oyr" ;
      }
      else if( vMD.varReqTableSheet == "cf3hr" )
      {
        vMD.varReqTableSheetAlt = "cf3hr" ;
        vMD.varReqTableSheet = "Amon" ;
        std::string saveCellMethods(vMD.cellMethods);
        vMD.cellMethods="time: point";

        is = checkVarReqTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.varReqTableSheet = "cf3hr" ;
        vMD.varReqTableSheetSub.clear() ;
        vMD.varReqTableSheetAlt = "Amon" ;

        if( ! is )
          vMD.cellMethods=saveCellMethods;
      }
      else if( vMD.varReqTableSheet == "cfSites" )
      {
        vMD.varReqTableSheetAlt = "cfSites" ;
        vMD.varReqTableSheet = "Amon" ;
        std::string saveCellMethods(vMD.cellMethods);
        vMD.cellMethods="time: point";

        is = checkVarReqTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.varReqTableSheet = "cfSites" ;
        vMD.varReqTableSheetSub =  "CFMIP 3-ho" ;
        vMD.varReqTableSheetAlt = "Amon" ;

        if( ! is )
          vMD.cellMethods=saveCellMethods;
      }
    }
  }

  // In case of an error in the check against the standard table,
  // the program would have exited.

  // ensure that each varMeDa instance has a mipSubTable, even if
  // the variable is not represented in the standard table. Necessary
  // for time_table scheduling. Remember: the ordering was adjusted
  // elsewhere to support this.
  if( vMD.var->isExcluded && vMD.varReqTableSheetSub.size() == 0 )
  {
     for(size_t i=0 ; i < varMeDa.size() ; ++i )
     {
       if( varMeDa[i].varReqTableSheetSub.size() )
       {
         vMD.varReqTableSheetSub = varMeDa[i].varReqTableSheetSub;
         break;
       }
     }
  }

  return;
}

bool
QA::checkVarReqTable(InFile &in, VariableMetaData &vMD,
             std::vector<struct DimensionMetaData> &dimNcMeDa)
{
   // We have arrived here, because no project table
   // wasn't defined, yet. Or no entry was found.
   // So, we scan through the variable requirements table.

   // return true for the very special case that a tracer
   // variable was not found in table sheet Omon, because
   // it is defined in Oyr

   if( !varReqTable.is )  // no standard table
      return false;

   std::string str;

   setTable( varReqTable.filename, "VR" );

//   std::fstream ifs(str0.c_str(), std::ios::in);
   // This class provides the feature of putting back an entire line
   std::string str0;

   ReadLine ifs(varReqTable.getFile());

   if( ! ifs.isOpen() )
   {
      std::string key("41") ;

      if( notes->inq( key, vMD.var->name) )
      {
         std::string capt("could not open variable-requirements table.") ;
         capt += varReqTable.getFilename() ;

         (void) notes->operate(capt) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }
      }
   }

   // headings for variables and dimensions
   std::map<std::string, size_t> v_col;
   std::map<std::string, size_t> d_col;

   size_t v_colMax, d_colMax;

   // read headings from the variable requirements table
   readHeadline(ifs, vMD, v_col, d_col, v_colMax, d_colMax);

   VariableMetaData tbl_entry(this);

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   // a helper vector
   std::vector<std::string> wa;

   // find the table sheet, corresponding to the 2nd item
   // in the variable name. The name of the table sheet is
   // present in the first column and begins with "CMOR Table".
   // The remainder is the name.
   // Unfortunately, the variable requirement table is in a shape of a little
   // distance from being perfect.

   while( ! ifs.getLine(str0) )
   {
     // try to identify the name of the table sheet in str0
     // return true, if not found
     if( findVarReqTableSheet(ifs, str0, vMD) )
       continue; // try next line

     // Now, test for a heading line that must be present for
     // each table sheet
     if( findNextVariableHeadline(ifs, str0, vMD, wa) )
       continue; // try next line

     // find entry for the requested variable
     if( findVarReqTableEntry(ifs, str0, vMD, v_col, v_colMax, wa) )
       continue; // try next line

     // We have found an entry
     splt_line = str0;

     // This was tested in findVarReqTableSheet()
     tbl_entry.varReqTableSheet=vMD.varReqTableSheet ;

     tbl_entry.priority = splt_line.toInt(v_col["priority"]);
     tbl_entry.name_alt = splt_line[v_col["output_variable_name"]];
     tbl_entry.longName
         = hdhC::clearInternalMultipleSpaces( splt_line[v_col["long_name"]] );
     tbl_entry.cellMethods = splt_line[v_col["cell_methods"]];
     tbl_entry.cellMeasures = splt_line[v_col["cell_measures"]];
     tbl_entry.typeStr = splt_line[v_col["type"]];

     tbl_entry.var->name = splt_line[v_col["CMOR_variable_name"]];
     tbl_entry.var->std_name = splt_line[v_col["standard_name"]];
     tbl_entry.var->units
         = hdhC::clearInternalMultipleSpaces( splt_line[v_col["unformatted_units"]] );
     if( tbl_entry.var->units.size() )
       tbl_entry.var->isUnitsDefined=true;
     else
       tbl_entry.var->isUnitsDefined=false;

     // check the dimensions of the variable from the table.
     Split x_tableVarDims(splt_line[v_col["CMOR_dimensions"]]) ;
     for( size_t x=0 ; x < x_tableVarDims.size() ; ++x )
        tbl_entry.var->dimName.push_back(x_tableVarDims[x]);

     // special for items declared across table sheets
     if( vMD.varReqTableSheetAlt == "cfSites"
            || vMD.varReqTableSheetAlt == "cf3hr" )
       tbl_entry.cellMethods="time: point";

     // netCDF properties are compared to those in the table.
     // Exit in case of any difference.
     checkVarReqTableEntry(vMD, tbl_entry);

     // get dimensions names from nc-file
     for(size_t l=0 ; l < vMD.var->dimName.size() ; ++l)
     {
       // new instance
       dimNcMeDa.push_back( DimensionMetaData() );

       // the spot where meta-data of variables is taken
       getDimMetaData(in, vMD, dimNcMeDa.back(), vMD.var->dimName[l]) ;
     }

     for(size_t l=0 ; l < x_tableVarDims.size() ; ++l)
       // check basic properties between the file and
       // requests in the table.
       checkDimVarReqTable(ifs, in, vMD, dimNcMeDa,
          d_col, x_tableVarDims[l], d_colMax );

     return false;
   }

   // there was no match, but we try alternatives

   // Is it one of those providing an alternative?.
   if( vMD.varReqTableSheet == "Omon" )
     return true;
   if( vMD.varReqTableSheet == "cf3hr" )
     return true;
   if( vMD.varReqTableSheet == "cfSites" )
     return true;

   if( vMD.varReqTableSheetAlt.size() )
   {
     // switch back to the original settings to get it right
     // for issuing the warning below.
     vMD.varReqTableSheet = "Omon" ;
     vMD.varReqTableSheetAlt = "Oyr" ;
   }

    std::string key("3_1") ;

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt("variable ") ;
      capt += vMD.var->name ;
      capt += " not found in the standard table.";

      std::ostringstream ostr(std::ios::app);
      ostr << "Standard table: " << varReqTable.filename  ;
      ostr << "\ntable sheet: " << vMD.varReqTableSheet;
      ostr << "\nVariable " << vMD.var->name;
      ostr << " not found in the table.";

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
   }

   // No variable found in the standard table.
   // Build a project table from netCDF file properties.
   return false;
}

void
QA::checkVarReqTableDimBounds(InFile &in, Split &splt_line,
    VariableMetaData &vMD,
    struct DimensionMetaData &dimFE,
    struct DimensionMetaData &dimTE,
    std::map<std::string,
    size_t> &col)
{
   // check checksum and size of bound in the file and the table,
   // respectively. Nothing from here will enter the project table.

    // no bounds requested for the currrent dimenion
   if( splt_line[col["bounds?"]] == "no" )
     return;

   if( in.nc.isDimUnlimited(dimFE.outname) )
     return;

   std::string captsIntro("VRT=");
   captsIntro += vMD.varReqTableSheet + ", var=";
   captsIntro += vMD.var->name + ", dim=";
   captsIntro += dimTE.outname + ": ";

   // check that the table has an entry for the 'bounds?'-column
   // Note: table contains "yes" or "no". No real name

   if( !( splt_line[col["bounds?"]] == "yes"
            || splt_line[col["bounds?"]] == "no" ) )
   {
       std::string key("51");

       if( notes->inq( key, vMD.var->name) )
       {
         std::ostringstream ostr(std::ios::app);

         std::string capt( captsIntro) ;
         capt += "missing value in table sheet 'dims' in column 'bounds?'" ;

         ostr << "Standard table: " << varReqTable.filename;
         ostr << "\nError in the dimension table: ";
         ostr << "item 'bounds?' must be 'yes' or 'no', but is ";
         ostr <<  splt_line[col["bounds?"]] << ".";

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }
     }
   }

  // check dimensional values

  dimTE.bnds_name=splt_line[col["bounds?"]];

  // get settings for the respective column; default separator is ' '
  Split splt_bounds_values( splt_line[col["bounds_values"]] );
  Split splt_bounds_requested( splt_line[col["bounds_requested"]] );

  // find the table entry providing the same number of
  // values as the one in the file.

  // The Fletcher32 checksum is calculated
  size_t sz= 2 * dimFE.size ;
  uint32_t checksum_tb=0;

  if( splt_bounds_values.size() == sz )
  {
    // apply the 'bounds_values' column
    bool reset;
    reset=true;
    if( splt_line[col["type"]] == "character" )
       for( size_t i=0 ; i < splt_bounds_values.size() ; ++i )
          checksum_tb = hdhC::fletcher32_cmip5(
              splt_bounds_values[i], &reset) ;
    else
       for( size_t i=0 ; i < splt_bounds_values.size() ; ++i )
           checksum_tb = hdhC::fletcher32_cmip5(
                 splt_bounds_values.toDouble(i), &reset) ;
  }
  else if( splt_bounds_requested.size() == sz )
  {
    // apply the 'bounds_requested' column
    bool reset;
    reset=true;
    if( splt_line[col["type"]] == "character" )
       for( size_t i=0 ; i < splt_bounds_requested.size() ; ++i )
          checksum_tb = hdhC::fletcher32_cmip5(
              splt_bounds_requested[i], &reset) ;
    else
       for( size_t i=0 ; i < splt_bounds_requested.size() ; ++i )
           checksum_tb = hdhC::fletcher32_cmip5(
              splt_bounds_requested.toDouble(i), &reset) ;
  }
  else if( splt_bounds_values.size() == 0
     &&  splt_bounds_requested.size() )
     // this is e.g. true for lat, lon
     return;
  else
  {
    std::string key("3_2");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(getCurrentTableSubst() ) ;
      capt += captsIntro ;
      capt += "Number of dim_bounds do not match those of the standard table." ;

      std::string text("                value:\t table: ") ;
      text += "not diagnosed" ;
      text += "\n                      \tncfile: " ;
      text += "2x";
      text += hdhC::double2String(dimFE.size);

      (void) notes->operate(capt, text) ;
      {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
      }
    }

    return ;
  }

  // get the checksum and size for the dim-bnds variable
  uint32_t checksum_fl=0;
  std::string bName( dimFE.bnds_name );

  // is bnds name a valid variable name?
  if( in.nc.getVarID( bName ) < 0 )
    return ;  // name of bounds not defined by attribute
              // or not provided as variable.

  // determine the checksum of limited var-presentations of dim
  if( ! in.nc.isDimUnlimited(bName) )
  {
    std::vector<std::string> vs;
    if( in.nc.getVarType(bName) == NC_CHAR )
    {
      in.nc.getData(vs, bName);
      bool reset=true;  // is set to false during the first call
      for(size_t i=0 ; i < vs.size() ; ++i)
      {
        vs[i] = hdhC::stripSurrounding(vs[i]);
        checksum_fl = hdhC::fletcher32_cmip5(vs[i], &reset) ;
      }
    }
    else
    {
      MtrxArr<double> mv;
      in.nc.getData(mv, bName);

      bool reset=true;
      for( size_t i=0 ; i < mv.size() ; ++i )
        checksum_fl = hdhC::fletcher32_cmip5(mv[i], &reset) ;
    }
  }

  // now, compare the checksums of the bounds

  if( checksum_fl != checksum_tb )
  {
     std::string key("3_12");

     if( notes->inq( key, vMD.var->name) )
     {
       std::string capt( getCurrentTableSubst() );
       capt += captsIntro ;
       capt += "values of dim_bounds do not match those of the standard table." ;

       std::string text("                value:\t table: ") ;
       text += hdhC::double2String(checksum_tb) ;
       text += "\n                      \tncfile: " ;
       text += hdhC::double2String(checksum_fl) ;

       (void) notes->operate(capt, text) ;
       {
          setExit( notes->getExitValue() ) ;
       }
    }
  }

  return ;
}

void
QA::checkVarReqTableDimValues(InFile &in, Split &splt_line,
    VariableMetaData &vMD,
    struct DimensionMetaData &dimFE,
    struct DimensionMetaData &dimTE,
    std::map<std::string, size_t> &col)
{
  // check dimensional values of limited dimensions
  if( in.nc.isDimUnlimited(dimFE.outname) )
    return;

  std::string captsIntro("VRT=");
  captsIntro += vMD.varReqTableSheet + ", var=";
  captsIntro += vMD.var->name + ", dim=";
  captsIntro += dimTE.outname + ": ";

  std::string t0;

  // get settings for the respective column; default separator is ' '
  Split splt_value( splt_line[col["value"]] );
  Split splt_requested( splt_line[col["requested"]] );

  // find the table entry providing the same number of
  // values as the one in the file.
  dimTE.checksum = 0 ;  //in case of no dimension

  // special: the 17 mandatory levels
  if( dimTE.cmor_name == "plevs" )
    dimFE.size = 17 ;

  // Important: dimensions of size==1 in the table are usually
  // not defined as dimension in the file, but only as variable
  // representation of a single value.

  if( splt_value.size() == 1 )
  {
    double is=false;
    std::vector<std::string> vd;
    MtrxArr<double> mv;

    // get the value of the var-representation from the file
    if( in.nc.getVarType(dimFE.outname) == NC_CHAR )
    {
      in.nc.getData(vd, dimFE.outname);
      if( vd[0] != splt_value[0] )
        is=true;
    }
    else
    {
      in.nc.getData(mv, dimFE.outname);
      if( mv[0] != splt_value.toDouble(0) )
        is=true;
    }

    if( is )
    {
       std::string key("3_11");

       if( notes->inq( key, vMD.var->name) )
       {
         std::ostringstream ostr(std::ios::app);

         std::string capt( captsIntro) ;
         capt += "different value for " ;
         capt += dimTE.outname;

         ostr << "Standard table: value: ";
         if( splt_value.size() )
           ostr << splt_value[0] ;
         else
           ostr << notAvailable ;

         ostr << "\nFile: value: " ;
         if( mv.size() )
            ostr << hdhC::double2String(mv[0]) ;
         else if( vd.size() )
            ostr << vd[0] ;
         else
            ostr << notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }

       }
    }
  }

  // The Fletcher32 checksum is calculated
  if( splt_value.size() == dimFE.size )
  {
    // apply the 'value' column
    dimTE.size = splt_value.size() ;

    bool reset;
    reset=true;
    if( splt_line[col["type"]] == "character" )
       for( size_t i=0 ; i < splt_value.size() ; ++i )
          dimTE.checksum = hdhC::fletcher32_cmip5(
                    splt_value[i], &reset) ;
     else
        for( size_t i=0 ; i < splt_value.size() ; ++i )
            dimTE.checksum = hdhC::fletcher32_cmip5(
                    splt_value.toDouble(i), &reset) ;
   }
   else if( splt_requested.size() == dimFE.size )
   {
     // apply the 'requested' column
     dimTE.size = splt_requested.size() ;

     bool reset;
     reset=true;
     if( splt_line[col["type"]] == "character" )
       for( size_t i=0 ; i < splt_requested.size() ; ++i )
         dimTE.checksum = hdhC::fletcher32_cmip5(
               splt_requested[i], &reset) ;
     else
       for( size_t i=0 ; i < splt_requested.size() ; ++i )
         dimTE.checksum = hdhC::fletcher32_cmip5(
               splt_requested.toDouble(i), &reset) ;
   }
   else
   {
     // for lat and lon; the table doesn't provide any data
     dimTE.size = dimFE.size; // to pass the test
     dimTE.checksum = dimFE.checksum ;
   }

   if( dimTE.cmor_name == "plevs" )
   {
      // There could be layers additionally
      // to the 17 mandatory ones for the dimension 'plevs'.
      // Thus, we have to take into account the 17 mandatory
      // ones for the comparison with the standard table and
      // the real number from the file for the project table.

      // get values for the 17 mandatory levels
      if( dimFE.size >= 17 )
      {
        MtrxArr<double> mv;
        in.nc.getData(mv, dimFE.outname);
        dimFE.checksum=0;

        bool reset=true;
        for( size_t l=0 ; l < 17 ; ++l )
          dimFE.checksum = hdhC::fletcher32_cmip5(mv[l], &reset) ;
      }
   }

   return ;
}

void
QA::checkVarReqTableEntry(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  // Do the variable properties found in the netCDF file
  // match those in the table (var-requirements or project)?
  // Exceptions: priority is not defined in the netCDF file.
  std::string t0;

  std::string captsIntro("VRT=");
  if( vMD.varReqTableSheetAlt.size() )
    captsIntro += vMD.varReqTableSheetAlt + ", var=";
  else
    captsIntro += vMD.varReqTableSheet + ", var=";
  captsIntro += vMD.var->name + ": ";

  if( tbl_entry.var->std_name != vMD.var->std_name )
  {
    // this takes into account an error of the variable requirements table
    if( vMD.var->name == "tos" && vMD.varReqTableSheet == "day" )
    {
       if( vMD.var->std_name != "sea_surface_temperature" )
       {
         std::string key("4_4");

         if( notes->inq( key, vMD.var->name) )
         {
           std::string capt( getCurrentTableSubst() ) ;
           capt += captsIntro;
           capt += "standard name conflict." ;

           std::ostringstream ostr(std::ios::app);
           ostr << "Variable conflict between file and table.";
           ostr << "\nTable: " << currTable  ;
           ostr << ", table sheet: " << vMD.varReqTableSheet;
           ostr << "\nVariable: ";
           ostr << vMD.var->name;

           ostr << "        standard name:\t table: " ;
           if( tbl_entry.var->std_name.size() )
             ostr << tbl_entry.var->std_name ;
           else
             ostr << notAvailable ;

           ostr << "\n                      \tncfile: " ;
           if( vMD.var->std_name.size() )
             ostr << vMD.var->std_name ;
           else
             ostr << notAvailable ;

           (void) notes->operate(capt, ostr.str()) ;
           {
             notes->setCheckMetaStr(fail);
             setExit( notes->getExitValue() ) ;
           }
         }
       }
    }
  }

  if( tbl_entry.longName != vMD.longName )
  {
    bool is=true;

    // special consideration for tracers in table sheet Omon
    if( vMD.varReqTableSheet == "Oyr" && vMD.varReqTableSheetAlt == "Omon" )
    {
      // the name from Oyr is contained somehow in Omon, but
      // without addition like 'at Surface' or 'Surface ...'
      std::string t(tbl_entry.longName);
      t += " at surface";
      if( t == vMD.longName )
        // I think this is due to a misspelling in the Omon table sheet
        is=false;
      else
      {
         t = tbl_entry.longName;
         t += " at Surface";
         if( t == vMD.longName )
           // this for the correct spelling
           is=false;
      }
    }

    if( is )
    {
      std::string key("4_5");

      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCurrentTableSubst() );
        capt += captsIntro;
        capt += "long name conflict." ;


        std::ostringstream ostr(std::ios::app);
        ostr << "Variable conflict between file and table.";
        ostr << "\nTable: " << currTable  ;
        ostr << ", Sheet: " << vMD.varReqTableSheet;
        ostr << ", Variable: "  << vMD.var->name;
        ostr << "\nlong name: table: " ;
        if( tbl_entry.longName.size() )
          ostr << tbl_entry.longName ;
        else
          ostr << notAvailable ;

        ostr << "\nncfile: " ;
        if( vMD.longName.size() )
          ostr << vMD.longName ;
        else
          ostr << notAvailable ;

        (void) notes->operate(capt, ostr.str()) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }
  }

  if( tbl_entry.var->units != vMD.var->units )
  {
    std::string key("4_6");

    if( vMD.var->units.size() == 0 )
    {
      // units must be specified; but, unit= is ok
      if( tbl_entry.var->units != "1" && ! vMD.var->isUnitsDefined )
      {
         if( notes->inq( key, vMD.var->name))
         {  // really empty
           std::string capt( getCurrentTableSubst() );
           capt += captsIntro;
           capt += "variable has no units attribute." ;

           std::ostringstream ostr(std::ios::app);
           ostr << "\nTable: " << currTable  ;
           ostr << ", Sheet: " << vMD.varReqTableSheet;
           ostr << ", Variable: ";
           ostr << vMD.var->name;
           ostr << "\nMissing units attribute" ;
           ostr << ", table requires " ;
           ostr << tbl_entry.var->units;

           (void) notes->operate(capt, ostr.str()) ;
           {
              notes->setCheckMetaStr(fail);
              setExit( notes->getExitValue() ) ;
           }
        }
      }
    }
    else if( notes->inq( key, vMD.var->name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro ;
      capt += "Conflict for the units attribute." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;
      ostr << "\nConflict between file and table.";
      ostr << "                units:\t table: " ;
      if( tbl_entry.var->units.size() )
        ostr << tbl_entry.var->units ;
      else
        ostr << notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.var->isUnitsDefined )
      {
        if( vMD.var->units.size() )
          ostr << vMD.var->units ;
        else
          ostr << notAvailable ;
      }
      else
        ostr << "not defined" ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( tbl_entry.cellMethods != vMD.cellMethods )
  {
    // strip anything within () from the cell-entry
    std::string tbl_cm( tbl_entry.cellMethods ) ;
    size_t p0, p1;

    while( (p0=tbl_cm.find('(') ) < std::string::npos )
    {
      // doesn't care for unmatched ()
      if( (p1=tbl_cm.find(')') ) < std::string::npos )
        tbl_cm.erase(p0, p1-p0+1);
    }

    // test whether the table's specification can be found in
    // a file's attribute stating additional info
    Split splt(tbl_cm);
    bool is=false;
    for(size_t i=0 ; i < splt.size() ; ++i)
      if( vMD.cellMethods.find(splt[i]) == std::string::npos )
        is=true;

    if( is )
    {
       std::string key("4_7");

       if( notes->inq( key, vMD.var->name) )
       {
         std::string capt( getCurrentTableSubst() );
         capt += captsIntro;
         capt += "cell-methods conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " << currTable  ;
         ostr << ", Sheet: " << vMD.varReqTableSheet;
         ostr << ", Variable: ";
         ostr << vMD.var->name;
         ostr << "\nConflict between file and table.";
         ostr << "         cell-methods:\t table: " ;
         if( tbl_entry.cellMethods.size() )
           ostr << tbl_entry.cellMethods ;
         else
           ostr << notAvailable ;

         ostr << "\n                      \tncfile: " ;
         if( vMD.cellMethods.size() )
           ostr << vMD.cellMethods ;
         else
           ostr << notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }
      }
    }
  }

  if( tbl_entry.cellMeasures != vMD.cellMeasures )
  {
    // strip anything within () from the cell-entry
    std::string tbl_cm( tbl_entry.cellMeasures ) ;
    size_t p0, p1;

    while( (p0=tbl_cm.find('(') ) < std::string::npos )
    {
      // doesn't care for unmatched ()
      if( (p1=tbl_cm.find(')') ) < std::string::npos )
        tbl_cm.erase(p0, p1-p0+1);
    }

    // test whether the table's specification can be found in
    // a file's attribute stating additional info
    Split splt(tbl_cm);
    bool is=false;
    for(size_t i=0 ; i < splt.size() ; ++i)
      if( vMD.cellMeasures.find(splt[i]) == std::string::npos )
        is=true;

    if( is )
    {
       std::string key("4_9");

       if( notes->inq( key, vMD.var->name) )
       {
         std::string capt( getCurrentTableSubst() );
         capt += captsIntro;
         capt += "cell-measures conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " << currTable  ;
         ostr << ", Sheet: " << vMD.varReqTableSheet;
         ostr << ", Variable: ";
         ostr << vMD.var->name;
         ostr << "\nConflict between file and table.";
         ostr << "\n         cell-measures:\t table: " ;
         if( tbl_entry.cellMeasures.size() )
           ostr << tbl_entry.cellMeasures ;
         else
           ostr << notAvailable ;

         ostr << "\n                      \tncfile: " ;
         if( vMD.cellMeasures.size() )
           ostr << vMD.cellMeasures ;
         else
           ostr << notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }
      }
    }
  }

  // the standard table has type==real. Is it for
  // float only, or also for double? So, in case of real,
  // any non-int type is accepted
  bool isTblTypeReal =
      tbl_entry.typeStr == "real"
         || tbl_entry.typeStr == "float"
             || tbl_entry.typeStr == "double" ;
  bool isNcTypeReal =
      vMD.typeStr == "real"
         || vMD.typeStr == "float"
              || vMD.typeStr == "double" ;

  if( currTable == varReqTable.filename
       && tbl_entry.typeStr.size() == 0 && vMD.typeStr.size() != 0 )
  {
    std::string key("4_11");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "type check discarded, not specified in the MIP Table." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;
      ostr << "\nConflict between file and table.";
      ostr << "\n                 type:\t table: " ;
      ostr << "not specified in the MIP Table" ;
      ostr << "\n                      \tncfile: " ;
      ostr << vMD.typeStr ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }
  else if( (isTblTypeReal && ! isNcTypeReal)
            || ( ! isTblTypeReal && isNcTypeReal) )
  {
    std::string key("4_8");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "type conflict." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;
      ostr << "\nConflict between file and table.";
      ostr << "\n                 type:\t table: " ;
      if( tbl_entry.typeStr.size() )
        ostr << tbl_entry.typeStr ;
      else
        ostr << notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.typeStr.size() )
        ostr << vMD.typeStr ;
      else
        ostr << notAvailable ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( tbl_entry.var->std_name != vMD.var->std_name )
  {
    std::string key("4_12");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "variable name matches only for case-insensitivity." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Variable conflict between file and table.";
      ostr << "\nTable: " << currTable  ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;

      ostr << "        variable name:\t table: " ;
      ostr << tbl_entry.var->name ;

      ostr << "\n                      \tncfile: " ;
      ostr << vMD.var->name ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}


void
QA::closeEntry(void)
{
   // This here is only for the regular QA time series file
   if( isCheckTime )
     storeTime();

   if( isCheckData )
   {
     // data: structure defined in hdhC.h
     std::vector<hdhC::FieldData> fA;
     for( size_t i=0 ; i < varMeDa.size() ; ++i )
     {
       // skip time test for proceeding time steps when var is fixed
       if( isNotFirstRecord && varMeDa[i].var->isFixed  )
         continue;

       fA.push_back( varMeDa[i].var->pDS->get() ) ;

       // test overflow of ranges specified in a table, or
       // plausibility of the extrema.
       varMeDa[i].qaData.test(i, fA.back() );
     }

     storeData(fA);
   }

   ++currQARec;

   return;
}

void
QA::createVarMetaData(void)
{
  // set corresponding isExcluded=true
  pIn->excludeVars();

  std::string str; // temporarily

  // create instances of VariableMetaData. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in the InFile::dataVarIndex vector.
  for( size_t i=0 ; i< pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pIn->variable[pIn->dataVarIndex[i]];

    pushBackVarMeDa( &var );  //push next instance
  }

   // very special: discard particular tests
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    VariableMetaData &vMD = varMeDa[i] ;

    int effDim = vMD.var->dimName.size() ;
    if( hdhC::isAmong(qaTime.name, vMD.var->dimName) )
      --effDim;

    if( replicationOpts.size() )
    {
      if( ReplicatedRecord::isSelected(
             replicationOpts, vMD.var->name, effDim ) )
      {
        vMD.qaData.replicated = new ReplicatedRecord(this, i, vMD.var->name);
        vMD.qaData.replicated->setAnnotation(notes);
        vMD.qaData.replicated->parseOption(replicationOpts) ;
      }
    }

    if( enablePostProc && outlierOpts.size() )
    {
      if( Outlier::isSelected(
             outlierOpts, vMD.var->name, effDim ) )
      {
        vMD.qaData.enableOutlierTest=true;

        vMD.qaData.outlier = new Outlier(this, i, vMD.var->name);
        vMD.qaData.outlier->setAnnotation(notes);
        vMD.qaData.outlier->parseOption(outlierOpts);
      }
    }
  }

  return;
}

bool
QA::entry(void)
{
  if( isCheckData )
   {
     // read next field
     pIn->entry() ;

     for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
     {
       Variable &var = pIn->variable[pIn->dataVarIndex[i]];

       if( var.isNoData )
          continue;

       if( pIn->currRec && var.isFixed )
         continue;

       var.pDS->clear();

       if( var.isArithmeticMean )
         // use the entire record
         var.pDS->add( (void*) var.pMA );
       else
       {
         // loop over the layers and there should be at least one.
         var.pGM->resetLayer();
         do
         {
           // due to area weighted statistics, we have to cycle
           // through the layers one by one.
//         if( var.pGM->isLayerMutable() )
//           ;

           var.pDS->add( var.pGM->getCellValue(),
                       *var.pGM->getCellWeight() );
           // break the while loop, if the last layer was processed.
         } while ( var.pGM->cycleLayers() ) ; //cycle through levels
       }
     }
   }

   // the final for this record
   closeEntry();
   return false;
}

int
QA::finally(int xCode)
{
  if( nc )
    setExit( finally_data(xCode) );

  if( xCode != 63 && isCheckTime )
    qaTime.finally( nc );

  setExit(xCode);

  // distinguish from a sytem crash (segmentation error)
  std::cout << "STATUS-BEG" << xCode << "STATUS-END";
  std::cout << std::flush;

  nc->close();

  return exitCode ;
}

int
QA::finally_data(int xCode)
{
  setExit(xCode);

  // write pending results to qa-file.nc. Modes are considered there
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
    setExit( varMeDa[i].finally() );

  // post processing, but not for conditions which indicate
  // incomplete checking.
  if( exitCode < 3 )
  {  // 3 or 4 interrupted any checking
    if( enablePostProc )
    {
      if( postProc() )
      {
        if( exitCode == 63 )
            exitCode=1;  // this is considered a change

        notes->setCheckDataStr(fail);
      }
    }
  }

  if( exitCode == 63 ||
     ( nc == 0 && exitCode ) || (currQARec == 0 && pIn->isTime ) )
  { // qa is up-to-date or a forced exit right from the start;
    // qa_>filename>.nc remains the same
    if( exitCode == 63 )
      exitCode=0 ;

    isExit=true;
    return exitCode ;
  }

  // read history from the qa-file.nc and append new entries
  appendToHistory(exitCode);

  // check for flags concerning the total data set
  if( pIn->currRec > 0 )
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
      varMeDa[j].qaData.checkFinally(varMeDa[j].var);

  if( isCheckData )
  {
    // write internal attributes
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
       varMeDa[j].qaData.setStatisticsAttribute(nc);
  }

  return exitCode ;
}

bool
QA::findNextVariableHeadline(ReadLine &ifs, std::string &str0,
   VariableMetaData &vMD, std::vector<std::string> &wa )
{
   // scan the table sheet.
   // Return true if no valid header line is found

   do
   {
     if( str0.substr(0,10) == "CMOR Table" )
     {
       ifs.putBackLine(); //try again for this CMOR Table
       return true;  // found the begin of another table sheet
     }

     if( str0.substr(0,13) == "In CMOR Table" )
       findVarReqTableSheetSub(str0, vMD, wa) ;

     // find the heading
     if( str0.substr(0,8) == "priority" )
       break;
   } while( ! ifs.getLine(str0) ) ;


   if( ifs.eof() )
   {
     std::string key("42");

     if( notes->inq( key, vMD.var->name) )
     {
       // we detected a table sheet that is not part of the standard table
       std::string capt("table sheet not found in the variable-requirements table.") ;

       std::string text("Table sheet: ");
       text += vMD.varReqTableSheet;
       text += "\nNo line beginning with key-word priority." ;

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
     }
   }

   // we have to read the next line.
   ifs.getLine(str0);

   return false ;
}

bool
QA::findVarReqTableEntry(ReadLine &ifs, std::string &str0,
   VariableMetaData &vMD,
   std::map<std::string, size_t> &col, size_t col_max,
   std::vector<std::string> &wa)
{
   // return true: entry is not the one we look for.

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   std::string s_e(vMD.var->name);

   // a very specific exception: convert to lower case
   if( isCaseInsensitiveVarName )
      (void) hdhC::Lower()(s_e, true);

   do
   {
     if( str0.substr(0,10) == "CMOR Table" )
//           || str0.substr(0,8) == "priority" )
     {
       ifs.putBackLine(); //try again for the current CMOR Table
       //Note: finding a "priority, long name,..." without a
       //preceding CMOR Table line is possible. But, it is assumed
       // the the last one found is still valid.

       return true;  // found the begin of another table sheet
     }

     // Get the string for the MIP-sub-table. This is used in
     // conjunction to the time table (parseTimeTable()).
     // Work-around when there are identical MIP sub-tables
     // after the truncation.
     if( str0.substr(0,13) == "In CMOR Table" )
       findVarReqTableSheetSub(str0, vMD, wa) ;

     splt_line=str0;
     std::string s_t( splt_line[col["CMOR_variable_name"]] );

     if( isCaseInsensitiveVarName )
       (void) hdhC::Lower()(s_t, true);

     if( s_t  == s_e )
       break;

   } while( ! ifs.getLine(str0) ) ;


   // netCDF variable not found in the table
   if( ifs.eof() )
     return true;

   if( splt_line.size() < col_max )
   {
     std::string key("43");

     if( notes->inq( key, vMD.var->name) )
     {
        std::string capt("missing column(s) in the standard table.") ;

        std::ostringstream ostr(std::ios::app);
        ostr << "Standard table: " << varReqTable.filename ;
        ostr << "\nCurrent line: " << str0 ;
        ostr << "\nRequired number of items ";
        ostr << "is " << col_max << ", but found ";
        ostr << splt_line.size() << "." ;

        (void) notes->operate(capt, ostr.str()) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
     }
   }


   return false;  // we have found an entry and the shape matches.
}

bool
QA::findVarReqTableSheet(ReadLine &ifs, std::string &str0,
  VariableMetaData &vMD)
{
   // return true, if str0 contains no table name sheet

   size_t pos;
   do
   {
     if( str0.substr(0,10) == "CMOR Table" )
       break;
   } while( ! ifs.getLine(str0) ) ;

   if( ifs.eof() )
     return true;  // could be unknown table sheet or unknown variable

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();
   splt_line=str0;

   // if (from any reason) the identification string
   // is not in the first column.

   for( pos=0 ; pos < splt_line.size() ; ++pos)
     if( splt_line[pos].find("CMOR Table") < std::string::npos )
       break;

   Split splt_col;  // default separator is ' '

   if( splt_line.size() > 0 )
      splt_col = splt_line[pos] ;
   else
     return true ; // try next: this can not happen

   // The name of the table is the 3rd blank-separated item;
   // it may have an appended ':'
   if( splt_col.size() > 2 )
   {
     if( (pos=splt_col[2].rfind(':')) < std::string::npos )
       str0=splt_col[2].substr(0, pos);
   }

   if( str0.size() > 0 && str0 == vMD.varReqTableSheet )
   {
     // we have to read the next line.
     ifs.getLine(str0);
     return false;  // sub table found
   }

   return true;  // not found; try next
}

void
QA::findVarReqTableSheetSub(std::string &str0,
   VariableMetaData &vMD, std::vector<std::string> &wa)
{
   // for a work-around when there are identical MIP sub-tables
   // after the truncation.

   // get the string for the MIP-sub-table. This is used in
   // conjunction to the time table (parseTimeTable()).
   size_t p;
   std::string tmp;
   if( (p = str0.find(':')) < std::string::npos )
   {
     // skip also a leading ' '
     tmp = str0.substr(p+2, 10) ;
     size_t iwa;

     // look for a duplicate
     for( iwa=0 ; iwa < wa.size() ; ++iwa )
       if( tmp == wa[iwa] )
         break;

     if( iwa < wa.size() )
     { // there was a duplicate; thus take a longer version
       // by extending a unique number
       tmp += '-' ;
       tmp += hdhC::itoa(wa.size()) ;
     }
   }

   wa.push_back( tmp );
   vMD.varReqTableSheetSub=tmp;

   return;
}

std::string
QA::getCurrentTableSubst(void)
{
   std::string t(": ");

   if( currTable == varReqTable.filename )
     t += "standard table" ;
   else
     t += "project table" ;

   t += ": " ;
   return t;
}

void
QA::getDimMetaData(InFile &in,
      VariableMetaData &vMD,
      struct DimensionMetaData &dimMeDa,
      std::string dName)
{
  // return 0:
  // collect dimensional meta-data in the struct.

  // a vector for different purposes
  std::vector<std::string> vs;

  std::vector<std::string> vsa;  // for attributes
  std::vector<std::string> vsav;  // for attribute values

  // pre-set
  dimMeDa.checksum=0;
  dimMeDa.outname=dName;
  dimMeDa.coordsAtt=dName;

  // dName is a dimension name from the table. Is it also
  // a dimension in the ncFile? A size of -1 indicates: no
  int sz = in.nc.getDimSize(dName);
  if( sz == -1 )
    dimMeDa.size = 0;
  else
    dimMeDa.size = static_cast<size_t>(sz);

  // Is dimension name also variable name?
  // Regular: variable representation of the dim
  // Exception: var specified in coords_attr
  if( in.nc.getVarID( dName ) == -2 )
  {
     if( in.nc.getAttString("coordinates", vMD.var->name ).size() == 0 )
       return ;  // nothing was found

     dName = in.nc.getAttString("coordinates", vMD.var->name ) ;
  }

  // var-rep of the dimension, may-be mapped to coordsAtt
  dimMeDa.coordsAtt=dName;

  // get the var type
  dimMeDa.type = in.nc.getVarTypeStr(dName);

  // get the attributes
  vsa = in.nc.getAttName( dName );

  // special: CF permits attribute setting: units= as units=1
  dimMeDa.isUnitsDefined=false;

  for( size_t j=0 ; j < vsa.size() ; ++j)
  {
    in.nc.getAttValues(vsav, vsa[j], dName);

    if( vsa[j] == "bounds" )
    {
      if( vsav.size() )
        dimMeDa.bnds_name = vsav[0] ;
    }
    else if( vsa[j] == "climatology" && dName == "time" )
    {
      if( vsav.size() )
        dimMeDa.bnds_name = vsav[0] ;
    }
    else if( vsa[j] == "units" )
    {
      if( vsav.size() )
      {
        dimMeDa.units = vsav[0] ;
        dimMeDa.isUnitsDefined=true;
      }
      else
        dimMeDa.isUnitsDefined=false;
    }
    else if( vsa[j] == "long_name" )
    {
      if( vsav.size() )
        dimMeDa.longname = vsav[0] ;
    }
    else if( vsa[j] == "standard_name" )
    {
      if( vsav.size() )
        dimMeDa.stndname = vsav[0] ;
    }
    else if( vsa[j] == "axis" )
    {
      if( vsav.size() )
        dimMeDa.axis = vsav[0] ;
    }
  }

  // determine the checksum of limited var-presentations of dim
  if( ! in.nc.isDimUnlimited(dName) )
  {
    if( in.nc.getVarType(dName) == NC_CHAR )
    {
      in.nc.getData(vs, dName);

      bool reset=true;  // is set to false during the first call
      for(size_t i=0 ; i < vs.size() ; ++i)
      {
        vs[i] = hdhC::stripSurrounding(vs[i]);
        dimMeDa.checksum = hdhC::fletcher32_cmip5(vs[i], &reset) ;
      }
    }
    else
    {
      MtrxArr<double> mv;
      in.nc.getData(mv, dName);

      bool reset=true;
      for( size_t i=0 ; i < mv.size() ; ++i )
        dimMeDa.checksum = hdhC::fletcher32_cmip5(mv[i], &reset) ;
    }
  }

  return ;
}

bool
QA::getExit(void)
{
  // note that isExit==true was forced
  if( exitCode > 1 || isExit )
    return true;

  return false;
}

std::string
QA::getFrequency(void)
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;  // no frequency provided

  // not found, but error issue is handled elsewhere

  // try the filename
  std::string f( pIn->file.basename );

  Split splt(f, "_");

  // the second term denotes the mip table for CMIP5
  std::string mip_f = splt[1];

  // now, try also global att 'table_id'
  Split mip_a( pIn->nc.getAttString("table_id") ) ;

  if( mip_a.size() > 1 )
  {
    if( mip_a[1] != mip_f && mip_a[1].size() )
      mip_f = mip_a[1]; // annotation issue?

    // convert mip table --> frequency
    if( mip_f.substr(mip_f.size()-2) == "yr" )
      frequency = "yr" ;
    else if( mip_f.substr(mip_f.size()-3) == "mon" )
      frequency = "mon" ;
    else if( mip_f.substr(mip_f.size()-3) == "Day" )
      frequency = "day" ;
    else if( mip_f == "day" )
      frequency = "day" ;
    else if( mip_f.substr(0,3) == "6hr" )
      frequency = "6hr" ;
    else if( mip_f.substr(0,3) == "3hr" )
      frequency = "3hr" ;
    else if( mip_f.substr(mip_f.size()-3) == "3hr" )
      frequency = "3hr" ;
  }

  return frequency ;
}

std::string
QA::getTableSheet(std::vector<std::string> &sTable)
{
  // the counter-parts in the attributes
  std::string gaTableSheet( pIn->nc.getAttString("table_id") );

  if( gaTableSheet.size() == 0 )
    return gaTableSheet;

  Split spltMT(gaTableSheet);

  // The table sheet name from the global attributes.
  // Ignore specific variations
  if( spltMT.size() > 1 )
  {
    if(  spltMT[0].substr(1,4) == "able"
        || spltMT[0].substr(1,4) == "ABLE" )
    gaTableSheet = spltMT[1] ;
  }
  else if( spltMT.size() > 0 )
    gaTableSheet = spltMT[0] ;

  //check for valid names
  bool is=true;
  for( size_t i=0 ; i < sTable.size() ; ++i )
  {
    if( sTable[i] == gaTableSheet )
    {
      is=false ;
      break;
    }
  }

  if( is )
  {
     std::string key("46_8");

     if( notes->inq( key, varMeDa[0].var->name) )
     {
       std::string capt("invalid table sheet name in CMIP5 attributes.") ;

       std::string text("MIP Table (Filename): ") ;
       text +=  gaTableSheet;

       gaTableSheet.clear();

       (void) notes->operate(capt, text) ;
       {
         setExit( notes->getExitValue() ) ;
       }
     }
  }

  return gaTableSheet;
}

void
QA::getTableSheet(VariableMetaData &vMD)
{
  // This is CMIP5 specific;
  // taken directly from the standard table.
  std::vector<std::string> sTable;
  sTable.push_back("fx");
  sTable.push_back("Oyr");
  sTable.push_back("Oclim");
  sTable.push_back("Amon");
  sTable.push_back("Omon");
  sTable.push_back("Lmon");
  sTable.push_back("LImon");
  sTable.push_back("OImon");
  sTable.push_back("aero");
  sTable.push_back("day");
  sTable.push_back("6hrLev");
  sTable.push_back("6hrPlev");
  sTable.push_back("3hr");
  sTable.push_back("cfMon");
  sTable.push_back("cfDay");
  sTable.push_back("cf3hr");
  sTable.push_back("cfSites");

  std::string tTable( getTableSheet(sTable) ) ;

  // compare filename to netCDF global attributes
  checkFilename( sTable, tTable );

  // Note: filename:= name_CMOR-MIP table_... .nc
  Split splt(pIn->file.basename, "_");

  std::string fTable;
  if( splt.size() > 1 )
    fTable = splt[1] ;

  // table sheet name from global attributes has been checked

  // finalise the table sheet check
  if( fTable != tTable )
  {
    if( tTable.size() )
      vMD.varReqTableSheet = tTable; // precedence: global attribute
    else if( ! fTable.size() )
      // no valid table sheet found
      vMD.varReqTableSheet ="Any";  // table sheet name in the project table

    std::string key("46_9");

    if( notes->inq( key, varMeDa[0].var->name) )
    {
       std::string capt("Diverging table sheet name in attribute and filename.") ;

       std::string text("MIP Table  (filename): ") ;
       text += fTable ;
       text += "\nMIP Table (attribute): " ;
       text += tTable;

       (void) notes->operate(capt, text) ;
         setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

std::string
QA::getSubjectsIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry)
{
  std::string intro("VRT=");
  intro += vMD.varReqTableSheet + ", var=";
  intro += vMD.var->name + ", dim=";
  intro += nc_entry.outname ;

  if( tbl_entry.outname == "basin" )
    intro += ", var-rep=region";
  if( tbl_entry.outname == "line" )
    intro += ", var-rep=passage";
  if( tbl_entry.outname == "type" )
    intro += ", var-rep=type_description";

  intro += ":";
  return intro;
}

std::string
QA::getVarnameFromFilename(std::string fName)
{
  size_t pos;
  if( (pos = fName.find("_")) < std::string::npos )
    fName = fName.substr(0,pos) ;

  return fName;
}

void
QA::help(void)
{
  std::cerr << "Option string of the quality control class QA:\n" ;
  std::cerr << "(may be embedded in option strings of Base derived\n" ;
  std::cerr << "classes)\n";
  std::cerr << "or connected to these by explicit index 'QA0...'.\n" ;
  std::cerr << "   checkTimeBounds\n" ;
  std::cerr << "   noCalendar\n" ;
  std::cerr << "   cycle=num\n" ;
  std::cerr << "   printASCII (disables writing to netCDF file.\n" ;
  std::cerr << "   printTimeBoundDates\n" ;
  std::cerr << std::endl;
  return;
}

bool
QA::init(void)
{
   // Open the qa-result.nc file, when available or create
   // it from scratch. Meta data checks are performed.
   // Initialisation of time and time boundary testing.
   // Eventually, entry() is called to test the data of fields.

   notes->init();  // safe

   // default
   setFilename( pIn->file );

   // apply parsed command-line args
   applyOptions();

   fVarname = getVarnameFromFilename(pIn->file.filename);
   getFrequency();

   // Create and set VarMetaData objects.
   createVarMetaData() ;

   // check existance of any data at all
   if( (isCheckTime || isCheckData )
            && pIn->ncRecBeg == 0 && pIn->ncRecEnd == 0 )
   {
      isCheckData=false;
      isCheckTime=false;

      std::string key("6_15");
      if( notes->inq( key, fileStr) )
      {
        std::string capt("No records in the file.") ;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          notes->setCheckTimeStr(fail);
          notes->setCheckDataStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
   }

   // get and check meta data
   checkMetaData(*pIn);

   if( qaTime.init(pIn, notes, this) )
   {
     // init the time object
     // note that freq is compared to the first column of the time table
     qaTime.applyOptions(optStr);
     qaTime.initTimeTable( getFrequency() );

     // note that this test is not part of the QA_Time class, because
     // coding depends on projects
     if( testPeriod() )
     {
        std::string key("9_2");
        if( notes->inq( key, qaTime.name) )
        {
          std::string capt("status is apparently in progress");

          (void) notes->operate(capt) ;
       }
     }
   }
   else
     isCheckTime=false;

   // open netCDF for continuation and resuming a session
   openQA_Nc(*pIn);

   if( getExit() || qaTime.isNoProgress )
   {
     isCheckData=false;
     isCheckTime=false;
     return true;
   }

   if( isCheckTime )
   {
     if( qaTime.isTime && ! pIn->nc.isAnyRecord() )
     {
       isCheckTime = false;
       notes->setCheckTimeStr(fail);
     }
     else if( ! qaTime.isTime )
     {
       isCheckTime = false;
       notes->setCheckTimeStr("FIXED");
     }
     else
       notes->setCheckTimeStr("PASS");
   }

   if( isCheckData )
   {
     if( ! pIn->nc.isAnyRecord() )
     {
       notes->setCheckDataStr(fail);
       return true;
     }

     notes->setCheckDataStr("PASS");

     // set pointer to function for operating tests
     execPtr = &IObj::entry ;
     bool is = entry();

     if( getExit() || is )
       return true;

     isNotFirstRecord = true;
     return false;
   }

   return true;  // only meta-data check
}

void
QA::initDataOutputBuffer(void)
{
  if( isCheckTime )
  {
    qaTime.timeOutputBuffer.initBuffer(this, currQARec, bufferSize);
    qaTime.sharedRecordFlag.initBuffer(this, currQARec, bufferSize);
  }

  if( isCheckData )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i)
      varMeDa[i].qaData.initBuffer(this, currQARec, bufferSize);
  }

  return;
}

void
QA::initDefaults(void)
{
  objName="QA";

  nc=0;
  notes=0;

  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  blank=" ";
  fail="FAIL";
  fileStr="file";
  no_blank="no_blank";
  notAvailable="not available";
  s_colon=":";
  s_mismatch="mismatch";
  s_upper="upper";

  enablePostProc=false;
  enableVersionInHistory=true;

  isCaseInsensitiveVarName=false;
  isCheckParentExpID=true;
  isCheckParentExpRIP=true;
  isExit=false;
  isForceStndTable=false;
  isNoProgress=false;
  isNotFirstRecord=false;
  isResumeSession=false;
  isUseStrict=false;

  nextRecords=0;  //see init()

  importedRecFromPrevQA=0; // initial #rec in out-nc-file.
  currQARec=0;

  // pre-set check-modes: all are used by default
  isCheckMeta=true;
  isCheckTime=true;
  isCheckData=true;

  bufferSize=1500;

  exitCode=0;

#ifdef REVISION
  // -1 by default
  revision=hdhC::double2String(static_cast<int>(REVISION));
#endif

  // set pointer to member function init()
  execPtr = &IObj::init ;
}

void
QA::initGlobalAtts(InFile &in)
{
  // global atts at creation.
  std::string today( Date::getTodayStr() );

  nc->setGlobalAtt( "project", "CMIP5");
  nc->setGlobalAtt( "product", "quality check of CMIP5 data set");

//  enableVersionInHistory=false;
  nc->setGlobalAtt( "QA revision", revision);
  nc->setGlobalAtt( "contact", "hollweg@dkrz.de");

  std::string t("csv formatted ");

  nc->setGlobalAtt( "standard_table", varReqTable.basename + ".xlsx");

  nc->setGlobalAtt( "creation_date", today);

  // helper vector
  std::vector<std::string> vs;

  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    vs.push_back( varMeDa[m].var->name );

  nc->copyAtts(in.nc, "NC_GLOBAL", &vs);

  return;
}

void
QA::initResumeSession(void)
{
  // this method may be used for two different purposes. First,
  // resuming within the same experiment. Second, continuation
  // from a parent experiment.

  // At first, a check over the ensemble of variables.
  // Get the name of the variable(s) used before
  // (only those with a trailing '_ave').
  std::vector<std::string> vss( nc->getVarName() ) ;

  std::vector<std::string> prevTargets ;

  size_t pos;
  for( size_t i=0 ; i < vss.size() ; ++i )
     if( (pos = vss[i].find("_ave")) < std::string::npos )
       prevTargets.push_back( vss[i].substr(0, pos) );

  // a missing variable?
  for( size_t i=0 ; i < prevTargets.size() ; ++i)
  {
    size_t j;
    for( j=0 ; j < varMeDa.size() ; ++j)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("3_14");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt("variable=");
         capt += prevTargets[i] + " is missing in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( fail );
           setExit( notes->getExitValue() ) ;
         }
       }
    }
  }

  // a new variable?
  for( size_t j=0 ; j < varMeDa.size() ; ++j)
  {
    size_t i;
    for( i=0 ; i < prevTargets.size() ; ++i)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( i == prevTargets.size() )
    {
       std::string key("3_15");
       if( notes->inq( key, fileStr) )
       {
         std::string capt("variable=");
         capt += varMeDa[j].var->name + " is new in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( fail );
           setExit( notes->getExitValue() ) ;
         }
       }
    }
  }

  // now, resume
  qaTime.timeOutputBuffer.setNextFlushBeg(currQARec);
  qaTime.setNextFlushBeg(currQARec);

  if( isCheckTime )
    qaTime.initResumeSession();

  if( isCheckData )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i )
    varMeDa[i].qaData.initResumeSession();
  }

  return;
}

void
QA::inqTables(void)
{
  if( ! varReqTable.isExisting(varReqTable.path) )
  {
     std::string key("57");

     if( notes->inq( key, fileStr) )
     {
        std::string capt("no path to the tables, tried ") ;
        capt += varReqTable.path;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
     }
  }

  // tables names usage: both project and standard tables
  // reside in the same path.
  // Naming of the project table:
  if( !projectTableFile.is )
  {
    if( !varReqTable.is )
      projectTableFile.setFile("project_table.csv");
    else
    {
      projectTableFile.setPath(varReqTable.path);
      projectTableFile.setFilename("pt_" + varReqTable.filename);
    }
  }
  else if( projectTableFile.extension != "csv" )
    projectTableFile.setExtension("csv");

  return;
}

void
QA::linkObject(IObj *p)
{
  std::string className = p->getObjName();

  if( className == "X" )
    notes = dynamic_cast<Annotation*>(p) ;
  else if( className ==  "CF" )
    cF = dynamic_cast<CF*>(p) ;
  else if( className == "FD_interface" )
    fDI = dynamic_cast<FD_interface*>(p) ;
  else if( className ==  "IN" )
    pIn = dynamic_cast<InFile*>(p) ;
  else if( className == "Oper" )
    pOper = dynamic_cast<Oper*>(p) ;
  else if( className == "Out" )
    pOut = dynamic_cast<OutFile*>(p) ;
  else if( className == "QA" )
    qA = dynamic_cast<QA*>(p) ;
  else if( className == "TC" )
    tC = dynamic_cast<TimeControl*>(p) ;

  return;
}

/*
bool
QA::locate( GeoData<float> *gd, double *alat, double *alon, const char* crit )
{
  std::string str(crit);

// This is a TODO; but not needed for QA
  MtrxArr<float> &va=gd->getCellValue();
  MtrxArr<double> &wa=gd->getCellWeight();

  size_t i=0;
  double val;  // store extrem value
  int ei=-1;   // store index of extreme

  // find first valid value for initialisation
  for( ; i < va.size() ; ++i)
  {
      if( wa[i] == 0. )
        continue;

      val= va[i];
      ei=i;
      break;
  }

  // no value found
  if( ei < 0 )
  {
    *alat = 999.;
    *alon = 999.;
    return true;
  }

  if( str == "min" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] < val )
        {
          val= va[i];
          ei=i;
        }
    }
  }
  else if( str == "max" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] > val )
        {
          val= va[i];
          ei=i;
        }
    }
  }

  *alat = gd->getCellLatitude(static_cast<size_t>(ei) );
  *alon = gd->getCellLongitude(static_cast<size_t>(ei) );

  return false;
}
*/

void
QA::openQA_Nc(InFile &in)
{
  // Generates a new nc file for QA results or
  // opens an existing one for appending data.
  // Copies time variable from input-nc file.

  // name of the result file was set before
  if ( !qaFile.is )
  {
    std::string key("00");

    if( notes->inq( key) )
    {
      std::string capt("openQA_Nc(): undefined file.") ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
      return;
    }
  }

  nc = new NcAPI;
  if( notes )
    nc->setNotes(notes);

  // don't create a netCDF file, when only meta data are checked.
  // but, a NcAPI object m ust exist
  if( ! isCheckTime )
    return;

  if( nc->open(qaFile.getFile(), "NC_WRITE", false) )
//   if( isQA_open ) // false: do not exit in case of error
  {
    // continue a previous session
    importedRecFromPrevQA=nc->getNumOfRecords();
    currQARec += importedRecFromPrevQA;
    if( currQARec )
      isNotFirstRecord = true;

    initDataOutputBuffer();

    initResumeSession();
    isResumeSession=true;

    return;
  }

  if( currQARec == 0 && in.nc.getNumOfRecords() == 1 )
    qaTime.isSingleTimeValue = true;

  // open new netcdf file
  if( qaNcfileFlags.size() )
    nc->create(qaFile.getFile(),  qaNcfileFlags);
  else
    nc->create(qaFile.getFile(),  "Replace");

  if( pIn->isTime )
  {
    // create a dimension for fixed variables only if there is any
    for( size_t m=0 ; m < varMeDa.size() ; ++m )
    {
      if( varMeDa[m].var->isFixed )
      {
        nc->defineDim("fixed", 1);
        break;
      }
    }

    qaTime.openQA_NcContrib(nc);
  }
  else if( isCheckTime )
  {
    // dimensions
    qaTime.name="fixed";
    nc->defineDim("fixed", 1);
  }

  // create variable for the data statics etc.
  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    varMeDa[m].qaData.openQA_NcContrib(nc, varMeDa[m].var);

  // global atts at creation.
  initGlobalAtts(in);

  initDataOutputBuffer();

  return;
}

bool
QA::postProc(void)
{
  bool retCode=false;

  if( ! isCheckData )
    return retCode;

  if( postProc_outlierTest() )
     retCode=true;

  return retCode;

}

bool
QA::postProc_outlierTest(void)
{
  bool retCode=false;

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
     VariableMetaData &vMD = varMeDa[i] ;

     if( ! vMD.qaData.enableOutlierTest )
        continue ;

     if( ! vMD.qaData.statAve.getSampleSize()  )
     {
       // compatibility mode: determine the statistics of old-fashioned results
       // note: the qa_<variable>.nc file becomes updated
       std::vector<std::string> vars ;
       std::string statStr;

       size_t recNum=nc->getNumOfRecords();
       double val;
       std::vector<double> dv;
       MtrxArr<double> mv;

       if( vMD.var->isNoData )
         continue;

       vars.clear();

       // post-processing of data before statistics was vMD inherent?
       vars.push_back( vMD.var->name + "_min" );
       vars.push_back( vMD.var->name + "_max" );
       vars.push_back( vMD.var->name + "_ave" );
       vars.push_back( vMD.var->name + "_std_dev" );

       bool is=false;
       bool is2=true;
       for( size_t j=0 ; j < vars.size() ; ++j )
       {
         // read the statistics
         nc->getAttValues( dv, "valid_range", vars[j]);
         if( dv[0] == MAXDOUBLE )
         {
            is=true; // no valid statistics
            break;
         }
         statStr  ="sampleMin=" ;
         statStr += hdhC::double2String( dv[0] );
         statStr +=", sampleMax=" ;
         statStr += hdhC::double2String( dv[1] );
         statStr += ", ";
         statStr += nc->getAttString("statistics", vars[j], is2) ;
         if( ! is2 )
         {
            is=true; // no valid statistics
            break;
         }

         if( j == 0 )
           vMD.qaData.statMin.setSampleProperties( statStr );
         else if( j == 1 )
           vMD.qaData.statMax.setSampleProperties( statStr );
         else if( j == 2 )
           vMD.qaData.statAve.setSampleProperties( statStr );
         else if( j == 3 )
           vMD.qaData.statStdDev.setSampleProperties( statStr );

         is=false;
       }

       if( is )
       {
         val = nc->getData(mv, vars[0], 0);
         if( val < MAXDOUBLE )
         {
           // build the statistics from scratch
           int nRecs = static_cast<int>(recNum);

           // constrain memory allocation to a buffer size
           int sz_max=1000;
           size_t start[] = {0};
           size_t count[] = {0};

           // buffer for netCDF data storage
           double vals_min[sz_max];
           double vals_max[sz_max];
           double vals_ave[sz_max];
           double vals_std_dev[sz_max];

           // read data from file, chunk by chunk
           while (nRecs > 0)
           {
             int sz = nRecs > sz_max ? sz_max : nRecs;
             nRecs -= sz;

             count[0]=static_cast<size_t>(sz);

             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[0]),
                   start, count, vals_min );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[1]),
                   start, count, vals_max );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[2]),
                   start, count, vals_ave );
             nc_get_vara_double(
                nc->getNcid(), nc->getVarID(vars[3]),
                   start, count, vals_std_dev );

             start[0] += static_cast<size_t>(sz) ;

// vals_max[10]=400.;
// vals_min[10]=150.;
             // feed data to the statistics
             vMD.qaData.statMin.add( vals_min, sz );
             vMD.qaData.statMax.add( vals_max, sz );
             vMD.qaData.statAve.add( vals_ave, sz );
             vMD.qaData.statStdDev.add( vals_std_dev, sz );
           }

           // write statistics attributes
           vMD.qaData.setStatisticsAttribute(nc);
         }
       }
     }

     // the test: regular post-processing on the basis of stored statistics
     if( vMD.qaData.outlier->test(&vMD.qaData) )
       retCode = true;
  }

  return retCode;
}

void
QA::pushBackVarMeDa(Variable *var)
{
   varMeDa.push_back( VariableMetaData(this, var) );

   if( var )
   {
     VariableMetaData &vMD = varMeDa.back();

     vMD.forkAnnotation(notes);

     // disable tests by given options
     vMD.qaData.disableTests(var->name);

     vMD.qaData.init(pIn, this, vMD.var->name);
   }

   return;
}

bool
QA::readHeadline(ReadLine &ifs,
   VariableMetaData &vMD,
   std::map<std::string, size_t> &v_col,
   std::map<std::string, size_t> &d_col,
   size_t &v_colMax, size_t &d_colMax )
{
   // find the capts for table sheets dims and for the variables

   std::string str0;

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   // This should give the heading of the table sheet for the dimensions.
   // The table sheet is identified by the first column.
   while( !ifs.getLine(str0) )
   {
     if( str0.substr(0,13) == "CMOR table(s)" )
       break ;
   }


   if( ifs.eof() )
   {
      std::string key("49");

      if( notes->inq( key, varMeDa[0].var->name) )
      {
        // no dim sheet of the standard  table
        std::string capt("table sheet for dimensions not found in the standard table.") ;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
     }
   }


   splt_line=str0;

   // identify columns of the Taylor table; look for the indexes
   for( d_colMax=0 ; d_colMax < splt_line.size() ; ++d_colMax)
   {
      if( d_colMax == 0 )
        d_col["cmorTables"] = d_colMax;
      else if( splt_line[d_colMax] == "CMOR dimension" )
        d_col["cmorName"] = d_colMax;
      else if( splt_line[d_colMax] == "output dimension name" )
        d_col["outputName"] = d_colMax;
      else if( splt_line[d_colMax] == "standard name" )
        d_col["standardName"] = d_colMax;
      else if( splt_line[d_colMax] == "long name" )
        d_col["longName"] = d_colMax;
      else if( splt_line[d_colMax] == "axis" )
        d_col["axis"] = d_colMax;
      else if( splt_line[d_colMax] == "index axis?" )
        d_col["index_axis"] = d_colMax;
      else if( splt_line[d_colMax] == "units" )
        d_col["units"] = d_colMax;
      else if( splt_line[d_colMax] == "coords_attrib" )
        d_col["coord"] = d_colMax;
      else if( splt_line[d_colMax] == "bounds?" )
        d_col["bounds?"] = d_colMax;
      else if( splt_line[d_colMax] == "type" )
        d_col["type"] = d_colMax;
      else if( splt_line[d_colMax] == "value" )
        d_col["value"] = d_colMax;
      else if( splt_line[d_colMax] == "requested" )
        d_col["requested"] = d_colMax;
      else if( splt_line[d_colMax] == "bounds _values"
               || splt_line[d_colMax] == "bounds_values" )
        d_col["bounds_values"] = d_colMax;
      else if( splt_line[d_colMax] == "bounds _requested"
               || splt_line[d_colMax] == "bounds_requested" )
        d_col["bounds_requested"] = d_colMax;
   }

   // find the capt for variables
   // back to the beginning.
   ifs.FStream->seekg(0, std::ios::beg) ;

   while( ! ifs.getLine(str0) )
   {
     if( str0.substr(0,8) == "priority" )
       break;
   } ;

   splt_line=str0;

   // now, look for the indexes of the variable's columns
   for( v_colMax=1 ; v_colMax < splt_line.size() ; ++v_colMax)
   {
     if( splt_line[v_colMax] == "CMOR variable name" )
       v_col["CMOR_variable_name"] = v_colMax;
     if( splt_line[v_colMax] == "output variable name" )
       v_col["output_variable_name"] = v_colMax;
     else if( splt_line[v_colMax] == "priority" )
       v_col["priority"] = v_colMax;
     else if( splt_line[v_colMax] == "standard name" )
       v_col["standard_name"] = v_colMax;
     else if( splt_line[v_colMax] == "long name" )
       v_col["long_name"] = v_colMax;
     else if( splt_line[v_colMax] == "unformatted units" )
       v_col["unformatted_units"] = v_colMax;
     else if( splt_line[v_colMax] == "cell_methods" )
       v_col["cell_methods"] = v_colMax;
     else if( splt_line[v_colMax] == "cell_measures" )
       v_col["cell_measures"] = v_colMax;
     else if( splt_line[v_colMax] == "type" )
       v_col["type"] = v_colMax;
     else if( splt_line[v_colMax] == "CMOR dimensions" )
       v_col["CMOR_dimensions"] = v_colMax;
     else if( splt_line[v_colMax] == "valid min" )
       v_col["validMin"] = v_colMax;
      else if( splt_line[v_colMax] == "valid max" )
       v_col["validMax"] = v_colMax;
   }

   // back to the beginning.
   ifs.FStream->seekg(0, std::ios::beg) ;

   return true;
}

void
QA::setCheckMode(std::string m)
{
  isCheckMeta=false;
  isCheckTime=false;
  isCheckData=false;

  Split cvs(m, ',');
  for( size_t j=0 ; j < cvs.size() ; ++j )
  {
    if( cvs[j] == "meta" )
      isCheckMeta=true ;
    else if( cvs[j] == "time" )
      isCheckTime=true ;
    else if( cvs[j] == "data" )
      isCheckData=true ;
  }

  return;
}

void
QA::setExit( int e )
{
  if( e > exitCode )
  {
    exitCode=e;
    isExit=true;
  }

  return ;
}

void
QA::setFilename(hdhC::FileSplit& fC)
{
  std::string f(fC.basename);

  Split x(f, '_');

  if( x.size() )
  {
    Split y(x[x.size()-1], '-');

    size_t sz = y.size();

    if( sz == 2 &&
          hdhC::isDigit( y[0]) && hdhC::isDigit( y[1]) )
    {
      f = "qa";

      for( size_t i=0 ; i < x.size()-1 ; ++i )
        f += "_" + x[i] ;
    }
  }

  qaFile.setFilename(f + ".nc");

  return ;
}

void
QA::setTable(std::string t, std::string acronym)
{
  // this method may be called from a spot,
  // where there is still no valid table name.
  if( t.size() )
  {
    currTable = t ;
    notes->setTable(t, acronym);
  }

  return;
}

void
QA::storeData(std::vector<hdhC::FieldData> &fA)
{
  //FieldData structure defined in geoData.h

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    VariableMetaData &vMD = varMeDa[i];

    if( vMD.var->isNoData )
      return;

    if( isNotFirstRecord && vMD.var->isFixed  )
      continue;

    vMD.qaData.store(fA[i]) ;
  }

  return ;
}

void
QA::storeTime(void)
{
   // testing time steps and bound (if any)
   qaTime.testDate(pIn->nc);

   qaTime.timeOutputBuffer.store(qaTime.currTimeValue, qaTime.currTimeStep);
   qaTime.sharedRecordFlag.store();

   return ;
}

bool
QA::testPeriod(void)
{
  // return true, if a file is supposed to be not complete.
  // return false, a) if there is no time-stamp in the filename
  //               b) if an error was found
  //               c) times of stamp and file match

  // The time value given in the first/last record is assumed to be
  // in the range of the time-stamp of the file, if there is any.

  // If the first/last date in the filename time-stamp and the
  // first/last time value match within the uncertainty of the
  // time-step, then the file is complete.
  // If the end of the time-stamp exceeds the time data figure,
  // then the nc-file is considered to be not completely processed.

  // Does the filename has a trailing date range?
  // Strip off the extension.
  std::string f( qaFile.getBasename() );

  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  // if designator '-clim' is appended, then remove it
  int f_sz = static_cast<int>(f.size()) -5 ;
  if( f_sz > 5 && f.substr(f_sz) == "-clim" )
    f=f.substr(0, f_sz);

  size_t p0, p1;
  if( (p0=f.rfind('_')) == std::string::npos )
    return false ;  // the filename is composed totally wrong

  if( (p1=f.find('-', p0)) == std::string::npos )
    return false ;  // no period in the filename

  sd[1]=f.substr(p1+1) ;
  if( ! hdhC::isDigit(sd[1]) )
    return false ;  // no pure digits behind '-'

  sd[0]=f.substr(p0+1, p1-p0-1) ;
  if( ! hdhC::isDigit(sd[0]) )
    return false ;  // no pure 1st date found

  // now we have found two candidates for a date
  // compose ISO-8601 strings
  std::vector<Date> period;
  qaTime.getDRSformattedDateRange(period, sd);

  Date* fN_left = &period[0];
  Date* fN_right = &period[1];

  // necessary for validity (not sufficient)
  if( *fN_left > *fN_right )
  {
     std::string key("1_7");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("invalid time-stamp in the filename, found ");
       capt += hdhC::tf_val(sd[0] + "-" + sd[1]);

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( fail );
     }

     return false;
  }

  Date* tV_left = 0 ;
  Date* tV_right = 0;

  Date* tV_left_obj = 0 ;
  Date* tV_right_obj = 0;
  Date* tB_left_obj = 0 ;
  Date* tB_right_obj = 0;

  bool isLeft_fT_not_tV ;
  bool isRight_fT_not_tV ;

  if( qaTime.isTimeBounds)
  {
    tB_left_obj = new Date(qaTime.refDate);
    tB_left_obj->addTime(qaTime.firstTimeBoundsValue[0]);
    tV_left = tB_left_obj;

    tB_right_obj = new Date(qaTime.refDate);
    tB_right_obj->addTime(qaTime.firstTimeBoundsValue[1]);
    tV_right = tB_right_obj;

    isLeft_fT_not_tV = *tB_left_obj != *fN_left ;
    isRight_fT_not_tV = *tB_right_obj != *fN_right ;

/*
    double db_centre=(qaTime.firstTimeBoundsValue[0]
                        + qaTime.firstTimeBoundsValue[1])/2. ;
    if( db_centre != qaTime.firstTimeValue )
    {
      std::string key("16_11");
      if( notes->inq( key, fVarname) )
      {
        std::string capt("Range of variable time_bnds is not centred around variable time.");

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( fail );
      }
    }
*/
  }
  else
  {
    tV_left_obj = new Date(qaTime.refDate);
    if( qaTime.firstTimeValue != 0. )
      tV_left_obj->addTime(qaTime.firstTimeValue);
    tV_left = tV_left_obj;

    tV_right_obj = new Date(qaTime.refDate);
    if( qaTime.lastTimeValue != 0. )
      tV_right_obj->addTime(qaTime.lastTimeValue);
    tV_right = tV_right_obj;

    isLeft_fT_not_tV = *tV_left != *fN_left ;
    isRight_fT_not_tV = *tV_right != *fN_right ;

    std::string key("16_12");
    if( notes->inq( key, fVarname) )
    {
      std::string capt("Variable time_bnds is missing");

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( fail );
    }
  }

  if( isLeft_fT_not_tV )
  {
    // CMOR isn't capable of working with the time_bounds. Hence, an exception
    // rule is added to the CORDEX archive design as to the period in filenames:
    // "It is also allowed to use time values of the first and last records
    // in NetCDF files for averaged data." However, this does not lead to a
    // working solution. In fact, CMOR sets StartTime to the beginning  of
    // the month bearing the first time value.
    // So, here is a LEX CMOR
    if( tV_left_obj == 0 )
    {
      tV_left_obj = new Date(qaTime.refDate);
      tV_left = tV_left_obj;
      if( qaTime.firstTimeValue != 0. )
        tV_left->addTime(qaTime.firstTimeValue);
    }
    double monDaysNum = tV_left->getMonthDaysNum();
    tV_left->addTime(-monDaysNum, "day");

    if( tV_right_obj == 0 )
    {
      tV_right_obj = new Date(qaTime.refDate);
      tV_right = tV_right_obj;
      if( qaTime.lastTimeValue != 0. )
        tV_right->addTime(qaTime.lastTimeValue);
    }

    isLeft_fT_not_tV = *tV_left != *fN_left ;
    isRight_fT_not_tV = *tV_right != *fN_right ;
  }

  // the annotation
  if( isLeft_fT_not_tV )
  {
     std::string key("1_6");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("First date ");
       capt += hdhC::sAssign("(filename)", fN_left->str()) ;
       capt += " and time " ;
       if( tB_left_obj )
         capt += "bounds ";
       capt += hdhC::sAssign("data", tV_left->str());
       capt += " are misaligned";

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( fail );
     }
  }

  // test completeness: the end of the file
  if( isFileComplete && isRight_fT_not_tV )
  {
     std::string key("1_6");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("Second date ");
       capt += hdhC::sAssign("(filename)", fN_right->str()) ;

       capt += " is misaligned to time " ;
       if( tB_left_obj )
         capt += "bounds ";
       capt += hdhC::sAssign("data", tB_right_obj->str());

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( fail );
     }
  }

  // format of period dates.
/*
  if( testPeriodFormat(sd) )
    // period requires a cut specific to the various frequencies.
    testPeriodCut(sd) ;
*/

  if( tV_left_obj )
    delete tV_left_obj;
  if( tV_right_obj )
    delete tV_right_obj;
  if( tB_left_obj )
    delete tB_left_obj;
  if( tB_right_obj )
    delete tB_right_obj;

  // complete
  return false;
}

void
QA::setVarMetaData(VariableMetaData &vMD)
{
  // collect some properties in struct varMeDa.

  // some more properties
  vMD.var->std_name = pIn->nc.getAttString("standard_name", vMD.var->name);
  vMD.longName = pIn->nc.getAttString("long_name", vMD.var->name);
  vMD.cellMethods = pIn->nc.getAttString("cell_methods", vMD.var->name);
  vMD.cellMeasures = pIn->nc.getAttString("cell_measures", vMD.var->name);

  vMD.typeStr= pIn->nc.getVarTypeStr(vMD.var->name);

  return;
}

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;

   if( v )
   {
     var = v;
     isOwnVar=false;
   }
   else
   {
     var = new Variable ;
     isOwnVar=true;
   }

   isForkedAnnotation=false;
}

VariableMetaData::~VariableMetaData()
{
  dataOutputBuffer.clear();

  if( isOwnVar )
    delete var;
}

int
VariableMetaData::finally(int xCode)
{
  // write pending results to qa-file.nc. Modes are considered there
  qaData.flush();

  // annotation obj forked by the parent VMD
  notes->printFlags();

  int rV = notes->getExitValue();
  xCode = ( xCode > rV ) ? xCode : rV ;

  return xCode ;
}

void
VariableMetaData::forkAnnotation(Annotation *n)
{
//   if( isForkedAnnotation )
//      delete notes;

   notes = new Annotation(n);

//   isForkedAnnotation=true;

   // this is not a mistaken
   qaData.setAnnotation(n);

   return;
}

void
VariableMetaData::setAnnotation(Annotation *n)
{
//   if( isForkedAnnotation )
//      delete notes;

   notes = n;

//   isForkedAnnotation=false;

   qaData.setAnnotation(n);

   return;
}
