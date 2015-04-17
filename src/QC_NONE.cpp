#include "qc.h"

// Macro option to enable output of all messages.
// Please compile with '-D RAISE_ERRORS'

QC::QC()
{
  initDefaults();
}

QC::~QC()
{
  if( nc )
    delete nc ;
}

void
QC::appendToHistory(size_t eCode)
{
  // date and time at run time
  std::string today( Date::getCurrentDate() );
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
      if( dataPath != hstPath )
      {
        hst += "\n" ;
        hst += today;
        hst += " changed path to data=";
        hst += dataPath + "\n" ;
      }
    }
  }
  else
  {
    // the root of the history string
    hst += today;
    hst += " path_to_data=";
    hst += dataPath ;
/*
    hst += "\nfilenames and tracking_id in file tid_";

    std::string t0(qcFilename.substr(3));
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
    for( ; ; --index )
    {
      if( splt[index] == "revision:" )
        break;

      // note: i==0; --i; i== 4294967295
      if( index == 0 )
        break;
    }

    std::string tmp ;
    if( index > 0 )
      // version number in the next position
      tmp = splt[++index] ;
    else
      // not in the history, so take the one from the version attribute
      tmp = nc->getAttString("QC_svn_revision");

    if( svnVersion != tmp )
    {
      hst += "\n" ;
      hst += today;
      hst += " changed QC svn revision=" ;
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
QC::applyOptions(bool isPost)
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
          qcFilename=split[1];
          continue;
       }
     }

     if( split[0] == "fS" || split[0] == "FileSequence"
         || split[0] == "file_sequence" )
     {
       if( split.size() == 2 )
       {
          fileSequenceState=split[1][0];
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

     if( split[0] == "qNF" || split[0] == "qcNcfileFlags"
       || split[0] == "qc_ncfile_flags" )
     {
       if( split.size() == 2 )
       {
          qcNcfileFlags=split[1];
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

   }

   return;
}

void
QC::checkMetaData(InFile &in)
{
  notes->setCheckMetaStr("PASS");

  //Check tables properties: path and names.
  // program exits for an invalid path
  inqTables();

  // Read or write the project table.
  ProjectTable projectTable(this, &in, tablePath, projectTableName);
  projectTable.setAnnotation(notes);
  projectTable.setExcludedAttributes( excludedAttribute );

  projectTable.check();

  // inquire whether the meta-data checks passed
  int ev;
  if( (ev = notes->getExitValue()) > 1 )
    setExit( ev );

  return;
}

void
QC::closeEntry(void)
{
   // This here is only for the regular QC time series file
   if( isCheckTime )
     storeTime();

   if( isCheckData )
   {
     // data: structure defined in hdhC.h
     std::vector<hdhC::FieldData> fA;
     for( size_t i=0 ; i < varMeDa.size() ; ++i )
     {
       if( varMeDa[i].var->isNoData )
          continue;

       // skip time test for proceeding time steps when var is fixed
       if( isNotFirstRecord && varMeDa[i].var->isFixed  )
         continue;

       fA.push_back( varMeDa[i].var->pDS->get() ) ;

       // test overflow of ranges specified in a table, or
       // plausibility of the extrema.
       varMeDa[i].qcData.test(i, fA.back() );
     }

     storeData(fA);
   }

   ++currQcRec;

   return;
}

void
QC::createVarMetaData(void)
{
  // set corresponding isExcluded=true
  pIn->excludeVars();

  // create instances of VariableMetaData:375. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in vector in.dataVarIndex.

  for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pIn->variable[pIn->dataVarIndex[i]];

    //push next instance
    pushBackVarMeDa( &var );
  }

   // very special: discard particular tests
  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    VariableMetaData &vMD = varMeDa[i] ;

    Split splt(vMD.dims);
    int effDim = splt.size() ;
    for( size_t j=0 ; j < splt.size() ; ++j )
      if( splt[j] == qcTime.time )
        --effDim;

    if( replicationOpts.size() )
    {
      if( ReplicatedRecord::isSelected(
             replicationOpts, vMD.name, enablePostProc, effDim ) )
      {
        vMD.qcData.replicated = new ReplicatedRecord(this, i, vMD.name);
        vMD.qcData.replicated->setAnnotation(notes);
        vMD.qcData.replicated->parseOption(replicationOpts) ;
      }
    }

    if( outlierOpts.size() )
    {
      if( Outlier::isSelected(
             outlierOpts, vMD.name, enablePostProc, effDim ) )
      {
        vMD.qcData.outlier = new Outlier(this, i, vMD.name);
        vMD.qcData.outlier->setAnnotation(notes);
        vMD.qcData.outlier->parseOption(outlierOpts);
      }
    }
  }

  return;
}

bool
QC::entry(void)
{
   if( isCheckData )
   {
     // read next field
     pIn->entry() ;

     for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
     {
       Variable& var = pIn->variable[ pIn->dataVarIndex[i] ];

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
QC::finally(int eCode)
{
  if( nc )
    setExit( finally_data(eCode) );

  // distinguish from a sytem crash (segmentation error)
//  notes->print() ;
  std::cout << "STATUS-BEG" << exitCode << "STATUS-END";
  std::cout << std::flush;

  setExit( exitCode ) ;

  return exitCode ;
}

int
QC::finally_data(int eCode)
{
  setExit(eCode);

  // write pending results to qc-file.nc. Modes are considered there
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
         if( varMeDa[i].qcData.enableOutlierTest )
           varMeDa[i].qcData.outlier->test( &(varMeDa[i].qcData) );
  }

  if( exitCode == 63 ||
     ( nc == 0 && exitCode ) || (currQcRec == 0 && pIn->isTime ) )
  { // qc is up-to-date or a forced exit right from the start;
    // no data to write
    nc->close();

    if( exitCode == 63 )
      exitCode=0 ;

    isExit=true;
    return exitCode ;
  }

  if( exitCode != 63 )
    qcTime.finally( nc );

  // read history from the qc-file.nc and append new entries
  appendToHistory(exitCode);

  // check for flags concerning the total data set,
  // but exclude the case of no record
  if( pIn->currRec > 0 )
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
      varMeDa[j].qcData.checkFinally(varMeDa[j].var);

  if( isCheckData )
  {
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
    {
       // write qc-results attributes about statistics
       varMeDa[j].qcData.setStatisticsAttribute(nc);
    }
  }

  nc->close();

  return exitCode ;
}

std::string
QC::getFrequency()
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;
  else
  {
    // not found, but error issue is handled elsewhere

    // try the filename
    std::string f( pIn->filename );
    size_t pos;
    if( (pos=f.rfind('/')) < std::string::npos )
      f = f.substr(pos+1);

    if( f.rfind(".nc" ) )
      f = f.substr( 0, f.size()-3 );  // strip ".nc"

    Split splt(f, "_");

    // test the last two items for time type. If both are,
    // then this would mean that they are separated by '_'.
    // This would be a fault for CORDEX.
    size_t off=0;  // takes into account a period separator '_'
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

    if( frequency.size() )
       return frequency;
  }

  // last resort: automatically from time values
  if( pIn->nc.getNumOfRecords() > 1 )
  {
    double t0 = Base::getTime(pIn->nc, 0, pIn->timeName);
    double t1 = Base::getTime(pIn->nc, 1, pIn->timeName);
    double dt=t1-t0;

    // only for units day (since ...)
    if ( dt > 27 && dt < 32)
      frequency = "mon";
    else if ( dt > 89. && dt < 93. )
      frequency = "sem";
    else if ( dt > 0.9 && dt < 1.1 )
      frequency = "day";
    else if ( dt > 0.2 && dt < 0.3 )
      frequency = "6hr";
    else if ( dt > 0.1 && dt < 0.15)
      frequency = "3hr";
  }

  return frequency ;
}

void
QC::help(void)
{
  std::cerr << "Option string of the quality control class QC:\n" ;
  std::cerr << "(may be embedded in option strings of Base derived\n" ;
  std::cerr << "classes)\n";
  std::cerr << "or connected to these by explicit index 'QC0...'.\n" ;
  std::cerr << "   checkTimeBounds\n" ;
  std::cerr << "   noCalendar\n" ;
  std::cerr << "   cycle=num\n" ;
  std::cerr << "   printASCII (disables writing to netCDF file.\n" ;
  std::cerr << "   printTimeBoundDates\n" ;
  std::cerr << std::endl;
  return;
}

bool
QC::init(void)
{
   // Open the qc-result.nc file, when available or create
   // it from scratch. Initialise time testing and
   // time boundary testing.
   // Eventually, entry() is called to test the data of fields.
   // Initial values are set such that they do not cause any
   // harm in testDate() called in closeEntry().

   notes->init();  // safe
   setFilename(pIn->filename);

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

      std::string key("64");
      if( notes->inq( key, fileStr) )
      {
        std::string capt("empty data section in the file") ;

        std::string text("Is the data section in the file empty? Please, check.");

        if( notes->operate(capt, text) )
        {
          notes->setCheckTimeStr( fail );
          notes->setCheckDataStr( fail );
          setExit( notes->getExitValue() ) ;
        }
      }
   }

   // get meta data from file and compare with tables
   checkMetaData(*pIn);

   if( isCheckTime && pIn->isTime )
   {
     // init the time object
     // note that freq is compared to the first column of the time table
     qcTime.init(pIn, notes, this);
     qcTime.applyOptions(optStr);
     qcTime.initTimeTable( getFrequency() );

     // note that this test is not part of the QC_Time class, because
     // coding depends on projects
     if( testPeriod() )
     {
        std::string key("82");
        if( notes->inq( key, qcTime.time) )
        {
          std::string capt("status is apparently in progress");

          (void) notes->operate(capt) ;
       }
     }
   }

   if(isExit)
     return true;

   // open netCDF for creating, continuation or resuming qc_<varname>.nc
   openQcNc(*pIn);

   if( isExit || isNoProgress )
   {
     isCheckData=false;
     isCheckTime=false;
     return true;
   }

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
QC::initDataOutputBuffer(void)
{
  if( isCheckTime )
  {
    qcTime.timeOutputBuffer.initBuffer(nc, currQcRec);
    qcTime.sharedRecordFlag.initBuffer(nc, currQcRec);
  }

  if( isCheckData )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i)
      varMeDa[i].qcData.initBuffer(nc, currQcRec);
  }

  return;
}

void
QC::initDefaults(void)
{
  setObjName("QC");

  // pre-setting of some pointers
  nc=0;

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qC=0;
  tC=0;

  fail="FAIL";
  notAvailable="not available";
  fileStr="file";
  fileTableMismatch="mismatch between file and table.";

  enablePostProc=false;
  enableVersionInHistory=true;

  isCaseInsensitiveVarName=false;
  isNoProgress=false;
  isExit=false;
  isInstantaneous=false;
  isNotFirstRecord=false;
  isResumeSession=false;

  nextRecords=0;  //see init()

  importedRecFromPrevQC=0; // initial #rec in out-nc-file.
  currQcRec=0;

  // by default
  tablePath="./";

  // pre-set check-modes: all are used by default
  isCheckMeta=true;
  isCheckTime=true;
  isCheckData=true;

  exitCode=0;

  // file sequence: f[first], s[equence], l[ast]
  fileSequenceState='x' ;  // 'x':= undefined

#ifdef SVN_VERSION
  // -1 by default
  svnVersion=hdhC::double2String(static_cast<int>(SVN_VERSION));
#endif

  // set pointer to member function init()
  execPtr = &IObj::init ;
}

void
QC::initGlobalAtts(InFile &in)
{
  // global atts at creation.
  std::string today( Date::getCurrentDate() );

  nc->setGlobalAtt( "QC_svn_revision", svnVersion);
  nc->setGlobalAtt( "contact", "hollweg@dkrz.de");

  nc->setGlobalAtt( "creation_date", today);

  // helper vector
  std::vector<std::string> vs;

  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    vs.push_back( varMeDa[m].name );

  nc->copyAtts(in.nc, "NC_GLOBAL", &vs);

  return;
}

void
QC::initResumeSession(void)
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
       std::string key("33a");
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
       std::string key("33b");
       if( notes->inq( key, varMeDa[j].name) )
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
  qcTime.timeOutputBuffer.setNextFlushBeg(currQcRec);
  qcTime.setNextFlushBeg(currQcRec);

  if( isCheckTime )
    qcTime.initResumeSession();

  if( isCheckData )
  {
    for( size_t i=0 ; i < varMeDa.size() ; ++i )
    varMeDa[i].qcData.initResumeSession();
  }

  return;
}

void
QC::inqTables(void)
{
  // check tablePath; must exist
  std::string testFile("/bin/bash -c \'test -d ") ;
  testFile += tablePath ;
  testFile += '\'' ;

  // see 'man system' for the return value, here we expect 0,
  // if directory exists.

  if( system( testFile.c_str()) )
  {
     std::string key("70_3");
     if( notes->inq( key, fileStr) )
     {
        std::string capt("no path to the tables") ;

        std::string text("No path to the tables; tried ");
        text += tablePath + ".";
        text += "\nPlease, check the setting in the configuration file.";

        if( notes->operate(capt, text) )
        {
          notes->setCheckMetaStr( fail );
          setExit( notes->getExitValue() ) ;
        }
     }
  }

  // tables names usage: both project and standard tables
  // reside in the same path.
  // Naming of the project table:
  if( projectTableName.size() == 0 )
    projectTableName = "pt_NONE";
  else if( projectTableName.find(".csv") == std::string::npos )
    projectTableName += ".csv" ;

  return;
}

void
QC::linkObject(IObj *p)
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
  else if( className == "QC" )
    qC = dynamic_cast<QC*>(p) ;
  else if( className == "TC" )
    tC = dynamic_cast<TimeControl*>(p) ;

  return;
}

void
QC::openQcNc(InFile &in)
{
  // Generates a new nc file for QC results or
  // opens an existing one for appending data.
  // Copies time variable from input-nc file.

  // name of the file begins with qc_
  if ( qcFilename.size() == 0 )
  {
    // use the input filename as basis;
    // there could be a leading path
    qcFilename = dataPath;
    if( qcFilename.size() > 0 )
      qcFilename += '/' ;
    qcFilename += "qc_";
    qcFilename += hdhC::getBasename(dataFilename);
    qcFilename += ".txt";
  }

  nc = new NcAPI;
  if( notes )
    nc->setNotes(notes);

  // don't create a netCDF file, when only meta data are checked.
  // but, a NcAPI object m ust exist
  if( ! isCheckTime )
    return;

  if( nc->open(qcFilename, "NC_WRITE", false) )
//   if( isQC_open ) // false: do not exit in case of error
  {
    // continue a previous session
    importedRecFromPrevQC=nc->getNumOfRecords();
    currQcRec += importedRecFromPrevQC;
    if( currQcRec )
      isNotFirstRecord = true;

    initDataOutputBuffer();
    qcTime.sharedRecordFlag.initBuffer(nc, currQcRec);

    initResumeSession();
    isResumeSession=true;

    // if files are synchronised, i.e. a file hasn't changed since
    // the last qc
    if( isCheckTime )
      isNoProgress = qcTime.sync( isCheckData, enablePostProc );

    return;
  }

  if( currQcRec == 0 && in.nc.getNumOfRecords() == 1 )
    qcTime.isSingleTimeValue = true;

  // So, we have to generate a netCDF file from almost scratch;

  // open new netcdf file
  if( qcNcfileFlags.size() )
    nc->create(qcFilename,  qcNcfileFlags);
  else
    nc->create(qcFilename,  "Replace");

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

    qcTime.openQcNcContrib(nc);
  }
  else if( isCheckTime )
  {
    // dimensions
    qcTime.time="fixed";
    nc->defineDim("fixed", 1);
  }

  // create variable for the data statics etc.
  for( size_t m=0 ; m < varMeDa.size() ; ++m )
    varMeDa[m].qcData.openQcNcContrib(nc, varMeDa[m].var);

  // global atts at creation.
  initGlobalAtts(in);

  initDataOutputBuffer();

  return;
}

bool
QC::postProc(void)
{
  bool retCode=false;

  if( ! isCheckData )
    return retCode;

  if( postProc_outlierTest() )
     retCode=true;

  return retCode;

}

bool
QC::postProc_outlierTest(void)
{
  bool retCode=false;

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
     VariableMetaData &vMD = varMeDa[i] ;

     if( ! vMD.qcData.enableOutlierTest )
        continue ;

     if( ! vMD.qcData.statAve.getSampleSize()  )
     {
       // compatibility mode: determine the statistics of old-fashioned results
       // note: the qc_<variable>.nc file becomes updated
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
           vMD.qcData.statMin.setSampleProperties( statStr );
         else if( j == 1 )
           vMD.qcData.statMax.setSampleProperties( statStr );
         else if( j == 2 )
           vMD.qcData.statAve.setSampleProperties( statStr );
         else if( j == 3 )
           vMD.qcData.statStdDev.setSampleProperties( statStr );

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
             vMD.qcData.statMin.add( vals_min, sz );
             vMD.qcData.statMax.add( vals_max, sz );
             vMD.qcData.statAve.add( vals_ave, sz );
             vMD.qcData.statStdDev.add( vals_std_dev, sz );
           }

           // write statistics attributes
           vMD.qcData.setStatisticsAttribute(nc);
         }
       }
     }

     // the test: regular post-processing on the basis of stored statistics
     if( vMD.qcData.outlier->test(&vMD.qcData) )
       retCode = true;
  }

  return retCode;
}

void
QC::pushBackVarMeDa(Variable *var)
{
   varMeDa.push_back( VariableMetaData(this, var) );

   if( var )
   {
     VariableMetaData &vMD = varMeDa.back();

     vMD.forkAnnotation(notes);

     // disable tests by given options
     vMD.qcData.disableTests(var->name);

     vMD.qcData.init(pIn, this, vMD.var->name);
   }

   return;
}

void
QC::setFilename(std::string f)
{
  dataFile = f;
  dataPath = hdhC::getPath(f);
  dataFilename = hdhC::getFilename(f);

  return;
}

void
QC::setCheckMode(std::string m)
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
QC::setExit( int e )
{
  if( e > exitCode )
  {
    exitCode=e;
    isExit=true;
  }

  return ;
}

void
QC::setTable(std::string t, std::string acronym)
{
  // it is possible that this method is called from a spot,
  // where there is still no valid table name.
  if( t.size() )
  {
    currTable = t ;
    notes->setTable(t, acronym);
  }

  return;
}

void
QC::storeData(std::vector<hdhC::FieldData> &fA)
{
  //FieldData structure defined in geoData.h

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    VariableMetaData &vMD = varMeDa[i];

    if( vMD.var->isNoData )
      return;

    if( isNotFirstRecord && vMD.var->isFixed  )
      continue;

    vMD.qcData.store(fA[i]) ;
  }

  return ;
}

void
QC::storeTime(void)
{
   // testing time steps and bound (if any)
   qcTime.testDate(pIn->nc);

   qcTime.timeOutputBuffer.store(qcTime.currTimeValue, qcTime.currTimeStep);
   qcTime.sharedRecordFlag.store();

   return ;
}

bool
QC::testPeriod(void)
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
  std::string f( dataFilename.substr(0, dataFilename.size()-3 ) );

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
  qcTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("42_4");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("invalid range for period in the filename");

       std::string text( "range=");
       text += sd[0];
       text += " - ";
       text += sd[1];

       (void) notes->operate(capt, text) ;
       notes->setCheckMetaStr( fail );
     }

     return false;
  }

  bool bLeft0, bLeft1, bRight0, bRight1;
  bLeft0 = bLeft1 = bRight0 = bRight1 = false;

  if( qcTime.isTimeBounds)
  {
    period.push_back( qcTime.getDate("first", "left") );
    period.push_back( qcTime.getDate("last", "right") );

    bLeft1=(period[0].getJulianDay() + 1.25*qcTime.referenceTimeStep)
              < period[2].getJulianDay();
    bRight0=(period[1].getJulianDay() - 1.25*qcTime.referenceTimeStep)
              < period[3].getJulianDay();
  }
  else
  {
    period.push_back( qcTime.getDate("first") );
    period.push_back( qcTime.getDate("last") );
  }

  bLeft0= period[2] < period[0] ;
  bRight1= period[3] > period[1] ;

  if( bLeft0 || bLeft1 )
  {
     std::string key("16_2");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("period in filename (1st date) is misaligned to time values");

       std::string text("from time data=");
       text += period[2].getDate();
       text += "\nfilename=" ;
       text += period[0].getDate() ;
       if( ! qcTime.isTimeBounds)
         text += "\n note: time_bounds not available" ;

       (void) notes->operate(capt, text) ;
       notes->setCheckMetaStr( fail );
     }
  }

  // test completeness: the end of the file
  if( isFileComplete && ( bRight0 || bRight1 ) )
  {
     std::string key("16_3");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("period in filename (2nd date) is misaligned to time values");

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
       notes->setCheckMetaStr( fail );
     }
  }

  return false;
}

VariableMetaData::VariableMetaData(QC *p, Variable *v)
{
   pQC = p;
   var = v;

   if( v )
     name = v->name;

   isForkedAnnotation=false;
}

VariableMetaData::~VariableMetaData()
{
  qcData.dataOutputBuffer.clear();
}

int
VariableMetaData::finally(int eCode)
{
  // write pending results to qc-file.nc. Modes are considered there
  qcData.flush();

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
   qcData.setAnnotation(n);

   return;
}

void
VariableMetaData::setAnnotation(Annotation *n)
{
//   if( isForkedAnnotation )
//      delete notes;

   notes = n;

//   isForkedAnnotation=false;

   qcData.setAnnotation(n);

   return;
}
