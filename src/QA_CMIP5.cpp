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
   }

   return;
}

void
DRS_Filename::checkFilename(void)
{
  checkFilenameEncoding();

  if( testPeriod() && pQA->qaExp.getFrequency() != "fx" )
  {
    // no period in the filename
     std::string key("1_13");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("filename requires a period") ;

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr(pQA->fail);
     }
  }

  return;
}

void
DRS_Filename::checkFilenameEncoding(void)
{
  // The items of the filename must match corresponding global attributes.
  // A filename is constructed by global attributes.

  bool isGridSpec = false;
  if( pQA->pIn->file.basename.substr(0,9) == "gridspec_" )
    isGridSpec=true;

  Split x_filename;
  x_filename.setSeparator("_");
  x_filename.enableEmptyItems();
  x_filename = pQA->pIn->file.basename ;

  std::vector<std::string> vs_gaFilename ;
  vs_gaFilename.push_back(x_filename[0]);  // first item

  size_t g_ix = pQA->pIn->varSz ;  // index to the global atts
  int ix;

  if( isGridSpec )
  {
    if( (ix=pQA->pIn->variable[g_ix].getAttIndex("modeling_realm")) == -1 )
      vs_gaFilename.push_back("");
    else
      vs_gaFilename.push_back(pQA->pIn->variable[g_ix].attValue[ix][0]);
  }
  else
  {
    checkMIP_tableName(x_filename) ;
    vs_gaFilename.push_back(pQA->qaExp.currMIP_tableName) ;  // could be empty or from filename

    vs_gaFilename.push_back( pQA->qaExp.getFrequency() );
    if( pQA->qaExp.getFrequency() != "fx" )
    {
      std::string key("1_5b");
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string capt("a gridspec file must have frequency fx, found ");
        capt += pQA->qaExp.getFrequency() ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  if( (ix=pQA->pIn->variable[g_ix].getAttIndex("model_id")) == -1 )
    vs_gaFilename.push_back("");
  else
    vs_gaFilename.push_back(pQA->pIn->variable[g_ix].attValue[ix][0]);

  if( (ix=pQA->pIn->variable[g_ix].getAttIndex("experiment_id")) == -1 )
    vs_gaFilename.push_back("");
  else
  {
    // needed later
    pQA->qaExp.experiment_id = pQA->pIn->variable[g_ix].attValue[ix][0];
    vs_gaFilename.push_back(pQA->qaExp.experiment_id);
  }

  vs_gaFilename.push_back( getEnsembleMember() );

  if( isGridSpec && vs_gaFilename.back() != "r0i0p0" )
  {
    std::string key("1_5a");
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("a gridspec file must have ensemble member r0i0p9, found ");
      capt += vs_gaFilename.back() ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  // CMIP5 filename encoding
  // <variable_name>_<varReqTableSheetSubs>_<model>_<experiment>_<ensemble_member>
  bool isFault=false;
  bool isSwapped=false;
  bool isSyntax=false;

  size_t i;
  for(i=0 ; i < vs_gaFilename.size() ; ++i )
  {
    if( ! (i < x_filename.size()) )
    {
      isSyntax=true ;
      break;
    }

    // check for the same position in the filename
    if( vs_gaFilename[i] == x_filename[i] )
      continue;

    // no further checks possible because of a missing attribute
    if( vs_gaFilename[i].size() == 0 )
      break;

    // swapped items?
    for( size_t j=0 ; j < x_filename.size() ; ++j )
    {
      if( x_filename[j] == vs_gaFilename[i] )
      {
        isSwapped=true;
        break;
      }
    }

    if( isSwapped )
      break;

    isFault=true;
    break;
  }

  if( isSyntax )
  {
    std::string key("1_4a");
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt;
      if( isGridSpec )
        capt = "gridspec ";
      capt += "filename structure with a syntax fault, found" ;
      capt += hdhC::unsplit(x_filename, "_", i) ;
      capt += "expected " + hdhC::unsplit(vs_gaFilename, "_", i) ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  if( isSwapped )
  {
    std::string key("1_4c");
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt;
      if( isGridSpec )
        capt = "gridspec ";
      capt += "filename with swapped items, found ";
      capt += hdhC::unsplit(x_filename, "_", i) ;
      capt += "expected " + hdhC::unsplit(vs_gaFilename, "_", i) ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  if( isFault )
  {
    std::string key("1_4b");
    if( notes->inq( key, pQA->fileStr))
    {
      std::string capt;
      if( isGridSpec )
        capt = "gridspec ";
      capt += "filename does not match file attributes, found" ;
      capt += hdhC::unsplit(x_filename, "_", i) ;
      capt += "expected " + hdhC::unsplit(vs_gaFilename, "_", i) ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  return;
}

void
DRS_Filename::checkMIP_tableName(Split& x_filename)
{
  // Note: filename:= name_CMOR-MIP-table_... .nc
  if( x_filename.size() < 2 )
    return ;  //corrupt filename

  // table sheet name from global attributes has been checked

  // compare file entry to the one in global header attribute table_id
  if( x_filename[1] != pQA->qaExp.currMIP_tableName )
  {
    std::string key("46_9");

    if( notes->inq( key, "MIP") )
    {
      std::string capt("Ambiguous MIP table names, found ") ;
      capt += hdhC::tf_val("MIP-table(file)", x_filename[1]) ;
      capt += "vs. global " + hdhC::tf_att("table_id",pQA->qaExp.currMIP_tableName);

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
    }

    // try the filename's MIP table name
    if( pQA->qaExp.currMIP_tableName.size() == 0 )
      pQA->qaExp.currMIP_tableName = pQA->qaExp.getMIP_tableName(x_filename[1]) ;
  }

  return ;
}

std::string
DRS_Filename::getEnsembleMember(void)
{
  // ensemble member
  int ix;
  size_t g_ix = pQA->pIn->varSz ;  // index to the global atts

  std::string ga_ensmb("r");
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("realization")) == -1 )
    ga_ensmb += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ga_ensmb += 'i' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("initialization_method")) == -1 )
    ga_ensmb += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ga_ensmb += 'p' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("physics_version")) == -1 )
    ga_ensmb += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  return ga_ensmb;
}

void
DRS_Filename::run(void)
{
   // compare filename to netCDF global attributes
   checkFilename();

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
  Split x_f( pQA->pIn->file.getBasename(), "_" );

  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  std::string f;

  // any geographic subset?
  int f_sz=x_f.size();
  if( x_f[f_sz -1].substr(0,2) == "g-" )
    f = hdhC::unsplit(x_f, "_", f_sz-1) ;

  // if designator '-clim' is appended, then remove it
  f_sz = static_cast<int>(f.size()) -5 ;
  if( f_sz > 5 && f.substr(f_sz) == "-clim" )
    f=f.substr(0, f_sz);
  else if( f_sz > 5 && f.substr(f_sz) == "-ave" )
    f=f.substr(0, f_sz);

  size_t p0, p1;
  if( (p0=f.rfind('_')) == std::string::npos )
    return false ;  // the filename is composed totally wrong

  if( (p1=f.find('-', p0)) == std::string::npos )
    return true ;  // no period in the filename

  sd[1]=f.substr(p1+1) ;
  if( ! hdhC::isDigit(sd[1]) )
    return true ;  // no pure digits behind '-'

  sd[0]=f.substr(p0+1, p1-p0-1) ;
  if( ! hdhC::isDigit(sd[0]) )
    return true ;  // no pure 1st date found

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
DRS_Filename::testPeriodAlignment(std::vector<std::string>& sd, Date** pDates, bool b[])
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
DRS_Filename::testPeriodCutRegular(std::vector<std::string>& sd,
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

    if( isBegin && isEnd )
    {
      // should be the same year
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
     if( isBegin && isEnd )
     {
      // 5 years or less
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
DRS_Filename::testPeriodFormat(std::vector<std::string>& sd)
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

     if( split[0] == "tCV"
           || split[0] == "tableControlledVocabulary"
                || split[0] == "table_controlled_vocabulary" )
     {
       if( split.size() == 2 )
          table_DRS_CV.setFile(split[1]) ;

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
          pQA->setCheckMode("meta");
     }
     continue;
   }

   // apply a general path which could have also been provided by setTablePath()
   if( table_DRS_CV.path.size() == 0 )
      table_DRS_CV.setPath(pQA->tablePath);

   if( varReqTable.path.size() == 0 )
      varReqTable.setPath(pQA->tablePath);

   return;
}

void
QA_Exp::checkDataVarNum(void)
{
  //each file must contain only a single output field fro ma single simulation
  if( pQA->pIn->dataVarIndex.size() == 1 )
    return;

  std::string key("9_1");
  if( notes->inq( key) )
  {
    std::string capt("multiple output fields in the file, found variables: ") ;

    for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
    {
      if(i)
        capt += ", ";

      capt += pQA->pIn->variable[pQA->pIn->dataVarIndex[i]].name;
    }
  }

  return;
}

bool
QA_Exp::checkDimlessVar(InFile& in, Split& splt_line,
   VariableMetaData& vMD,
   struct DimensionMetaData *&p_dimFE,
   struct DimensionMetaData& dimFE_altern,
   struct DimensionMetaData& dimTE,
   std::map<std::string, size_t>& col)
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
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
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
QA_Exp::checkDimVarReqTable(ReadLine& ifs, InFile& in,
  VariableMetaData& vMD,
  std::vector<struct DimensionMetaData>& dimNcMeDa,
  std::map<std::string, size_t>& col,
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
   VariableMetaData   tmp_vMD(pQA) ;

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
              notes->setCheckDataStr(pQA->fail);
              pQA->setExit( notes->getExitValue() ) ;
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
    ostr << "QA_Exp::checkDimVarReqTable()";
    ostr << "\nStandard_output table: " << varReqTable.filename;
    ostr << "\nDimension " <<  dimName;
    ostr << " not found in the table.";

    std::string t0(dimName);
    t0 += ": not found in the standard table";

    (void) notes->operate(capt, ostr.str()) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

bool
QA_Exp::checkDimSpecialValue(InFile& in, VariableMetaData& vMD,
      struct DimensionMetaData& dimTE, std::string& dimName)
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
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
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
QA_Exp::checkDimVarReqTableEntry(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
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
  if( tbl_entry.outname == "lev" )
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
QA_Exp::checkDimAxis(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  std::string key("3_5");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));
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
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA_Exp::checkDimBndsName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  // standard table
  if( nc_entry.bnds_name.size() )
    return ;

  std::string key("3_7");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry)) ;
    capt += " bounds not available in the table, requested: " ;
    capt += tbl_entry.bnds_name ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA_Exp::checkDimChecksum(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("3_6");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry)) ;
    capt += " layout (checksum) changed, expected " ;
    capt += hdhC::double2String(tbl_entry.checksum) ;
    capt += ", found " + hdhC::double2String(nc_entry.checksum) ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
QA_Exp::checkLonLatParamRep(InFile& in,
           VariableMetaData& vMD,
           std::string& dimFE_name, std::string& dimTE_name)
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
QA_Exp::checkDimLongName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  std::string key("3_4");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));

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
      text += pQA->notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.longname.size() )
      text += nc_entry.longname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA_Exp::checkDimOutName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  std::string key("47_1");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));

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
      text += pQA->notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.outname.size() )
      text += nc_entry.outname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
QA_Exp::checkDimSize(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  // cf...: dim is variable across files
  if( nc_entry.outname == "loc" )
    return;

  std::string key("3_8");
  key += nc_entry.outname;
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));
    capt += " size conflict.";

    std::string text("             dim-size:\t table: ") ;
    text += hdhC::double2String(tbl_entry.size) ;
    text += "\n                      \tncfile: " ;
    text += hdhC::double2String(nc_entry.size) ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA_Exp::checkDimStndName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& nc_entry,
    struct DimensionMetaData& tbl_entry)
{
  std::string key("3_3");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));

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
      text += pQA->notAvailable ;

    text += "\n                      \tncfile: " ;
    if( nc_entry.stndname.size() )
      text += nc_entry.stndname ;
    else
      text += pQA->notAvailable ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
QA_Exp::checkDimULD(
     VariableMetaData& vMD,
     struct DimensionMetaData& nc_entry,
     struct DimensionMetaData& tbl_entry)
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
        std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry));

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
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
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
          std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry)) ;

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
            notes->setCheckMetaStr(pQA->fail);
            pQA->setExit( notes->getExitValue() ) ;
          }
        }
      }
      else // deviating reference dates
      {
        // There is nothing wrong with deviating references
        if( ! pQA->qaTime.isReferenceDate )
          return ;  // do not check at all

        // do not check at the beginning of a new experiment.
        if( ! pQA->qaTime.isReferenceDateAcrossExp && pQA->currQARec == 0 )
          return ;

        Date tbl_ref( tbl_entry.units, pQA->qaTime.calendar );
        Date nc_ref( nc_entry.units, pQA->qaTime.calendar );

        std::string key("3_10");
        if( notes->inq( key, vMD.var->name) )
        {
          std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_entry)) ;

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
              text += pQA->notAvailable ;

            text += "\n                      \tncfile: " ;
            if( nc_entry.units.size() )
              text += nc_entry.units ;
            else
              text += pQA->notAvailable ;
          }

          (void) notes->operate(capt, text) ;
          {
            notes->setCheckMetaStr(pQA->fail);
            pQA->setExit( notes->getExitValue() ) ;
          }
        }
      }
    }
  }

  return ;
}

void
QA_Exp::checkDimUnits(InFile& in,
     VariableMetaData& vMD,
     struct DimensionMetaData& nc_entry,
     struct DimensionMetaData& tbl_dim_entry)
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

  std::string t0;

  // I) dimension's var-rep without units in the standard table
  if( tbl_dim_entry.outname == "site")
  {
    std::string key("47_6");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry)) ;

      if( nc_entry.units.size() && nc_entry.units != "1" )
        capt += " dimension=site: has units." ;

      std::string text("                units:\t table: not defined") ;
      text += "\n                      \tncfile: " ;
      if( nc_entry.units.size() )
        text += nc_entry.units ;

      (void) notes->operate(capt, text) ;
      {
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
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
        std::string capt( getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry) );

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
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
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
        std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry));
        capt += " incorrect units." ;

        t0 = dimVar[ix] + ": incorrect units";

        std::string text ;
        text = "\nFile: coordinate attribute variable: units:",
        text += " valid: empty, 1, or absent. But is: " ;
        text += tmp;

        (void) notes->operate(capt, text) ;
        {
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
        }
      }
    }

    return;
  }

  // III) regular case
  std::string key("47_6");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(getSubjectsIntroDim(vMD, nc_entry, tbl_dim_entry));

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
      text += pQA->notAvailable ;
    else
      text += "not defined" ;

    (void) notes->operate(capt, text) ;
    {
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return ;
}

void
QA_Exp::checkMetaData(InFile& in)
{
  notes->setCheckMetaStr("PASS");

  // is filename compliant to DRS and CV?
//  checkFilename(in);

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

     if( notes->inq( key, pQA->fileStr) )
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
         notes->setCheckMetaStr(pQA->fail);
       }
     }
  }

  // inquire passing the meta-data check
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    pQA->setExit( ev );

  return;
}

void
QA_Exp::checkNetCDF(InFile& in)
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
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("format does not conform to netCDF classic") ;
      capt += ", found" + s;

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  return;
}

void
QA_Exp::checkTables(InFile& in, VariableMetaData& vMD)
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
QA_Exp::checkVarReqTable(InFile& in, VariableMetaData& vMD,
             std::vector<struct DimensionMetaData>& dimNcMeDa)
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
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
         }
      }
   }

   // headings for variables and dimensions
   std::map<std::string, size_t> v_col;
   std::map<std::string, size_t> d_col;

   size_t v_colMax, d_colMax;

   // read headings from the variable requirements table
   readHeadline(ifs, vMD, v_col, d_col, v_colMax, d_colMax);

   VariableMetaData tbl_entry(pQA);

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
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
   }

   // No variable found in the standard table.
   // Build a project table from netCDF file properties.
   return false;
}

void
QA_Exp::checkVarReqTableDimBounds(InFile& in, Split& splt_line,
    VariableMetaData& vMD,
    struct DimensionMetaData& dimFE,
    struct DimensionMetaData& dimTE,
    std::map<std::string,
    size_t>& col)
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
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
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
      std::string capt(captsIntro) ;
      capt += "Number of dim_bounds do not match those of the standard table." ;

      std::string text("                value:\t table: ") ;
      text += "not diagnosed" ;
      text += "\n                      \tncfile: " ;
      text += "2x";
      text += hdhC::double2String(dimFE.size);

      (void) notes->operate(capt, text) ;
      {
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
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
       std::string capt(captsIntro) ;
       capt += "values of dim_bounds do not match those of the standard table." ;

       std::string text("                value:\t table: ") ;
       text += hdhC::double2String(checksum_tb) ;
       text += "\n                      \tncfile: " ;
       text += hdhC::double2String(checksum_fl) ;

       (void) notes->operate(capt, text) ;
       {
          pQA->setExit( notes->getExitValue() ) ;
       }
    }
  }

  return ;
}

void
QA_Exp::checkVarReqTableDimValues(InFile& in, Split& splt_line,
    VariableMetaData& vMD,
    struct DimensionMetaData& dimFE,
    struct DimensionMetaData& dimTE,
    std::map<std::string, size_t>& col)
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
           ostr << pQA->notAvailable ;

         ostr << "\nFile: value: " ;
         if( mv.size() )
            ostr << hdhC::double2String(mv[0]) ;
         else if( vd.size() )
            ostr << vd[0] ;
         else
            ostr << pQA->notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
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
QA_Exp::checkVarReqTableEntry(
    VariableMetaData& vMD,
    VariableMetaData& tbl_entry)
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
           std::string capt(captsIntro);
           capt += "standard name conflict." ;

           std::ostringstream ostr(std::ios::app);
           ostr << "Variable conflict between file and table.";
           ostr << "\nTable: " ;
           ostr << ", table sheet: " << vMD.varReqTableSheet;
           ostr << "\nVariable: ";
           ostr << vMD.var->name;

           ostr << "        standard name:\t table: " ;
           if( tbl_entry.var->std_name.size() )
             ostr << tbl_entry.var->std_name ;
           else
             ostr << pQA->notAvailable ;

           ostr << "\n                      \tncfile: " ;
           if( vMD.var->std_name.size() )
             ostr << vMD.var->std_name ;
           else
             ostr << pQA->notAvailable ;

           (void) notes->operate(capt, ostr.str()) ;
           {
             notes->setCheckMetaStr(pQA->fail);
             pQA->setExit( notes->getExitValue() ) ;
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
        std::string capt(captsIntro);
        capt += "long name conflict." ;


        std::ostringstream ostr(std::ios::app);
        ostr << "Variable conflict between file and table.";
        ostr << "\nTable: " ;
        ostr << ", Sheet: " << vMD.varReqTableSheet;
        ostr << ", Variable: "  << vMD.var->name;
        ostr << "\nlong name: table: " ;
        if( tbl_entry.longName.size() )
          ostr << tbl_entry.longName ;
        else
          ostr << pQA->notAvailable ;

        ostr << "\nncfile: " ;
        if( vMD.longName.size() )
          ostr << vMD.longName ;
        else
          ostr << pQA->notAvailable ;

        (void) notes->operate(capt, ostr.str()) ;
        {
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
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
           std::string capt(captsIntro);
           capt += "variable has no units attribute." ;

           std::ostringstream ostr(std::ios::app);
           ostr << "\nTable: " ;
           ostr << ", Sheet: " << vMD.varReqTableSheet;
           ostr << ", Variable: ";
           ostr << vMD.var->name;
           ostr << "\nMissing units attribute" ;
           ostr << ", table requires " ;
           ostr << tbl_entry.var->units;

           (void) notes->operate(capt, ostr.str()) ;
           {
              notes->setCheckMetaStr(pQA->fail);
              pQA->setExit( notes->getExitValue() ) ;
           }
        }
      }
    }
    else if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(captsIntro) ;
      capt += "Conflict for the units attribute." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;
      ostr << "\nConflict between file and table.";
      ostr << "                units:\t table: " ;
      if( tbl_entry.var->units.size() )
        ostr << tbl_entry.var->units ;
      else
        ostr << pQA->notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.var->isUnitsDefined )
      {
        if( vMD.var->units.size() )
          ostr << vMD.var->units ;
        else
          ostr << pQA->notAvailable ;
      }
      else
        ostr << "not defined" ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
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
         std::string capt(captsIntro);
         capt += "cell-methods conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " ;
         ostr << ", Sheet: " << vMD.varReqTableSheet;
         ostr << ", Variable: ";
         ostr << vMD.var->name;
         ostr << "\nConflict between file and table.";
         ostr << "         cell-methods:\t table: " ;
         if( tbl_entry.cellMethods.size() )
           ostr << tbl_entry.cellMethods ;
         else
           ostr << pQA->notAvailable ;

         ostr << "\n                      \tncfile: " ;
         if( vMD.cellMethods.size() )
           ostr << vMD.cellMethods ;
         else
           ostr << pQA->notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
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
         std::string capt(captsIntro);
         capt += "cell-measures conflict." ;

         std::ostringstream ostr(std::ios::app);
         ostr << "Table: " ;
         ostr << ", Sheet: " << vMD.varReqTableSheet;
         ostr << ", Variable: ";
         ostr << vMD.var->name;
         ostr << "\nConflict between file and table.";
         ostr << "\n         cell-measures:\t table: " ;
         if( tbl_entry.cellMeasures.size() )
           ostr << tbl_entry.cellMeasures ;
         else
           ostr << pQA->notAvailable ;

         ostr << "\n                      \tncfile: " ;
         if( vMD.cellMeasures.size() )
           ostr << vMD.cellMeasures ;
         else
           ostr << pQA->notAvailable ;

         (void) notes->operate(capt, ostr.str()) ;
         {
           notes->setCheckMetaStr(pQA->fail);
           pQA->setExit( notes->getExitValue() ) ;
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

  if( tbl_entry.typeStr.size() == 0 && vMD.typeStr.size() != 0 )
  {
    std::string key("4_11");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(captsIntro);
      capt += "type check discarded, not specified in the MIP Table." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " ;
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
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }
  else if( (isTblTypeReal && ! isNcTypeReal)
            || ( ! isTblTypeReal && isNcTypeReal) )
  {
    std::string key("4_8");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(captsIntro);
      capt += "type conflict." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Table: " ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;
      ostr << "\nConflict between file and table.";
      ostr << "\n                 type:\t table: " ;
      if( tbl_entry.typeStr.size() )
        ostr << tbl_entry.typeStr ;
      else
        ostr << pQA->notAvailable ;

      ostr << "\n                      \tncfile: " ;
      if( vMD.typeStr.size() )
        ostr << vMD.typeStr ;
      else
        ostr << pQA->notAvailable ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( tbl_entry.var->std_name != vMD.var->std_name )
  {
    std::string key("4_12");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(captsIntro);
      capt += "variable name matches only for case-insensitivity." ;

      std::ostringstream ostr(std::ios::app);
      ostr << "Variable conflict between file and table.";
      ostr << "\nTable: " ;
      ostr << ", Sheet: " << vMD.varReqTableSheet;
      ostr << ", Variable: ";
      ostr << vMD.var->name;

      ostr << "        variable name:\t table: " ;
      ostr << tbl_entry.var->name ;

      ostr << "\n                      \tncfile: " ;
      ostr << vMD.var->name ;

      (void) notes->operate(capt, ostr.str()) ;
      {
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
QA_Exp::createVarMetaData(void)
{
  // create instances of VariableMetaData. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in the InFile::dataVarIndex vector.
  for( size_t i=0 ; i< pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    pushBackVarMeDa( &var );  //push next instance
  }

  return;
}

bool
QA_Exp::findNextVariableHeadline(ReadLine& ifs, std::string& str0,
   VariableMetaData& vMD, std::vector<std::string>& wa )
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
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
       }
     }
   }

   // we have to read the next line.
   ifs.getLine(str0);

   return false ;
}

bool
QA_Exp::findVarReqTableEntry(ReadLine& ifs, std::string& str0,
   VariableMetaData& vMD,
   std::map<std::string, size_t>& col, size_t col_max,
   std::vector<std::string>& wa)
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
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
        }
     }
   }


   return false;  // we have found an entry and the shape matches.
}

bool
QA_Exp::findVarReqTableSheet(ReadLine& ifs, std::string& str0,
  VariableMetaData& vMD)
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
QA_Exp::findVarReqTableSheetSub(std::string& str0,
   VariableMetaData& vMD, std::vector<std::string>& wa)
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

void
QA_Exp::getDimMetaData(InFile& in,
      VariableMetaData& vMD,
      struct DimensionMetaData& dimMeDa,
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

std::string
QA_Exp::getFrequency(void)
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pQA->pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;  // no frequency provided

  // not found, but error issue is handled elsewhere

  // try the filename
  std::string f( pQA->pIn->file.basename );

  Split splt(f, "_");

  // the second term denotes the mip table for CMIP5
  std::string mip_f = splt[1];

  // now, try also global att 'table_id'
  Split mip_a( pQA->pIn->nc.getAttString("table_id") ) ;

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
QA_Exp::getMIP_tableName(std::string tName)
{
  if( currMIP_tableName.size() )
    return currMIP_tableName;

  // the counter-parts in the attributes
  std::string tableID;
  if( tName.size() )
    tableID = tName ;
  else
    tableID = pQA->pIn->nc.getAttString("table_id") ;

  if( tableID.size() == 0 )
    return tableID;

  Split x_tableID(tableID);

  // The table sheet name from the global attributes.
  // Ignore specific variations
  if( x_tableID.size() > 1 )
  {
    if(  x_tableID[0].substr(1,4) == "able"
        || x_tableID[0].substr(1,4) == "ABLE" )
    tableID = x_tableID[1] ;
  }
  else if( x_tableID.size() )
    tableID = x_tableID[0] ;

  //check for valid names
  bool is=true;
  for( size_t i=0 ; i < MIP_tableNames.size() ; ++i )
  {
    if( MIP_tableNames[i] == tableID )
    {
      is=false ;
      break;
    }
  }

  if( is )
  {
    if( tName.size() == 0)
    {
      std::string key("46_8");

      if( notes->inq( key, "global") )
      {
        std::string capt("invalid MIP table name in global attribute, found ") ;
        capt += hdhC::tf_att(pQA->s_empty, "table_id", x_tableID.getStr()) ;

        (void) notes->operate(capt) ;
        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    tableID.clear();
  }

  return tableID;
}

std::string
QA_Exp::getTableEntryID(std::string vName)
{
  vName += "," ;
  vName += getFrequency() ;

  return vName + ",";
}

std::string
QA_Exp::getSubjectsIntroDim(VariableMetaData& vMD,
                   struct DimensionMetaData& nc_entry,
                   struct DimensionMetaData& tbl_entry)
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
QA_Exp::init(std::vector<std::string>& optStr)
{
  // apply parsed command-line args
  applyOptions(optStr);

  fVarname = getVarnameFromFilename(pQA->pIn->file.filename);
  getFrequency();

  // Create and set VarMetaData objects.
  createVarMetaData() ;

  if( inqTables() )
  {
    currMIP_tableName = getMIP_tableName() ;

    if( table_DRS_CV.is )
    {
      DRS_Filename drsFN(pQA, optStr);
      drsFN.run();
    }

    // get meta data from file and compare with tables
    checkMetaData(*(pQA->pIn));
  }

  return ;
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
       std::string key("3_14");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt("variable=");
         capt += prevTargets[i] + " is missing in sub-temporal file" ;

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
       std::string key("3_15");
       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("variable=");
         capt += varMeDa[j].var->name + " is new in sub-temporal file" ;

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
  if( ! varReqTable.isExisting(varReqTable.path) )
  {
     std::string key("57");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("no path to the tables, tried ") ;
        capt += varReqTable.path;

        (void) notes->operate(capt) ;
        {
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
        }
     }

     return false;
  }

  return true;
}

/*
bool
QA_Exp::locate( GeoData<float> *gd, double *alat, double *alon, const char* crit )
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

      // some more properties
      vMD.var->std_name = pQA->pIn->nc.getAttString("standard_name", var->name);
      vMD.longName = pQA->pIn->nc.getAttString("long_name", var->name);
      vMD.cellMethods = pQA->pIn->nc.getAttString("cell_methods", var->name);
      vMD.cellMeasures = pQA->pIn->nc.getAttString("cell_measures", var->name);
      vMD.typeStr = var->type;
   }

   return;
}

bool
QA_Exp::readHeadline(ReadLine& ifs,
   VariableMetaData& vMD,
   std::map<std::string, size_t>& v_col,
   std::map<std::string, size_t>& d_col,
   size_t& v_colMax, size_t& d_colMax )
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
          notes->setCheckMetaStr(pQA->fail);
          pQA->setExit( notes->getExitValue() ) ;
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
QA_Exp::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
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
