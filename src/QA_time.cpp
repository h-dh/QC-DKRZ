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

  if( firstDate <= currDate )
  {
    // not true for a gap across a file, because then currDate
    // represents the last date from the previous file.
    std::string out( "PERIOD-BEG " );

    // if bounds are available, then use them
    if( isTimeBounds )
    {
      out += firstTimeBoundsDate[0].getDate();
      out += " - " ;
      out += lastTimeBoundsDate[1].getDate();;
    }
    else
    {
      out += firstDate.getDate() ;
      out += " - " ;
      out += lastDate.getDate() ;
    }

    out += "PERIOD-END";  // mark of the end of an output line
    std::cout << out << std::endl;
  }

  // write internal data to variable time
  nc->setAtt( "time", "last_time", lastTimeValue);
  nc->setAtt( "time", "last_date", lastDate.getDate() );
  nc->setAtt( "time", "isTimeBoundsTest", static_cast<double>(isTimeBounds));

  if( pIn->currRec )
  {
    // catch a very special one: the initial file got only a
    // single record and the period is longer than a day, then
    // the time step is twice as long.
    double tmp;

    if( pQA->currQcRec == 1 )  // due to the final loop increment
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

Date
QA_Time::getDate(std::string id, std::string bound)
{
  if( bound.size() == 0 )
  {
    if( id == "first" )
       return firstDate;
    else if( id == "curr" )
       return currDate;
    else if( id == "last" )
       return lastDate;
  }
  else if( isTimeBounds )
  {
    size_t pos=0;
    if( bound == "right" )
       pos=1;

    if( id == "first" )
    {
       if( ! firstTimeBoundsDate[pos].size() )
       {
         if( isFormattedDate )
           firstTimeBoundsDate[pos].setDate(
              Date::getIso8601(firstTimeBoundsValue[pos]), calendar) ;
         else
         {
           firstTimeBoundsDate[pos] = refDate ;
           firstTimeBoundsDate[pos].addTime(firstTimeBoundsValue[pos]);
         }
       }

       return firstTimeBoundsDate[pos];
    }
    else if( id == "curr" )
    {
       if( ! currTimeBoundsDate[pos].size() )
       {
         if( isFormattedDate )
           currTimeBoundsDate[pos].setDate(
              Date::getIso8601(currTimeBoundsValue[pos]), calendar) ;
         else
         {
           currTimeBoundsDate[pos] = refDate ;
           currTimeBoundsDate[pos].addTime(currTimeBoundsValue[pos]);
         }
       }

       return currTimeBoundsDate[pos];
    }
    else if( id == "last" )
    {
       if( ! lastTimeBoundsDate[pos].size() )
       {
         if( isFormattedDate )
           lastTimeBoundsDate[pos].setDate(
              Date::getIso8601(lastTimeBoundsValue[pos]), calendar) ;
         else
         {
           lastTimeBoundsDate[pos] = refDate ;
           lastTimeBoundsDate[pos].addTime(lastTimeBoundsValue[pos]);
         }
       }

       return lastTimeBoundsDate[pos];
    }
  }

  return Date();
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

bool
QA_Time::getTimeBounds(double *b, size_t curr)
{
  std::string name_bnds;

  for( size_t i=0 ; i < pIn->variable.size() ; ++i )
  {
     if( pIn->variable[i].name == "time" )
     {
       if( pIn->variable[i].bounds.size() )
       {
         name_bnds = pIn->variable[i].bounds ;
         break;
       }
     }
  }

  if( name_bnds.size() == 0 )
  {
    firstTimeBoundsValue[0]=0.;
    firstTimeBoundsValue[1]=0.;
    lastTimeBoundsValue[0]=0.;
    lastTimeBoundsValue[1]=0.;
    return false;
  }

  size_t start_x[2];
  size_t count_x[2];
  start_x[0] = curr;
  start_x[1] = 0;
  count_x[0] = 1;
  count_x[1] = 2;

  nc_get_vara_double(
       pIn->nc.getNcid(), pIn->nc.getVarID(name_bnds), start_x, count_x, b );

  return true;
}

void
QA_Time::init(InFile *p, Annotation *n, QA *q)
{
   pIn = p;
   pQA = q;
   notes = n;

   time=pIn->timeName;

   timeInputBuffer.init(pIn);

   timeOutputBuffer.initBuffer(pQA->nc, pQA->currQcRec);
   timeOutputBuffer.setName(time);

   sharedRecordFlag.initBuffer(pQA->nc, pQA->currQcRec);
   sharedRecordFlag.setName( time + "_flag" );

   // set date to a reference time
   std::string str(pIn->getTimeUnit());

   if( str.size() == 0 ) // fixed field
     return ;
   else if( str.find("%Y") < std::string::npos )
   {
     if( initTimeAbsolute(str) )
        return;
   }
   else
   {
     if( initTimeRelative(str) )
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

   refTimeDelay=0.; // !=0, if there are different reference dates

   bufferCount=0;
   maxBufferSize=1500;

  return;
}

bool
QA_Time::initTimeAbsolute(std::string &units)
{
  size_t i;
  for( i=0 ; i < pIn->variable.size() ; ++i)
    if( pIn->variable[i].name == time )
       break;

  if( i == pIn->variable.size() )
     return true;  // no time

  isFormattedDate = true;

  // proleptic Gregorian, no-leap, 360_day
  calendar = pIn->nc.getAttString("calendar", time);
  if( calendar.size() )
  {
     isNoCalendar=false;
     refDate.setCalendar(calendar);
     firstDate.setCalendar(calendar);
     currDate.setCalendar(calendar);
     lastDate.setCalendar(calendar);
     firstTimeBoundsDate[0].setCalendar(calendar);
     firstTimeBoundsDate[1].setCalendar(calendar);
     currTimeBoundsDate[0].setCalendar(calendar);
     currTimeBoundsDate[1].setCalendar(calendar);
     lastTimeBoundsDate[0].setCalendar(calendar);
     lastTimeBoundsDate[1].setCalendar(calendar);
  }

  // time_bounds
  disableTimeBoundsTest();
  timeBoundsName = pIn->variable[i].bounds;

  // time_bnds available? Yes, then enable a check
  if( timeBoundsName.size() )
     if( ! pIn->variable[i].isExcluded )
        enableTimeBoundsTest();

  refTimeDelay=0.;

  Split x_units(units);
  for( size_t i=0 ; i< x_units.size() ; ++i )
  {
     if( x_units[i][0] == '%' )
     {
        dateFormat = x_units[i] ;
        break;
     }
  }

  if( (currTimeValue=Base::getTime(pIn->nc, 0, time, refTimeDelay)) == MAXDOUBLE)
     return true;

  currDate.setDate( currTimeValue ) ;

  firstTimeValue = currTimeValue;
  firstDate = currDate ;

  lastTimeValue = Base::getTime( pIn->nc, pIn->nc.getNumOfRecords()-1,
                     time, refTimeDelay);
  lastDate.setDate( Date::getIso8601(lastTimeValue), calendar ) ;

  if( prevTimeValue == MAXDOUBLE )
  {
     size_t sz = pIn->getNumOfRecords() ;

     // 0) no value; caught elsewhere
     if( sz == 0 )
       return false;

     // 1) out of two time values within a file
     if( sz > 1 )
     {
       if( pIn->currRec < pIn->getNumOfRecords() )
       {
         prevTimeValue=Base::getTime(pIn->nc, 1, time, refTimeDelay);
         prevDate.setDate(prevTimeValue, calendar);
       }

       // an arbitrary setting that would pass the first test;
       // the corresponding results is set to FillValue
       referenceTimeStep=fabs(prevDate.getJulianDay() - currDate.getJulianDay());
       prevDate.addTime( -2.*referenceTimeStep ) ;

       std::string s(prevDate.getDate());
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
       prevDate=currDate;
       referenceTimeStep=0.;
     }

     if( isTimeBounds )
     {
       Base::getTime(pIn->nc, pIn->currRec, timeBoundsName, tmp_mv, refTimeDelay) ;
       Date d[2];
       d[0].setDate(tmp_mv[0], calendar);
       d[1].setDate(tmp_mv[1], calendar);
       double dtb = fabs( d[1].getJulianDay() - d[0].getJulianDay() );

       // valid for isFormatted true and false
       prevTimeBoundsDate[0]=d[0] ;
       prevTimeBoundsDate[0].addTime(-dtb) ;
       prevTimeBoundsDate[1]=d[1] ;
       prevTimeBoundsDate[1].addTime(-dtb) ;
     }
  }

  if( isTimeBounds )
  {
    getTimeBounds( firstTimeBoundsValue, 0 ) ;
    firstTimeBoundsDate[0].setDate(firstTimeBoundsValue[0]);
    firstTimeBoundsDate[1].setDate(firstTimeBoundsValue[1]);

    getTimeBounds( lastTimeBoundsValue, pIn->getNumOfRecords() - 1 ) ;
    lastTimeBoundsDate[0].setDate(lastTimeBoundsValue[0]);
    lastTimeBoundsDate[1].setDate(lastTimeBoundsValue[1]);
  }

  return false;
}

bool
QA_Time::initTimeRelative(std::string &units)
{
   size_t i;
   for( i=0 ; i < pIn->variable.size() ; ++i)
     if( pIn->variable[i].name == time )
        break;

   if( i == pIn->variable.size() )
      return true;  // no time

   // proleptic Gregorian, no-leap, 360_day
   calendar = pIn->nc.getAttString("calendar", time);

   if( calendar.size() )
   {
      isNoCalendar=false;
      refDate.setCalendar(calendar);
      currDate.setCalendar(calendar);
   }
   refDate.setDate( units );

   // time_bounds
   disableTimeBoundsTest();
   timeBoundsName = pIn->variable[i].bounds;

   // time_bnds available? Yes, then enable a check
   if( timeBoundsName.size() )
      if( ! pIn->variable[i].isExcluded )
         enableTimeBoundsTest();

   if( (currTimeValue=Base::getTime(pIn->nc, 0, time, refTimeDelay)) == MAXDOUBLE)
      return true;

   currDate   = refDate.getDate( hdhC::double2String(currTimeValue) ) ;
   firstDate = currDate ;
   firstTimeValue = currTimeValue ;

   lastDate = refDate ;
   lastTimeValue = Base::getTime( pIn->nc, pIn->nc.getNumOfRecords()-1,
                     time, refTimeDelay);
   lastDate.addTime( hdhC::double2String(lastTimeValue));

   if( prevTimeValue == MAXDOUBLE )
   {
     size_t sz = pIn->getNumOfRecords() ;

     // 0) no value; caught elsewhere
     if( sz == 0 )
       return false;

     // 1) out of two time values within a file
     if( sz > 1 )
     {
       double t1;
       if( pIn->currRec < pIn->getNumOfRecords() )
         t1=Base::getTime(pIn->nc, 1, time, refTimeDelay);
       else
         t1=2.*currTimeValue ;  // will raise an error message. ok.

       // an arbitrary setting that would pass the first test;
       // the corresponding results is set to FillValue
       prevTimeValue = 2.*currTimeValue -t1 ;

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

       prevTimeValue = currTimeValue - referenceTimeStep ;
     }

     if( isTimeBounds )
     {
       Base::getTime(pIn->nc, pIn->currRec, timeBoundsName, tmp_mv, refTimeDelay) ;
       double dtb = tmp_mv[1] - tmp_mv[0];

       prevTimeBoundsValue[0]=tmp_mv[0] - dtb ;
       prevTimeBoundsValue[1]=tmp_mv[1] - dtb  ;
     }
   }


   if( isTimeBounds )
   {
     getTimeBounds( firstTimeBoundsValue, 0 ) ;
     getTimeBounds( lastTimeBoundsValue, pIn->getNumOfRecords() - 1 ) ;
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
   pQA->nc->getAttValues( dv, "last_time", time);
   prevTimeValue=dv[0];

   pQA->nc->getAttValues( dv, "last_time_bnd_0", time);
   prevTimeBoundsValue[0]=dv[0];

   pQA->nc->getAttValues( dv, "last_time_bnd_1", time);
   prevTimeBoundsValue[1]=dv[0];

   pQA->nc->getAttValues( dv, "last_time_step", time + "_step");
   referenceTimeStep=dv[0];

   // case: two different reference dates are effective.
   std::string tu_0(pQA->nc->getAttString("units", time));
   std::string tu_1(pIn->nc.getAttString("units", time));
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
        refTimeDelay= d_x0.getSince( d_x1 );
      }
   }

   // get internal values
   isTimeBounds =
     static_cast<bool>(pQA->nc->getAttValue("isTimeBoundsTest", time));

   return;
}

void
QA_Time::initTimeTable(std::string id_1st, std::string id_2nd)
{
   if( timeTableMode == NONE )
     return;

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

     if( currDate < tt_dateRange[0] )
     { // error: time record out of range (before)
       std::string key("63_1");
       if( notes->inq( key, time) )
       {
         std::string capt("time value before the first time-table range");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << id_1st;
         ostr << "\nrec#=0" ;
         ostr << "\ndate in record=" << currDate.getDate() ;
         ostr << "\nrange from time-table=" ;
         ostr << tt_dateRange[0].getDate() << " - " ;
         ostr << tt_dateRange[1].getDate() ;

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

   if( timeTableMode == CYCLE && currDate < tt_dateRange[1])
     return false; //an error happened within the range

  // other modes are updated in cycles
  if( tt_xmode[0][0] == 'N' )
  {
     ++tt_count_recs ;
     size_t num = static_cast<size_t>(
           hdhC::string2Double( tt_xmode[0] ) ) ;

     if( tt_count_recs > num )
     {  // issue an error flag; number of records too large
        std::string key("63_4");
        if( notes->inq( key, time) )
        {
          std::string capt("too many time values compared to the time-table");

          std::ostringstream ostr(std::ios::app);
          ostr << "frequency=" << tt_id;
          ostr << "\nrec#=" << rec;
          ostr << "\ndate in record=" << currDate.getDate() ;
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
     std::string currDateStr( currDate.getDate());
     std::string t0( currDateStr.substr(0,4) );
     while( t0 > tt_xmode[tt_index] )
       ++tt_index;

     if( tt_index == tt_xmode.size() || t0 != tt_xmode[tt_index] )
     {
       std::string key("63_3");
       if( notes->inq( key, time) )
       {
         std::string capt("time record does not match time-table value");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << tt_id;
         ostr << "\nrec#=" << rec;
         ostr << "\ndate in record=" << currDate.getDate() ;
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
       std::string key("63_2");
       if( notes->inq( key, time) )
       {
         std::string capt("time record after the last time-table range");

         std::ostringstream ostr(std::ios::app);
         ostr << "frequency=" << tt_id;
         ostr << "\nrec#=" << rec;
         ostr << "\ndate in record=" << currDate.getDate() ;
         ostr << "\nrange from time-table="
              << tt_dateRange[0].getDate() << " - "
              << tt_dateRange[1].getDate() ;

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
  } while( currDate > tt_dateRange[1] ) ;

  tt_block_rec = rec ;
  tt_isBlock=true;

  return true;
}


void
QA_Time::openQcNcContrib(NcAPI *nc)
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

   currTimeValue=Base::getTime(pIn->nc, 0, "time", refTimeDelay);
   nc->setAtt( "time", "first_time", currTimeValue);

   if( isFormattedDate )
     nc->setAtt( "time", "first_date", Date::getIso8601(currTimeValue) );
   else
     nc->setAtt( "time", "first_date",
            refDate.getDate( hdhC::double2String(currTimeValue) ));

   nc->defineVar( "time_step", NC_DOUBLE, vs);
   nc->setAtt( "time_step", "long_name", "time_step") ;
   nc->setAtt( "time_step", "units", tInc);

   nc->setAtt( "time", "isTimeBoundsTest", static_cast<double>(0.));

   vs.clear();
   std::string str0( time + "_flag") ;
   vs.push_back(time);
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
QA_Time::sync(bool isCheckData, bool enablePostProc )
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
    if( ! enablePostProc )
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
    inTime = timeInputBuffer.getTimeValue(inRec, refTimeDelay) ;

    if( qa_t == inTime )
    {
      if( (inRec+1) == inRecNum )
      {
        //nothing has changed since the last QA
        if( ! enablePostProc )
          return true;
      }

      // case b)
      pIn->setCurrRec(inRec+1);

      // read for the changed record number
      if( isCheckData )
        pIn->getData( pIn->currRec );
      return false ;
    }

    // case a)
    if( qa_t < inTime )
      return false ;  // the usual case
  }

  // arriving here is an error, because the infile production
  // was reset or the file was shortened.
  std::string key("80");
  if( notes->inq( key, time) )
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

  timeInputBuffer.getTimeBnds(currTimeBoundsValue, pIn->currRec);

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
    if( notes->inq( key) )
    {
      sharedRecordFlag.currFlag += 8 ;

      // store this text only once in an attribute
      std::string capt("negative/zero time-bounds range") ;

      std::ostringstream ostr(std::ios::app);
      ostr.setf( std::ios::fixed, std::ios::floatfield);

      ostr << "rec#:" << pIn->currRec;
      ostr << ", tb0= " << currTimeBoundsValue[0];
      ostr << ", tb1= " << currTimeBoundsValue[1] ;

      if( isFormattedDate )
      {
        ostr  << " " << Date::getIso8601(currTimeBoundsValue[0]);
        ostr  << " " << Date::getIso8601(currTimeBoundsValue[1]);
      }
      else
      {
        std::string cT( hdhC::double2String(currTimeBoundsValue[0]) );
        ostr  << " " << refDate.getDate( cT );
        cT = hdhC::double2String(currTimeBoundsValue[1]) ;
        ostr  << " " << refDate.getDate( cT ) ;
      }

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
      std::string key=("R16");
      if( notes->inq( key) )
      {
        sharedRecordFlag.currFlag += 16;

        std::string capt("overlapping time bounds ranges");
        if( pIn->currRec == 0 )
        {
          if( parseTimeTable(pIn->currRec) )
          {
            prevTimeBoundsValue[0]=currTimeBoundsValue[0];
            prevTimeBoundsValue[1]=currTimeBoundsValue[1];
            return ;  // no error messaging
          }

          capt += " across files";
        }

        if( timeTableMode == CYCLE )
        {
          if( parseTimeTable(pIn->currRec) )
          {
            prevTimeBoundsValue[0]=currTimeBoundsValue[0];
            prevTimeBoundsValue[1]=currTimeBoundsValue[1];
            return ;  // no error messaging
          }
        }

        std::ostringstream ostr(std::ios::app);
        ostr.setf(std::ios::fixed, std::ios::floatfield);
        ostr << "rec#="  << pIn->currRec << std::setprecision(0);
        ostr << "\nprev time values=[" << prevTimeBoundsValue[0] << " - " ;
        ostr                           << prevTimeBoundsValue[1] << "]" ;

        if( ! isFormattedDate )
        {
          std::string cT( hdhC::double2String(prevTimeBoundsValue[0]) );
          ostr << ", dates=[" << refDate.getDate( cT ) << " - ";
          cT = hdhC::double2String(prevTimeBoundsValue[1]) ;
          ostr << refDate.getDate( cT ) << "]" ;
        }

        ostr << "\ncurr time values=[" << currTimeBoundsValue[0]
                                       << " - " << currTimeBoundsValue[1] << "]" ;
        if( ! isFormattedDate )
        {
          std::string cT( hdhC::double2String(currTimeBoundsValue[0]) );
          ostr << ", dates=[" << refDate.getDate( cT ) << " - ";
          cT = hdhC::double2String(currTimeBoundsValue[1]) ;
          ostr << refDate.getDate( cT ) << "]";
        }

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
      std::string key=("R32");
      if( notes->inq( key) )
      {
        sharedRecordFlag.currFlag += 32 ;
        std::string capt("gap between adjacent time bounds ranges");
        std::ostringstream ostr(std::ios::app);

        if( pIn->currRec == 0 )
        {
          if( parseTimeTable(pIn->currRec) )
          {
            prevTimeBoundsValue[0]=currTimeBoundsValue[0];
            prevTimeBoundsValue[1]=currTimeBoundsValue[1];
            return ;  // no error messaging
          }

          capt += " across files";
        }

        if( timeTableMode == CYCLE )
        {
          if( parseTimeTable(pIn->currRec) )
          {
            prevTimeBoundsValue[0]=currTimeBoundsValue[0];
            prevTimeBoundsValue[1]=currTimeBoundsValue[1];
            return ;  // no error messaging
          }
        }

        ostr.setf(std::ios::fixed, std::ios::floatfield);
        ostr << "rec#="  << pIn->currRec << std::setprecision(0);
        ostr << "\nprev time values=[" << prevTimeBoundsValue[0] << " - " ;
        ostr                           << prevTimeBoundsValue[1] << "]" ;

        if( ! isFormattedDate )
        {
          std::string cT( hdhC::double2String(prevTimeBoundsValue[0]) );
          ostr << ", dates=[" << refDate.getDate( cT ) << " - ";
          cT = hdhC::double2String(prevTimeBoundsValue[1]) ;
          ostr << refDate.getDate( cT ) << "]" ;
        }

        ostr << "\ncurr time values=[" << currTimeBoundsValue[0] << " - "
                                       << currTimeBoundsValue[1] << "]";

        if( ! isFormattedDate )
        {
          std::string cT( hdhC::double2String(currTimeBoundsValue[0]) );
          ostr << ", dates=[" << refDate.getDate( cT ) << " - ";
          cT = hdhC::double2String(currTimeBoundsValue[1]) ;
          ostr << refDate.getDate( cT ) << "]";
        }

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
  currTimeValue = timeInputBuffer.getTimeValue(pIn->currRec, refTimeDelay) ;

  if( isFormattedDate )
  {
    currDate.setDate( currTimeValue ) ;
  }
  else
  {
    currDate   = refDate.getDate( currTimeValue ) ;
  }

  // is current time reasonable?
  (void) testTimeStep() ;

  if( isTimeBounds  )
    testTimeBounds(nc);
}

bool
QA_Time::testTimeStep(void)
{
  // returns true, if an error was detected.

  // timeBound set here is a substitute if there are no
  // time_bnds in the netCDF

  // no time checks
  if( timeTableMode == DISABLE )
  {
    prevTimeValue=currTimeValue;
    return false;
  }

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
    if( timeTableMode == CYCLE )
    {
      // varMeDa[0] provides the name of the MIP table
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }
    }

    std::string capt("negative time step");
    if( pIn->currRec == 0 )
    {
      // varMeDa[0] provides the MIP table
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }

      capt +=" across files";
    }

    std::string key=("R1");
    if( notes->inq( key) )
    {
      sharedRecordFlag.currFlag += 1;

      std::ostringstream ostr(std::ios::app);

      if( pIn->currRec == 0 )
      {
        ostr << "last time of previous file=";
        ostr << prevTimeValue ;
        ostr << ", first time of this file=" ;
        ostr << currTimeValue << ", " ;
      }
      else
      {
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
    std::string capt("identical time step");

    if( pIn->currRec == 0 )
    {
      // varMeDa[0] provides the MIP table
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }

      capt += " across files";
    }

    if( timeTableMode == CYCLE )
    {
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }
    }

    std::string key=("R4");
    if( ! isSingleTimeValue && notes->inq( key) )
    {
      sharedRecordFlag.currFlag += 4 ;

      std::ostringstream ostr(std::ios::app);

      if( isNoCalendar )
      {
        if( pIn->currRec == 0 )
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

    prevTimeValue=currTimeValue;

    return true;
  }

      // missing time step(s)

  if( isRegularTimeSteps &&
         dif > (1.25*referenceTimeStep ))
  {
    std::string capt("missing time step");

    if( pIn->currRec == 0 )
    {
      // varMeDa[0] provides the MIP table
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }

      capt +=" across files";
    }

    if( timeTableMode == CYCLE )
    {
      if( parseTimeTable(pIn->currRec) )
      {
        prevTimeValue=currTimeValue;
        return false ;  // no error messaging
      }
    }

    std::string key=("R2");
    if( notes->inq( key) )
    {
      sharedRecordFlag.currFlag += 2 ;
      std::ostringstream ostr(std::ios::app);

      if( isNoCalendar )
      {
        if( pIn->currRec == 0 )
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
        if( pIn->currRec == 0 )
        {
           std::string cT( hdhC::double2String(prevTimeValue) );
           ostr << "last time of previous file=" << prevTimeValue;
           ostr << " (date=" << refDate.getDate( cT ) ;
           ostr << "), first time of this file=" << currTimeValue
                << " (date=" << currDate.getDate() << ")" ;

        }
        else
        {
           std::string cT( hdhC::double2String(prevTimeValue) );
           ostr << "prev=" << prevTimeValue;
           ostr << " (date=" << refDate.getDate( cT ) ;
           ostr << ")\ncurr=" << currTimeValue << " (date:" << currDate.getDate() ;
           ostr << ")" ;
        }
      }

      if( notes->operate(capt, ostr.str()) )
      {
        notes->setCheckTimeStr(fail);

        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    prevTimeValue=currTimeValue;

    return true;
  }

  prevTimeValue=currTimeValue;

  return false;
}

// ===========  class TimeInputBuffer ===============

void
TimeInputBuffer::getTimeBnds(double *b, size_t curr, double offset)
{
  if( ! hasBnds )
    return;

  if( curr < sz_passed )
  {
    size_t start_x[2];
    size_t count_x[2];
    start_x[0] = curr;
    start_x[1] = 0;
    count_x[0] = 1;
    count_x[1] = 2;

    nc_get_vara_double(
       pInNc->getNcid(), pInNc->getVarID(name_bnds), start_x, count_x, b );

    return;
  }

  // b is a pointer to a 2-elem variable
  while( curr >= start[0] )
    update(offset);

  size_t c = 2*(curr - sz_passed ) ;
  b[0] = pBnds[c] ;
  b[1] = pBnds[c+1] ;

  return ;
}

double
TimeInputBuffer::getTimeValue(size_t curr, double offset)
{
  if( hasNoTime )
    return 0.;

  if( curr < sz_passed )
  {
    // reading a value which is out of scope and not ready for an update
    size_t start_x = curr;
    size_t count_x;
    start_x = curr;
    count_x = 1;
    double v;

    nc_get_vara_double(
       pInNc->getNcid(), pInNc->getVarID(name), &start_x, &count_x, &v );
    return v + offset ;
  }

  while( curr >= start[0] )
    update(offset);

  return pTime[curr - sz_passed] ;
}

void
TimeInputBuffer::init(InFile *pIn)
{
  pInNc = &pIn->nc;
  if( (remainingRecs=static_cast<int>(pInNc->getNumOfRecords()) ) )
    isValid=true;
  else
    isValid=false;

  hasNoTime = true;
  hasBnds = false;

  name = pInNc->getUnlimitedDimVarName() ;
  if( name.size() )
    hasNoTime = false;

  for( size_t i=0 ; i < pIn->variable.size() ; ++i )
  {
     if( pIn->variable[i].name == name )
     {
       name_bnds = pIn->variable[i].bounds ;
       if( name_bnds.size() )
       {
         hasBnds = true;
         break;
       }
     }
  }

  if( hasNoTime )
    return;

  // constrain memory allocation to a buffer size
  sz_chunk=10000;
  if(remainingRecs < sz_chunk)
    sz_chunk = remainingRecs +1 ;

  start = new size_t [1];
  count = new size_t [1];
  start[0] = 0 ;

  pTime = new double [sz_chunk] ;
  if( name_bnds.size() )
  {
    pBnds = new double [2 * sz_chunk] ;

    start_b = new size_t [2];
    count_b = new size_t [2];
    start_b[0] = 0 ;
    start_b[1] = 0 ;
    count_b[1] = 2;
  }

  sz_passed=0 ; // forces first time approach with first time reading
}

void
TimeInputBuffer::update(double offset)
{
  // read data from file, leg by leg
  if(remainingRecs > 0)
  {
    int nnRecs=remainingRecs;
    sz = nnRecs > sz_chunk ? sz_chunk : nnRecs;
    remainingRecs -= sz ;

    count[0] = sz ;
    sz_passed = start[0];

    nc_get_vara_double(
       pInNc->getNcid(), pInNc->getVarID(name), start, count, pTime );

    if( offset != 0. )
       for(size_t i = *start ; i < ( *start + *count ) ; ++i )
          pTime[i] += offset ;

    if( hasBnds )
    {
      count_b[0] = sz ;

      nc_get_vara_double(
         pInNc->getNcid(), pInNc->getVarID(name_bnds), start_b, count_b, pBnds );

      if( offset != 0. )
      {
         size_t end= *start_b + 2* *count_b;
         for(size_t i = *start_b ; i < end ; ++i )
            pBnds[i] += offset ;
      }

      start_b[0] += sz ;
    }

    start[0] += sz ;
    return;
  }

  delete [] pTime ;
  delete [] start ;
  delete [] count ;
  pTime = 0 ;  // pointer is set

  if( hasBnds )
  {
    delete [] pBnds ;
    pBnds = 0 ;  // pointer is set

    delete [] start_b ;
    delete [] count_b ;
  }

  return;
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
