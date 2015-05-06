#include "qa.h"

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
QA::appendToHistory(size_t eCode)
{
  // date and time at run time
  std::string today( Date::getCurrentDate() );
  today += ":";

  std::string s;
  std::string hst;
  std::string path( filename.substr(0, filename.rfind('/')) ) ;
  std::string file( filename.substr(filename.rfind('/')+1) ) ;
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
      if( path != hstPath )
      {
        hst += "\n" ;
        hst += today;
        hst += " changed path to data: ";
        hst += path + "\n" ;
      }
    }
  }
  else
  {
    // the root of the history string
    hst += today;
    hst += " path_to_data: ";
    hst += path ;
/*
    hst += "\n Filenames and tracking_id in file tid_";

    std::string t0(qaFilename.substr(3));
    t0 = t0.substr(0, t0.size()-3);
    hst += t0;
    hst += ".txt" ;
*/
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
      tmp = nc->getAttString("svn_revision");

    if( svnVersion != tmp )
    {
      hst += "\n" ;
      hst += today;
      hst += " changed QA svn revision: " ;
      hst += svnVersion ;
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
       {
          // path to the directory where the execution takes place
          dataPath=split[1];
          continue;
       }
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
       {
          qaFilename=split[1];
          continue;
       }
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
       {
          nextRecords=static_cast<size_t>(split.toDouble(1));
          continue;
       }
     }

     if( split[0] == "oT"
           || split[0] == "outlierTest"
                || split[0] == "outlier_test" )
     {
       if( split.size() == 2 )
       {
          if( split[1].find("POST") < std::string::npos
                   && ! enablePostProc )
            continue;

          outlierOpts.push_back(split[1]);
          continue;
       }
     }

     if( split[0] == "pEI" || split[0] == "parentExperimentID"
         || split[0] == "parent_experiment_id" )
     {
       if( split.size() == 2 )
       {
          parentExpID=split[1];
          if( parentExpID == "none" )
            isCheckParentExpID=false ;
          continue;
       }
     }

     if( split[0] == "pER" || split[0] == "parentExperimentRIP"
         || split[0] == "parent_experiment_rip" )
     {
       if( split.size() == 2 )
       {
          parentExpRIP=split[1];
          if( parentExpRIP == "none" )
            isCheckParentExpRIP=false ;
          continue;
       }
     }

     if( split[0] == "qNF" || split[0] == "qaNcfileFlags"
       || split[0] == "qa_ncfile_flags" )
     {
       if( split.size() == 2 )
       {
          qaNcfileFlags=split[1];
          continue;
       }
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
          continue;
       }
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
       {
          tablePath=split[1];
          continue;
       }
     }

     if( split[0] == "totP"
          || split[0] == "totalPeriod" || split[0] == "total_period")
     {
       if( split.size() == 2 )
       {
          totalPeriod = split[1] ;
          continue;
       }
     }


     if( split[0] == "tPr"
          || split[0] == "tableProject" )  // dummy
     {
       if( split.size() == 2 )
       {
          size_t pos;
          if( (pos=split[1].rfind('/')) < std::string::npos )
          {
            if( split[1][0] == '/' )
            {  // absolute path
              tablePath=split[1].substr(0, pos);
              projectTableName=split[1].substr(pos+1);
            }
            else // relative path remains part of the tablename
              projectTableName=split[1];
          }
          else
            projectTableName=split[1];

          continue;
       }
     }

     if( split[0] == "tStd"
          || split[0] == "tableStandard" )
     {
       if( split.size() == 2 )
       {
          size_t pos;
          if( (pos=split[1].rfind('/')) < std::string::npos )
          {
            if( split[1][0] == '/' )
            {  // absolute path
              tablePath=split[1].substr(0, pos);
              standardTable=split[1].substr(pos+1);
            }
            else // relative path remains part of the tablename
              standardTable=split[1];
          }
          else
            standardTable=split[1];

          continue;
       }
     }

     if( split[0] == "uS"
         || split[0] == "useStrict"
            || split[0] == "use_strict" )
     {
          isUseStrict=true ;
          isForceStndTable=true;
          setCheckMode("meta");
          continue;
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
    std::string key("56_5");
    if( notes->inq( key, vName) )
    {
      std::string capt("missing auxiliary in the file") ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Conflict for auxiliary between file and table." ;
      ostr << "\nStandard table: " << vMD.stdTable ;
      ostr << ", MIP-Table: " << vMD.stdSubTable ;
      ostr << ", Variable: " << vMD.name ;
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
QA::checkDimStandardTable(ReadLine &ifs, InFile &in,
  VariableMetaData &vMD,
  std::vector<struct DimensionMetaData> &dimNcMeDa,
  std::map<std::string, size_t> &col,
  std::string dimName, size_t colMax )
{
  // dimName is a so-called CMOR-dimension in the standard table.
  // This method is called for each of such a dimension.

  // dimName is the only unique connection between the dimensional
  // MIP table and those MIP tables for the variables.
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

   // This should give the heading for the dimension MIP table.
   // The MIP table is identified by the first column.
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

     // end-of-MIP table
     if( ifs.eof()
         || splt_line[0].substr(0,10) == "CMOR Table" )
       break;

     // is it in the table?
     if( splt_line[col["cmorName"]] == dimName )
     {

       if( splt_line.size() < colMax )
       {
          std::string key("50");

          if( notes->inq( key) )
          {
            std::string capt("corrupt standard sub-table for dimensions: wrong number of columns.") ;

            std::ostringstream ostr(std::ios::app);
            ostr << "Standard table: " << standardTable;
            ostr << "\nCurrent line: " << str0 ;
            ostr << "\nRequired number of items of the MIP table for ";
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
               checkDimStandardTable(ifs, in, vMD, dimNcMeDa,
                  col, dimTE.cmor_name, colMax );
             }
          }
          else
          {
            if( in.nc.isVariableValid(dimTE.outname) )
            {
              dimNcMeDa.push_back( DimensionMetaData() );
              getDimMetaData(in, vMD, dimNcMeDa.back(), dimTE.outname) ;
              checkDimStandardTable(ifs, in, vMD, dimNcMeDa,
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

       // dimension 'lev' is usually generic.
       // special: variable 'lev' contains the added coefficients
       // a and b of the hybrid sigma pressure coordinates.
       if( dimTE.outname == "lev" )
       {
         if( dimTE.stndname ==
                "atmosphere_hybrid_sigma_pressure_coordinate" )
         {
           std::vector<std::string> vs( in.nc.getVarNames() );
           checkSigmaPressureCoordinates( *p_vMD, vs);
         }

         // discard any dim checks
         return;
       }

       // Purpose: a dim in the table is not defined in the file
       // but given as variable in the file.
       // Switch to a pseudo dimFE object.
       if( !checkDimlessVar(in, splt_line,
              vMD, p_dimFE, tmp_dimFE, dimTE, col) )
         return; // aux is not in the file

       // size and checksum of dimensional values
       checkStandardTableDimValues(in, splt_line,
              *p_vMD, *p_dimFE, dimTE, col);

       // size and checksum of dimensional bounds
       checkStandardTableDimBounds(in, splt_line,
              *p_vMD, *p_dimFE, dimTE, col);

       // compare findings from the file with those from the table
       checkDimTableEntry(in, *p_vMD, *p_dimFE, dimTE) ;

       if( dimTE.cmor_name == "plevs" )
       {
         // restore for the project table
         dimNcMeDa[index].size     = plevs_file_size  ;
         dimNcMeDa[index].checksum = plevs_file_checksum ;
       }

       return;
    }
  }


  // Arriving here is an error, because the dimensions looked for is
  // from a CMOR Table for variables, but was not found in the
  // MIP table for dimensions.

  std::string key("48");

  if( notes->inq( key, vMD.name) )
  {
    std::string capt("dimension not found in the standard table.") ;

    std::ostringstream ostr(std::ios::app);
    ostr << "QA::checkDimStandardTable()";
    ostr << "\nStandard table: " << standardTable;
    ostr << "\nDimension " <<  dimName;
    ostr << " not found in the standard table.";

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
   // this method is visited whenever a dimension in the standard
   // table has no corresponding var-rep in the file of the same name.

   // required to enter the second if-clause in case the first one wasn't
   size_t ix=in.variable.size();

   // the user could have defined a dimension of size==1, but the var-rep
   // is given by a different name. On the other hand, the dimension in the table
   // corresponds to a single value;

   if( in.nc.isDimValid(dimTE.outname) && in.nc.getDimSize( dimTE.outname ) == 1 )
   {
     // find var-rep with a single dim of size==1
     // possible that the var-rep ios named slightly differently, e.g. height and height2m
     size_t sz=dimTE.outname.size() ;

     for( ix=0 ; ix < in.variable.size() ; ++ix )
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
   if( ix == in.variable.size() )
   {
     for( ix=0 ; ix < in.variable.size() ; ++ix )
     {
       if( in.variable[ix].dimName.size() == 0 )
       {
         // only a single value: postponed coding
         return true;
       }
     }
   }

   if( ix == in.variable.size() )
   {
     std::string key("52");

     if( notes->inq( key, vMD.name) )
     {
       std::string capt("dimension from the table not found in the file.") ;

       std::ostringstream ostr(std::ios::app);
       ostr << "Standard table: ";
       ostr << standardTable;
       ostr << ", variable: ";
       ostr << vMD.name;
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
   // there are var-reps which bear the value they ought to have in the name, e.g. height10m
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
QA::checkDimTableEntry(InFile &in,
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
  // Is done in separately.
  if( currTable == standardTable && tbl_entry.outname == "lev" )
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
  // declaration of a variable across MIP tables
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
  std::string key("47_4");

  if( notes->inq( key, vMD.name) )
  {
    std::string capt( getCurrentTableSubst() );
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

    if( tbl_entry.axis.size() && nc_entry.axis.size() )
      capt += " axis conflict." ;
    else if( tbl_entry.axis.size() )
      capt += " axis not available in the file." ;
    else
      capt += " axis not available in the table." ;

    std::string text("                 axis:\t table: ") ;
    if( tbl_entry.axis.size() )
      text += tbl_entry.axis ;
    else
      text += notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.axis.size() )
      text += nc_entry.axis ;
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
QA::checkDimBndsName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // standard table
  if( nc_entry.bnds_name.size() )
    return ;

  std::string key("47_7");

  if( notes->inq( key, vMD.name) )
  {
    std::string capt( getCurrentTableSubst() );
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;
    capt += " bounds not available in the table." ;

    std::string text("          bounds-name:\t table: ") ;
    text += "requested? " + tbl_entry.bnds_name ;

    text += "\n                      \tncfile: " ;
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
QA::checkDimChecksum(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("47_5");

  if( notes->inq( key, vMD.name) )
  {
    std::string capt = getCurrentTableSubst() ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;
    capt += " layout (checksum) conflict." ;

    std::string text("             checksum:\t table: ") ;
    text += hdhC::double2String(tbl_entry.checksum) ;
    text += "\n                      \tncfile: " ;
    text += hdhC::double2String(nc_entry.checksum) ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(fail);
      setExit( notes->getExitValue() ) ;
    }
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
   // parameter representation; thera is a var(...,dimFE_name,...)
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
  std::string key("47_3");

  if( notes->inq( key, vMD.name) )
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

  if( notes->inq( key, vMD.name) )
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

  std::string key("47_8");
  key += nc_entry.outname;
  if( notes->inq( key, vMD.name) )
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
  std::string key("47_2");

  if( notes->inq( key, vMD.name) )
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
      std::string key("47_9");
      if( notes->inq( key, vMD.name) )
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
        std::string key("47_9");
        if( notes->inq( key, vMD.name) )
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
        if( ! qaTime.isReferenceDateAcrossExp && currQcRec == 0 )
          return ;

        Date tbl_ref( tbl_entry.units, qaTime.calendar );
        Date nc_ref( nc_entry.units, qaTime.calendar );

        std::string key("47_10");
        if( notes->inq( key, vMD.name) )
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
     struct DimensionMetaData &tbl_entry)
{
  // Special: units related to any dimension but 'time'

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?

  // index axis has no units
  if( tbl_entry.index_axis == "ok")
    return;

  if( tbl_entry.units == nc_entry.units )
    return;

  // I) dim==lev: generic
  if( tbl_entry.outname == "lev" )
    return;  // this may have units or not

  std::string tableType;
  if( currTable == standardTable )
    tableType = "standard table: ";
  else
    tableType = "project table: ";

  std::string t0;

  // I) dimension's var-rep without units in the standard table
  if( tbl_entry.outname == "site")
  {
    std::string key("47_6");

    if( notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() );
      capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry) ;

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
    if( tbl_entry.outname != dim[ix] )
      continue;

    // Is coordinates-att set in the corresponding variable?
    bool isDefined=true;
    std::string tmp( in.nc.getAttString(
                    "coordinates",vMD.name, isDefined ) );

    if( tmp != dimVar[ix] )
    {
      std::string key("58_8");
      if( notes->inq( key, vMD.name) )
      {
        std::string capt( getCurrentTableSubst() );
        capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

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
      std::string key("47_13");
      if( notes->inq( key, vMD.name) )
      {
        std::string capt( getCurrentTableSubst() ) ;
        capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);
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
  if( notes->inq( key, vMD.name) )
  {
    std::string capt( getCurrentTableSubst() ) ;
    capt += getSubjectsIntroDim(vMD, nc_entry, tbl_entry);

    if( tbl_entry.units.size() && nc_entry.units.size() )
      capt += " units conflict." ;
    else if( tbl_entry.isUnitsDefined )
      capt += " units not available in the file." ;
    else
      capt += " units not defined in the table." ;

    std::string text("                units:\t table: ") ;
    if( tbl_entry.units.size() )
      text += tbl_entry.units ;
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
  std::string &ga_MIP_table)
{
  // This is only for CMIP5.
  // The global attributes in the section for global attributes
  // must satisfy the filename.
  // Unfortunately, a check for DRS path constraints is infeasable,
  // because arbitrary directory names and links might be found.

  // CMIP5 filename encoding
  // <variable_name>_<stdSubTables>_<model>_<experiment>_<ensemble_member>
  // variable_name is not textd in an attribute
  std::string f( pIn->filename);
  size_t pos;
  if( (pos=f.rfind('/')) < std::string::npos )
    f = f.substr(pos+1);

  Split splt(f, "_");

  if( splt.size() > 4 )
  {
    // changed context for f
    f = ("<variable>_");
    f += splt[1] + "_" ;  // MIP table
    f += splt[2] + "_" ;  // model_id
    f += splt[3] + "_" ;  // experiment_id

    // needed later
    experiment_id = splt[3] ;

    if( splt[1] != "fx" )
      f += splt[4] + "[_<temporal subset>].nc" ;  // ensemble member
  }
  else
  {
     std::string key("46_6");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("filename is inconsistent with CMOR encoding.");

       (void) notes->operate(capt) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
    }
  }

  std::string a_model( pIn->nc.getAttString("model_id") );
  std::string a_experiment( pIn->nc.getAttString("experiment_id") );
  std::string a_ensemble_member;

  // ensemble_member is constructed from three separate attributes
  int tmp = -1 ;
  tmp = static_cast<int>( pIn->nc.getAttValue("realization") ) ;
  if( tmp > -1 )
  {
     a_ensemble_member  ='r' ;
     a_ensemble_member += hdhC::itoa(tmp) ;

     tmp = -1 ;
     tmp = static_cast<int>( pIn->nc.getAttValue("initialization_method") ) ;
     if( tmp > -1 )
     {
       a_ensemble_member +='i' ;
       a_ensemble_member += hdhC::itoa(tmp) ;

       tmp = -1 ;
       tmp = static_cast<int>( pIn->nc.getAttValue("physics_version") );
       if( tmp > -1 )
       {
         a_ensemble_member +='p' ;
         a_ensemble_member += hdhC::itoa(tmp) ;
       }
       else
       {
         std::string key("46_2");
         if( notes->inq( key, fileStr) )
         {
           std::string capt("missing attribute: physics_version.") ;

           a_ensemble_member.clear();

           (void) notes->operate(capt) ;
           {
             notes->setCheckMetaStr(fail);
             setExit( notes->getExitValue() ) ;
           }
         }
       }
     }
     else
     {
       std::string key("46_3");
       if( notes->inq( key, fileStr) )
       {
         std::string capt( "missing attribute: initialization_method." );

         a_ensemble_member.clear();

         (void) notes->operate(capt) ;
         {
           notes->setCheckMetaStr(fail);
           setExit( notes->getExitValue() ) ;
         }
      }
    }
  }
  else
  {
    std::string key("46_4");
    if( notes->inq( key, fileStr) )
    {
      std::string capt( "missing attribute: realization." ) ;

      (void) notes->operate(capt) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  std::string a_experiment_id( pIn->nc.getAttString("experiment_id") );

  if( a_experiment_id.size() == 0 )
  {
    std::string key("46_5");
    if( notes->inq( key, fileStr))
    {
      std::string capt( "missing attribute: experiment_id." );

      (void) notes->operate(capt) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  std::string a_str ;

  if( isCheckParentExpID )
  {
    a_str = pIn->nc.getAttString("parent_experiment_id") ;
    if( a_str.size() == 0 )
    {
      std::string key("30_3");
      if( notes->inq( key, fileStr) )
      {
        std::string capt("missing attribute:  parent_experiment_id.") ;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }
  }

  a_str.clear();
  if( isCheckParentExpRIP )
  {
    a_str = pIn->nc.getAttString("parent_experiment_rip") ;
    if( a_str.size() == 0 )
    {
      std::string key("30_4");
      if( notes->inq( key, fileStr))
      {
        std::string capt("missing attribute: parent_experiment_rip.") ;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }
  }

  if( a_ensemble_member.size() == 0 )
    return ;  // due to missing attribute(s)

  std::string a("<variable>_");
  a += ga_MIP_table + "_" ;
  a += a_model + "_" ;
  a += a_experiment + "_" ;

  if( splt[1] != "fx" )
    a += a_ensemble_member + "[_<temporal subset>].nc" ;

  if( a != f )
  {
    std::string key("46_7");
    if( notes->inq( key, fileStr))
    {
       std::string capt("filename does not match CMIP5 attributes.") ;

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

  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    if( varMeDa[i].var->isDATA )
    {
      getMIP_table(varMeDa[i]) ;
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
     std::string key("45_3");

     if( notes->inq( key, fileStr) )
     {
       std::string capt("variable name in filename does not match any variable in the file.") ;

       std::string text("Variable name in the filename: ");
       text += fVarname ;
       text += "\nCandidates in the file: ";
       for( size_t j=0 ; j < varMeDa.size()-1 ; ++j)
         text += varMeDa[j].name + ", ";
       text += varMeDa[varMeDa.size()-1].name ;

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
       }
     }
  }

  // Read or write the project table.
  ProjectTable projectTable(this, &in, tablePath, projectTableName);
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
QA::check_ProjectName(InFile &in)
{
  // get attributes
  std::string project_id( in.nc.getAttString("project_id") );

  if ( project_id.size() == 0 )
  {
    std::string key("46_1a");
    if( notes->inq( key, fileStr))
    {
      // only if project is CMIP5
      std::string capt( "missing project_id attribute." ) ;

      (void) notes->operate(capt) ;
      {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
      }

      return;
    }
  }
  else if ( project_id != "CMIP5" )
  {
    std::string key("46_1b");
    if( notes->inq( key, fileStr))
    {
      // only if project is CMIP5
      std::string capt( "failed project_id attribute." ) ;

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
QA::checkSigmaPressureCoordinates(VariableMetaData &vMD,
           std::vector<std::string> &vs)
{
  // special: variable 'lev' contains the added coefficients a and b
  // of the hybrid sigma pressure coordinates.
  // This quantity is meaningless.
  // However, it is good to test for the existance of the
  // 'a(k)' and 'b(k)' auxiliaries. At present, it seems that
  // the names a and b are commonly in use.

  size_t countCoeffs=0;
  for( size_t i=0 ; i < vs.size() ; ++i )
  {
    if( vs[i] == "a" )
      ++countCoeffs;
    if( vs[i] == "b" )
      ++countCoeffs;
  }

  // this comes late, because it has to be outside of the loop


  if( countCoeffs != 2 )
  {
    std::string key("58_6");

    if( notes->inq( key, vMD.name) )
    {
       std::string capt( "no variable a(k) or b(k) found for the hybrid sigma pressure coordinates.") ;

       std::string text("Variable: lev ");
       text += "\nExpecting auxiliaries a(k) and b(k) for the hybrid sigma pressure coordinates.";

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
    }
  }

  return;
}

bool
QA::checkStandardTable(InFile &in, VariableMetaData &vMD,
             std::vector<struct DimensionMetaData> &dimNcMeDa)
{
   // We have arrived here, because no project table
   // was defined, yet. Or no entry was found.
   // So, we scan through the standard table.

   // return true for the very special case that a tracer
   // variable was not found in MIP table Omon, because
   // it is defined in Oyr

   if(standardTable.size() == 0 )  // no standard table
      return false;

   std::string str0(tablePath);
   str0 += "/" + standardTable ;

   setTable( standardTable, "ST" );

//   std::fstream ifs(str0.c_str(), std::ios::in);
   // This class provides the feature of putting back an entire line
   ReadLine ifs(str0);

   if( ! ifs.isOpen() )
   {
      std::string key("41") ;

      if( notes->inq( key, vMD.name) )
      {
         std::string capt("could not open standard table.") ;

         std::string text("Could not open standard table: ") ;
         text += str0 ;

         (void) notes->operate(capt, text) ;
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

   // read headings from the standard table
   readHeadline(ifs, vMD, v_col, d_col, v_colMax, d_colMax);

   VariableMetaData tbl_entry(this);

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   // a helper vector
   std::vector<std::string> wa;

   // find the MIP table, corresponding to the 2nd part
   // in the variable name. The name of the MIP table is
   // present in the first column and begins with "CMOR Table".
   // The remainder is the name.
   // Unfortunately, the standard table is in a shape of a little
   // distance to being perfect.
   while( ! ifs.getLine(str0) )
   {
     // try to identify the name of the MIP table in str0
     // return true, if not found
     if( findStdTables(ifs, str0, vMD) )
       continue; // try next line

     // Now, test for a heading line that must be present for
     // each MIP table
     if( findNextVariableHeadline(ifs, str0, vMD, wa) )
       continue; // try next line

     // find entry for the requested variable
     if( findStandardEntry(ifs, str0, vMD, v_col, v_colMax, wa) )
       continue; // try next line

     // We have found an entry
     splt_line = str0;

     // This was tested in findStdTables()
     tbl_entry.stdTable=vMD.stdTable ;

     tbl_entry.priority = splt_line.toInt(v_col["priority"]);
     tbl_entry.name = splt_line[v_col["CMOR_variable_name"]];
     tbl_entry.name_alt = splt_line[v_col["output_variable_name"]];
     tbl_entry.standardName = splt_line[v_col["standard_name"]];
     tbl_entry.longName = splt_line[v_col["long_name"]];
     tbl_entry.longName
         = hdhC::clearInternalMultipleSpaces(tbl_entry.longName);
     tbl_entry.units = splt_line[v_col["unformatted_units"]];
     tbl_entry.units
         = hdhC::clearInternalMultipleSpaces(tbl_entry.units);
     if( tbl_entry.units.size() )
       tbl_entry.isUnitsDefined=true;
     else
       tbl_entry.isUnitsDefined=false;

     tbl_entry.cellMethods = splt_line[v_col["cell_methods"]];
     tbl_entry.cellMeasures = splt_line[v_col["cell_measures"]];
     tbl_entry.type = splt_line[v_col["type"]];
     tbl_entry.dims = splt_line[v_col["CMOR_dimensions"]];

     // special for items declared across MIP tables
     if( vMD.stdTableAlt == "cfSites" || vMD.stdTableAlt == "cf3hr" )
       tbl_entry.cellMethods="time: point";

     // netCDF properties are compared to those in the table.
     // Exit in case of any difference.
     checkVarTableEntry(vMD, tbl_entry);

     // get dimensions names from nc-file
     std::vector<std::string> vd( in.nc.getDimNames(vMD.name) );
     for(size_t l=0 ; l < vd.size() ; ++l)
     {
       // new instance
       dimNcMeDa.push_back( DimensionMetaData() );

       // the spot where meta-data of variables is taken
       getDimMetaData(in, vMD, dimNcMeDa.back(), vd[l]) ;
     }

     // check for the dimensions of the variable from the table.
     Split splt_dims(tbl_entry.dims) ;  // default separator is ' '

     for(size_t l=0 ; l < splt_dims.size() ; ++l)
       // check basic properties between the file and
       // requests in the table.
       checkDimStandardTable(ifs, in, vMD, dimNcMeDa,
          d_col, splt_dims[l], d_colMax );

     return false;
   }

   // there was no match, but we try alternatives

   // Is it one of those providing an alternative?.
   if( vMD.stdTable == "Omon" )
     return true;
   if( vMD.stdTable == "cf3hr" )
     return true;
   if( vMD.stdTable == "cfSites" )
     return true;

   if( vMD.stdTableAlt.size() )
   {
     // switch back to the original settings to get it right
     // for issuing the warning below.
     vMD.stdTable = "Omon" ;
     vMD.stdTableAlt = "Oyr" ;
   }

    std::string key("44") ;

    if( notes->inq( key, vMD.name) )
    {
      std::string capt("variable ") ;
      capt += vMD.name ;
      capt += " not found in the standard table.";

      std::ostringstream ostr(std::ios::app);
      ostr << "Standard table: " << standardTable  ;
      ostr << "\nMIP table: " << vMD.stdTable;
      ostr << "\nVariable " << vMD.name;
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
QA::checkStandardTableDimBounds(InFile &in, Split &splt_line,
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

   std::string captsIntro("STD-T=");
   captsIntro += vMD.stdTable + ", var=";
   captsIntro += vMD.name + ", dim=";
   captsIntro += dimTE.outname + ": ";

   // check that the table has an entry for the 'bounds?'-column
   // Note: table contains "yes" or "no". No real name

   if( !( splt_line[col["bounds?"]] == "yes"
            || splt_line[col["bounds?"]] == "no" ) )
   {
       std::string key("51");

       if( notes->inq( key, vMD.name) )
       {
         std::ostringstream ostr(std::ios::app);

         std::string capt( captsIntro) ;
         capt += "missing value in MIP table 'dims' in column 'bounds?'" ;

         ostr << "Standard table: " << standardTable;
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
    std::string key("47_11");

    if( notes->inq( key, vMD.name) )
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
     std::string key("47_12");

     if( notes->inq( key, vMD.name) )
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
QA::checkStandardTableDimValues(InFile &in, Split &splt_line,
    VariableMetaData &vMD,
    struct DimensionMetaData &dimFE,
    struct DimensionMetaData &dimTE,
    std::map<std::string, size_t> &col)
{
  // check dimensional values of limited dimensions
  if( in.nc.isDimUnlimited(dimFE.outname) )
    return;

  std::string captsIntro("STD-T=");
  captsIntro += vMD.stdTable + ", var=";
  captsIntro += vMD.name + ", dim=";
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
       std::string key("47_11");

       if( notes->inq( key, vMD.name) )
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
QA::checkTables(InFile &in, VariableMetaData &vMD)
{
  // Matching the ncfile inherent meta-data against a pre-defined
  // so-called standard table ensures conformance.

  // Meta data in data-member struct varMeDa have been extracted
  // from the current ncFile. Similar for dimMeDa for the
  // associated dimensions will be done when necessary.

  std::vector<struct DimensionMetaData> dimNcMeDa;

  // Scan through the standard table.
  // "Any" indicates that there was no valid MIP table
  if( vMD.stdTable != "Any" && ! vMD.var->isExcluded )
  {
    bool is;
    if( checkStandardTable(in, vMD, dimNcMeDa) )
    {
      // very special: a tracer variable was not found
      // in MIP table Omon, because it is defined in Oyr
      if( vMD.stdTable == "Omon" )
      {
        vMD.stdTableAlt = "Omon" ;
        vMD.stdTable = "Oyr" ;

       is = checkStandardTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.stdTable = "Omon" ;
        vMD.stdSubTable =  "Marine Bioge" ;
        vMD.stdTableAlt = "Oyr" ;
      }
      else if( vMD.stdTable == "cf3hr" )
      {
        vMD.stdTableAlt = "cf3hr" ;
        vMD.stdTable = "Amon" ;
        std::string saveCellMethods(vMD.cellMethods);
        vMD.cellMethods="time: point";

        is = checkStandardTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.stdTable = "cf3hr" ;
        vMD.stdSubTable.clear() ;
        vMD.stdTableAlt = "Amon" ;

        if( ! is )
          vMD.cellMethods=saveCellMethods;
      }
      else if( vMD.stdTable == "cfSites" )
      {
        vMD.stdTableAlt = "cfSites" ;
        vMD.stdTable = "Amon" ;
        std::string saveCellMethods(vMD.cellMethods);
        vMD.cellMethods="time: point";

        is = checkStandardTable(in, vMD, dimNcMeDa) ;

        // switch back to the original table required for the project table entry
        vMD.stdTable = "cfSites" ;
        vMD.stdSubTable =  "CFMIP 3-ho" ;
        vMD.stdTableAlt = "Amon" ;

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
  if( vMD.var->isExcluded && vMD.stdSubTable.size() == 0 )
  {
     for(size_t i=0 ; i < varMeDa.size() ; ++i )
     {
       if( varMeDa[i].stdSubTable.size() )
       {
         vMD.stdSubTable = varMeDa[i].stdSubTable;
         break;
       }
     }
  }

  return;
}

void
QA::checkVarTableEntry(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  // Do the variable properties found in the netCDF file
  // match those in the table (standard or project)?
  // Exceptions: priority is not defined in the netCDF file.
  std::string key_base;
  std::string key;
  std::string t0;

  std::string captsIntro("STD-T=");
  if( vMD.stdTableAlt.size() )
    captsIntro += vMD.stdTableAlt + ", var=";
  else
    captsIntro += vMD.stdTable + ", var=";
  captsIntro += vMD.name + ": ";

  // This has already been tested in the calling method
  // if( tbl_entry.name != varMeDa.name )

  bool is=true;

  if( tbl_entry.standardName != vMD.standardName )
  {
    // this takes into account an error of the standard table
    if( vMD.name == "tos" && vMD.stdTable == "day" )
    {
       if( vMD.standardName == "sea_surface_temperature" )
         is=false;
    }

    key_base = "58_1";
    key = key_base;
    if( is && notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "standard name conflict." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Variable conflict between file and table.";
      ostr << "\nTable: " << currTable  ;
      ostr << ", MIP table: " << vMD.stdTable;
      ostr << "\nVariable: ";
      ostr << vMD.name;

      ostr << "        standard name:\t table: " ;
      if( tbl_entry.standardName.size() )
        ostr << tbl_entry.standardName ;
      else
        ostr << notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.standardName.size() )
        ostr << vMD.standardName ;
      else
        ostr << notAvailable ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  is=false;

  if( tbl_entry.longName != vMD.longName )
  {
    is=true;

    // special consideration for tracers in MIP table Omon
    if( vMD.stdTable == "Oyr" && vMD.stdTableAlt == "Omon" )
    {
      // the name from Oyr is contained somehow in Omon, but
      // without addition like 'at Surface' or 'Surface ...'
      std::string t(tbl_entry.longName);
      t += " at surface";
      if( t == vMD.longName )
        // I think this is due to a misspelling in the Omon MIP table
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
      key_base = "58_2";
      key = key_base;

      if( notes->inq( key, vMD.name) )
      {
        std::string capt( getCurrentTableSubst() );
        capt += captsIntro;
        capt += "long name conflict." ;


        std::ostringstream ostr(std::ios::app);
        ostr << "Variable conflict between file and table.";
        ostr << "\nTable: " << currTable  ;
        ostr << ", MIP table: " << vMD.stdTable;
        ostr << ", Variable: "  << vMD.name;
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

  if( tbl_entry.units != vMD.units )
  {
    key_base = "58_3c";
    key = key_base;
    is=false;

    if( vMD.units.size() == 0 )
    {
      // units must be specified; but, unit= is ok
      if( tbl_entry.units != "1" && ! vMD.isUnitsDefined )
      {
         if( notes->inq( key, vMD.name))
         {  // really empty
           std::string capt( getCurrentTableSubst() );
           capt += captsIntro;
           capt += "variable has no units attribute." ;

           std::ostringstream ostr(std::ios::app);
           ostr << "\nTable: " << currTable  ;
           ostr << "\nMIP table: " << vMD.stdTable;
           ostr << "\nVariable: ";
           ostr << vMD.name;
           ostr << "\nMissing units attribute" ;
           ostr << ", table requires " ;
           ostr << tbl_entry.units;

           (void) notes->operate(capt, ostr.str()) ;
           {
              notes->setCheckMetaStr(fail);
              setExit( notes->getExitValue() ) ;
           }
        }
      }
    }
    else if( notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro ;
      capt += "Conflict for the units attribute." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << "\nMIP table: " << vMD.stdTable;
      ostr << "\nVariable: ";
      ostr << vMD.name;
      ostr << "\nConflict between file and table.";
      ostr << "                units:\t table: " ;
      if( tbl_entry.units.size() )
        ostr << tbl_entry.units ;
      else
        ostr << notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.isUnitsDefined )
      {
        if( vMD.units.size() )
          ostr << vMD.units ;
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
       key_base = "58_4";
       key = key_base;

       if( notes->inq( key, vMD.name) )
       {
         std::string capt( getCurrentTableSubst() );
         capt += captsIntro;
         capt += "cell-methods conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " << currTable  ;
         ostr << "\nMIP table: " << vMD.stdTable;
         ostr << "\nVariable: ";
         ostr << vMD.name;
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

    key_base = "58_7";
    key = key_base;
    if( is )
    {
       if( notes->inq( key, vMD.name) )
       {
         std::string capt( getCurrentTableSubst() );
         capt += captsIntro;
         capt += "cell-measures conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " << currTable  ;
         ostr << ", MIP table: " << vMD.stdTable;
         ostr << "\nVariable: ";
         ostr << vMD.name;
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
      tbl_entry.type == "real"
         || tbl_entry.type == "float"
             || tbl_entry.type == "double" ;
  bool isNcTypeReal =
      vMD.type == "real"
         || vMD.type == "float"
              || vMD.type == "double" ;

  if( currTable == standardTable
       && tbl_entry.type.size() == 0 && vMD.type.size() != 0 )
  {
    key_base = "58_9";
    key = key_base;
    if( notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "type check discarded, not specified in the MIP Table." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << ", MIP table: " << vMD.stdTable;
      ostr << "\nVariable: ";
      ostr << vMD.name;
      ostr << "\nConflict between file and table.";
      ostr << "\n                 type:\t table: " ;
      ostr << "not specified in the MIP Table" ;
      ostr << "\n                      \tncfile: " ;
      ostr << vMD.type ;

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
    key_base = "58_5";
    key = key_base;
    if( notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "type conflict." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " << currTable  ;
      ostr << "\nMIP table: " << vMD.stdTable;
      ostr << "\nVariable: ";
      ostr << vMD.name;
      ostr << "\nConflict between file and table.";
      ostr << "\n                 type:\t table: " ;
      if( tbl_entry.type.size() )
        ostr << tbl_entry.type ;
      else
        ostr << notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.type.size() )
        ostr << vMD.type ;
      else
        ostr << notAvailable ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(fail);
        setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( tbl_entry.standardName != vMD.standardName )
  {
    key_base = "58_10";
    key = key_base;
    if( notes->inq( key, vMD.name) )
    {
      std::string capt( getCurrentTableSubst() ) ;
      capt += captsIntro;
      capt += "variable name matches only for case-insensitivity." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Variable conflict between file and table.";
      ostr << "\nTable: " << currTable  ;
      ostr << ", MIP table: " << vMD.stdTable;
      ostr << "\nVariable: ";
      ostr << vMD.name;

      ostr << "        variable name:\t table: " ;
      ostr << tbl_entry.name ;

      ostr << "\n                      \tncfile: " ;
      ostr << vMD.name ;

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

   ++currQcRec;

   return;
}

void
QA::createVarMetaData(void)
{
  // set corresponding isExcluded=true
  pIn->excludeVars();

  // take time and other info from inFile
  std::string tU;  // units of time
  std::string str; // temporarily

  // time increment
  std::string tInc;

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

    Split splt(vMD.dims);
    int effDim = splt.size() ;
    for( size_t j=0 ; j < splt.size() ; ++j )
      if( splt[j] == qaTime.time )
        --effDim;

    if( replicationOpts.size() )
    {
      if( ReplicatedRecord::isSelected(
             replicationOpts, vMD.name, enablePostProc, effDim ) )
      {
        vMD.qaData.replicated = new ReplicatedRecord(this, i, vMD.name);
        vMD.qaData.replicated->setAnnotation(notes);
        vMD.qaData.replicated->parseOption(replicationOpts) ;
      }
    }

    if( outlierOpts.size() )
    {
      if( Outlier::isSelected(
             outlierOpts, vMD.name, enablePostProc, effDim ) )
      {
        vMD.qaData.outlier = new Outlier(this, i, vMD.name);
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
QA::finally(int eCode)
{
  if( nc )
    setExit( finally_data(eCode) );

  // distinguish from a sytem crash (segmentation error)
//  notes->print() ;
  std::cout << "STATUS-BEG" << eCode << "STATUS-END";
  std::cout << std::flush;

  setExit( exitCode ) ;

  return exitCode ;
}

int
QA::finally_data(int eCode)
{
  setExit(eCode);

  // write pending results to qa-file.nc. Modes are considered there
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
    setExit( varMeDa[i].finally() );

  // post processing, but not for conditions which indicate
  // incomplete checking.
  if( exitCode < 3 )
  {  // 3 or 4 interrupted any checking
    if( enablePostProc )
      if( postProc() )
        if( exitCode == 63 )
            exitCode=0;  // this is considered a change
  }
  else
  {
    // outlier test based on slope across n*sigma
    // must follow flushOutput(), if the latter is effective
    if( isCheckData )
      for( size_t i=0 ; i < varMeDa.size() ; ++i )
         if( varMeDa[i].qaData.enableOutlierTest )
           varMeDa[i].qaData.outlier->test( &(varMeDa[i].qaData) );
  }

  if( exitCode == 63 ||
     ( nc == 0 && exitCode ) || (currQcRec == 0 && pIn->isTime ) )
  { // qa is up-to-date or a forced exit right from the start;
    // qa_>filename>.nc remains the same
    nc->close();

    if( exitCode == 63 )
      exitCode=0 ;

    isExit=true;
    return exitCode ;
  }

  if( exitCode != 63 )
    qaTime.finally( nc );

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

  nc->close();

  return exitCode ;
}

bool
QA::findNextVariableHeadline(ReadLine &ifs, std::string &str0,
   VariableMetaData &vMD, std::vector<std::string> &wa )
{
   // scan the MIP table.
   // Return true if no valid header line is found

   do
   {
     if( str0.substr(0,10) == "CMOR Table" )
     {
       ifs.putBackLine(); //try again for this CMOR Table
       return true;  // found the begin of another MIP table
     }

     if( str0.substr(0,13) == "In CMOR Table" )
       findStdSubTables(str0, vMD, wa) ;

     // find the heading
     if( str0.substr(0,8) == "priority" )
       break;
   } while( ! ifs.getLine(str0) ) ;


   if( ifs.eof() )
   {
     std::string key("42");

     if( notes->inq( key, vMD.name) )
     {
       // we detected a MIP table that is not part of the standard table
       std::string capt("MIP table name not found in the standard table.") ;

       std::string text("MIP table: ");
       text += vMD.stdTable;
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
QA::findStandardEntry(ReadLine &ifs, std::string &str0,
   VariableMetaData &vMD,
   std::map<std::string, size_t> &col, size_t col_max,
   std::vector<std::string> &wa)
{
   // return true: entry is not the one we look for.

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   std::string s_e(vMD.name);

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

       return true;  // found the begin of another MIP table
     }

     // Get the string for the MIP-sub-table. This is used in
     // conjunction to the time table (parseTimeTable()).
     // Work-around when there are identical MIP sub-tables
     // after the truncation.
     if( str0.substr(0,13) == "In CMOR Table" )
       findStdSubTables(str0, vMD, wa) ;

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

     if( notes->inq( key, vMD.name) )
     {
        std::string capt("missing column(s) in the standard table.") ;

        std::ostringstream ostr(std::ios::app);
        ostr << "Standard table: " << standardTable ;
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
QA::findStdTables(ReadLine &ifs, std::string &str0,
  VariableMetaData &vMD)
{
   // return true, if str0 contains no MIP table name

   size_t pos;
   do
   {
     if( str0.substr(0,10) == "CMOR Table" )
       break;
   } while( ! ifs.getLine(str0) ) ;

   if( ifs.eof() )
     return true;  // could be unknown MIP table or unknown variable

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

   if( str0.size() > 0 && str0 == vMD.stdTable )
   {
     // we have to read the next line.
     ifs.getLine(str0);
     return false;  // sub table found
   }

   return true;  // not found; try next
}

void
QA::findStdSubTables(std::string &str0,
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
   vMD.stdSubTable=tmp;

   return;
}

std::string
QA::getCurrentTableSubst(void)
{
   std::string t(": ");

   if( currTable == standardTable )
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
     if( in.nc.getAttString("coordinates", vMD.name ).size() == 0 )
       return ;  // nothing was found

     dName = in.nc.getAttString("coordinates", vMD.name ) ;
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

  if( dName == "time" )
  {
    // exclude time from size
    dimMeDa.size = 0;

    if( ! dimMeDa.isUnitsDefined )
    {
      std::string key("45_1");

      if( notes->inq( key, vMD.name) )
      {
        std::string capt("variable time has no unit attribute.") ;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
      }
    }
  }

  return ;
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
  std::string f( pIn->filename );
  size_t pos;
  if( (pos=f.rfind('/')) < std::string::npos )
    f = f.substr(pos+1);

  if( f.rfind(".nc" ) )
    f = f.substr( 0, f.size()-3 );  // strip ".nc"

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
QA::getGA_MIP_table(std::vector<std::string> &sTable)
{
  // the counter-parts in the attributes
  std::string ga_MIP_table( pIn->nc.getAttString("table_id") );

  if( ga_MIP_table.size() == 0 )
    return ga_MIP_table;

  Split spltMT(ga_MIP_table);

  // The MIP table name from the global attributes.
  // Ignore specific variations
  if( spltMT.size() > 1 )
  {
    if(  spltMT[0].substr(1,4) == "able"
        || spltMT[0].substr(1,4) == "ABLE" )
    ga_MIP_table = spltMT[1] ;
  }
  else if( spltMT.size() > 0 )
    ga_MIP_table = spltMT[0] ;

  //check for valid names
  bool is=true;
  for( size_t i=0 ; i < sTable.size() ; ++i )
  {
    if( sTable[i] == ga_MIP_table )
    {
      is=false ;
      break;
    }
  }

  if( is )
  {
     std::string key("46_8");

     if( notes->inq( key, varMeDa[0].name) )
     {
       std::string capt("invalid MIP table name in CMIP5 attributes.") ;

       std::string text("MIP Table (Filename): ") ;
       text +=  ga_MIP_table;

       ga_MIP_table.clear();

       (void) notes->operate(capt, text) ;
       {
         setExit( notes->getExitValue() ) ;
       }
     }
  }

  return ga_MIP_table;
}

void
QA::getMIP_table(VariableMetaData &vMD)
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

  std::string tTable( getGA_MIP_table(sTable) ) ;

  // compare filename to netCDF global attributes
  checkFilename( sTable, tTable );

  // Note: filename:= name_CMOR-MIP table_... .nc
  Split splt(hdhC::getBasename(filename), "_");

  std::string fTable;
  if( splt.size() > 1 )
    fTable = splt[1] ;

  // MIP table name from global attributes has been checked

  // finalise the MIP table check
  if( fTable != tTable )
  {
    if( tTable.size() )
      vMD.stdTable = tTable; // precedence: global attribute
    else if( ! fTable.size() )
      // no valid MIP table found
      vMD.stdTable ="Any";  // MIP table name in the project table

    std::string key("46_9");

    if( notes->inq( key, varMeDa[0].name) )
    {
       std::string capt("Diverging MIP table name in attribute and filename.") ;

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
  std::string intro("STD-T=");
  intro += vMD.stdTable + ", var=";
  intro += vMD.name + ", dim=";
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

void
QA::getVarnameFromFilename(std::string &fName)
{
  size_t pos;
  if( (pos = fName.rfind('/')) < std::string::npos )
    fVarname =fName.substr(pos+1);
  if( (pos = fVarname.find("_")) < std::string::npos )
    fVarname = fVarname.substr(0,pos) ;

  return;
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
   // Initialise time testing,
   // time boundary testing, and cycles within a time step.
   // Eventually, entry() is called to test the data of fields.
   // Initial values are set such that they do not cause any
   // harm in testDate() called in closeEntry().

   notes->init();  // safe

   setFilename(pIn->filename);
   getVarnameFromFilename(pIn->filename);

   // apply parsed command-line args
   applyOptions();

   // Create and set VarMetaData objects.
   createVarMetaData() ;

   // check existance of any data at all
   if( (isCheckTime || isCheckData )
            && pIn->ncRecBeg == 0 && pIn->ncRecEnd == 0 )
   {
      isCheckData=false;
      isCheckTime=false;

      std::string key("73");
      if( notes->inq( key, fileStr) )
      {
        std::string capt("empty data section in the file.") ;

        std::string text("Is there an empty data section in the file? Please, check.");
        notes->setCheckTimeStr(fail);
        notes->setCheckDataStr(fail);

        (void) notes->operate(capt, text) ;
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

   if( isCheckTime && pIn->isTime )
   {
     // init the time object
     // note that freq is compared to the first column of the time table
     qaTime.init(pIn, notes, this);
     qaTime.applyOptions(optStr);
     qaTime.initTimeTable( getFrequency() );

     // note that this test is not part of the QA_Time class, because
     // coding depends on projects
     if( testPeriod() )
     {
        std::string key("82");
        if( notes->inq( key, qaTime.name) )
        {
          std::string capt("status is apparently in progress");

          (void) notes->operate(capt) ;
       }
     }
   }

   if( isExit )
     return true;

   // open netCDF for continuation and resuming a session
   openQcNc(*pIn);

   if( isExit || isNoProgress )
   {
     isCheckData=false;
     isCheckTime=false;
     return true;
   }

   // This is for the case of processing several layers per record.
   // The order must be kept: at first, the time stuff and then
   // the field testing.

   if( isCheckTime )
   {
     if( ! pIn->nc.isAnyRecord() )
     {
       isCheckTime = false;
       notes->setCheckTimeStr(fail);
     }
     else if( ! pIn->isTime )
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
     if( isExit || is )
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
    qaTime.timeOutputBuffer.initBuffer(nc, currQcRec);
    qaTime.sharedRecordFlag.initBuffer(nc, currQcRec);
  }

  if( isCheckData )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i)
      varMeDa[i].qaData.initBuffer(nc, currQcRec);
  }

  return;
}

void
QA::initDefaults(void)
{
  objName="QA";

  nc=0;
  notes=0;

  fail="FAIL";
  notAvailable="not available";
  fileStr="file";

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
  currQcRec=0;

  // by default
  tablePath="./";

  // pre-set check-modes: all are used by default
  isCheckMeta=true;
  isCheckTime=true;
  isCheckData=true;

  exitCode=0;

#ifdef SVN_VERSION
  // -1 by default
  svnVersion=hdhC::double2String(static_cast<int>(SVN_VERSION));
#else
  svnVersion="-1";
#endif

  // set pointer to member function init()
  execPtr = &IObj::init ;
}

void
QA::initGlobalAtts(InFile &in)
{
  // global atts at creation.
  std::string today( Date::getCurrentDate() );

  nc->setGlobalAtt( "project", "CMIP5");
  nc->setGlobalAtt( "product", "quality check of CMIP5 data set");

//  enableVersionInHistory=false;
  nc->setGlobalAtt( "QA_svn_revision", svnVersion);
  nc->setGlobalAtt( "contact", "hollweg@dkrz.de");

  std::string t("csv formatted ");
  t += standardTable.substr( standardTable.rfind('/')+1 ) ;
  t = t.substr(0,t.size()-3) + "xlsx" ;
  nc->setGlobalAtt( "standard_table", t);

  nc->setGlobalAtt( "creation_date", today);

  // helper vector
  std::vector<std::string> vs;

  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    vs.push_back( varMeDa[m].name );

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
  std::vector<std::string> vss( nc->getVarNames() ) ;

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
      if( prevTargets[i] == varMeDa[j].name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("60a");
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
      if( prevTargets[i] == varMeDa[j].name )
        break;

    if( i == prevTargets.size() )
    {
       std::string key("60b");
       if( notes->inq( key, fileStr) )
       {
         std::string capt("variable=");
         capt += varMeDa[j].name + " is new in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( fail );
           setExit( notes->getExitValue() ) ;
         }
       }
    }
  }

  // now, resume
  qaTime.timeOutputBuffer.setNextFlushBeg(currQcRec);
  qaTime.setNextFlushBeg(currQcRec);

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
  // check tablePath; must exist
  std::string testFile="/bin/bash -c \'test -d " ;
  testFile += tablePath ;
  testFile += '\'' ;

  // see 'man system' for the return value, here we expect 0,
  // if directory exists.

  if( system( testFile.c_str()) )
  {
     std::string key("57");

     if( notes->inq( key, fileStr) )
     {
        std::string capt("no path to the tables.") ;

        std::string text("No path to the tables; tried: ");
        text += tablePath + ".";
        text += "\nPlease, check the setting in the configuration file.";

        (void) notes->operate(capt, text) ;
        {
          notes->setCheckMetaStr(fail);
          setExit( notes->getExitValue() ) ;
        }
     }
  }

  // tables names usage: both project and standard tables
  // reside in the same path.
  // Naming of the project table:
  if( projectTableName.size() == 0 )
  {
    if( standardTable.size() == 0 )
      projectTableName = "project_table.csv";
    else
    projectTableName = "pt_" + standardTable;
  }
  else if( projectTableName.find(".csv") == std::string::npos )
    projectTableName += ".csv" ;

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
QA::openQcNc(InFile &in)
{
  // Generates a new nc file for QA results or
  // opens an existing one for appending data.
  // Copies time variable from input-nc file.

  // name of the file begins with qa_
  if ( qaFilename.size() == 0 )
  {
    // use the input filename as basis;
    // there could be a leading path
    qaFilename = hdhC::getPath(filename);
    if( qaFilename.size() > 0 )
      qaFilename += '/' ;
    qaFilename += "qa_";
    qaFilename += hdhC::getBasename(filename);
    qaFilename += ".txt";
  }

  nc = new NcAPI;
  if( notes )
    nc->setNotes(notes);

  // don't create a netCDF file, when only meta data are checked.
  // but, a NcAPI object m ust exist
  if( ! isCheckTime )
    return;

  if( nc->open(qaFilename, "NC_WRITE", false) )
//   if( isQA_open ) // false: do not exit in case of error
  {
    // continue a previous session
    importedRecFromPrevQA=nc->getNumOfRecords();
    currQcRec += importedRecFromPrevQA;
    if( currQcRec )
      isNotFirstRecord = true;

    initDataOutputBuffer();
    qaTime.sharedRecordFlag.initBuffer(nc, currQcRec);

    initResumeSession();
    isResumeSession=true;

    // if files are synchronised, i.e. a file hasn't changed since
    // the last qa, this will exit in member finally(6?)
    if( isCheckTime )
      isNoProgress = qaTime.sync( isCheckData, enablePostProc );

    return;
  }

  if( currQcRec == 0 && in.nc.getNumOfRecords() == 1 )
    qaTime.isSingleTimeValue = true;

  // So, we have to generate a netCDF file from almost scratch;
  std::string str; // temporarily

  // open new netcdf file
  // open new netcdf file
  if( qaNcfileFlags.size() )
    nc->create(qaFilename,  qaNcfileFlags);
  else
    nc->create(qaFilename,  "Replace");

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

    qaTime.openQcNcContrib(nc);
  }
  else if( isCheckTime )
  {
    // dimensions
    qaTime.name="fixed";
    nc->defineDim("fixed", 1);
  }

  // create variable for the data statics etc.
  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    varMeDa[m].qaData.openQcNcContrib(nc, varMeDa[m].var);

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
       vars.push_back( vMD.name + "_min" );
       vars.push_back( vMD.name + "_max" );
       vars.push_back( vMD.name + "_ave" );
       vars.push_back( vMD.name + "_std_dev" );

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
   // find the capts for MIP tables dims and for the variables

   std::string str0;

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   // This should give the heading of the MIP table for the dimensions.
   // The MIP table is identified by the first column.
   while( !ifs.getLine(str0) )
   {
     if( str0.substr(0,13) == "CMOR table(s)" )
       break ;
   }


   if( ifs.eof() )
   {
      std::string key("49");

      if( notes->inq( key, varMeDa[0].name) )
      {
        // no dim sheet of the standard  table
        std::string capt("MIP table for dimensions not found in the standard table.") ;

        std::string text("Standard table: ") ;
        text +=  standardTable ;
        text += "\nDid not find the sheet for dimensions." ;

        (void) notes->operate(capt, text) ;
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
QA::setTable(std::string t, std::string acronym)
{
  // is is possible that this method is called from a spot,
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
  std::string f( filename.substr(0, filename.size()-3 ) );

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

  // convert ascii formated date to class Date
  qaTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("97");

     if( notes->inq( key, fileStr) )
     {
       std::string capt("invalid time-stamp in filename.");

       (void) notes->operate(capt) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
     }

     return false;
  }

  if( qaTime.isTimeBounds)
  {
    period.push_back( qaTime.getDate("first", "left") );
    period.push_back( qaTime.getDate("last", "right") );
  }
  else
  {
    period.push_back( qaTime.getDate("first") );
    period.push_back( qaTime.getDate("last") );
  }

  if( period[2] != period[0] )
  {
     std::string key("95");

     if( notes->inq( key, fileStr) )
     {
       std::string capt("filename time-stamp (1st date): misaligned to time values.");

       std::string text("from time data=");
       text += period[2].getDate();
       text += "\nfilename=" ;
       text += period[0].getDate() ;

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
     }
  }

  if( isFileComplete && period[3] != period[1] )
  {
     std::string key("96");

     if( notes->inq( key, fileStr) )
     {
       std::string capt("filename time-stamp (2nd date): misaligned to time values.");

       std::string text;
       if( period[3] > period[1] )
         text = "Time values exceed period in filename." ;
       else
         text = "Period in filename exceeds time values." ;

       text += "\ndata=" ;
       text += period[3].getDate() ;
       text += "\nfilename=" ;
       text += period[1].getDate();

       (void) notes->operate(capt, text) ;
       {
         notes->setCheckMetaStr(fail);
         setExit( notes->getExitValue() ) ;
       }
     }

     return false;
  }

  // complete
  return false;
}

void
QA::setVarMetaData(VariableMetaData &vMD)
{
  // collect some properties in struct varMeDa.

    // take time and other info from inFile
  std::string tU;  // units of time
  std::string str; // temporarily

  // time increment
  std::string tInc;
  std::string lat_name;
  std::string lon_name;

  vMD.name = vMD.var->name;

  // is toggled, if not defined
  vMD.isUnitsDefined=true;
  vMD.units = pIn->nc.getAttString("units",
              vMD.name, vMD.isUnitsDefined);
  vMD.units
     = hdhC::clearInternalMultipleSpaces(vMD.units);

  // Note: check of units is post-poned

  // get the name of the unlimited dimensions (assumption: only one)
  if( vMD.var->isUnlimited() )
    vMD.unlimitedDim=pIn->nc.getUnlimitedDimName();

  // get original dimensions and convert names into a string
  std::vector<std::string>
      vs2( pIn->nc.getDimNames(vMD.name) );

  vMD.dims += vs2[0] ;
  for( size_t k=1; k < vs2.size() ; ++k)
  {
    vMD.dims +=  " ";
    vMD.dims +=  vs2[k] ;

    // Test if variable(s) are defined on a lat-lon-field.
    // If not, then the data body represents something different;
    // presumably cross-sections across oceanic basins.
    // Unchecked assumption: all variables have identical grid-layout.
    // Part I
    if( vs2[k] == "lon" || vs2[k] == "longitude" )
      lon_name=vMD.name;
    else if( vs2[k] == "lat" || vs2[k] == "latitude" )
      lat_name=vs2[k];
  }

  // some more properties
  vMD.standardName = pIn->nc.getAttString("standard_name", vMD.name);
  vMD.longName = pIn->nc.getAttString("long_name", vMD.name);
  vMD.cellMethods = pIn->nc.getAttString("cell_methods", vMD.name);
  vMD.cellMeasures = pIn->nc.getAttString("cell_measures", vMD.name);

  vMD.type= pIn->nc.getVarTypeStr(vMD.name);

  return;
}

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;
   var = v;

   if( v )
     name = v->name;

   isForkedAnnotation=false;
}

VariableMetaData::~VariableMetaData()
{
  dataOutputBuffer.clear();
}

int
VariableMetaData::finally(int eCode)
{
  // write pending results to qa-file.nc. Modes are considered there
  qaData.flush();

  // annotation obj forked by the parent VMD
  notes->printFlags();

  int rV = notes->getExitValue();
  eCode = ( eCode > rV ) ? eCode : rV ;

  return eCode ;
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
