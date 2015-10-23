//#include "qa.h"

DRS_Filename::DRS_Filename(QA* p, std::vector<std::string>& optStr)
{
  pQA = p;
  notes = pQA->notes;

  enabledCompletenessCheck=true;

  applyOptions(optStr);
}

void
DRS_Filename::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "dIP"
          || split[0] == "dataInProduction"
              || split[0] == "data_in_production" )
     {
        // effects completeness test in testPeriod()
        enabledCompletenessCheck=false;
        continue;
     }

     if( split[0] == "tGCM"
           || split[0] == "table_GCM_NAME" )
     {
       if( split.size() == 2 )
          GCM_ModelnameTable.setFile(split[1]) ;

       continue;
     }

     if( split[0] == "tRCM"
           || split[0] == "table_RCM_NAME" )
     {
       if( split.size() == 2 )
          RCM_ModelnameTable.setFile(split[1]) ;

       continue;
     }
   }

   // apply a general path which could have also been provided by setTablePath()
   if( GCM_ModelnameTable.path.size() == 0 )
      GCM_ModelnameTable.setPath(pQA->tablePath);

   if( RCM_ModelnameTable.path.size() == 0 )
      RCM_ModelnameTable.setPath(pQA->tablePath);

   return;
}

void
DRS_Filename::check(InFile &in)
{
  // names of the global attributes in a sequence corresponding to
  // the path components from right to left.
  std::vector<std::string> a_name;
//  a_name.push_back("project_id");
//  a_name.push_back("product");
  a_name.push_back("CORDEX_domain");
  a_name.push_back("institute_id");
  a_name.push_back("driving_model_id");
  a_name.push_back("driving_experiment_name");
  a_name.push_back("driving_model_ensemble_member");
  a_name.push_back("model_id");
  a_name.push_back("rcm_version_id");
  a_name.push_back("frequency");
  a_name.push_back("VariableName");

  // Get global att values. Note that vName is not an attribute.
  // Note that the existance was tested elsewhere.

  // Complication: attribute driving_experiment could contain
  // three attributes; it is not clear whether these have to be
  // stated also on their own. Thus, they are substituted when missing.
  std::string dr_exp( in.nc.getAttString("driving_experiment") );
  Split x_dr_exp(dr_exp, ",");

  std::vector<std::string> a_value;

  for( size_t i=0 ; i < a_name.size()-1 ; ++i )
  {
    a_value.push_back( in.nc.getAttString( a_name[i] ) );

    if( a_value[i].size() == 0 )
    {
      if( a_name[i] == "driving_model_ensemble_member"
             && x_dr_exp.size() > 2 )
        a_value[i] = x_dr_exp[2] ;
      else if( a_name[i] == "driving_experiment_name"
             && x_dr_exp.size() > 1 )
        a_value[i] = x_dr_exp[1] ;
      else if( a_name[i] == "driving_model_id" && x_dr_exp.size() )
        a_value[i] = x_dr_exp[0] ;
    }
  }
  a_value.push_back(pQA->qaExp.getVarnameFromFilename());

  // names of the path components
  std::vector<std::string> p_name;

//  p_name.push_back("activity") ;
//  p_name.push_back("product") ;
  p_name.push_back("Domain") ;
  p_name.push_back("Institute") ;
  p_name.push_back("GCMModelName") ;
  p_name.push_back("CMIP5ExperimentName") ;
  p_name.push_back("CMIP5EnsembleMember") ;
  p_name.push_back("RCMModelName") ;
  p_name.push_back("RCMVersionID");
  p_name.push_back("Frequency");
  p_name.push_back("VariableName");

  // preset vector
  std::vector<std::string> p_value;
  for( size_t i=0 ; i < p_name.size() ; ++i)
    p_value.push_back("");

  // check validity of particular components
  if( a_value[2].size() )
    checkModelName(in, a_name[2], a_value[2], 'G') ;
  if( a_value[5].size() )
    checkModelName(in, a_name[5], a_value[5], 'R', a_name[1], a_value[1]) ;

  // components of the path
  Split p_items(in.file.path,"/");

  // check for identical sequences
  std::string text;

  // the very first thing on the right side of the path components
  // might be a kind of version-item not defined in the Cordex Archive Design,
  // thus the variable name is found in the last or the one before.
  // The algorithm below works around this.
  int p_ix=p_items.size()-1;
  while( p_ix && p_items[p_ix] != pQA->qaExp.getVarnameFromFilename() )
    --p_ix ;

  int a_ix=p_name.size()-1;
  int i,j;

  for( i=p_ix, j=a_ix ; j > -1 && i > -1 ; --i, --j )
  {
     bool is = a_value[j] != p_items[i];

     // space for exceptions
     if( is )
     {
       if( p_name[j] == "CMIP5EnsembleMember" )
       {
          // not considered in the documents
          if( a_value[j] == "r0i0p0" || p_items[i] == "r0i0p0")
            is=false;
       }
     }

     if( is )
     {
        text += "Expected ";
        text += p_name[j] + "=";
        text += a_value[j];
        text += ", found ";
        text += p_items[i];

        text  += "\nDRS(found)=...";
        for( i=p_ix-a_ix ; i <= p_ix ; ++i )
        {
           text += "/";
           text += p_items[i] ;
        }

        text  += "\nDRS(by global atts)=...";
        for( j=0 ; j <= a_ix ; ++j )
        {
           text += "/";
           text += a_value[j] ;
        }

        break;
     }
  }

  if( text.size() )
  {
    std::string key("10_1");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt( "directory structure does not match global attributes");

      (void) notes->operate(capt, text) ;
      notes->setCheckMetaStr( pQA->fail );
    }
  }
  else
    return ;  // passed the test

  // Any missing component?
  // Check only existance of global att as path component.
  // Note: p_items in the actual sequence of the path,
  //       p_name/p_value corresponde to a_name/a_value
  text.clear();
  for( size_t i=0 ; i < a_name.size() ; ++i )
  {
    bool is=true;

    // Search the path components.
    // Are the global atts available in the path? Reverse counting.
    for(int j=p_items.size()-1 ; j > -1 ; --j )
    {
       if( a_value[i] == p_items[j] )
       {
          is=false;
          break;
       }
    }

    if( is )
    {
       if( text.size() )
         text += ", ";

       text += a_name[i] + "=";
       text += a_value[i] ;
    }
  }

  if( text.size() )
  {
    std::string key("10_2");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt( "directory structure expects global attribute ");

      (void) notes->operate(capt + text) ;
      notes->setCheckMetaStr( pQA->fail );
    }
  }

  return;
}

void
DRS_Filename::checkModelName(InFile &in, std::string &aName, std::string &aValue,
   char des, std::string instName, std::string instValue )
{
   hdhC::FileSplit* tbl;

   if( des == 'G' )
     tbl = &GCM_ModelnameTable ;
   else
     tbl = &RCM_ModelnameTable ;

   ReadLine ifs(tbl->getFile(), false);  // no test of existence

   if( ! ifs.isOpen() )
   {
      std::string key("70_") ;

      if( des == 'G' )
        key += "3" ;
      else
        key += "4" ;

      if( notes->inq(key) )
      {
         std::string capt("could not open " + tbl->getBasename()) ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
           pQA->setExit( notes->getExitValue() ) ;
         }
      }

      return;
   }

   std::string line;
   Split x_line;

   // parse table; trailing ':' indicates variable or 'global'
   ifs.skipWhiteLines();
   ifs.skipBashComment();

   bool isModel=false;
   bool isInst=false;
   bool isModelInst=false;

   bool isRCM = (des == 'R') ? true : false ;

   while( ! ifs.getLine(line) )
   {
     x_line = line ;

     if( aValue == x_line[0] )
       isModel=true;

     if( isRCM )
     {
       if( x_line.size() > 1 && instValue == x_line[1] )
       {
         isInst=true;
         if( isModel )
         {
            isModelInst=true;
            break;
         }
       }
     }
     else if( isModel )
        break;
   }

   ifs.close();

   if( !isModel )
   {
     std::string key;
     if( des == 'G' )
       key = "10_3" ;
     else
       key = "10_4" ;

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("global " + hdhC::tf_att(pQA->s_empty, aName, aValue));
       capt += "is not registered in table " ;
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   if( isRCM && !isInst )
   {
     std::string key("10_5");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("global " + hdhC::tf_att(pQA->s_empty, instName, instValue));
       capt += "is not registered in table " ;
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   if( isRCM && isModel && isInst && !isModelInst )
   {
     std::string key("10_6");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("combination of global") ;
       capt += hdhC::tf_att(pQA->s_empty, aName, aValue);
       capt += "and " + hdhC::tf_val(instName, instValue);
       capt += "is unregistered in table ";
       capt += tbl->getBasename();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   return;
}

void
DRS_Filename::checkFilename(InFile &in )
{
  // The global attributes in the section for global attributes
  // must satisfy the filename.

  // get global attributes required in terms of the filename
  std::vector<std::string> a_glob_att ;
  std::map<std::string, std::string> a_glob_value ;

  std::string str;
  std::string frequency;

  a_glob_att.push_back("CORDEX_domain");
  a_glob_att.push_back("driving_model_id");
  a_glob_att.push_back("driving_experiment_name");
  a_glob_att.push_back("driving_model_ensemble_member");
  a_glob_att.push_back("model_id");
  a_glob_att.push_back("frequency");
  a_glob_att.push_back("rcm_version_id");  // optionally !!!
  a_glob_att.push_back("institute_id");

  // missing global atts is checked elsewhere
  if( pQA->pIn->varSz == pQA->pIn->varSz )
     return;

  Variable &glob = pQA->pIn->variable[ pQA->pIn->varSz ];

  for( size_t i=0 ; i < a_glob_att.size() ; ++i )
  {
     str = glob.getAttValue(a_glob_att[i]) ;

     a_glob_value[a_glob_att[i]] = hdhC::stripSurrounding( str ) ;
  }

  // test filename for corrext syntax and decompose filename
  bool isMiss=false;

  // CORDEX filename encoding in the order of below;
  // variable_name is not an attribute
  std::string f( in.file.getFilename());

  if( f.rfind(".nc" ) )
    f = f.substr( 0, f.size()-3 );  // strip ".nc"

  Split splt(f, "_");

  // file name components
  std::map<std::string, std::string> a_file_value;

  // test the last two items of time type. If both are,
  // then this would mean that they are separated by '_'.
  // This would be a fault for CORDEX.
  size_t off=0;  // takes into account a period separator '_'
  if( splt.size() > 2 &&
         hdhC::isDigit( splt[ splt.size() -1 ])
             && hdhC::isDigit( splt[ splt.size() -2 ]) )
  {
    isMiss=true;
    off=1;

    std::string key("16_1");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("wrong separator in the period in the filename") ;

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  if( splt.size() > 7 )
  {
    a_file_value["Domain"] = splt[1] ;
    a_file_value["GCMModelName"] = splt[2] ;
    a_file_value["CMIP5ExperimentName"] = splt[3] ;
    a_file_value["CMIP5Ensemble_member"] = splt[4] ;
    a_file_value["RCMModelName"] = splt[5] ;

    if( splt.size() == (9+off) )
    {
      a_file_value["RCMVersionID"] = splt[6] ;
      a_file_value["Frequency"] = splt[7] ;
      frequency = splt[7] ;
    }
    else if( splt.size() == (8+off) )
    {
       if( hdhC::isDigit( splt[7][0] ) )
       {  // with period; no RCMVersionID
         a_file_value["RCMVersionID"] = "" ;
         a_file_value["Frequency"] = splt[6] ;
         frequency = splt[6] ;
       }
       else
       {  // no period; with RCMVersionID
         a_file_value["RCMVersionID"] = splt[6] ;
         a_file_value["Frequency"] =  splt[7] ;
         frequency = splt[7] ;
       }
    }
    else if( splt.size() == (7+off) )
    {
       // no period; no RCMVersionID
       a_file_value["RCMVersionID"] = "" ;
       a_file_value["Frequency"] = splt[6] ;
       frequency = splt[6] ;
    }
  }
  else
  {
     std::string key("15_1");
     if( notes->inq( key, pQA->fileStr ) )
     {
       std::string capt("filename not compliant to CORDEX encoding");

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr(pQA->fail);

       isMiss = true;
    }
  }

  if( isMiss )  // comparison not feasable
    return;

  // compare filename components to global attributes
  std::string f_name;
  std::string a_name;
  std::string f_val;
  std::string a_val;

  // do filename components match global attributes?
  std::vector<std::string> f_vs;
  std::vector<std::string> a_vs;

  f_vs.push_back("Domain");
  a_vs.push_back("CORDEX_domain");

  f_vs.push_back("GCMModelName");
  a_vs.push_back("driving_model_id");

  f_vs.push_back("CMIP5ExperimentName");
  a_vs.push_back("driving_experiment_name");

  f_vs.push_back("CMIP5Ensemble_member");
  a_vs.push_back("driving_model_ensemble_member");

  f_vs.push_back("RCMModelName");
  a_vs.push_back("model_id");

  f_vs.push_back("Frequency");
  a_vs.push_back("frequency");

  // optionally !!!
  f_name="RCMVersionID";
  a_name="rcm_version_id";
  if( a_file_value[f_name].size() &&
             a_file_value[f_name] != a_glob_value[a_name] )
  {
    f_vs.push_back(f_name);
    a_vs.push_back(a_name);
  }

  for( size_t i=0 ; i < f_vs.size() ; ++i )
  {
    f_name=f_vs[i];
    a_name=a_vs[i];

    if( a_file_value[f_name] != a_glob_value[a_name] )
    {
      if( a_file_value[f_name] == "r0i0p0"
                && a_glob_value[a_name] == "N/A" )
        ;
      else if( a_file_value[f_name] == "r0i0p0"
                && a_glob_value[a_name] == "r1i1p1" )
        ;
      else
      {
        f_val=a_file_value[f_name];
        a_val=a_glob_value[a_name];

        std::string key("15_2");
        if( notes->inq( key, pQA->fileStr))
        {
           std::string capt("filename does not match global ");
           capt += hdhC::tf_att(pQA->s_empty, a_name, a_val);
           capt += ", found " + hdhC::sAssign(f_name, f_val) ;

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );
        }
      }
    }
  }

  return;
}

void
DRS_Filename::checkDrivingExperiment(InFile &in)
{
  std::string str;

  // special: optional driving_experiment could contain
  // driving_model_id, driving_experiment_name, and
  // driving_model_ensemble_member
  str = in.getAttValue("driving_experiment") ;

  if( str.size() == 0 )
    return;  // optional attribute not available

  Split svs;
  svs.setSeparator(',');
  svs.enableEmptyItems();
  svs = str;

  std::vector<std::string> vs;
  for( size_t i=0 ; i < svs.size() ; ++i )
  {
    vs.push_back( svs[i] );
    vs[i] = hdhC::stripSurrounding( vs[i] ) ;
  }

  if( vs.size() != 3 )
  {
    std::string key("2_9");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("global " + hdhC::tf_att("driving_experiment") );
      capt += "with wrong number of items" ;
      capt += ", found " + hdhC::tf_val(str) ;

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }

    return;
  }

  // index of the pseudo-variable 'NC_GLOBAL'
  size_t ix;
  if( (ix=in.varSz) == in.varSz )
     return; // no global attributes; checked and notified elsewhere

  Variable &glob = in.variable[ix];

  // att-names corresponding to the three items in drinving_experiment
  std::vector<std::string> rvs;
  rvs.push_back("driving_model_id");
  rvs.push_back("driving_experiment_name");
  rvs.push_back("driving_model_ensemble_member");

  std::string value;

  for( size_t i=0 ; i < rvs.size() ; ++i )
  {
    value = glob.getAttValue( rvs[i] ) ;

    // a missing required att is checked elsewhere
    if( value.size() == 0 )
      continue;

    // allow anything for r0i0p0
    if( value != vs[i] && value != "r0i0p0" && vs[i] != "r0i0p0" )
    {
      std::string key("2_10");
      if( notes->inq( key, pQA->fileStr ) )
      {
        std::string capt("global ");
        capt += hdhC::tf_att("driving_experiment(" + hdhC::itoa(i) + ")" );
        capt += "is in conflict with " + hdhC::tf_att(rvs[i]) ;

        std::string text(rvs[i]) ;
        text += "=" ;
        text += value ;
        text += "\ndriving_experiment=" ;
        if( i==0 )
        {
          text += vs[i] ;
          text += ", ..., ..." ;
        }
        else if( i==1 )
        {
          text += "...," ;
          text += vs[i] ;
          text += ", ..." ;
        }
        else if( i==2 )
        {
          text += ", ..., ..." ;
          text += vs[i] ;
        }

        (void) notes->operate( capt, text) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_Filename::checkNetCDF(InFile &in)
{
  // NC_FORMAT_CLASSIC (1)
  // NC_FORMAT_64BIT   (2)
  // NC_FORMAT_NETCDF4 (3)
  // NC_FORMAT_NETCDF4_CLASSIC  (4)

  NcAPI& nc = pQA->pIn->nc;

  int fm = nc.inqNetcdfFormat();
  std::string s;

  if( fm < 3 )
    s = "3";
  else if( fm == 3 )
  {
    s = "4, not classic, ";

    if( ! nc.inqDeflate() )
      s += "not ";

    s+= "deflated (compressed)";
  }

  if( s.size() )
  {
    std::string key("12");
    if( notes->inq( key ) )
    {
      std::string capt("NetCDF4 classic deflated (compressed) required") ;
      std::string text("this is NetCDF");
      text += s;

      (void) notes->operate( capt, text ) ;
      notes->setCheckMetaStr( pQA->fail);
    }
  }

  return;
}

void
DRS_Filename::run(void)
{
  InFile& in = *(pQA->pIn);

  // check for the required dir structure
  check(in);

  // compare filename to netCDF global attributes
  checkFilename(in);

  // is it NetCDF-4, is it compressed?
  checkNetCDF(in);

  // optional, but if, then with three prescribed members
  checkDrivingExperiment(in);

  // note that this test is not part of the QA_Time class, because
  // coding depends on projects
  if( pQA->isCheckTime && testPeriod() )
  {
    std::string key("9_2");
    if( notes->inq( key, pQA->qaTime.name) )
    {
      std::string capt("status is apparently in progress");

      (void) notes->operate(capt) ;
    }
  }

  return;
}

bool
DRS_Filename::testPeriod(void)
{
  // return true, if a file is supposed to be not complete.
  // return false, a) if there is no period in the filename
  //               b) if an error was found
  //               c) times of period and file match

  // The time value given in the first/last record is assumed to be
  // in the range of the period of the file, if there is any.

  // If the first/last date in the filename period and the
  // first/last time value match within the uncertainty of the
  // time-step, then the file is complete.
  // If the end of the period exceeds the time data figure,
  // then the nc-file is considered to be not completely processed.

  // Does the filename has a trailing date range?
  // Strip off the extension.
  std::string f( pQA->pIn->file.getBasename() );

  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  // if designator '-clim' is appended, then remove it
  size_t f_sz = static_cast<int>(f.size()) -5 ;
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
  pQA->qaTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("42_4");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("invalid range for period in the filename, found ");
       capt += hdhC::tf_val(sd[0] + "-" + sd[1]);

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( pQA->fail );
     }

     return false;
  }

  Date* pDates[6];
  pDates[0] = &period[0];  // StartTime in the filename
  pDates[1] = &period[1];  // EndTime in the filename

  // index 2: date of first time value
  // index 3: date of last  time value
  // index 4: date of first time-bound value, if available; else 0
  // index 5: date of last  time-bound value, if available; else 0

  for( size_t i=2 ; i < 6 ; ++i )
    pDates[i] = 0 ;

  if( pQA->qaTime.isTimeBounds)
  {
    pDates[4] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.firstTimeBoundsValue[0] != 0 )
      pDates[4]->addTime(pQA->qaTime.firstTimeBoundsValue[0]);

    pDates[5] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.lastTimeBoundsValue[1] != 0 )
      pDates[5]->addTime(pQA->qaTime.lastTimeBoundsValue[1]);

    double db_centre=(pQA->qaTime.firstTimeBoundsValue[0]
                        + pQA->qaTime.firstTimeBoundsValue[1])/2. ;
    if( ! hdhC::compare(db_centre, '=', pQA->qaTime.firstTimeValue) )
    {
      std::string key("16_11");
      if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
      {
        std::string capt("Range of variable time_bnds is not centred around variable time.");

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }
  else
  {
    if( pQA->qaTime.time_ix > -1 &&
        ! pQA->pIn->variable[pQA->qaTime.time_ix].isInstant )
    {
      std::string key("16_12");
      if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
      {
        std::string capt("Variable time_bnds is missing");

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  pDates[2] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.firstTimeValue != 0. )
    pDates[2]->addTime(pQA->qaTime.firstTimeValue);

  pDates[3] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.lastTimeValue != 0. )
    pDates[3]->addTime(pQA->qaTime.lastTimeValue);

  // alignment of of contained dates and those in the filename
  // the booleanx indicate faults
//  bool is_t_beg = is_t_end = is_tb_beg = is_tb_end = false;
  bool isFault[4];
  for(size_t i=0 ; i < 4 ; ++i )
    isFault[i]=false;

  // time value: left-side
  Date myDate( *pDates[2] );
  myDate.addTime(-pQA->qaTime.refTimeStep);
  isFault[0] = myDate == *pDates[0] ;

  // time value: right-side
  myDate = *pDates[3] ;
  myDate.addTime(pQA->qaTime.refTimeStep);
  isFault[1] = myDate ==*pDates[1] ;

  if(pQA->qaTime.isTimeBounds)
  {
    // time_bounds: left-side
    myDate = *pDates[4] ;
    myDate.addTime(pQA->qaTime.refTimeStep);
    isFault[2] = myDate == *pDates[0] ;

    // time_bounds: right-side
    myDate = *pDates[5] ;
    myDate.addTime(-pQA->qaTime.refTimeStep);
    isFault[3] = myDate == *pDates[1] ;
  }

  // the annotations
  if( testPeriodAlignment(sd, pDates, isFault) )
  {
    std::string key("16_12");
    std::string capt("period in the filename: ") ;
    capt +="note that StartTime-EndTime is compliant with CMOR peculiarity";

    if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
    {
      (void) notes->operate(capt) ;

      notes->setCheckMetaStr( pQA->fail );
    }
  }
  else if( testPeriodFormat(sd) ) // format of period dates.
  {
    // period requires a cut specific to the various frequencies.
    std::vector<std::string> text ;
    testPeriodCutRegular(sd, text) ;

    if( text.size() )
    {
      std::string key("16_6");
      std::string capt("period in the filename") ;

      for( size_t i=0 ; i < text.size() ; ++i )
      {
        if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
        {
          (void) notes->operate(capt + text[i]) ;

          notes->setCheckMetaStr( pQA->fail );
        }
      }
    }
  }

  // note that indices 0 and 1 belong to a vector
  for(size_t i=2 ; i < 6 ; ++i )
    if( pDates[i] )
      delete pDates[i];

  // complete
  return false;
}

bool
DRS_Filename::testPeriodAlignment(std::vector<std::string> &sd, Date** pDates, bool b[])
{
  // some pecularities of CMOR, which will probably not be modified
  // for a behaviour as expected.
  // The CORDEX Archive Design doc states in Appendix C:
  // "..., for using CMOR, the StartTime-EndTime element [in the filename]
  //  is based on the first and last time value included in the file ..."

  // if a test for a CMOR setting fails, testing goes on
  if( *pDates[0] == *pDates[2] && *pDates[1] == *pDates[3] )
      return true;

  for(size_t i=0 ; i < 2 ; ++i)
  {
    // i == 0: left; 1: right

    // skip for the mode of checking during production
    if( i && !pQA->isFileComplete )
       continue;

    if( !( !b[0+i] || !b[2+i] ) )
    {
      std::string key("16_2");
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string token;

        std::string capt("Misaligned ");
        if( i == 0 )
          capt += "begin" ;
        else
          capt += "end" ;
        capt += " of periods in filename and ";

        size_t ix;

        if( pQA->qaTime.isTimeBounds )
        {
          capt="time bounds: ";
          ix = 4 + i ;
        }
        else
        {
          capt="time values: ";
          ix = 2 + i ;
        }

        capt += sd[i] ;
        capt += " vs. " ;
        capt += pDates[ix]->str();

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }

  return false;
}

void
DRS_Filename::testPeriodCutRegular(std::vector<std::string> &sd,
                  std::vector<std::string>& text)
{
  // Partitioning of files check are equivalent.
  // Note that the format was tested before.
  // Note that desplaced start/end points, e.g. '02' for monthly data, would
  // lead to a wrong cut.

  bool isInstant = ! pQA->qaTime.isTimeBounds ;

  bool isBegin = pQA->fileSequenceState == 'l' || pQA->fileSequenceState == 's' ;
  bool isEnd   = pQA->fileSequenceState == 'f' || pQA->fileSequenceState == 's' ;

  std::string frequency(pQA->qaExp.getFrequency());

  // period length per file as recommended?
  if( frequency == "3hr" || frequency == "6hr" )
  {
    // same year.
    bool isA = sd[1].substr(4,4) == "1231";
    bool isB = sd[1].substr(4,4) == "0101" && sd[1].substr(8,2) == "00" ;

    // should be the same year
    if( isBegin && isEnd )
    {
      int yBeg=hdhC::string2Double( sd[0].substr(0,4) );
      int yEnd=hdhC::string2Double( sd[1].substr(0,4) );
      if( isB )  // begin of a next year
        --yEnd;

      if( (yEnd-yBeg) )
        text.push_back(": time span of a full year is exceeded");
    }

    // cut of period
    std::string s_sd0( sd[0].substr(4,4) );
    std::string s_sd1( sd[1].substr(4,4) );

    std::string s_hr0( sd[0].substr(8,2) );
    std::string s_hr1( sd[1].substr(8,2) );

    std::string t_found(", found ");
    std::string t_hr0("00");
    std::string t_hr1;

    if( frequency == "3hr" )
      t_hr1 = "21";
    else
      t_hr1 = "18";

    if( isBegin )
    {
      if( s_sd0 != "0101" )
        text.push_back( ": not the begin of the year, found " + s_sd0);

      if( isInstant )
      {
        if( s_hr0 != t_hr0 )
        {
          text.push_back(" (instantaneous + 1st date): expected hr=") ;
          text.back() += t_hr0 + t_found + s_hr0 ;
        }
      }
      else
      {
        if( sd[0].substr(8,2) != t_hr0 )
        {
          text.push_back(" (average + 1st date): expected hr=");
          text.back() += t_hr0 + t_found + s_hr0 ;
        }
      }
    }

    if( isEnd )
    {
      if( !(isA || isB)  )
        text.push_back( ": not the end of the year, found " + s_sd1);

      if( isInstant )
      {
        if( s_hr1 != t_hr1 )
        {
          text.push_back(" (instantaneous + 2nd date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
      }
      else
      {
        if( isA && s_hr1 != "24" )
        {
          text.push_back(" (averaged + 1st date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
        else if( isB && s_hr1 != "00" )
        {
          text.push_back(" (averaged + 2nd date): expected hr=");
          text.back() += t_hr1 + t_found + s_hr1 ;
        }
      }
    }
  }

  else if( frequency == "day" )
  {
     // 5 years or less
     if( isBegin && isEnd )
     {
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 5 )
        text.push_back(": time span of 5 years is exceeded");
     }

     if( isBegin )
     {
       if( ! (sd[0][3] == '1' || sd[0][3] == '6') )
         text.push_back(": StartTime should begin with YYY1 or YYY6");

       if( sd[0].substr(4,4) != "0101" )
         text.push_back(": StartTime should be YYYY0101");
     }

     if( isEnd )
     {
       if( ! (sd[1][3] == '0' || sd[1][3] == '5') )
         text.push_back(": EndTime should begin with YYY0 or YYY5");

       if( sd[1].substr(4,4) != "1231" )
         text.push_back(": EndTime should be YYYY1231");
     }
  }
  else if( frequency == "mon" )
  {
     if( isBegin && isEnd )
     {
      // 10 years or less
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 10 )
        text.push_back(": time span of 10 years is exceeded");
     }

     if( isBegin )
     {
       if( sd[0][3] != '1')
         text.push_back(": StartTime should begin with YYY1");

       if( sd[0].substr(4,4) != "01" )
         text.push_back(": StartTime should be YYYY01");
     }

     if( isEnd )
     {
       if( ! (sd[1][3] == '0' || sd[1][3] == '5') )
         text.push_back(": EndTime should begin with YYY0");

       if( sd[1].substr(4,4) != "12" )
         text.push_back(": EndTime should be YYYY1231");
     }
  }
  else if( frequency == "sem" )
  {
     if( isBegin && isEnd )
     {
      // 10 years or less
      int yBeg=hdhC::string2Double(sd[0].substr(0,4));
      int yEnd=hdhC::string2Double(sd[1].substr(0,4));
      if( (yEnd-yBeg) > 11 )  // because of winter across two year
        text.push_back(": time span of 10 years is exceeded");
     }

     if( frequency == "sem" )
     {
       if( isBegin )
       {
          if( sd[0].substr(4,2) != "12" )
            text.push_back(": StartTime should be YYYY12");
       }

       if( isEnd )
       {
          if( sd[1].substr(4,2) != "11" )
            text.push_back(": EndTime should be YYYY11");
       }
     }
  }

  return;
}

bool
DRS_Filename::testPeriodFormat(std::vector<std::string> &sd)
{
  // return: true means go on for testing the period cut
  std::string key("16_5");
  std::string capt;
  std::string str;
  std::string frequency(pQA->qaExp.getFrequency());

  // partitioning of files
  if( sd.size() != 2 )
  {
      if( pQA->pIn->nc.isDimUnlimited() )
      {
        if( pQA->pIn->nc.getNumOfRecords() > 1 )
        {
          key = "16_10";
          capt = "a period is required in the filename";
          str.clear();
        }
      }

      return false;  // no period; is variable time invariant?
  }
  else if( frequency == "3hr" || frequency == "6hr" )
  {
      if( sd[0].size() != 10 || sd[1].size() != 10 )
      {
        str += "YYYYMMDDhh for ";
        if( frequency == "3hr" )
          str += "3";
        else
          str += "6";
        str += "-hourly time step";
      }
  }
  else if( frequency == "day" )
  {
      if( sd[0].size() != 8 || sd[1].size() != 8 )
        str += "YYYYMMDD for daily time step";
  }
  else if( frequency == "mon" || frequency == "sem" )
  {
     if( sd[0].size() != 6 || sd[1].size() != 6 )
     {
        str += "YYYYMM for ";
        if( frequency == "mon" )
          str += "monthly";
        else
          str += "seasonal";
        str += " data";
     }
  }

  if( str.size() )
  {
     if( notes->inq( key, pQA->fileStr) )
     {
        capt = "period in filename of incorrect format";
        capt += ", found " + sd[0] + "-" +  sd[1];
        capt += " expected " + str ;

        (void) notes->operate(capt) ;

        notes->setCheckMetaStr( pQA->fail );
     }
  }

  return true;
}



// Class with project specific purpose
QA_Exp::QA_Exp()
{
  initDefaults();
}

void
QA_Exp::applyOptions(std::vector<std::string>& optStr)
{
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

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);

       continue;
     }

     if( split[0] == "tCV"
           || split[0] == "tableControlledVocabulary"
                || split[0] == "table_controlled_vocabulary" )
     {
       if( split.size() == 2 )
          table_DRS_CV.setFile(split[1]) ;

       continue;
     }

     if( split[0] == "tVR"
          || split[0] == "tableVariableRequirement" )
     {
       if( split.size() == 2 )
          varReqTable.setFile(split[1]);

       continue;
     }

     if( split[0] == "uS" || split[0] == "useStrict"
            || split[0] == "use_strict" )
     {
       isUseStrict=true ;
       pQA->setCheckMode("meta");
       continue;
     }
   }

   // apply a general path which could have also been provided by setTablePath()
   if( table_DRS_CV.path.size() == 0 )
      table_DRS_CV.setPath(pQA->tablePath);

   if( varReqTable.path.size() == 0 )
      varReqTable.setPath(pQA->tablePath);

   return;
}

void
QA_Exp::checkDimTableEntry(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?
  // Failed checks against a project table result in issuing an
  // error message. Only warnings for a failed check against
  // the standard table.

  std::string text;
  std::string t0;

  if( tbl_entry.outname != nc_entry.outname )
     checkDimOutName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.stndname != nc_entry.stndname )
     checkDimStndName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.longname != nc_entry.longname )
     checkDimLongName(in, vMD, nc_entry, tbl_entry);

  if( tbl_entry.axis != nc_entry.axis )
     checkDimAxis(in, vMD, nc_entry, tbl_entry);

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
QA_Exp::checkDimAxis(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("47_4");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry) );

    if( tbl_entry.axis.size() && nc_entry.axis.size() )
      capt += pQA->s_mismatch ;
    else if( tbl_entry.axis.size() )
      capt+= "not available in the file" ;
    else
      capt += "not available in the table" ;

    std::string text("axis (table)=") ;
    if( tbl_entry.axis.size() )
      text += tbl_entry.axis ;
    else
      text += pQA->notAvailable ;

    text += "\naxis (file)=" ;
    if( nc_entry.axis.size() )
      text += nc_entry.axis ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return;
}

void
QA_Exp::checkDimChecksum(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string t0;

  std::string key("47_5");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry) ) ;
    capt += "checksum of values changed" ;

    std::string text("checksum (table)=") ;
    text += hdhC::double2String(tbl_entry.checksum) ;
    text += "\nchecksum (file)=" ;
    text += hdhC::double2String(nc_entry.checksum) ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return;
}

void
QA_Exp::checkDimLongName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{

  std::string key("40_3");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getCaptIntroDim(vMD, nc_entry, tbl_entry, pQA->n_long_name) ) ;

    if( tbl_entry.longname.size() && nc_entry.longname.size() )
      capt += pQA->s_mismatch ;
    else if( tbl_entry.longname.size() )
      capt += "not available in the file" ;
    else
      capt += "not available in the table" ;

    std::string text("long name (table)=") ;
    if( tbl_entry.longname.size() )
      text += tbl_entry.longname ;
    else
      text += pQA->notAvailable ;

    text += "\nlong name (file)=" ;
    if( nc_entry.longname.size() )
      text += nc_entry.longname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return;
}

void
QA_Exp::checkDimOutName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("40_1");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry) );

    if( tbl_entry.outname.size() && nc_entry.outname.size() )
    {
      capt += "output name with " ;
      capt += pQA->s_mismatch ;
    }
    else if( tbl_entry.outname.size() )
      capt += "output name not available in the file" ;
    else
      capt += "output name not available in the table" ;

    std::string text("output name (table)=") ;
    if( tbl_entry.outname.size() )
      text += tbl_entry.outname ;
    else
      text += pQA->notAvailable ;

    text += "\noutput name (file)=" ;
    if( nc_entry.outname.size() )
      text += nc_entry.outname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return ;
}

void
QA_Exp::checkDimSize(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("40_5");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry) ) ;
    capt += "different size";

    std::string text("dim-size (table)=") ;
    text += hdhC::double2String(tbl_entry.size) ;
    text += "\ndim-size (file)=" ;
    text += hdhC::double2String(nc_entry.size) ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return;
}

void
QA_Exp::checkDimStndName(InFile &in,
    VariableMetaData &vMD,
    struct DimensionMetaData &nc_entry,
    struct DimensionMetaData &tbl_entry)
{
  std::string key("40_2");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry, pQA->n_standard_name) ) ;

    if( tbl_entry.stndname.size() && nc_entry.stndname.size() )
      capt += pQA->s_mismatch ;
    else if( tbl_entry.stndname.size() )
      capt += "not available in the file" ;
    else
      capt += "not available in the table" ;

    std::string text("table=");
    text += varReqTable.filename + ", frequency=";
    text += getFrequency() + ", variable=";
    text += vMD.var->name + ", dimension=";
    text += nc_entry.outname + "\nstandard_name (table)=" ;
    if( tbl_entry.stndname.size() )
      text += tbl_entry.stndname ;
    else
      text += pQA->notAvailable ;

    text += "\nstandard_name (file)=" ;
    if( nc_entry.stndname.size() )
      text += nc_entry.stndname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return;
}

void
QA_Exp::checkDimULD(
     VariableMetaData &vMD,
     struct DimensionMetaData &nc_entry,
     struct DimensionMetaData &tbl_entry)
{
  // Special for the unlimited dimension

  // Do the dimensional meta-data found in the netCDF file
  // match those in the standard table?
  // Checking ranges is senseless, thus discarded.

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
      std::string key("36_1");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry, pQA->n_units) ) ;

        if( tbl_entry.units.size() && nc_entry.units.size() )
          capt += "different periods" ;
        else if( tbl_entry.units.size() )
          capt += "missing period string (file)" ;
        else
          capt += "missing period string (table)" ;

        std::string text("units (table)=") ;
        if( tbl_entry.units.size() )
          text += tbl_entry.units ;
        else
          text += "missing key string=days since" ;

        text += "\nunits (file): " ;
        if( nc_entry.units.size() )
          text += nc_entry.units ;
        else
          text += "missing key string=days since" ;

        (void) notes->operate(capt, text) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
    else // deviating reference dates
    {
      // There is nothing wrong with deviating references
      if( ! pQA->qaTime.isReferenceDate )
        return ;  // do not check at all

      // do not check at the beginning of a new experiment.
      if( ! pQA->qaTime.isReferenceDateAcrossExp
               && pQA->currQARec == 0 )
        return ;

      Date tbl_ref( tbl_entry.units, pQA->qaTime.calendar );
      Date nc_ref( nc_entry.units, pQA->qaTime.calendar );

      std::string key("36_2");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry, pQA->n_units) ) ;

        if( tbl_entry.units.size() && nc_entry.units.size() )
          capt += "different reference dates" ;
        else if( tbl_entry.units.size() )
          capt += "missing reference date in the file" ;
        else
          capt += "missing reference date in the table" ;

        std::string text ;
        if( tbl_ref != nc_ref )
        {
          text += "units (table)=" ;
          if( tbl_entry.units.size() )
            text += tbl_entry.units ;
          else
            text += pQA->notAvailable ;

          text += "\nunits (file)=" ;
          if( nc_entry.units.size() )
            text += nc_entry.units ;
          else
            text += pQA->notAvailable ;
        }

        (void) notes->operate(capt, text) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return ;
}

void
QA_Exp::checkDimUnits(InFile &in,
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

  //dimension's var-rep without units in the standard table
  std::string key("40_4");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt( getCaptIntroDim(vMD, nc_entry, tbl_entry, pQA->n_units) ) ;

    if( tbl_entry.units.size() && nc_entry.units.size() )
      capt += pQA->s_mismatch ;
    else if( tbl_entry.isUnitsDefined )
      capt += "not available in the file" ;
    else
      capt += "not defined in the table" ;

    std::string text("units (table)=") ;
    if( tbl_entry.units.size() )
      text += tbl_entry.units ;
    else
      text += "not defined" ;

    text += "\nunits (file)=" ;
    if( nc_entry.units.size() )
      text += nc_entry.units ;
    else if( nc_entry.isUnitsDefined )
      text += pQA->notAvailable ;
    else
      text += "not defined" ;

    (void) notes->operate(capt, text) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return ;
}

void
QA_Exp::checkPressureCoord(InFile &in)
{
   // Check for missing pressure coordinate and for the correct value
   // This has to be done for all variables with appended number
   // indicating the pressure level.

   std::string fV = fVarname.substr(0,2);

   if ( !( fV == "ua" || fV == "va" || fV == "ta" || fV == "zg" ) )
      return;  //not a variable on a specific pressure level

   // exclude variables like tas, tasmax etc.
   // true: return 0 for no 1st number
   bool is = hdhC::string2Double(fVarname, 1, true) == 0. ;

   if( is && fVarname.size() > 2 )
     return;

   // a trailing level after the file-var-name?
   if( fVarname.size() == 2 )
   {
     std::string key("15_4");
     if( notes->inq( key, fVarname ) )
     {
       std::string capt("The variable acronym");
       capt += hdhC::tf_val(fVarname) ;
       capt += "in the filename is missing a pressure level." ;

       (void) notes->operate( capt ) ;
       notes->setCheckMetaStr(pQA->fail);
     }
   }

   std::string fVal = fVarname.substr(2);

   if( ! (fVal == "200" || fVal == "500" || fVal == "850" ) )
   {
     std::string key("15_5");
     if( notes->inq( key ) )
     {
       std::string capt("Pressure level value=");
       capt += fVal + " in the filename is inappropriate" ;

       std::string text("Expected: 200 500 or 850");

       (void) notes->operate( capt, text ) ;
       notes->setCheckMetaStr(pQA->fail);
     }
   }

   std::string pFile( fVarname.substr(2) + "00" ); // conversion to Pa
   std::string pVarname;
   int    plev_ix=-1;

   for( size_t ix=0 ; ix < in.varSz ; ++ix )
   {
     Variable &var = in.variable[ix];

     if( var.name == "plev")
       plev_ix=ix;
     else if( var.isUnlimited() )
     {
        // true: return 0 for no 1st number
        double anum = hdhC::string2Double(var.name, 1, true);
        if( anum > 0. )
           pVarname = hdhC::double2String(anum) + "00";
     }
   }

   if( plev_ix == -1 )
   {
     std::string key("5_3");
     if( notes->inq( key ) )
     {
       std::string capt("Auxiliary " + hdhC::tf_var("plev") + "is missing") ;

       (void) notes->operate( capt ) ;
       notes->setCheckMetaStr(pQA->fail);
     }

     return ;
   }

   // is there a difference between filename and variable name?
   if( pFile.size() && pFile != pVarname )
   {
     std::string key("37_1");
     if( notes->inq( key ) )
     {
       std::string capt("p-level in the filename and the variable name do not match") ;
       capt += ", found: " + hdhC::sAssign("p(file)", pFile);
       capt += " and " + hdhC::sAssign("p(var-name)", pVarname);

       (void) notes->operate( capt) ;
       notes->setCheckMetaStr(pQA->fail);
     }
   }

   // look for pressure, units, Pa
   size_t ix=0;
   size_t jx=0;
   for( ; ix < in.varSz ; ++ix )
   {
     Variable &var = in.variable[ix];

     if( var.name == "plev")
     {
       for( jx=0 ; jx < var.attName.size() ; ++jx )
       {
         if( var.attName[jx] == pQA->n_units )
         {
            if( var.attValue[jx][0] != "Pa" )
              return;  // is annotated elsewhere;
                       // a check of a value is meaningless
         }
       }

       break;
     }
   }

   Variable &var = in.variable[ix];

   in.nc.getData(tmp_mv, var.name);
   if( tmp_mv.size() )
   {
     std::string pData( hdhC::double2String(tmp_mv[0]) );

     if( pData != pVarname )
     {
       std::string key("5_4");
       if( notes->inq( key, in.variable[ix].name ) )
       {
         std::string capt(hdhC::tf_var("plev", pQA->s_colon));
         capt += "Data value does not match Pa units." ;
         capt += ", found: " + hdhC::sAssign("p", pData);
         capt += ", expected" + hdhC::sAssign("p", pVarname);

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr(pQA->fail);
       }
     }
   }

   return;
}

void
QA_Exp::domainCheck(ReadLine &ifs)
{
   std::string line;
   ifs.getLine(line);

   std::vector<std::vector<std::string> > table1;
   std::vector<std::vector<std::string> > table2;

   Split csv;
   csv.setSeparator(',') ;
   bool isFirst=true;

   while( ! ifs.getLine(line) )
   {
     line = hdhC::stripSurrounding( line );

     if( line.size() == 0)
       continue;
     if( line[0] == '#')
       continue;

     if( line == "End: Table1" )
     {
        isFirst=false;
        continue;
     }

     if( line == "Begin: Table2" )
        continue;

     if( line == "End: Table2" )
        break;

     // find corresponding table entry
     csv = line;
     std::vector<std::string> row;
     row.clear();

     for( size_t i=0 ; i < csv.size() ; ++i )
     {
       // make reading of a table independent on a leading index
       if( i == 0 && hdhC::isDigit(csv[0][0]) )
          continue;

       row.push_back( hdhC::stripSurrounding( csv[i] ) ) ;
     }

     if( isFirst )
        table1.push_back( row ) ;
     else
        table2.push_back( row ) ;
   }

   std::string s0;
   int ix_tbl1, ix_tbl2;

   std::string t_lon ;
   std::string t_lat ;
   std::string f_lonName;
   std::string f_latName;

   domainFindTableType(table1, table2, ix_tbl1, ix_tbl2) ;

   // name
   std::string domName ;
   bool isTbl_1st=false;
   bool isTbl_2nd=false;
   size_t irow;

   if( ix_tbl1 > -1 && ix_tbl2 == -1 )
   {
     isTbl_1st=true;
     irow = static_cast<size_t>( ix_tbl1 );
   }
   else if( ix_tbl1 ==-1 && ix_tbl2 > -1 )
   {
     isTbl_2nd=true;
     irow = static_cast<size_t>( ix_tbl2 );
   }

   if( ! isTbl_1st && ! isTbl_2nd )
   {
     // no valid Name was found at all,
     // so we don't know which Table to use.
     // Try to find out.
     int table_id;

     if( domainFindTableTypeByRange(table1, table2, table_id, irow) )
     {
        if( table_id == 1 )
          isTbl_1st = true;
        else if( table_id == 2 )
          isTbl_2nd = true;
     }
   }

   if( isTbl_1st )
   {
     // Note: if the data file is produced by a model not using rotated
     // coordinates, then the checks will be discarded.
     bool is=true;
     std::string v;
     for( size_t i=0 ; i < pQA->pIn->varSz ; ++i )
     {
       v = hdhC::Lower()(pQA->pIn->variable[i].name);

       // check attributes for key-word 'rotated' and 'pole'
       for( size_t j=0 ; j < pQA->pIn->variable[i].attName.size() ; ++j )
       {
         if( pQA->pIn->variable[i].attName[j].find("rotated") < std::string::npos
              &&  pQA->pIn->variable[i].attName[j].find("pole") < std::string::npos )
         {
            is=false;
            break;
         }
       }
     }

     // assumption that the model is non-rotational
     if( is )
        return;

     // check dimensions
     t_lon = table1[irow][5];
     t_lat = table1[irow][6];

     domainCheckDims("Nlon", t_lon, f_lonName, "1") ;
     domainCheckDims("Nlat", t_lat, f_latName, "1") ;

     t_lon = table1[irow][3];
     t_lat = table1[irow][4];

     domainCheckPole("N.Pole lon", t_lon, f_lonName);
     domainCheckPole("N.Pole lat", t_lat, f_latName);

     domainCheckData(f_lonName, f_latName, table1[irow], "Table 1");

     return ;
  }

   if( isTbl_2nd )
   {
     isRotated=false;

     // check dimensions
     t_lon = table2[irow][3];
     t_lat = table2[irow][4];

     domainCheckDims("Nlon", t_lon, f_lonName, "2") ;
     domainCheckDims("Nlat", t_lat, f_latName, "2") ;

     // no pole check

     domainCheckData(f_lonName, f_latName, table2[irow], "Table 2");

     return ;
  }

  // used domain name not found in Table 1 or 2
  std::string key = "7_6";
  if( notes->inq(key, pQA->fileStr) )
  {
    std::string capt("Domain not specified neither in Table 1 or 2.") ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return ;
}

bool
QA_Exp::domainFindTableTypeByRange(
   std::vector<std::vector<std::string> > &T1,
   std::vector<std::vector<std::string> > &T2,
   int &table_id, size_t &row )
{
   // Try to identify table and row  by data properties.

   // Identify a target variable and get its dimensions.
   // Collect all candidates for being (r)lon or (r)lat.
   std::vector<std::string> cName;
   std::vector<size_t> cSize;

   for(size_t i=0 ; i < pQA->pIn->varSz ; ++i )
   {
      if( pQA->pIn->variable[i].isDATA )
      {
         for(size_t j=0 ; j < pQA->pIn->variable[i].dimName.size() ; ++j )
         {
           std::string &name = pQA->pIn->variable[i].dimName[j];

           for(size_t k=0 ; k < pQA->pIn->varSz ; ++k )
           {
              if( pQA->pIn->variable[k].name == name )
              {
                if( pQA->pIn->variable[k].isUnlimited() )
                  break;

                if( pQA->pIn->variable[k].coord.isCoordVar )
                {
                  cName.push_back( name );  // is a candidate
                  cSize.push_back( pQA->pIn->variable[k].dimSize ) ;
                }

                break;
              }
           }
         }
      }
   }

   // remove duplicates
   std::vector<std::string> candidate;
   std::vector<std::string> candidate_sz;

   for( size_t i=0 ; i < cName.size() ; ++i )
   {
     size_t j;
     for( j=0 ; j < candidate.size() ; ++j )
       if( cName[i] == candidate[j] )
         break;

     if( j == candidate.size() )
     {
       candidate_sz.push_back( hdhC::itoa(
                      static_cast<int>(cSize[i] ) ) );
       candidate.push_back( cName[i] ) ;
     }
   }

   if( candidate.size() == 0 )
     return false;  // no candidate found at all; no further checking
              // is possible.

   // the combination of Nlon and Nlat in Table 1 and 2 is
   // an unambiguous indicator for table and row.

   // search in Table1
   for( size_t l=0 ; l < T1.size() ; ++l )
   {
     for( size_t i=0 ; i < candidate_sz.size() ; ++i )
     {
       if( T1[l][5] != candidate_sz[i] )
         continue; //no match

       for( size_t j=0 ; j < candidate_sz.size() ; ++j )
       {
         if( T1[l][6] == candidate_sz[j] )
         {
           // found a candidate for a matching row in Tables 1
           table_id = 1 ;
           row = l ;
           return true;
         }
       }
     }
   }

   // search in Table2
   for( size_t l=0 ; l < T2.size() ; ++l )
   {
     for( size_t i=0 ; i < candidate_sz.size() ; ++i )
     {
       if( T2[l][3] != candidate_sz[i] )
         continue; //no match

       for( size_t j=0 ; j < candidate_sz.size() ; ++j )
       {
         if( T2[l][4] == candidate_sz[j] )
         {
           // found a candidate for a matching row in Tables 2
           table_id = 2 ;
           row = static_cast<int>(l) ;
           return true;
         }
       }
     }
   }

   return false;
}

void
QA_Exp::domainCheckData(std::string &var_lon, std::string &var_lat,
    std::vector<std::string> &row, std::string tName)
{
  // compare values of lat/lon specified in CORDEX Table 1 with
  // corresponding values of the file.

  size_t add=0 ;
  if( tName == "Table 1" )
    add=2 ;

  // get data
  MtrxArr<double> mv_lon;
  MtrxArr<double> mv_lat;
  pQA->pIn->nc.getData(mv_lon, var_lon);
  pQA->pIn->nc.getData(mv_lat, var_lat);

  if( mv_lon.size() < 2 || mv_lat.size() < 2 )
  {
    std::string key = "7_9";
    if( notes->inq(key, pQA->fileStr) )
    {
      std::string capt("CORDEX " + hdhC::sAssign("domain", tName)) ;
      capt += " with missing data for ";

      if( mv_lon.size() < 2 && mv_lat.size() < 2 )
      {
        capt += hdhC::tf_var(var_lon) ;
        capt += "and " + hdhC::tf_var(var_lat);

        return;
      }
      else if( mv_lon.size() < 2 )
      {
        capt += hdhC::tf_var(var_lon) ;
      }
      else
      {
        capt += hdhC::tf_var(var_lat) ;
      }

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  // check resolution (with rounding)

  // resolution from table
  std::string t_resol(hdhC::double2String(
      hdhC::string2Double( row[2]), -5) ) ;

  std::string f_resol_lon;
  bool is_lon=false;
  if( mv_lon.size() > 1 )
  {
     f_resol_lon = hdhC::double2String( (mv_lon[1] - mv_lon[0]), -5);
     if( f_resol_lon != t_resol )
       is_lon=true;
  }
  else
    is_lon=true;

  std::string f_resol_lat;
  bool is_lat=false;
  if( mv_lat.size() > 1 )
  {
     f_resol_lat = hdhC::double2String( (mv_lat[1] - mv_lat[0]), -5);
     if( f_resol_lat != t_resol )
       is_lat=true;
  }
  else
    is_lat=true;

  if( is_lon || is_lat )
  {
    std::string key = "7_10";
    if( notes->inq(key, pQA->fileStr) )
    {
      std::string capt("resolution of CORDEX ");
      capt += hdhC::sAssign("domain", tName) ;
      capt += " does not match. Found " ;
      if( is_lon && is_lat )
      {
        capt += var_lon;
        capt += hdhC::sAssign("resol("+var_lon+")", f_resol_lon) ;
        capt += " and ";
        capt += hdhC::sAssign("resol("+var_lat+")", f_resol_lat) ;
        return;
      }
      else if( is_lon )
      {
        capt += hdhC::sAssign("resol("+var_lon+")", f_resol_lon) ;
        capt += f_resol_lon ;
      }
      else
      {
        capt += hdhC::sAssign("resol("+var_lat+")", f_resol_lat) ;
      }

      capt += "; required is "  + t_resol;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  // check edges of the domain for grid-cell centres
  // vs. the boundaries from file data. Note: file data range may
  // be enlarged.
  std::string edge_value[4];
  bool is_edge[4];
  if( ! is_lon )
  {
     int j = pQA->pIn->getVarIndex(var_lon) ;
     Variable &var = pQA->pIn->variable[j];

     std::string units(pQA->pIn->nc.getAttString(pQA->n_units, var.name) );

     size_t i_1st  = 0;
     size_t i_last = mv_lon.size()-1;
     if( units == "degrees_south" )
     {
        i_1st = i_last;
        i_last= 0;
     }

     edge_value[0] = hdhC::double2String( mv_lon[i_1st], -5);
     if( edge_value[0] <= row[5+add] )
       is_edge[0]=false;
     else
       is_edge[0]=true;

     edge_value[1] = hdhC::double2String( mv_lon[i_last], -5);
     if( edge_value[1] >= row[6+add] )
       is_edge[1]=false;
     else
       is_edge[1]=true;
  }

  if( ! is_lat )
  {
     int j = pQA->pIn->getVarIndex(var_lat) ;
     Variable &var = pQA->pIn->variable[j];

     std::string units( var.getAttValue(pQA->n_units) );

     size_t i_1st  = 0;
     size_t i_last = mv_lat.size()-1;
     if( units == "degrees_west" )
     {
        i_1st = i_last;
        i_last= 0;
     }

     edge_value[2] = hdhC::double2String( mv_lat[i_1st], -5);
     if( edge_value[2] <= row[7+add] )
       is_edge[2]=false;
     else
       is_edge[2]=true;

     edge_value[3] = hdhC::double2String( mv_lat[i_last], -5);
     if( edge_value[3] >= row[8+add] )
       is_edge[3]=false;
     else
       is_edge[3]=true;
  }

  std::string text1;
  std::string text2;
  bool is=false;
  bool isComma=false;
  for( size_t i=0 ; i < 4 ; ++i )
  {
    if( is_edge[i] )
    {
      if( i==0 )
      {
        text1 += "West=" ;
        text1 += hdhC::double2String( hdhC::string2Double( row[5+add]), -5) ;
        text2 += "West=" ;
        text2 += edge_value[0] ;
        isComma=true;
      }
      else if( i==1 )
      {
        if(isComma)
        {
          text1 += ", ";
          text2 += ", ";
        }

        text1 += "East=" ;
        text1 += hdhC::double2String( hdhC::string2Double( row[6+add]), -5) ;
        text2 += "East=" ;
        text2 += edge_value[1] ;
        isComma=true;
      }

      if( i==2 )
      {
        if(isComma)
        {
          text1 += ", ";
          text2 += ", ";
        }

        text1 += "South=" ;
        text1 += hdhC::double2String( hdhC::string2Double( row[7+add]), -5) ;
        text2 += "South=" ;
        text2 += edge_value[2] ;
        isComma=true;
      }
      else if( i==3 )
      {
        if(isComma)
        {
          text1 += ", ";
          text2 += ", ";
        }

        text1 += "North=" ;
        text1 += hdhC::double2String( hdhC::string2Double( row[8+add]), -5) ;
        text2 += "North=" ;
        text2 += edge_value[3] ;
        isComma=true;
      }

      is = true;
    }
  }

  if( is )
  {
    std::string key = "7_11";
    if( notes->inq(key, pQA->fileStr) )
    {
      std::string capt("unmatched CORDEX boundaries for ");
      capt += hdhC::sAssign("domain", tName) ;
      capt += ", found " + hdhC::sAssign("(file)", text2);
      capt += ", required " + hdhC::tf_val(text1);

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  return ;
}

void
QA_Exp::domainCheckDims(std::string item,
    std::string &t_num, std::string &f_name, std::string tbl_id)
{
  // compare lat/lon specified in CORDEX Table 1 or 2 with the dimensions
  // and return corresponding name.
  std::string f_num;

  // names of all dimensions
  std::vector<std::string> dNames(pQA->pIn->nc.getDimName() );

  for( size_t i=0 ; i < dNames.size() ; ++i )
  {
     int num = pQA->pIn->nc.getDimSize(dNames[i]);
     f_num = hdhC::itoa(num);

     if( f_num == t_num )
     {
        f_name = dNames[i] ;
        return;
     }
  }

  std::string key = "7_7 (";
  key += item;
  key += ")";
  if( notes->inq(key, pQA->fileStr) )
  {
    std::string capt("CORDEX domain Table ") ;
    capt += tbl_id ;
    capt += ": Value of ";
    capt += f_name ;
    capt += " does not match" ;

    capt += ", found " + hdhC::tf_val(f_num) ;
    capt += ", required: " + hdhC::tf_val(t_num);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
  }

  return ;
}

void
QA_Exp::domainCheckPole(std::string item,
    std::string &t_num, std::string &f_name)
{
  // compare lat/lon of N. Pole  specified in CORDEX Table 1 with
  // corresponding values in the NetCF file.

  if( item == "N.Pole lon" && t_num == "N/A" )
    return;  // any lon value would do for the identity of rotated and
             // unrotated North pole.

  // remove trailing zeros and a decimal point
  t_num = hdhC::double2String( hdhC::string2Double(t_num), -5) ;

  std::string f_num;

  // try for the assumption of variable name 'rotated_pole' with
  // attribute grid_north_pole_latitude / ..._longitude
  int ix;
  if( (ix=pQA->pIn->getVarIndex("rotated_pole")) > -1 )
  {
    Variable &var = pQA->pIn->variable[ix];

    if( item == "N.Pole lon" )
    {
       std::string s(var.getAttValue("grid_north_pole_longitude")) ;

       if( s.size() )
       {
         // found the attribute
         f_num = hdhC::double2String( hdhC::string2Double(s, -5) ) ;
         if( f_num == t_num )
           return;
       }
       else
       {
         s = var.getAttValue("grid_north_pole_latitude") ;
         if( s.size() )
         {
           // found the attribute
           f_num = hdhC::double2String( hdhC::string2Double(s, -5) ) ;
           if( f_num == t_num )
             return;
         }
       }
    }
  }

  // may-be the names didn't match. Try again more generally.
  if( ix > -1 )
  {
    std::string key = "7_8 (";
    key += item ;
    key += ")";
    if( notes->inq(key, pQA->fileStr) )
    {
      std::string capt("rotated N.Pole of CORDEX domain Table 1 does not match");
      capt += ", found " + hdhC::tf_val(f_num);
      capt += ", required " + hdhC::tf_val(t_num);

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }
  else
  {
    std::string key = "50_1 (";
    key += item ;
    key += ")";
    if( notes->inq(key, pQA->fileStr) )
    {
      std::string capt("auxiliary " + hdhC::tf_var("rotated_pole") );
      capt += "is missing in sub-temporal file" ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  return ;
}

void
QA_Exp::domainFindTableType(
    std::vector<std::vector<std::string> > &tbl1,
    std::vector<std::vector<std::string> > &tbl2,
    int &ix_1, int &ix_2)
{
   // CORDEX Area: table vs. file properties.
   // Return the index if it is unambiguous.
   std::string domName;

   // guess the format of the domain: rotated vs. back-rotated

   std::vector<std::string> candidate ;

   // a) the domain is given as attribute
   candidate.push_back( (pQA->pIn->getAttValue("CORDEX_domain")) ) ;

   // b) domain name from the filename, e.g. EUR-11
   Split fd(pQA->pIn->file.getFilename(), "_");
   if( fd.size() > 1 )
      candidate.push_back( fd[1] );
   else
      candidate.push_back("");

   // looking for a valid index in the two cases a) and/or b)
   int sz[2] ;
   sz[0] = static_cast<int>(tbl1.size());
   sz[1] = static_cast<int>(tbl2.size());

   int it_1[2] ; // index for the row of table 1; two cases
   int it_2[2] ;

   // don't confuse index of sz and it_1, it_2

   for( size_t i=0 ; i < 2 ; ++i )
   {
     for( it_1[i]=0 ; it_1[i] < sz[0] ; ++it_1[i] )
        if( candidate[i] == tbl1[it_1[i]][1] )
          break;

     if( it_1[i] == sz[0])  // no match found
     {
       it_1[i] = -1;

       // looking for a valid index in table2
       for( it_2[i]=0 ; it_2[i] < sz[1] ; ++it_2[i] )
          if( candidate[i] == tbl2[it_2[i]][1] )
            break;

       if( it_2[i] == sz[1])  // no match found
         it_2[i] = -1 ;
     }
     else
       it_2[i] = -1;
   }

   if( (it_1[0] == it_1[1]) && (it_2[0] == it_2[1] ) )
   {
      if( it_1[0] > -1 && it_2[0] == -1 )
      {
        ix_1 = it_1[0] ;
        ix_2 = -1;
        return  ; // found unambiguous name in table 1
      }

      if( it_1[0] == -1 && it_2[0] > -1 )
      {
        ix_1 = -1 ;
        ix_2 = it_2[0];
        return  ; // found unambiguous name in table 2
      }
   }

   ix_1 = -1 ;
   ix_2 = -1 ;

  return ;
}

void
QA_Exp::checkHeightValue(InFile &in)
{
   // Check near-surface height value between 0 - 10m
   // Note that a variable height may be available for something differnt,
   // e.g. TOA

   // find pattern 'near-surface' in the long_name. If long_name is missing,
   // then a false fault could be raised for height value

   int ix;
   if( (ix=pQA->pIn->getVarIndex("height")) == -1 )
     return;

   Variable &var = in.variable[ix];

   // check the units
   std::string units(var.getAttValue(pQA->n_units));

   if( units.size() == 0 )
     return;  // annotated elsewhere

   if( units != "m" )
     return;  // annotated elsewhere

   std::string longName(var.getAttValue(pQA->n_long_name));

   // try for a variable height and a variable with pattern nearXsurface
   // in lon_name where X means space, - or _
   Split x_longName;
   x_longName.setSeparator(" -_");
   x_longName = hdhC::Lower()(longName) ;

   bool is=false;
   for( size_t i=0 ; i < x_longName.size()-1 ; ++i )
   {
     if( x_longName[i] == "near" && x_longName[i+1] == "surface" )
     {
       is=true;
       break;
     }
   }

   if( is )
   {
     is=false;
     in.nc.getData(tmp_mv, var.name);
     if( tmp_mv.size() )
     {
       if( tmp_mv[0] < 0. || tmp_mv[0] > 10. )
         is=true ;
     }
     else
       is=true;

     if( is )
     {
       std::string key("5_6");
       if( notes->inq( key, var.name ) )
       {
         std::string capt(hdhC::tf_var("height") + "requires a value [0-10]m") ;
         if( tmp_mv.size() )
         {
           capt += ", found " ;
           capt += hdhC::tf_val( hdhC::double2String( tmp_mv[0]) ) ;
         }

         (void) notes->operate( capt) ;
         notes->setCheckMetaStr(pQA->fail);
       }
     }
   }

   return;
}

void
QA_Exp::checkMetaData(InFile &in)
{
  notes->setCheckMetaStr("PASS");

  // check attributes required in the meta data section of the file
  requiredAttributes_check(in) ;

  // check existance (and data) of the pressure coordinate for those
  // variables defined on a level indicated by a trailing number
  checkPressureCoord(in);

  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  std::vector<struct DimensionMetaData> dimNcMeDa;

  if(varReqTable.is )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i )
    {
      // Scan through the standard table.
      if( ! varMeDa[i].var->isExcluded )
        varReqTableCheck(in, varMeDa[i], dimNcMeDa) ;
    }
  }

  // inquire whether the meta-data checks passed
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    pQA->setExit( ev );

  return;
}

void
QA_Exp::checkVarTableEntry(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  // Do the variable properties found in the netCDF file
  // match those in the table (standard or project)?
  // Exceptions: priority is not defined in the netCDF file.

  // This has already been tested in the calling method
  // if( tbl_entry.name != varMeDa.name )

  checkVarTableEntry_standardName(vMD, tbl_entry);
  checkVarTableEntry_longName(vMD, tbl_entry);
  checkVarTableEntry_units(vMD, tbl_entry);
  checkVarTableEntry_cell_methods(vMD, tbl_entry);
  checkVarTableEntry_name(vMD, tbl_entry);

  return;
}

void
QA_Exp::checkVarTableEntry_cell_methods(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  // many CORDEX data are derived on the basis of cell_methods, given in a former
  // version of CORDEX_variables_requirements_table, which does not comply to CMIP5.
  // The current table provides two variants of cell_methods: the former and the CMIP5 alike
  // called 'cell_methods (2nd option)' in the caption of the table.

  // Rule: If a file is based on the former variant of cell_methods, then this is only checked.
  // Does the file contain more words than given in the former table, then the check
  // has to pass the CMIP5 compliant variant.

  // the direct comparison be the total string is discarded, because,
  // the 'time: point' possibiliity is checked below

  if( frequency == "fx" )
    return;

  bool isOld=false;

  // disjoint into words; this method ingnores spaces
  Split x_t_cm(tbl_entry.cellMethods);
  Split x_t_cmo(tbl_entry.cellMethodsOpt);
  Split x_f_cm;

  int cm_ix = vMD.var->getAttIndex(pQA->n_cell_methods) ;

  if( cm_ix > -1 && ( x_t_cm.size() || x_t_cmo.size() ) )
  {
    // the definition of cell_methods changed for some CORDEX variables.
    // The CORDEX ADD contains two definitions. The older ones has fewer terms.
    // Here, the one is taken which matches the cm attribute in the file.
    std::string& cm_name = vMD.var->attName[cm_ix] ;
    std::string& cm_val = vMD.var->attValue[cm_ix][0];

    x_f_cm = cm_val;

    if( x_f_cm.size() == x_t_cm.size() )
    {
      size_t i;
      for( i=0 ; i < x_t_cm.size() ; ++i )
        if( x_f_cm[i] != x_t_cm[i] )
          break;

      if( i == x_t_cm.size() )
        return; // identical
    }

    else if( x_f_cm.size() == x_t_cmo.size() )
    {
      size_t i;
      for( i=0 ; i < x_t_cmo.size() ; ++i )
        if( x_f_cm[i] != x_t_cmo[i] )
          break ;

      if( i == x_t_cmo.size() )
        return; // identical

      isOld=true;
    }

    // found a difference
    std::string key("32_6");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt;

      if( cm_val.size() )
      {
        capt = hdhC::tf_att(vMD.var->name, cm_name, cm_val) ;
        capt += "does not match " ;
        if( isOld )
          capt += hdhC::tf_val(tbl_entry.cellMethods) ;
        else
          capt += hdhC::tf_val(tbl_entry.cellMethodsOpt) ;
      }
      else
      {
        capt = hdhC::tf_att(vMD.var->name, cm_name) ;
        capt += "is missing";
      }

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( pQA->fail );
    }
  }

  return;
}

void
QA_Exp::checkVarTableEntry_longName(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  if( tbl_entry.longName == vMD.longName )
    return;

  // tolerate case differences
  std::string f( hdhC::Lower()(vMD.longName) );
  std::string t( hdhC::Lower()(tbl_entry.longName) );

  // case independence is accepted
  if( f == t )
    return;

  // long name is not ruled by conventions
  // Accepted variations:
  // a) '-' instead of ' '
  // b) ignore words (pre-positions) smaller than 3 character
  // c) one typo of two sequent characters per word in the long name components is tolerated
  // e) position of the words does not matter
  // d) one deviating word ok, if the number of requested words >= 3

  Split f_splt;
  Split t_splt;

  // split in a way that '-' is equivalent to a space
  f_splt.setSeparator(" -");
  t_splt.setSeparator(" -");

  t_splt = t ;
  f_splt = f ;

  size_t t_splt_sz=t_splt.size();
  size_t f_splt_sz=f_splt.size();

  int count_t=0;
  int count_f=0;

  // check words; position doesn't matter
  for( size_t i=0 ; i < t_splt_sz ; ++i )
  {
    if( t_splt[i].size() < 3 )
       continue;  // skip pre-positions

    ++count_t;

    bool is=true;
    for( size_t j=0 ; j < f_splt_sz ; ++j )
    {
      if( f_splt[j].size() < 3 )
         continue;  // skip pre-positions

      if( t_splt[i] == f_splt[j] )
      {
        ++count_f;
        is=false;
        break;
      }
    }

    if( is )  // check for one or two typos
    {
      std::string tt( t_splt[i] );
      size_t t_sz=tt.size() ;

      for( size_t j=0 ; j < f_splt_sz ; ++j )
      {
        std::string ff( f_splt[j] );
        size_t f_sz=ff.size() ;

        size_t c_left=0;
        size_t c_right=0;

        // the shorter one
        size_t fix=0;
        size_t tix=0;

        // check equality from the left
        while( fix < f_sz && tix < t_sz )
        {
           if( ff[fix++] == tt[tix++] )
             ++c_left;
           else
             break;
        }

        // check equality from the right
        fix=f_sz - 1 ;
        tix=t_sz - 1 ;
        while( fix >= 0 && tix >= 0 )
        {
           if( ff[fix--] == tt[tix--] )
             ++c_right;
           else
             break;
        }

        if( (c_left + c_right + 2) >= t_sz )
        {
          ++count_f;
          break;
        }
      }
    }
  }

  if( count_t > 3 && (count_t - count_f) < 2  )
     return;

  std::string key("32_3");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt;

    if( vMD.longName.size() )
    {
      capt = hdhC::tf_att(vMD.var->name, pQA->n_long_name, vMD.longName) ;
      capt += "does not match required value" ;
      capt += hdhC::tf_val(tbl_entry.longName);
    }
    else
    {
      capt = hdhC::tf_att(vMD.var->name, pQA->n_long_name) ;
      capt += "is missing required value" ;
      capt += hdhC::tf_val(tbl_entry.longName);
    }

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr( pQA->fail );
  }

  return;
}

void
QA_Exp::checkVarTableEntry_name(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  if( tbl_entry.name != vMD.var->name )
  {
    // This will happen only if case-insensitivity occurred
    std::string key("32_7");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(hdhC::tf_var(vMD.var->name, pQA->s_colon) ) ;
      capt += "output variable name matches only for case-insensitivity" ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( pQA->fail );
    }
  }

  return;
}

void
QA_Exp::checkVarTableEntry_standardName(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  if( tbl_entry.standardName != vMD.var->std_name
        && vMD.var->name != "evspsblpot" )
  {  // note that evspsblpot is not a standard_name definded by CF conventions
     // CORDEX_variables_requirement table
    std::string key("32_2");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt;

      if( vMD.var->std_name.size() )
      {
        capt += hdhC::tf_att(vMD.var->name, pQA->n_standard_name, vMD.var->std_name) ;
        capt += "does not match required " + hdhC::tf_val(tbl_entry.standardName);
      }
      else
      {
        capt += hdhC::tf_att(vMD.var->name, pQA->n_standard_name) ;
        capt += "is missing" ;
      }

      (void) notes->operate(capt) ;

      notes->setCheckMetaStr( pQA->fail );
    }
  }

  return ;
}

void
QA_Exp::checkVarTableEntry_units(
    VariableMetaData &vMD,
    VariableMetaData &tbl_entry)
{
  if( tbl_entry.units != vMD.var->units )
  {
      std::string key("32_5");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( hdhC::tf_att(vMD.var->name,
               pQA->n_units, vMD.var->units, pQA->no_blank) ) ;
        capt += ", but required is " ;
        capt += hdhC::tf_val(tbl_entry.units);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
  }

  return;
}

void
QA_Exp::createVarMetaData(void)
{
  // sub table name, i.e. frequency, has previously been checked

  // create instances of VariableMetaData. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in vector in.dataVarIndex.

  size_t i;
  for( i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    //push next instance
    pushBackVarMeDa( &var );

    VariableMetaData &vMD = varMeDa.back() ;

    for( size_t k=0; k < var.dimName.size() ; ++k)
    {
      int sz;
      if( (sz=pQA->pIn->nc.getDimSize( var.dimName[k] )) == 1 )
      {
        std::string key("41");
        if( notes->inq( key, var.name) )
        {
          std::string capt(hdhC::tf_var(var.getDimNameStr(true), pQA->s_colon));
          capt += "CORDEX favours a scalar variable for dimension " ;

          for( size_t l=0 ; l < var.dimName.size() ; ++l )
          {
             if( pQA->pIn->nc.getDimSize(var.dimName[l]) == 1 )
             {
               capt += var.dimName[l] ;
               capt += "=1";
               break;
             }
          }

          (void) notes->operate(capt) ;
          notes->setCheckMetaStr( pQA->fail );
        }
      }
    }

    // initially set false; will change later for attributes
    // requested in the standard table.
//    for( size_t k=0 ; k < var.attName.size() ; ++k )
//      varMeDa.back().isInStandardTable.push_back( false ) ;

    // some more properties
    vMD.longName     = var.getAttValue(pQA->n_long_name) ;
    vMD.positive     = var.getAttValue(pQA->n_positive) ;
  }

  // Check varname from filename with those in the file.
  // Is the shortname in the filename also defined in the nc-header?
  for( i=0 ; i < pQA->pIn->varSz ; ++i )
    if( fVarname == pQA->pIn->variable[i].name )
      break;

  if( i == pQA->pIn->varSz )
  {
     std::string key("15_3");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("variable ");
       capt += hdhC::sAssign("acronym", fVarname);
       capt += " in the filename does not match any variable in the file" ;

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( pQA->fail );
     }
  }

  return;
}

bool
QA_Exp::findTableEntry(ReadLine &ifs, std::string &name_f,
     size_t col_outName, std::string &str0 )
{
   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   std::string name_t;

   do
   {
     splt_line=str0;

     if( splt_line.size() > 1 && splt_line[1] == "Table:" )
       return false;

     name_t = splt_line[col_outName] ;

     if( isCaseInsensitiveVarName )
       (void) hdhC::Lower()(name_t, true);

     if( name_t == name_f )
       return true;
   }
   while( ! ifs.getLine(str0) ) ;

   // netCDF variable not found in the table
   return false;
}

bool
QA_Exp::findTableEntry(ReadLine &ifs, std::string &name_f,
   VariableMetaData &tbl_entry)
{
   // return true: entry is not the one we look for.

   std::map<std::string, size_t> col;

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   std::string name_t;
   std::string str0;

   // a very specific exception: convert to lower case
   if( isCaseInsensitiveVarName )
      (void) hdhC::Lower()(name_f, true);

   bool isFound=false;

   std::vector<std::string> vs_freq;
   vs_freq.push_back(frequency);
   vs_freq.push_back("all");

   bool isRewind=true;

   for(size_t i=0 ; i < vs_freq.size() ; ++i )
   {
     if( readTableCaptions(ifs, vs_freq[i], col, str0) )
     {
       if( (isFound = findTableEntry(ifs, name_f, col["outVarName"], str0) ) )
       {
         splt_line = str0 ;

         // delete footnotes from the table entries
         std::string str1;
         for(size_t j=0 ; j < splt_line.size() ; ++j )
         {
            std::string &s = splt_line[j] ;
            size_t sz = s.size();
            if( sz && s[sz-1] == ')' && isdigit(s[sz-2]) )
            {
              --sz;
              while( sz && isdigit(s[sz-1]) )
                --sz;
            }

            str1 += s.substr(0, sz) ;
            str1 += ',' ;
         }

         splt_line = str1;
         // found an entry and the shape matched.
         if( tbl_entry.name.size() == 0 &&
                col.find("outVarName") != col.end() )
             tbl_entry.name = splt_line[ col["outVarName"] ];

         if( tbl_entry.standardName.size() == 0 &&
                col.find(pQA->n_standard_name) != col.end() )
             tbl_entry.standardName = splt_line[ col[pQA->n_standard_name] ];

         if( tbl_entry.longName.size() == 0 &&
                col.find(pQA->n_long_name) != col.end() )
         {
             tbl_entry.longName = splt_line[ col[pQA->n_long_name] ];
             tbl_entry.longName
                = hdhC::clearInternalMultipleSpaces(tbl_entry.longName);
         }

         if( tbl_entry.units.size() == 0 &&
                col.find(pQA->n_units) != col.end() )
         {
             tbl_entry.units = splt_line[ col[pQA->n_units] ];
             tbl_entry.units
                = hdhC::clearInternalMultipleSpaces(tbl_entry.units);

             if( tbl_entry.units.size() )
               tbl_entry.isUnitsDefined=true;
             else
               tbl_entry.isUnitsDefined=false;
         }

         if( tbl_entry.positive.size() == 0 &&
                col.find(pQA->n_positive) != col.end() )
                   tbl_entry.positive = splt_line[ col[pQA->n_positive] ];

         if( tbl_entry.cellMethods.size() == 0 &&
                col.find(pQA->n_cell_methods) != col.end() )
                   tbl_entry.cellMethods = splt_line[ col[pQA->n_cell_methods] ];

         if( tbl_entry.cellMethodsOpt.size() == 0 &&
                col.find("cell_methods_opt") != col.end() )
                   tbl_entry.cellMethodsOpt = splt_line[ col["cell_methods_opt"] ];
       }
       else
         return isFound ; //== false
     }
     else
     {
       // if 'all' precedes frequency
       if( isRewind )
       {
          // do it again
          ifs.rewind();
          --i;
          isRewind=false;
       }
     }
   }

   return isFound;
}

std::string
QA_Exp::getCaptIntroDim(VariableMetaData &vMD,
                   struct DimensionMetaData &nc_entry,
                   struct DimensionMetaData &tbl_entry,
                   std::string att )
{
  std::string intro("var=");
  intro += vMD.var->name + ", dim=";
  intro += nc_entry.outname ;

  if( tbl_entry.outname == "basin" )
    intro += ", var-rep=region";
  if( tbl_entry.outname == "line" )
    intro += ", var-rep=passage";
  if( tbl_entry.outname == "type" )
    intro += ", var-rep=type_description";

  if( att.size() )
  {
    intro += "att=" ;
    intro += att + ", " ;
  }

  return intro;
}

bool
QA_Exp::getDimMetaData(InFile &in,
      VariableMetaData &vMD,
      struct DimensionMetaData &dimMeDa,
      std::string &dName)
{
  // return 0:
  // collect dimensional meta-data in the struct.

  // Note: this method is called from two different spots, from
  // a standard table check and for project purposes.

  // pre-set
  dimMeDa.checksum=0;
  dimMeDa.outname=dName;

  // dName is a dimension name from the table. Is it also
  // a dimension in the ncFile? A size of -1 indicates: no
  int sz = in.nc.getDimSize(dName);
  if( sz == -1 )
     return true;
  dimMeDa.size = static_cast<size_t>(sz);

  // regular: variable representation of the dim
  // except.: variable specified in coords_attr and not a regular case

  // is dimension name also variable name?
  if( in.nc.getVarID( dName ) == -2 )
     return true;  // nothing was found

  // attributes from the file
  for(size_t l=0 ; l < vMD.var->dim_ix.size() ; ++l)
  {
    Variable &var = pQA->pIn->variable[vMD.var->dim_ix[l]];

    if( var.name != dName )
       continue;

    for( size_t j=0 ; j < var.attName.size() ; ++j)
    {
      std::string &aN = var.attName[j] ;
      std::string &aV = var.attValue[j][0] ;

      if( aN == "bounds" )
          dimMeDa.bounds = aV ;
      else if( aN == "climatology" && aN == "time" )
          dimMeDa.bounds = aV ;
      else if( aN == pQA->n_units )
      {
          if( var.isUnitsDefined )
          {
            dimMeDa.units = aV ;
            dimMeDa.isUnitsDefined=true;
          }
          else
            dimMeDa.isUnitsDefined=false;
      }
      else if( aN == pQA->n_long_name )
          dimMeDa.longname = aV ;
      else if( aN == pQA->n_standard_name )
          dimMeDa.stndname = aV ;
      else if( aN == pQA->n_axis )
          dimMeDa.axis = aV ;
    }
  }

  // determine the checksum of limited var-presentations of dim
  if( ! in.nc.isDimUnlimited(dName) )
  {
    std::vector<std::string> vs;

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

  if( dName == pQA->qaTime.name )
  {
    // exclude time from size
    dimMeDa.size = 0;

  }

  return false;
}

std::string
QA_Exp::getFrequency(void)
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pQA->pIn->nc.getAttString("frequency") ;

  if( ! frequency.size() )
  {
    // try the filename
    std::string f( pQA->pIn->file.basename );

    Split splt(f, "_");

    // test the last two items for time type. If both are,
    // then this would mean that they are separated by '_'.
    // This would be a fault for CORDEX.
    size_t off=0;  // take into account a period separator '_'
    if( splt.size() > 2 &&
           hdhC::isDigit( splt[ splt.size() -1 ])
             && hdhC::isDigit( splt[ splt.size() -2 ]) )
      off=1;

    if( splt.size() > 7 )
    {
      if( splt.size() == (9+off) )
        frequency = splt[7] ;
      else if( splt.size() == (8+off) )
      {
         if( hdhC::isDigit( splt[7][0] ) )
           // with period; no RCMVersionID
           frequency = splt[6] ;
         else
           // no period; with RCMVersionID
           frequency = splt[7] ;
      }
      else if( splt.size() == (7+off) )
         // no period; no RCMVersionID
         frequency = splt[6] ;
    }
  }

  if( frequency == "fx" && pQA->pIn->nc.isDimUnlimited() )
  {
     std::string key("17");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt(hdhC::sAssign("frequency","fx") + " with time dependency" );

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
        }
     }
  }

  return frequency ;
}

void
QA_Exp::getSubTable(void)
{
  if( subTable.size() )
    return ;  // already checked

  // This is CORDEX specific; taken directly from the standard table.
  std::vector<std::string> sTables;

  sTables.push_back("3hr");
  sTables.push_back("6hr");
  sTables.push_back("day");
  sTables.push_back("mon");
  sTables.push_back("sem");
  sTables.push_back("fx");

  // the counter-parts in the attributes

  // actually, there are no standard table names specified in the
  // original tables, but frequencies are embedded in the caption
  std::string sTable( getFrequency() );

  //check for valid names
  bool is=true;
  for( size_t i=0 ; i < sTables.size() ; ++i )
  {
    if( sTables[i] == sTable )
    {
      is=false ;
      break;
    }
  }

  if( is )
  {
     std::string key("7_5");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt(hdhC::sAssign("frequency","fx"));
       capt += " not found in the CORDEX_variables_requirement table" ;

       subTable.clear();

       if( notes->operate(capt) )
       {
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
        }
     }
  }

  subTable=sTable;
  return ;
}

std::string
QA_Exp::getTableEntryID(std::string vName)
{
  vName += "," ;
  vName += getFrequency() ;
  if( !isRotated )
    vName += "_i" ;

  return vName + ",";
}

std::string
QA_Exp::getVarnameFromFilename(void)
{
  if( fVarname.size() )
    return fVarname;

  return getVarnameFromFilename(pQA->pIn->file.getFilename()) ;
}

std::string
QA_Exp::getVarnameFromFilename(std::string fName)
{
  size_t pos;
  if( (pos = fName.find("_")) < std::string::npos )
    fName = fName.substr(0,pos) ;

  return fName;
}

void
QA_Exp::initDataOutputBuffer(void)
{
    for( size_t i=0 ; i < varMeDa.size() ; ++i)
      varMeDa[i].qaData.initBuffer(pQA, pQA->currQARec, bufferSize);

  return;
}

void
QA_Exp::initDefaults(void)
{
  isCaseInsensitiveVarName=false;
  isCheckParentExpID=true;
  isCheckParentExpRIP=true;
  isRotated=true;
  isUseStrict=false;

  bufferSize=1500;

  return;
}

void
QA_Exp::initResumeSession(std::vector<std::string>& prevTargets)
{
  // a missing variable?
  for( size_t i=0 ; i < prevTargets.size() ; ++i)
  {
    size_t j;
    for( j=0 ; j < varMeDa.size() ; ++j)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("33a");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt(hdhC::tf_var(prevTargets[i]));
         capt +=  + "is missing in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
           pQA->setExit( notes->getExitValue() ) ;
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
       std::string key("33b");
       if( notes->inq( key, varMeDa[j].var->name) )
       {
         std::string capt(hdhC::tf_var(varMeDa[j].var->name));
         capt += "is new in sub-temporal file" ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
           pQA->setExit( notes->getExitValue() ) ;
         }
       }
    }
  }

  return;
}

bool
QA_Exp::inqTables(void)
{
  bool ret=false;
  std::vector<std::string*> pPath;

  if( ! varReqTable.isExisting(varReqTable.path) )
    ret=true;
  else
    pQA->setTable( varReqTable.filename, "ST" );

  if( ! ifs_DRS_CV.open(table_DRS_CV.getFile()) )
    table_DRS_CV.is=false;

  for( size_t i=0 ; i <  pPath.size() ; ++i)
  {
     std::string key("7_1");
     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("no path to the tables, tried " + *(pPath[i])) ;

        if( notes->operate(capt) )
        {
          notes->setCheckMetaStr( pQA->fail );
          pQA->setExit( notes->getExitValue() ) ;
        }
     }
  }

  return ret;
}

void
QA_Exp::pushBackVarMeDa(Variable *var)
{
   varMeDa.push_back( VariableMetaData(pQA, var) );

   if( var )
   {
     VariableMetaData &vMD = varMeDa.back();

     vMD.forkAnnotation(notes);

     // disable tests by given options
     vMD.qaData.disableTests(var->name);

     vMD.qaData.init(pQA, var->name);
   }

   return;
}

bool
QA_Exp::readTableCaptions(ReadLine &ifs, std::string freq,
   std::map<std::string, size_t> &v_col, std::string &str0 )
{
   // Each sub table is indicted by "Table:" in the second column and
   // the frequency in the third.

   // Each csv between # and a pure integer indicates a caption of a column.
   // Captions may span several lines, but not corresponding table entries.

   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   std::string sx;

   // find the begin of the sub table
   while( ! ifs.getLine(str0) )
   {
     splt_line=str0;

     if( splt_line[1] == "Table:" )
     {
       // there are two tables of different format; the only difference
       // in parsing should be given here.
       str0 = hdhC::stripSurrounding(splt_line[2], "()") ;

       if( str0 == freq )
       {
         while( ! ifs.getLine(str0) )
         {
           sx = hdhC::clearChars(str0, " ") ;
           if( sx.find("outputvariablename") < std::string::npos )
             goto BREAK2; // positioned to the beginning of a caption
         }
       }
     }
   }

BREAK2:

   // sub-table not found?
   if( ifs.eof() )
     return false;

   splt_line = str0;

   while( ! ifs.getLine(str0) )
   {
     // find the first line after a (multiple) header-line
     size_t p0=str0.size();
     size_t p;
     if( (p=str0.find(',')) < std::string::npos )
       p0=p;

     if( hdhC::isAlpha( str0.substr(0,p0) ) )
         break;

     splt_line += str0;
   }

   std::vector<std::string> vs_ix;
   vs_ix.push_back( "outVarName" );
   vs_ix.push_back( pQA->n_cell_methods );
   vs_ix.push_back( pQA->n_cell_methods + "_opt" );
   vs_ix.push_back( pQA->n_long_name );
   vs_ix.push_back( pQA->n_standard_name );
   vs_ix.push_back( pQA->n_units );
   vs_ix.push_back( pQA->n_positive );

   std::vector<std::string> vs_val;
   vs_val.push_back( pQA->n_outputVarName );
   vs_val.push_back( pQA->n_cell_methods );
   vs_val.push_back( pQA->n_cell_methods + " (2nd option)" );
   vs_val.push_back( pQA->n_long_name );
   vs_val.push_back( pQA->n_standard_name );
   vs_val.push_back( pQA->n_units );
   vs_val.push_back( pQA->n_positive );

   std::string t;

   // Now, look for the indexes of the variable's columns
   for( size_t c=0 ; c < splt_line.size() ; ++c)
   {
     if( splt_line[c].substr(0,3) == "frq" || splt_line[c] == "ag" )
       continue;

     t = hdhC::clearInternalMultipleSpaces(splt_line[c]);
     // some user like to set a trailing ':' randomly
     if ( t[ t.size()-1 ] == ':' )
       t = t.substr(0,t.size()-1) ;

     for( size_t i=0 ; i < vs_ix.size() ; ++i )
     {
       if( t == vs_val[i] && v_col.find( vs_ix[i] ) == v_col.end() )
       {
           v_col[ vs_ix[i] ] = c;
           break;
       }
     }
   }

   return true;
}

void
QA_Exp::requiredAttributes_check(InFile &in)
{
  std::vector<std::vector<std::string> > reqA ;
  std::vector<std::string> reqVname ;

  requiredAttributes_readFile( reqVname, reqA );

  // check required attributes of variables, which could
  // be given or not
  for(size_t i=0 ; i < reqVname.size() ; ++i)
  {
    for( size_t j=0 ; j < in.varSz ; ++j )
    {
      // global attributes
      if( reqVname[i] == "global"
             && in.variable[j].name == "NC_GLOBAL" )
      {
         requiredAttributes_checkGlobal(in, reqA[i]);
         break;
      }

      //check for required attributes of an existing variable
      if( reqVname[i] == in.variable[j].name )
      {
         requiredAttributes_checkVariable(in, in.variable[j], reqA[i]);
         break;
      }
    }
  }

  // check attribute _FillValue and missing_value, if available
  std::string fV("_FillValue");
  std::string mV("missing_value");

  std::vector<std::string> fVmv;
  fVmv.push_back(fV);
  fVmv.push_back(mV);

  for( size_t j=0 ; j < in.varSz ; ++j )
  {
    Variable& var = in.variable[j] ;

    for( size_t l=0 ; l < 2 ; ++l )
    {
      int k;
      if( (k=var.getAttIndex(fVmv[l])) > -1 )
      {
        std::vector<float> aV;
        in.nc.getAttValues(aV, fVmv[l], var.name ) ;
        float rV = 1.E20 ;

        if( aV.size() == 0 || aV[0] != rV )
        {
          std::string key("34");

          if( notes->inq( key, var.name) )
          {
            std::string capt(
              hdhC::tf_att(var.name, fVmv[l], var.attValue[j][0])) ;
            capt += "does not match required value=1.E20";

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( pQA->fail );
          }
        }
      }
    }
  }

  // both _FillValue and missing_value must be defined in CORDEX
  for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    bool is_fV = var.isValidAtt(fV) ;
    bool is_mV = var.isValidAtt(mV) ;

    if( (is_fV || is_mV) && (is_fV != is_mV) )
    {
      std::string key("34b");

      if( notes->inq( key, var.name) )
      {
        std::string capt(hdhC::tf_var(var.name, pQA->s_colon)) ;
        capt += "if " + hdhC::tf_att(fV);
        capt += "or " + mV ;
        capt += ", then both must be defined";

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }

  return ;
}

void
QA_Exp::requiredAttributes_checkCloudVariableValues(InFile &in,
      std::string &auxName, std::string &reqA)
{
  // reqA  := vector of required: att_name=att_value
  // vName := name of an existing variable (already tested)
  // auxName := name of an existing auxiliary (already tested)

  Split reqValues;
  reqValues.setSeparator('|');
  reqValues=reqA ;

  std::vector<int> iReqValues;

  // check required values.

  MtrxArr<double> fValues;
  bool isErr=true;

  int index=-1;
  if( fVarname == "clh" )
    index=0;
  else if( fVarname == "clm" )
    index=1;
  else if( fVarname == "cll" )
    index=2;

  if( auxName == "plev" )
  {
    in.nc.getData(fValues, "plev" );
    int iV = static_cast<int>( fValues[0] );
    iReqValues.push_back( iV );

    if( fValues.size() == 1 && iV == reqValues.toInt(index))
      isErr=false;
  }

  if( auxName == "plev_bnds" )
  {
    in.nc.getData(fValues, "plev_bnds" );
    int iV0, iV1;

    if( fValues.size() == 2 )
    {
       Split rV;
       rV.setSeparator(',');
       rV = reqValues[index] ;
       int iRV0 = rV.toInt(0) ;
       int iRV1 = rV.toInt(1) ;

       // swap, if necessary
       int tmp;
       if( iRV0 < iRV1 )
       {
          tmp=iRV0;
          iRV0=iRV1;
          iRV1=tmp;
       }

       iReqValues.push_back( iRV0 );
       iReqValues.push_back( iRV1 );

       iV0 = static_cast<int>( fValues[0] );
       iV1 = static_cast<int>( fValues[1] );

       if( iV0 == iRV0 && iV1 == iRV1 )
         isErr=false;
    }
  }

  if( isErr )
  {
    std::string key("5_1");
    if( notes->inq( key, auxName) )
    {
       std::string capt( "auxiliary ");
       capt += hdhC::tf_var(auxName + " for cloud amounts") ;
       capt += " no match to required value";
       if( iReqValues.size() != 1 )
         capt += "s" ;
       capt += "=";
       capt += reqValues[index] ;

       std::ostringstream ostr(std::ios::app);

       if( iReqValues.size() == 1 )
       {
         ostr << "\n" << auxName << " value (required)=" << iReqValues[0] ;
         ostr << "\n" << auxName << " value (file)=" << fValues[0] ;
       }
       else if( iReqValues.size() == 2 )
       {
         ostr << "\n" << auxName << " values (required)=  [ " << iReqValues[0] ;
         ostr << ", " << iReqValues[1] << " ]" ;
         ostr << "\n" << auxName << " values (file)=[ " << fValues[0] ;
         ostr << ", " << fValues[1] << " ]" ;
       }

       (void) notes->operate(capt, ostr.str()) ;
       notes->setCheckMetaStr( pQA->fail );
     }
  }

  return;
}

void
QA_Exp::requiredAttributes_checkGlobal(InFile &in,
     std::vector<std::string> &reqA)
{
  // reqA  := vector of required: att_name=att_value

  Split x_reqA;
  x_reqA.setSeparator('=');

  // it is clear that global attributes exist
  Variable &glob = pQA->pIn->variable[pQA->pIn->varSz];

  // check required attributes
  // note: first item is the variable name itself
  for( size_t k=0 ; k < reqA.size() ; ++k )
  {
    x_reqA=reqA[k] ;  // split name and value of attribute

    // missing required global attribute
    if( ! glob.isValidAtt(x_reqA[0]) )
    {
       std::string key("2_6");

       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("required global " + hdhC::tf_att(x_reqA[0]));
         capt += "is missing" ;

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr( pQA->fail );

         continue;
       }
    }

    // required attribute expects a required value
    if( x_reqA.size() == 2 )  // a value is required
    {
      std::string aV = glob.getAttValue(x_reqA[0]) ;

      if( aV.size() == 0 )
      {
        std::string key("2_7");
        if( notes->inq( key, pQA->fileStr) )
        {
           std::string capt("global attribute=");
           capt += x_reqA[0];
           capt=" missing required value=";
           capt += x_reqA[1];

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }

       // there is a value. is it the required one?
       else if( aV != x_reqA[1] )
       {
         std::string key("2_8");
         if( notes->inq( key, pQA->fileStr) )
         {
           std::string capt("global " + hdhC::tf_att(pQA->s_empty, x_reqA[0], aV));
           capt="does not match required value=" + x_reqA[1] ;

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }
       break;
    }

  } // end of for-loop

  return;
}

void
QA_Exp::requiredAttributes_checkVariable(InFile &in,
     Variable &var, std::vector<std::string> &reqA)
{
  // reqA  := vector of required: att_name=att_value
  // vName := name of an existing variable (already tested)
  // ix    := index of in.variable[ix]

  std::string &vName = var.name;

  Split x_reqA;
  x_reqA.setSeparator('=');

  // special: is the variable == plev_bnds; i.e. for cloud amounts?
  bool isCloudAmount=false;
  if( in.getVarIndex("cll") > -1
        || in.getVarIndex("clm") > -1
           || in.getVarIndex("clh") > -1 )
    isCloudAmount=true;

  // find required attributes missing in the file
  // note: first item is the variable name itself
  for( size_t k=0 ; k < reqA.size() ; ++k )
  {
    x_reqA=reqA[k] ;  // split name and value of attribute

    // find corresponding att_name of given vName in the file
    int jx=var.getAttIndex(x_reqA[0]);

    // special: values may be available for cloud amounts

    // missing required attribute
    if( jx == -1 && x_reqA[0] != "values" )
    {
       std::string key("2_1");

       if( notes->inq( key, vName) )
       {
         std::string capt(hdhC::tf_att(vName,x_reqA[0]));
         capt += "is missing";

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr( pQA->fail );

         continue;
       }
    }

    // special: for cloud amounts
    if( isCloudAmount )
    {
      // check for attribute: plev_bnds
      std::string plev("plev");

      if( vName == plev )
      {
         if( !var.isValidAtt("bounds") )
         {
           std::string key("2_11");

           if( notes->inq( key, vName) )
           {
             std::string capt("auxiliary ");
             capt += hdhC::tf_var(plev + " for cloud amounts", pQA->s_colon);
             capt += "missing " + hdhC::tf_att("plev_bnds");

             (void) notes->operate(capt) ;
             notes->setCheckMetaStr( pQA->fail );
           }
         }

         // check for auxiliary: plev_bnds
         if( ! in.getVarIndex("plev_bnds") )
         {
           std::string key("5_2");

           if( notes->inq( key, vName) )
           {
             std::string capt("auxiliary ");
             capt += hdhC::tf_var("plev_bnds") ;
             capt += "for cloud amounts is missing";

             (void) notes->operate(capt) ;
             notes->setCheckMetaStr( pQA->fail );
           }
         }
       }

       if( x_reqA[0] == "values" )
       {
          requiredAttributes_checkCloudVariableValues( in, vName, x_reqA[1]);

          // word 'values' is only for cloud amount case.
          continue;
       }
    }

    std::string &aN = var.attName[jx] ;

    // required attribute expects a value (not for cloud amounts)
    if( x_reqA.size() == 2 )  // a value is required
    {
      if( ! isCloudAmount && x_reqA[0] == "values" )
	       continue;

      std::string aV( var.attValue[jx][0] ) ;
      for( size_t i=1 ; i < var.attValue[jx].size() ; ++i )
      {
        aV += " " ;
        aV += var.attValue[jx][i] ;
      }

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, vName) )
        {
           std::string capt(hdhC::tf_att(vName, aN, pQA->s_colon));
           capt="missing required value=" ;
           capt += x_reqA[1];

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }

       // there is a value. is it the required one?
       else if( aV != x_reqA[1] )
       {
         bool is=true;
         if( x_reqA[0] == pQA->n_long_name)
         {
           // mismatch tolerated, because the table does not agree with CMIP5
           if( x_reqA[1] == "pressure" && aV == "pressure level" )
             is=false;
           if( x_reqA[1] == "pressure level" && aV == "pressure" )
             is=false;
         }
         else if( (x_reqA[0] == pQA->n_positive) || (x_reqA[0] == pQA->n_axis) )
         {
           // case insensitive
           if( x_reqA[1] == hdhC::Lower()(aV) )
             is=false;
         }

         std::string key("2_3");
         if( is &&  notes->inq( key, vName ) )
         {
           std::string capt(hdhC::tf_att(vName, aN, aV));
           capt += "does not match required value=" ;
           capt += x_reqA[1];

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }
    }
  } // end of for-loop

  return;
}

void
QA_Exp::requiredAttributes_readFile(
    std::vector<std::string> &reqVname,
    std::vector<std::vector<std::string> > &reqA)
{
   if( ! ifs_DRS_CV.isOpen() )
   {
      std::string key("7_2") ;
      if( notes->inq( key, pQA->fileStr) )
      {
         std::string capt("could not open Table_DRS_CV, tried") ;
         capt += hdhC::tf_val(table_DRS_CV.filename) ;

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );
           pQA->setExit( notes->getExitValue() ) ;
         }
      }
   }

   std::string s0;
   std::vector<std::string> vs;

   // parse table; trailing ':' indicates variable or 'global'
   ifs_DRS_CV.skipWhiteLines();
   ifs_DRS_CV.skipBashComment();
   ifs_DRS_CV.clearSurroundingSpaces();

   while( ! ifs_DRS_CV.getLine(s0) )
   {
     if( s0.size() == 0 )
       continue;

     if( s0 == "Begin: Table1" )
     {
        // check all domain related properties
        // and leave boolean 'isRotated'; even in the case
        // that the model is a 'non-rotational' one.
        domainCheck(ifs_DRS_CV);
        break;
     }

     // special: accept single ':' or "global"
     if( s0[0] == ':' || s0 == "global" )
       s0 = "global:";

     size_t sz= s0.size();

     // for the next variable
     if( s0[sz-1] == ':' )
     {
       reqA.push_back( vs ) ;  // add empty vector
       reqVname.push_back( s0.substr(0,sz-1) );

       continue;
     }

     // attributes
     // any colon as sep between varName and attName
/*
     size_t pos;
     if( (pos=so.find(':')) < std::string::npos )
     {
       std::string t0(s0.substr(0,pos);
       if( t0 != vvs.back() )
          ; // error
     }
*/
     reqA.back().push_back( s0 );
   }

   return;
}

void
QA_Exp::run(std::vector<std::string>& optStr)
{
   // apply parsed command-line args
   applyOptions(optStr);

   fVarname = getVarnameFromFilename();
   getFrequency();
   getSubTable() ;

   bool isNoTable = inqTables() ;

   if( table_DRS_CV.is )
   {
     DRS_Filename drsFN(pQA, optStr);
     drsFN.run();
   }

   // Create and set VarMetaData objects.
   createVarMetaData() ;

   if( !isNoTable )
   {
      // get meta data from file and compare with tables
      checkMetaData(*(pQA->pIn));
   }

   return ;
}

void
QA_Exp::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

void
QA_Exp::varReqTableCheck(InFile &in, VariableMetaData &vMD,
             std::vector<struct DimensionMetaData> &dimNcMeDa)
{
   // scanning the standard table.

//   std::fstream ifs(str0.c_str(), std::ios::in);
   // This class provides the feature of putting back an entire line
   ReadLine ifs(varReqTable.getFile());

   if( ! ifs.isOpen() )
   {
      std::string key("7_3") ;
      if( notes->inq( key, vMD.var->name) )
      {
         std::string capt("could not open the CORDEX_variables_requirement table, tried") ;
         capt += hdhC::tf_val(varReqTable.getFile());

         if( notes->operate(capt) )
         {
           notes->setCheckMetaStr( pQA->fail );

           pQA->setExit( notes->getExitValue() ) ;
         }
      }
   }

   // remove all " from input
   ifs.skipCharacter('"');

   VariableMetaData tbl_entry(pQA);

   // find the sub table, corresponding to the frequency column
   // try to identify the name of the sub table in str0,
   // return true, if not found
//   if( ! find_CORDEX_SubTable(ifs, str0, vMD) )
//     return false;

   // find entry for the required variable
   if( findTableEntry(ifs, vMD.var->name, tbl_entry) )
   {
     // netCDF properties are compared to those in the table.
     // Exit in case of any difference.
     checkVarTableEntry(vMD, tbl_entry);

     // a specifi check for the value of height if available
     checkHeightValue(in);

     // get meta-data of var-reps of dimensions from nc-file
     std::map<std::string, size_t> d_col;

     for(size_t l=0 ; l < vMD.var->dim_ix.size() ; ++l)
     {
       if( pQA->pIn->variable[ vMD.var->dim_ix[l] ].dimSize == 1 )
          continue;

       // new instance
       dimNcMeDa.push_back( DimensionMetaData() );

       // the spot where meta-data of var-reps of dims is taken
       // note: CORDEX standard tables doesn't provide anything, but
       // it is necessary for writing the project table.
       getDimMetaData(in, vMD, dimNcMeDa.back(),
                pQA->pIn->variable[ vMD.var->dim_ix[l] ].name) ;
     }

     return;
   }

   // no match
   std::string key("3_1") ;
   if( notes->inq( key, vMD.var->name) )
   {
     std::string capt(hdhC::tf_var(vMD.var->name + " for "
        + hdhC::sAssign("frequency", getFrequency()), pQA->s_colon)) ;
     capt += "Not found in the CORDEX_variables_requirement table";

     (void) notes->operate(capt) ;
     notes->setCheckMetaStr( pQA->fail );
   }

   // variable not found in the standard table.
   return;
}



VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;
   var = v;

   isForkedAnnotation=false;
}

VariableMetaData::~VariableMetaData()
{
  dataOutputBuffer.clear();
}

void
VariableMetaData::verifyPercent(void)
{
   // % range
   if( var->units == "%" )
   {
      if( qaData.statMin.getSampleMin() >= 0.
           && qaData.statMax.getSampleMax() <= 1. )
      {
        std::string key("6_8");
        if( notes->inq( key, var->name) )
        {
          std::string capt( hdhC::tf_var(var->name, ":"));
          capt += "Suspicion of fractional data range for units [%], found range ";
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckMetaStr( pQA->fail );
        }
      }
   }

   if( var->units == "1" || var->units.size() == 0 )
   {
      // is it % range? Not all cases are detectable
      if( qaData.statMin.getSampleMin() >= 0. &&
           qaData.statMax.getSampleMax() > 1.
             && qaData.statMax.getSampleMax() <= 100.)
      {
        std::string key("6_9");
        if( notes->inq( key, var->name) )
        {
          std::string capt( "Suspicion of percentage data range for units <1>, found range " ) ;
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckMetaStr( pQA->fail );
        }
      }
   }

   return;
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
