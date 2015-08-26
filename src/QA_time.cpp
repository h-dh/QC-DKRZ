QA_Time::QA_Time()
{
  initDefaults();
}

/*
QA_Time::~QA_Time()
{
 ;
}
*/

void
QA_Time::applyOptions(std::vector<std::string> &optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "aMDR"
            || split[0] == "applyMaximumDateRange"
                 || split[0] == "apply_maximum_date_range" )
     {
          isMaxDateRange=true;

          if( split.size() == 2 )
             maxDateRange=split[1];

          continue;
     }

     if( split[0] == "dTB" || split[0] == "disableTimeBoundsCheck"
         || split[0] == "disable_time_bounds_check" )
     {
          disableTimeBoundsTest() ;
          continue;
     }

     if( split[0] == "iRD" || split[0] == "ignoreReferenceDate"
         || split[0] == "ignore_reference_date" )
     {
          isReferenceDate=false ;
          continue;
     }

     if( split[0] == "iRDAE" || split[0] == "ignoreRefDateAcrossExp"
         || split[0] == "ignore_ref_date_across_exp" )
     {
          isReferenceDateAcrossExp=false ;
          continue;
     }

     if( split[0] == "calendar" )
     {
          calendar=split[1];
          isNoCalendar=false ;
          continue;
     }

     if( split[0] == "nRTS" || split[0] == "nonRegularTimeSteps"
       || split[0] == "non_regular_time_steps" )
     {
          isRegularTimeSteps=false ;
          continue;
     }

     if( split[0] == "tP"
          || split[0] == "tablePath" || split[0] == "table_path")
     {
       if( split.size() == 2 )
       {
          tablePath=split[1] + "/" ;
          continue;
       }
     }

     if( split[0] == "tTR"
         || split[0] == "tableTimeRanges"
            || split[0] == "table_time_ranges" )
     {
          timeTable=split[1] ;
          timeTableMode=NONE;
          continue;
     }
   }

   return;
}

void
QA_Time::finally(NcAPI *nc)
{
  timeOutputBuffer.flush();
  sharedRecordFlag.flush();

  if( refDate.getJulianDate(firstTimeValue)
        <= refDate.getJulianDate(currTimeValue) )
  {
    // not true for a gap across a file, because then currDate
    // represents the last date from the previous file.
    std::string out( "PERIOD-BEG " );

    // if bounds are available, then use them
    if( isTimeBounds )
    {
      out += getDateStr(firstTimeBoundsValue[0]);
      out += " - " ;
      out += getDateStr(lastTimeBoundsValue[1]);
    }
    else
    {
      out += getDateStr(firstTimeValue) ;
      out += " - " ;
      out += getDateStr(lastTimeValue) ;
    }

    out += "PERIOD-END";  // mark of the end of an output line
    std::cout << out << std::endl;
  }

  // write internal data to variable time
  nc->setAtt( "time", "last_time", lastTimeValue);
  nc->setAtt( "time", "last_date", getDateStr(lastTimeValue) );
  nc->setAtt( "time", "isTimeBoundsTest", static_cast<double>(isTimeBounds));

  if( pIn->currRec )
  {
    // catch a very special one: the initial file got only a
    // single record and the period is longer than a day, then
    // the time step is twice as long.
    double tmp;

    if( pQA->currQARec == 1 )  // due to the final loop increment
    {
      referenceTimeStep *= 2. ;
      tmp=2.*currTimeStep;
    }
    else
      tmp=currTimeStep;

    nc->setAtt( "time_step", "last_time_step", tmp);
  }

  nc->setAtt( "time", "last_time_bnd_0", prevTimeBoundsValue[0]);
  nc->setAtt( "time", "last_time_bnd_1", prevTimeBoundsValue[1]);

  return;
}

void
QA_Time::getDate(Date& d, double t)
{
/*
  if( isFormattedDate )
  {
     d.setDate( Date::getIso8601(t), calendar ) ;
  }
  else
  {
*/
    d=refDate;
    d.addTime(t) ;
//  }

  return ;
}

std::string
QA_Time::getDateStr(double val, bool isAbsolute)
{
  if(isAbsolute || isFormattedDate)
  {
    Date d(Date::convertFormattedToISO_8601(val), calendar );
    return d.str();
  }
  else
    return refDate.getDate(val).str() ;
}

void
QA_Time::getDRSformattedDateRange(std::vector<Date> &period,
       std::vector<std::string> &sd)
{
  std::vector<std::string> iso;

  // be prepared for some short-cuts to be valid
  // (e.g. 1961 equiv. 1961-01 equiv. 1961-01-01)
  iso.push_back( "0000-01-01T00:00:00" );
  iso.push_back( "0000-01-01T00:00:00" );

  // set dates of time periods
  for( size_t i=0 ; i < sd.size() ; ++i )
  {
    if( sd[i].size() > 3 )
      iso[i].replace(0, 4, sd[i], 0, 4);
    if( sd[i].size() > 5 )
      iso[i].replace(5, 2, sd[i], 4, 2);
    if( sd[i].size() > 7 )
      iso[i].replace(8, 2, sd[i], 6, 2);
    if( sd[i].size() > 9 )
      iso[i].replace(11, 2, sd[i], 8, 2);
    if( sd[i].size() > 11 )
      iso[i].replace(14, 2, sd[i], 10, 2);
    if( sd[i].size() > 13 )
      iso[i].replace(17, 2, sd[i], 12, 2);
  }

  // Apply maximum date ranges when enabled; times remain sharp.
  // Take into account individual settings 'Yx-Mx-...' with
  // x=s for a sharp deadline and x=e for extended maximum period.
  // Defaults for x=s.

  if( sd.size() == 2 && isMaxDateRange )
  {
    if( maxDateRange.size() == 0 )
      maxDateRange = "Ye-Me-De-hs-ms-ss" ;

     // year
     if( sd[1].size() == 4 )
     {
        if( maxDateRange.find("Ye") < std::string::npos )
          iso[1].replace(5, 2, "12-31T24");
     }
     else if( sd[1].size() == 6 )
     {
        // months
        if( maxDateRange.find("Me") < std::string::npos )
        {
           // find the end of the month specified
           Date xD(sd[1], refDate.getCalendar());
           double md = xD.getMonthDaysNum(
              hdhC::string2Double( iso[1].substr(5,2) ),    // month
              hdhC::string2Double( iso[1].substr(0,4) ) ) ; //year

          // convert to string with leading zero(s)
          std::string smd( hdhC::double2String(md, 0, "2_0") );

          // the number of days of the end month of the period
          iso[1].replace(8, 2, smd);
          iso[1].replace(11, 2, "24");
        }
     }
     else if( sd[1].size() == 8 )
     {
        // days
        if( maxDateRange.find("De") < std::string::npos )
           iso[1].replace(11, 2, "24");
     }
     else if( sd[1].size() == 10 )
     {
        // hours
        if( maxDateRange.find("he") < std::string::npos )
           iso[1].replace(14, 2, "60");
     }
     else if( sd[1].size() == 12 )
     {
        // minutes
        if( maxDateRange.find("me") < std::string::npos )
           iso[1].replace(17, 2, "60");
     }
     // note: no seconds
  }

  // set dates of time periods
  if( period.size() )
  {
    for( size_t i=0 ; i < sd.size() ; ++i )
    {
      period[i].setUnits( refDate.getUnits() );
      period[i].setDate(iso[i], calendar) ;
    }
  }
  else
  {
    for( size_t i=0 ; i < sd.size() ; ++i )
    {
      period.push_back( Date(iso[i], calendar) );
      period.back().setUnits( refDate.getUnits() );
    }
  }

  return;
}

void
QA_Time::getTimeBoundsValues(double* pair, size_t rec, double offset)
{
  int mv_ix = pIn->nc.getData(mv_tb, boundsName, rec );

  double** m2D =mv_tb.getM();
  pair[0]=m2D[mv_ix][0] + offset;
  pair[1]=m2D[mv_ix][1] + offset;

  return ;
}

bool
QA_Time::initTimeBounds(double offset)
{
  if( timeBounds_ix == -1 )
  {
    firstTimeBoundsValue[0]=0.;
    firstTimeBoundsValue[1]=0.;
    lastTimeBoundsValue[0]=0.;
    lastTimeBoundsValue[1]=0.;
    return false;
  }

  (void) pIn->nc.getData(mv_tb, boundsName, 0 );
  double** m2D =mv_tb.getM();
  firstTimeBoundsValue[0]=m2D[0][0] + offset;
  firstTimeBoundsValue[1]=m2D[0][1] + offset;

  int mv_ix = pIn->nc.getData(mv_tb, boundsName,
                                  pIn->nc.getNumOfRows(boundsName)-1);
  m2D =mv_tb.getM();
  lastTimeBoundsValue[0]=m2D[mv_ix][0] + offset;
  lastTimeBoundsValue[1]=m2D[mv_ix][1] + offset;

  return true;
}

void
QA_Time::init(InFile *p, Annotation *n, QA *q)
{
   pIn = p;
   pQA = q;
   notes = n;

   if( pIn->cF )
   {
     name = pIn->cF->timeName;
     time_ix  = pIn->cF->time_ix;
     boundsName = pIn->variable[time_ix].bounds ;
   }
   else
   {
     size_t i;
     for( i=0 ; i < pIn->varSz ; ++i )
     {
       if( pIn->variable[i].name == "time" )
       {
         name = pIn->variable[i].name ;
         time_ix = static_cast<int>(i) ;

         boundsName = pIn->variable[i].bounds ;
       }
     }
   }
   timeBounds_ix = pIn->getVarIndex(boundsName);

   // time_bnds available? Yes, then enable a check
   if( boundsName.size() )
      if( ! pIn->variable[timeBounds_ix].isExcluded )
         enableTimeBoundsTest();

   timeOutputBuffer.initBuffer(pQA->nc, pQA->currQARec);
   timeOutputBuffer.setName(name);

   sharedRecordFlag.initBuffer(pQA->nc, pQA->currQARec);
   sharedRecordFlag.setName( name + "_flag" );

   // set date to a reference time
   std::string str(pIn->getTimeUnit());

   if( str.size() == 0 ) // fixed field
     return ;
   else if( str.find("%Y") < std::string::npos )
   {
     if( initAbsoluteTime(str) )
        return;
   }
   else
   {
     if( initRelativeTime(str) )
       return;  // could  not read any time value
   }

   return ;
}

void
QA_Time::initDefaults(void)
{
   notes=0;
   pIn=0;
   pQA=0;

   currTimeStep=0 ;
   prevTimeValue=MAXDOUBLE;
   prevTimeBoundsValue[0]=0.;
   prevTimeBoundsValue[1]=0.;
   referenceTimeStep=0.;

   firstTimeValue=0.;
   currTimeValue=0.;
   lastTimeValue=0.;

   firstTimeBoundsValue[0]=0.;
   currTimeBoundsValue[0]=0.;
   lastTimeBoundsValue[0]=0.;
   firstTimeBoundsValue[1]=0.;
   currTimeBoundsValue[1]=0.;
   lastTimeBoundsValue[1]=0.;

   // time steps are regular. Unsharp logic (i.e. month
   // Jan=31, Feb=2? days is ok, but also numerical noise).

   isFormattedDate=false;
   isMaxDateRange=false;
   isNoCalendar=true;
   isPrintTimeBoundDates=false;
   isReferenceDate=true ;
   isRegularTimeSteps=true;
   isSingleTimeValue=false;
   isTimeBounds=false;

   time_ix = -1 ;
   timeBounds_ix = -1 ;

   refTimeOffset=0.; // !=0, if there are different reference dates

   bufferCount=0;
   maxBufferSize=1500;

  return;
}

bool
QA_Time::initAbsoluteTime(std::string &units)
{
  size_t i;
  for( i=0 ; i < pIn->varSz ; ++i)
    if( pIn->variable[i].name == name )
       break;

  if( i == pIn->varSz )
     return true;  // no time

  isFormattedDate = true;

  // proleptic Gregorian, no-leap, 360_day
  calendar = pIn->nc.getAttString("calendar", name);
  if( calendar.size() )
  {
     isNoCalendar=false;
     refDate.setCalendar(calendar);
  }

  // time_bounds
  disableTimeBoundsTest();
  boundsName = pIn->variable[i].bounds;

  // time_bnds available? Yes, then enable a check
  if( boundsName.size() )
     if( ! pIn->variable[i].isExcluded )
        enableTimeBoundsTest();

  refTimeOffset=0.;

  Split x_units(units);
  for( size_t i=0 ; i< x_units.size() ; ++i )
  {
     if( x_units[i][0] == '%' )
     {
        dateFormat = x_units[i] ;
        break;
     }
  }

  if( mv_t[ pIn->nc.getData(mv_t, name, 0) ] == MAXDOUBLE)
     return true;

  currTimeValue += refTimeOffset;
  firstTimeValue = currTimeValue;

  size_t recSz = pIn->nc.getNumOfRows(name) ;
  lastTimeValue=mv_t[ pIn->nc.getData(mv_t, name, recSz-1) ] + refTimeOffset;

  if( prevTimeValue == MAXDOUBLE )
  {
     // 0) no value; caught elsewhere
     if( recSz == 0 )
       return false;

     // 1) out of two time values within a file
     if( recSz > 1 )
     {
       if( pIn->currRec < recSz )
         prevTimeValue = mv_t[ pIn->nc.getData(mv_t, name, 1) ]
                            + refTimeOffset ;

       // an arbitrary setting that would pass the first test;
       // the corresponding results is set to FillValue
       Date prevDate(prevTimeValue, calendar);
       Date currDate(currTimeValue, calendar);
       referenceTimeStep=fabs(prevDate.getJulianDay() - currDate.getJulianDay());
       prevDate.addTime( -2.*referenceTimeStep ) ;

       std::string s(prevDate.str());
       std::string t(s.substr(0,4));
       t += s.substr(5,2);
       t += s.substr(8,2);
       double h = hdhC::string2Double(s.substr(11,2));
       h += hdhC::string2Double(s.substr(14,2)) / 60.;
       h += hdhC::string2Double(s.substr(17,2)) / 60.;
       prevTimeValue = hdhC::string2Double(t);
       prevTimeValue += h/24.;
     }
     else
     {
       prevTimeValue = 0. ;
       referenceTimeStep=0.;
     }

     if( isTimeBounds )
     {
       getTimeBoundsValues(currTimeBoundsValue, pIn->currRec, refTimeOffset) ;
       double dtb = currTimeBoundsValue[1] - currTimeBoundsValue[0] ;
       prevTimeBoundsValue[0] = currTimeBoundsValue[0] - dtb ;
       prevTimeBoundsValue[0] = currTimeBoundsValue[1] ;
     }
  }

  if( isTimeBounds )
  {
    getTimeBoundsValues( firstTimeBoundsValue, 0, refTimeOffset ) ;
    getTimeBoundsValues( lastTimeBoundsValue, recSz-1, refTimeOffset ) ;
  }

  return false;
}

bool
QA_Time::initRelativeTime(std::string &units)
{
   if( time_ix == -1 )
     return true;  // no time

   // proleptic Gregorian, no-leap, 360_day
   calendar = pIn->nc.getAttString("calendar", name);

   if( calendar.size() )
   {
      isNoCalendar=false;
      refDate.setCalendar(calendar);
   }
   refDate.setDate( units );

   currTimeValue=mv_t[ pIn->nc.getData(mv_t, name, 0) ] ;
   if( currTimeValue == MAXDOUBLE)
      return true;

   currTimeValue += refTimeOffset;
   firstTimeValue = currTimeValue ;

   size_t recSz = pIn->nc.getNumOfRows(name) ;

   lastTimeValue=mv_t[ pIn->nc.getData(mv_t, name, recSz-1) ]
                    + refTimeOffset ;

   if( isTimeBounds )
     initTimeBounds(refTimeOffset) ;

   if( prevTimeValue == MAXDOUBLE )
   {
     // 0) no value; caught elsewhere
     if( recSz == 0 )
       return false;

     // 1) out of two time values within a file
     if( recSz > 1 )
     {
       double t1=2.*currTimeValue;

       if( pIn->currRec < recSz )
       {
         if( mv_t.getDimSize() == 1 && mv_t.size() > (pIn->currRec +1) )
           t1=mv_t[pIn->currRec+1] + refTimeOffset ;
       }

       // an arbitrary setting that would pass the first test;
       // the corresponding results is set to FillValue
       referenceTimeStep=fabs(t1 - currTimeValue);
     }

     // 2) guess
     else
     {
       // a) from time:units and :frequency
       std::string freq( pQA->getFrequency() );
       if( freq.size() && refDate.getUnits() == "day" )
       {
         if( freq == "3hr" )
           referenceTimeStep = 1./8. ;
         else if( freq == "6hr" )
           referenceTimeStep = 1./4. ;
         else if( freq == "day" )
           referenceTimeStep = 1. ;
         else if( freq == "mon" )
           referenceTimeStep = 30. ;
         else if( freq == "yr" )
         {
           if( refDate.getCalendar() == "equal_month" )
             referenceTimeStep = 360. ;
           else
             referenceTimeStep = 365. ;
         }
       }
       else
         referenceTimeStep=currTimeValue;
     }

     prevTimeValue = currTimeValue - referenceTimeStep ;

     if( isTimeBounds )
     {
       double dtb = firstTimeBoundsValue[1] - firstTimeBoundsValue[0];

       prevTimeBoundsValue[0]=firstTimeBoundsValue[0] - dtb ;
       prevTimeBoundsValue[1]=firstTimeBoundsValue[0]  ;
     }
   }

   return false;
}

void
QA_Time::initResumeSession(void)
{
   // this method may be used for two different purposes. First,
   // resuming within the same experiment. Second, continuation
   // from a parent experiment.

   std::vector<double> dv;

   // Simple continuation.
   // Note: this fails, if the previous file has a
   //       different reference date AND the QA resumes the
   //       current file after an error.
   pQA->nc->getAttValues( dv, "last_time", name);
   prevTimeValue=dv[0];

   pQA->nc->getAttValues( dv, "last_time_bnd_0", name);
   prevTimeBoundsValue[0]=dv[0];

   pQA->nc->getAttValues( dv, "last_time_bnd_1", name);
   prevTimeBoundsValue[1]=dv[0];

   pQA->nc->getAttValues( dv, "last_time_step", name + "_step");
   referenceTimeStep=dv[0];

   // case: two different reference dates are effective.
   std::string tu_0(pQA->nc->getAttString("units", name));
   std::string tu_1(pIn->nc.getAttString("units", name));
   if( ! (tu_0 == tu_1 ) )
   {
      if( ! isFormattedDate )
      { // note: nothing for fomatted dates
        Date thisRefDate( tu_1, calendar );

        // Current refDate is set to thisDate.
        // Reset it to the previous date.
        refDate.setDate( tu_0 );
        Date d_x0( refDate );
        Date d_x1( thisRefDate );

        // adjust all time values of the current file to
        // the reference date of the first chunk, which is
        // stored in the qa-nc file
        refTimeOffset= d_x0.getSince( d_x1 );
      }
   }

   // get internal values
   isTimeBounds =
     static_cast<bool>(pQA->nc->getAttValue("isTimeBoundsTest", name));

   return;
}

void
QA_Time::initTimeTable(std::string id_1st, std::string id_2nd)
{
   if( timeTableMode == NONE )
     return;

   if( id_1st.size() == 0 )
   {
     timeTableMode = NONE ;
     return;
   }

   tt_block_rec=0;
   tt_count_recs=0;

   tt_id = id_1st; // only purpose: designator i output
   if( id_2nd.size() )
   {
      tt_id += '_' ;
      tt_id += id_2nd ;
   }

   // is called from QA::init()
   std::string str(tablePath);  // includes trailing '/' or is empty
   str += timeTable ;

   // default for no table found: "id_1st,,any(regular)"
   if( timeTable.size() == 0 )
   {
     timeTableMode = REGULAR ;
     return ;
   }

   // This class provides the feature of putting back an entire line
   ReadLine ifs(str);

   if( ! ifs.isOpen() )
   {
     timeTableMode = REGULAR ;
     return ;
   }

   // find the identifier in the table
   Split splt_line;
   splt_line.setSeparator(',');
   splt_line.enableEmptyItems();

   do
   {
     // the loop cycles over the sub-tables

     //find a line with the current table name
     if( ifs.findLine(str, id_1st) )
     {
        //default for missing/omitted frequencies in the time table
        timeTableMode = REGULAR ;
//        ifs.close();  // no longer needed
        return ;
     }

     splt_line=str;
     if( splt_line[1].size() )
     {
       // find the sub-table, if there is any (not CORDEX)
       std::string sstr(splt_line[1].substr(0,10)) ;

       // substr is necessary for sub-tables with a counting number added
       if( sstr == id_2nd.substr(0,10) )
          break ;
     }
     else
        break;
   } while( true );

   // ok, the right line was found. Now, extract experiments.
   // Take continuation lines into account.
   std::string str_exp( splt_line[2] ) ;
   size_t pos;

   do
   {
      if( ifs.getLine(str) )
        break;

      if( (pos=str.find(':')) == std::string::npos )
        break; // this is no continuation line

      str_exp += str.substr(++pos) ;
   } while ( true );

   // erase all blanks:
   str = hdhC::clearSpaces(str_exp);
   str_exp.clear();

   // insert a ' ' after each closing ')', but the last
   size_t pos0=0;
   size_t sz = str.size();
   do
   {
      if( (pos = str.find(')',pos0 )) < std::string::npos )
      {
        ++pos;
        str_exp += str.substr(pos0, pos - pos0);

        if( pos == sz )
          break;

        str_exp += ' ';
        pos0 = pos ;
      }
   } while( true );

   // scan the time-table entry for the current experiments
   size_t p0=0;
   Split splt_exp ;
   splt_exp = str_exp ;

   for( size_t i=0 ; i < splt_exp.size() ; ++i )
   {
     str_exp = splt_exp[i] ;

     p0=str_exp.find('(') + 1;

     // find matching experiment
     std::string str2(str_exp.substr(0,p0-1));
   }

   // Get value(s) from the time-table.
   // Pairs of () must match; this is not checked.
   tt_xmode.setSeparator('|');
   tt_xmode = str_exp.substr(p0, str_exp.size() - p0 - 1) ;

   if( tt_xmode[0] == "regular" )
     timeTableMode = REGULAR ;
   else if( tt_xmode[0] == "orphan" )
     timeTableMode = ORPHAN ;
   else if( tt_xmode[0] == "disable" )
     timeTableMode = DISABLE ;
   else
   {
     // other modes are updated in cycles
     timeTableMode = CYCLE ;
     tt_index=0;

     // init a range
     Split splt_range;
     splt_range.setSeparator('-');
     std::vector<std::string> sd;
     splt_range = tt_xmode[0] ;

     sd.push_back( splt_range[0]) ;

     if( splt_range.size() == 2 )
       sd.push_back( splt_range[1] );

     getDRSformattedDateRange(tt_dateRange, sd);

     if( refDate.getJulianDate(currTimeValue) < tt_dateRange[0].getJulianDate() )
     { // error: time record out of range (before)
       std::string key("6_4");
       if( notes->inq( key, name) )
       {
         std::string capt("time value before the first time-table range");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << id_1st;
         ostr << "\nrec#=0" ;
         ostr << "\ndate in record=" << getDateStr(currTimeValue) ;
         ostr << "\nrange from time-table=" ;
         ostr << tt_dateRange[0].str() << " - " ;
         ostr << tt_dateRange[1].str() ;

         if( notes->operate(capt, ostr.str()) )
         {
           notes->setCheckTimeStr(fail);
           pQA->setExit( notes->getExitValue() ) ;
         }
       }
     }
   }

   return;
}

bool
QA_Time::parseTimeTable(size_t rec)
{
   if( timeTableMode == NONE )
     return false;

   // return == true: no error messaging, leave the
   // calling method, i.e. testTimeStep, immediately.

   // note: this method could be called severalm times,
   //       even for the mode ORPHAN

   // preserve over the record for some operational modes
   if( tt_block_rec == rec && tt_isBlock )
     return true;
   else
     tt_isBlock=false;

   // regular time test for each record
   if( timeTableMode == REGULAR )
     return false;

   if( timeTableMode == DISABLE )
     return true;

   // No time test for the first record, after this regular.
   if( timeTableMode == ORPHAN )
   {
     tt_isBlock=true;
     tt_block_rec = rec ;
     timeTableMode = REGULAR ;
     return true;
   }

   if( timeTableMode == CYCLE
          && refDate.getJulianDate(currTimeValue)
                  < tt_dateRange[1].getJulianDate() )
     return false; //an error happened within the range

  // other modes are updated in cycles
  if( tt_xmode[0][0] == 'N' )
  {
     ++tt_count_recs ;
     size_t num = static_cast<size_t>(
           hdhC::string2Double( tt_xmode[0] ) ) ;

     if( tt_count_recs > num )
     {  // issue an error flag; number of records too large
        std::string key("6_7");
        if( notes->inq( key, name) )
        {
          std::string capt("too many time values compared to the time-table");

          std::ostringstream ostr(std::ios::app);
          ostr << "frequency=" << tt_id;
          ostr << "\nrec#=" << rec;
          ostr << "\ndate in record=" << getDateStr(currTimeValue) ;
          ostr << "\nvalue from time-table=" << tt_xmode[tt_index] ;

          if( notes->operate(capt, ostr.str()) )
          {
            notes->setCheckTimeStr(fail);
            pQA->setExit( notes->getExitValue() ) ;
          }
        }
     }

     tt_block_rec = rec ;
     tt_isBlock=true;

     return true;
  }
  else if( tt_xmode[0][0] == '+' )
  {
    // too less information  for an algorithmic approach
    timeTableMode = DISABLE ;
    return true;
  }
  else if( tt_xmode[tt_index].find('-') == std::string::npos )
  {  // singular values must match; but this could happen
     // several times (e.g. 12 months a year)
     // update the static index
     std::string currDateStr(getDateStr(currTimeValue));
     std::string t0( currDateStr.substr(0,4) );
     while( t0 > tt_xmode[tt_index] )
       ++tt_index;

     if( tt_index == tt_xmode.size() || t0 != tt_xmode[tt_index] )
     {
       std::string key("6_6");
       if( notes->inq( key, name) )
       {
         std::string capt("time record does not match time-table value");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << tt_id;
         ostr << "\nrec#=" << rec;
         ostr << "\ndate in record=" << getDateStr(currTimeValue) ;
         ostr << "\nvalue from time-table=" << tt_xmode[tt_index] ;

         if( notes->operate(capt, ostr.str()) )
         {
            notes->setCheckTimeStr(fail);
            pQA->setExit( notes->getExitValue() ) ;
         }
       }
     }

     tt_block_rec = rec ;
     tt_isBlock=true;

     return true;
  }

  // a range is specified
  Split splt_range;
  splt_range.setSeparator('-');
  std::vector<std::string> sd(2);

  do
  {
    if( tt_index == tt_xmode.size() )
    { // error: not enough ranges or misplaced
       std::string key("6_5");
       if( notes->inq( key, name) )
       {
         std::string capt("time record after the last time-table range");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << tt_id;
         ostr << "\nrec#=" << rec;
         ostr << "\ndate in record=" << getDateStr(currTimeValue) ;
         ostr << "\nrange from time-table="
              << tt_dateRange[0].str() << " - "
              << tt_dateRange[1].str() ;

         if( notes->operate(capt, ostr.str()) )
         {
            notes->setCheckTimeStr(fail);
            pQA->setExit( notes->getExitValue() ) ;
         }
       }

       timeTableMode = DISABLE ;
       return true;
    }

    splt_range = tt_xmode[tt_index++] ;

    sd[0] = splt_range[0];
    sd[1] = splt_range[1];

    getDRSformattedDateRange(tt_dateRange, sd);
  } while( refDate.getJulianDate(currTimeValue)
              > tt_dateRange[1].getJulianDate() ) ;

  tt_block_rec = rec ;
  tt_isBlock=true;

  return true;
}


void
QA_Time::openQA_NcContrib(NcAPI *nc)
{
   // dimensions
   nc->defineDim("time");
   nc->copyVarDef(pIn->nc, "time");

   std::vector<std::string> vs;

   // time increment
   std::string tU;

   // get time attribute units and extract keyword
   pIn->nc.getAttValues(vs, "units", "time") ;
   std::string tInc( vs[0].substr(0, vs[0].find(" since")) );

   vs.clear();
   vs.push_back( "time");

   currTimeValue=mv_t[ pIn->nc.getData(mv_t, name, 0) ] + refTimeOffset;
   nc->setAtt( "time", "first_time", currTimeValue);
   nc->setAtt( "time", "first_date", getDateStr(currTimeValue) );

   nc->defineVar( "time_step", NC_DOUBLE, vs);
   nc->setAtt( "time_step", "long_name", "time_step") ;
   nc->setAtt( "time_step", "units", tInc);

   nc->setAtt( "time", "isTimeBoundsTest", static_cast<double>(0.));

   vs.clear();
   std::string str0( name + "_flag") ;
   vs.push_back(name);
   nc->defineVar( str0, NC_INT, vs);
   vs[0]="accumulated record-tag number";
   nc->setAtt( str0, "long_name", vs[0]);
   nc->setAtt( str0, "units", "1");
   nc->setAtt( str0, "_FillValue", static_cast<int>(-1));
   vs[0]="tag number composite by Rnums of the check-list table.";
   nc->setAtt( str0, "comment", vs[0]);

   return;
}

void
QA_Time::setNextFlushBeg(size_t r)
{
   nextFlushBeg=r;
   sharedRecordFlag.setNextFlushBeg(r);
   timeOutputBuffer.setNextFlushBeg(r);

   return;
}

void
QA_Time::setTable(std::string &p, std::string t)
{
  tablePath=p;

  if( t.size() )
    timeTable=t;

  return;
}

bool
QA_Time::sync(bool isCheckData, bool enabledPostProc )
{
  // Synchronise the in-file and the qa-netCDF file.
  // Failure: call setExit(error_code).

  // return value: true for isNoProgress

  // num of recs in current data file
  size_t inRecNum = pIn->nc.getNumOfRecords() ;

  // Any records available in the qa-ncfile?
  size_t qaRecNum;
  if( (qaRecNum=pQA->nc->getNumOfRecords() ) == 0 )
  {
    // for a file with fixed variable(s)
    if( ! enabledPostProc )
      return true;
  }

  // get last time value from the previous file
  std::vector<double> dv;
  pQA->nc->getAttValues( dv, "last_time", pIn->timeName);
  double qa_t=dv[0];

  double inTime=0.;

  // Now, the sync-loop situations:
  // a) a previous QA worked on a complete infile and the
  //    time continues in a new infile.
  // b) a previous QA worked on an infile that was extended
  //    in the meanwhile.

  // Note: QA always continues a previous session

  for( size_t inRec=0 ; inRec < inRecNum ; ++inRec )
  {
    inTime = mv_t[ pIn->nc.getData(mv_t, name, inRec) ] + refTimeOffset ;

    if( qa_t == inTime )
    {
      if( (inRec+1) == inRecNum )
      {
        //nothing has changed since the last QA
        if( ! enabledPostProc )
          return true;
      }

      // case b)
      pIn->setCurrRec(inRec+1);

      return false ;
    }

    // case a)
    if( qa_t < inTime )
      return false ;  // the usual case
  }

  // arriving here is an error, because the infile production
  // was reset or the file was shortened.
  std::string key("80");
  if( notes->inq( key, name) )
  {
     std::string capt("renewal of a file?") ;

     std::ostringstream ostr(std::ios::app);
     ostr << "\nlast time of previous QA=" << qa_t;
     ostr << "\nfirst time in file=" << inTime;

     if( notes->operate(capt, ostr.str()) )
     {
       notes->setCheckMetaStr( fail );
       notes->setCheckTimeStr(fail);
       notes->setCheckDataStr(fail);

       pQA->setExit( notes->getExitValue() ) ;
     }
  }

  return true;
}

void
QA_Time::testTimeBounds(NcAPI &nc)
{
  // check time bounds
  if( timeTableMode == DISABLE )
  {
    prevTimeBoundsValue[0]=currTimeBoundsValue[0];
    prevTimeBoundsValue[1]=currTimeBoundsValue[1];
    return ;
  }

  getTimeBoundsValues(currTimeBoundsValue, pIn->currRec, refTimeOffset);

  double diff;

  if( isFormattedDate )
  {
    refDate.setDate( currTimeBoundsValue[0] );
    long double j0=refDate.getJulianDate();
    refDate.setDate( currTimeBoundsValue[1] );
    long double j1=refDate.getJulianDate();
    diff=static_cast<double>( j1-j0 ) ;
  }
  else
    diff=(currTimeBoundsValue[1] - currTimeBoundsValue[0]) ;

  if( diff <= 0. )
  {
    std::string key=("R8");  // no multiple
    if( notes->inq( key, name, "NO_MT") )
    {
      sharedRecordFlag.currFlag += 8 ;

      // store this text only once in an attribute
      std::string capt("negative/zero time-bounds range") ;

      std::ostringstream ostr(std::ios::app);
      ostr.setf( std::ios::fixed, std::ios::floatfield);

      ostr << "rec#:" << pIn->currRec;
      ostr << ", tb0= " << currTimeBoundsValue[0];
      ostr << ", tb1= " << currTimeBoundsValue[1] ;

      ostr  << " " << getDateStr(currTimeBoundsValue[0]);
      ostr  << " " << getDateStr(currTimeBoundsValue[1]);

      (void) notes->operate(capt, ostr.str()) ;
      notes->setCheckTimeStr(fail);
    }
  }

  if( isFormattedDate )
  {
    refDate.setDate( currTimeBoundsValue[0] );
    long double j0=refDate.getJulianDate();
    refDate.setDate( prevTimeBoundsValue[1] );
    long double j1=refDate.getJulianDate();
    diff=static_cast<double>( j1-j0 );
  }
  else
    diff = currTimeBoundsValue[0] - prevTimeBoundsValue[1] ;

  if( diff < 0. )
  {
    // ignore effect of least digits; rounding to the 5-th decimal
    double tb0 =
      hdhC::string2Double( hdhC::double2String(currTimeBoundsValue[0], -5) );
    double tb1 =
      hdhC::string2Double( hdhC::double2String(prevTimeBoundsValue[1], -5) );

    if( (tb0 - tb1 ) < 0. )
    {
      std::string key;
      bool isAcrossFiles = pIn->currRec == 0 ;

      if( isAcrossFiles || timeTableMode == CYCLE )
      {
        if( parseTimeTable(pIn->currRec) )
        {
          prevTimeBoundsValue[0]=currTimeBoundsValue[0];
          prevTimeBoundsValue[1]=currTimeBoundsValue[1];
          return ;  // no error messaging
        }

        key="6_10";
      }
      else
      {
        key="R16";
        sharedRecordFlag.currFlag += 16;
      }

      if( notes->inq( key, boundsName, "NO_MT") )
      {
        std::string capt("overlapping time bounds");
        if( isAcrossFiles )
          capt = " across files" ;

        std::ostringstream ostr(std::ios::app);
        ostr.setf(std::ios::fixed, std::ios::floatfield);
        ostr << "rec#="  << pIn->currRec << std::setprecision(0);
        ostr << "\nprev time values=[" << prevTimeBoundsValue[0] << " - " ;
        ostr                           << prevTimeBoundsValue[1] << "]" ;

        ostr << ", dates=[" << getDateStr(prevTimeBoundsValue[0]) << " - ";
        ostr << getDateStr(prevTimeBoundsValue[1]) << "]" ;

        ostr << "\ncurr time values=[" << currTimeBoundsValue[0]
                                       << " - " << currTimeBoundsValue[1] << "]" ;
        std::string cT( hdhC::double2String(currTimeBoundsValue[0]) );
        ostr << ", dates=[" << getDateStr(currTimeBoundsValue[0]) << " - ";
        ostr << getDateStr(currTimeBoundsValue[1]) << "]";

        (void) notes->operate(capt, ostr.str()) ;
        notes->setCheckTimeStr(fail);
      }
    }
  }
  else if( isRegularTimeSteps && diff > 0. )
  {
    // ignore effect of least digits; rounding to the 5-th decimal
    double tb0 =
      hdhC::string2Double( hdhC::double2String(prevTimeBoundsValue[1], -5) );
    double tb1 =
      hdhC::string2Double( hdhC::double2String(currTimeBoundsValue[0], -5) );

    if( isMaxDateRange && (tb1 - tb0 ) == 1. )
       tb0 += 1. ;

    if( (tb1 - tb0 ) > 0. )
    {
      std::string key;
      bool isAcrossFiles = pIn->currRec == 0 ;

      if( isAcrossFiles )
        key="6_11";
      else
      {
        key="R32";
        sharedRecordFlag.currFlag += 32 ;
      }

      if( notes->inq( key, boundsName, "NO_MT") )
      {
        std::string capt("gap between time bounds ranges");

        if( timeTableMode == CYCLE || isAcrossFiles )
        {
          if( parseTimeTable(pIn->currRec) )
          {
            prevTimeBoundsValue[0]=currTimeBoundsValue[0];
            prevTimeBoundsValue[1]=currTimeBoundsValue[1];
            return ;  // no error messaging
          }

          capt += " across files";
        }

        std::ostringstream ostr(std::ios::app);
        ostr.setf(std::ios::fixed, std::ios::floatfield);
        ostr << "rec#="  << pIn->currRec << std::setprecision(0);
        ostr << "\nprev time values=[" << prevTimeBoundsValue[0] << " - " ;
        ostr                           << prevTimeBoundsValue[1] << "]" ;

        ostr << ", dates=[" << getDateStr(prevTimeBoundsValue[0]) << " - ";
        ostr << getDateStr(prevTimeBoundsValue[1]) << "]" ;

        ostr << "\ncurr time values=[" << currTimeBoundsValue[0] << " - "
                                       << currTimeBoundsValue[1] << "]";
        ostr << ", dates=[" << getDateStr(currTimeBoundsValue[0]) << " - ";
        ostr << getDateStr(currTimeBoundsValue[1]) << "]";

        (void) notes->operate(capt, ostr.str()) ;
        notes->setCheckTimeStr( fail );
      }
    }
  }

  prevTimeBoundsValue[0]=currTimeBoundsValue[0];
  prevTimeBoundsValue[1]=currTimeBoundsValue[1];

  return ;
}

void
QA_Time::testDate(NcAPI &nc)
{
  currTimeValue = mv_t[ pIn->nc.getData(mv_t, name, pIn->currRec) ]
                    + refTimeOffset ;

  // is current time reasonable?
  (void) testTimeStep() ;

  if( isTimeBounds  )
    testTimeBounds(nc);

  prevTimeValue=currTimeValue;

  return;
}

bool
QA_Time::testTimeStep(void)
{
  // returns true, if an error was detected.

  // timeBound set here is a substitute if there are no
  // time_bnds in the netCDF

  // no time checks
  if( timeTableMode == DISABLE )
    return false;

  // time step(s) into the past
  // method returns true in case of error
  double dif;
  if( isFormattedDate )
  {
    refDate.setDate( prevTimeValue );
    long double j0=refDate.getJulianDate();

    refDate.setDate( currTimeValue );
    long double j1=refDate.getJulianDate();

    dif=static_cast<double>( j1-j0 );
  }
  else
    dif=currTimeValue - prevTimeValue;

  currTimeStep=dif;

  if( dif < 0. )
  {
    bool isAcrossFiles = pIn->currRec == 0 ;

    if( timeTableMode == CYCLE || isAcrossFiles)
    {
      // varMeDa[0] provides the name of the MIP table
      if( parseTimeTable(pIn->currRec) )
        return false ;  // no error messaging
    }

    std::string key;
    if( isAcrossFiles )
      key="6_12";
    else
    {
      key="R1";
      sharedRecordFlag.currFlag += 1;
    }

    if( notes->inq( key, name, "NO_MT") )
    {
      std::string capt ;
      std::ostringstream ostr(std::ios::app);

      if( isAcrossFiles )
      {
        capt = "overlapping time values across files" ;

        ostr << "last time of previous file=";
        ostr << prevTimeValue ;
        ostr << ", first time of this file=" ;
        ostr << currTimeValue << ", " ;
      }
      else
      {
        capt = "negative time step" ;

        ostr << "prev rec# ";
        ostr << (pIn->currRec-1) << ", time: " ;
        ostr << prevTimeValue ;
        ostr << ", curr " << "rec# " << pIn->currRec ;
        ostr << ", time:" << currTimeValue  ;
      }

      prevTimeValue=currTimeValue;

      if( notes->operate(capt, ostr.str()) )
      {
        notes->setCheckTimeStr( fail );
        notes->setCheckDataStr( fail );

        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    return true;
  }

  // identical time step
  if( prevTimeValue==currTimeValue  )
  {
    bool isAcrossFiles = pIn->currRec == 0 ;

    if( timeTableMode == CYCLE || isAcrossFiles )
    {
      // varMeDa[0] provides the MIP table
      if( parseTimeTable(pIn->currRec) )
        return false ;  // no error messaging
    }

    std::string key ;
    if( isAcrossFiles )
      key="6_14";
    else
      key="R4";

    if( ! isSingleTimeValue )
    {
      if( isAcrossFiles )
        sharedRecordFlag.currFlag += 4 ;

      if( notes->inq( key, name, "NO_MT") )
      {
        std::string capt("identical time values");
        std::ostringstream ostr(std::ios::app);

        if( isAcrossFiles )
           capt += " across files";

        if( isNoCalendar )
        {
          if( isAcrossFiles )
          {
            ostr << "last time of previous file=" << prevTimeValue ;
            ostr << ", first time of this file=" << currTimeValue  ;
          }
          else
          {
            ostr << "prev rec# " << (pIn->currRec-1) ;
            ostr << ", time=" << prevTimeValue ;
            ostr << ", curr rec# " << pIn->currRec ;
            ostr << ", time=" << currTimeValue ;
          }
        }

        if( notes->operate(capt, ostr.str()) )
        {
          notes->setCheckTimeStr( fail );

          pQA->setExit( notes->getExitValue() ) ;
        }
      }
    }

    return true;
  }

  // missing time step(s)
  if( isRegularTimeSteps &&
         dif > (1.25*referenceTimeStep ))
  {
    bool isAcrossFiles = pIn->currRec == 0 ;

    if( timeTableMode == CYCLE || isAcrossFiles )
    {
      // varMeDa[0] provides the MIP table
      if( parseTimeTable(pIn->currRec) )
        return false ;  // no error messaging
    }

    std::string key ;
    if( isAcrossFiles )
      key="6_13";
    else
      key="R2";

    if( notes->inq( key, name, "NO_MT") )
    {
      std::string capt;
      std::ostringstream ostr(std::ios::app);

      if( isAcrossFiles )
        capt = "gap between time values across files";
      else
      {
        capt = "missing time step";
        sharedRecordFlag.currFlag += 2 ;
      }

      if( isNoCalendar )
      {
        if( isAcrossFiles )
        {
          ostr << "last time of previous file=";
          ostr << prevTimeValue ;
          ostr << ", first time of this file=" ;
          ostr << currTimeValue << ", " ;
        }
        else
        {
          ostr << "prev rec# " << (pIn->currRec-1);
          ostr << ", time=" << prevTimeValue ;
          ostr << "curr rec# " << pIn->currRec ;
          ostr << ", time=" << currTimeValue ;
        }
      }
      else
      {
        if( isAcrossFiles )
        {
           ostr << "last time of previous file=" << prevTimeValue;
           ostr << " (date=" << getDateStr(prevTimeValue) ;
           ostr << "), first time of this file=" << currTimeValue
                << " (date=" << getDateStr(currTimeValue) << ")" ;

        }
        else
        {
           ostr << "prev=" << prevTimeValue;
           ostr << " (date=" << refDate.getDate(prevTimeValue).str() ;
           ostr << ")\ncurr=" << currTimeValue << " (date:"
                << getDateStr(currTimeValue) ;
           ostr << ")" ;
        }
      }

      if( notes->operate(capt, ostr.str()) )
      {
        notes->setCheckTimeStr(fail);

        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    return true;
  }

  return false;
}

// ===========  class TimeOutputBuffer ===============

TimeOutputBuffer::TimeOutputBuffer()
{
  buffer=0; // 0-pointer
  bufferCount=0;

  // may be changed with setFlushCounter()
  maxBufferSize=1500; //change with setFlushCounter

  nc=0;
}

TimeOutputBuffer::~TimeOutputBuffer()
{
  if( buffer )
    clear();
}

void
TimeOutputBuffer::clear(void)
{
  if( buffer )
  {
    delete buffer;
    delete buffer_step;

    bufferCount=0;
    buffer=0;
  }
}

void
TimeOutputBuffer::flush(void)
{
  // Flush temporarily arrayed values
  nc->putData(nextFlushBeg, bufferCount, name, buffer );

  if( nextFlushBeg )
    nc->putData(nextFlushBeg, bufferCount, name_step, buffer_step );
  else
    // Leave out the first time for fillingValue
    nc->putData(1, bufferCount-1, name_step, (buffer_step + 1) );

  nextFlushBeg += bufferCount;
  bufferCount = 0;

  return;
}

void
TimeOutputBuffer::initBuffer(NcAPI *n, size_t nxt, size_t mx)
{
  nc = n;
  maxBufferSize=mx;
  nextFlushBeg=nxt;

  if( buffer )
  {
     if( bufferCount )
       flush();

     clear();
  }

  buffer =       new double [maxBufferSize] ;
  buffer_step =  new double [maxBufferSize] ;

  bufferCount = 0;

  return;
}

void
TimeOutputBuffer::setName(std::string nm)
{
  name = nm;
  name_step = nm + "_step";
  return;
}

void
TimeOutputBuffer::store(double val, double step)
{
   // flush collected qa results to the qa-results netCDF file
   if( bufferCount == maxBufferSize )
     flush();  // resets countTime, too

   buffer[bufferCount]=val;
   buffer_step[bufferCount++]=step;

   return;
}
