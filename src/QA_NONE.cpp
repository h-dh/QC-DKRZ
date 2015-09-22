//#include "qa.h"

// Macro option to enable output of all messages.
// Please compile with '-D RAISE_ERRORS'

QA::QA()
{
  initDefaults();
}

QA::~QA()
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
        hst += " changed path to data=";
        hst += qaFile.path + "\n" ;
      }
    }
  }
  else
  {
    // the root of the history string
    hst += today;
    hst += " path_to_data=";
    hst += qaFile.path ;
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
      tmp = nc->getAttString("QA revision");

    if( revision != tmp )
    {
      hst += "\n" ;
      hst += today;
      hst += " changed QA revision=" ;
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

     if( split[0] == "fS" || split[0] == "FileSequence"
         || split[0] == "file_sequence" )
     {
       if( split.size() == 2 )
          fileSequenceState=split[1][0];
       continue;
     }

     if( split[0] == "fq" || split[0] == "Frequency"
         || split[0] == "frequency" )
     {
       if( split.size() == 2 )
          frequency=split[1];
       continue;
     }

     if( split[0] == "fqp" || split[0] == "FrequencyPosition"
         || split[0] == "frequency_position" )
     {
       if( split.size() == 2 )
          frequency_pos = split.toInt(1);
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

     if( split[0] == "tPr"
          || split[0] == "tableProject" )  // dummy
     {
       if( split.size() == 2 )
          projectTableFile.setFile(split[1]);

       continue;
     }
   }

   // apply a general path which could have also been provided by setTablePath()
   if( projectTableFile.path.size() == 0 )
      projectTableFile.setPath(tablePath);

   return;
}

bool
QA::checkDataBody(std::string vName)
{
  // any empty data segments?

  if( vName.size() )
  {
    for( size_t i=0 ; i < pIn->varSz ; ++i)
    {
      Variable& var = pIn->variable[i];
      if( var.name == vName )
        return pIn->nc.isEmptyData(var.name) ;
    }
  }
  else if( ! pIn->dataVarIndex.size() )
    return true;

  std::vector<size_t> vs;
  for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i)
  {
    Variable& var = pIn->variable[ pIn->dataVarIndex[i] ];
    if( ! pIn->nc.isEmptyData(var.name) )
    {
      vs.push_back(pIn->dataVarIndex[i]) ;
      notes->setCheckDataStr(fail);
    }
  }

  if( vs.size() )
    // only try for variables with data
    pIn->dataVarIndex = vs ;
  else
    // all data variables without any data
    return true;

  return false;
}

void
QA::checkProjectTable(InFile &in)
{
  // Read or write the project table.
  ProjectTable projectTable(this, &in, projectTableFile);
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
       if( varMeDa[i].var->isNoData )
          continue;

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
      if( splt[j] == qaTime.name )
        --effDim;

    if( replicationOpts.size() )
    {
      if( ReplicatedRecord::isSelected(
             replicationOpts, vMD.name, effDim ) )
      {
        vMD.qaData.replicated = new ReplicatedRecord(this, i, vMD.name);
        vMD.qaData.replicated->setAnnotation(notes);
        vMD.qaData.replicated->parseOption(replicationOpts) ;
      }
    }

    if( enablePostProc && outlierOpts.size() )
    {
      if( Outlier::isSelected(
             outlierOpts, vMD.name, effDim ) )
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
QA::finally(int xCode)
{
  if( nc )
    setExit( finally_data(xCode) );

  if( xCode != 63 && isCheckTime )
    qaTime.finally( nc );

  setExit(xCode);

  // distinguish from a sytem crash (segmentation error)
  std::cout << "STATUS-BEG" << exitCode << "STATUS-END";
  std::cout << std::flush;

  nc->close() ;

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
    // no data to write
    if( exitCode == 63 )
      exitCode=0 ;

    isExit=true;
    return exitCode ;
  }

  if( exitCode != 63 )
    qaTime.finally( nc );

  // read history from the qa-file.nc and append new entries
  appendToHistory(exitCode);

  // check for flags concerning the total data set,
  // but exclude the case of no record
  if( pIn->currRec > 0 )
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
      varMeDa[j].qaData.checkFinally(varMeDa[j].var);

  if( isCheckData )
  {
    for( size_t j=0 ; j < varMeDa.size() ; ++j )
    {
       // write qa-results attributes about statistics
       varMeDa[j].qaData.setStatisticsAttribute(nc);
    }
  }

  return exitCode ;
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
QA::getFrequency()
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;
  else if( frequency_pos > -1 )
  {
    // try the frequency posdition within the filename
    std::string f( pIn->file.basename );
    Split splt;
    splt.enableEmptyItems();
    splt.setSeparator("_");

    splt = f ;

    // test the last two items for time type. If both are,
    // then this would mean that they are separated by '_'.
    if( static_cast<int>(splt.size()) > frequency_pos )
      frequency = splt[ frequency_pos ] ;
  }

  return frequency ;
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
   // it from scratch. Initialise time testing and
   // time boundary testing.
   // Eventually, entry() is called to test the data of fields.
   // Initial values are set such that they do not cause any
   // harm in testDate() called in closeEntry().

   notes->init();  // safe

   // default
   setFilename( pIn->file );

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
//   checkMetaData(*pIn);   // not for NONE
   notes->setCheckMetaStr("PASS");

   //Check tables properties: path and names.
   // program exits for an invalid path
   inqTables();


   if(qaTime.init(pIn, notes, this))
   {
     // init the time object
     // note that freq is compared to the first column of the time table
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
   else
     isCheckTime=false;


   // open netCDF for creating, continuation or resuming qa_<varname>.nc
   openQA_Nc(*pIn);

   // check consistency between sub-sequent files. Must come after
   // openQA_Nc.
   checkProjectTable(*pIn);

   if( getExit() || qaTime.isNoProgress )
   {
     isCheckData=false;
     isCheckTime=false;
     return true;
   }

   if( isCheckTime )
   {
     if( qaTime.isTime && checkDataBody(qaTime.name) )
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
     if( checkDataBody() )
     {
       notes->setCheckDataStr("FAIL");
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
  setObjName("QA");

  // pre-setting of some pointers
  nc=0;

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
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

  importedRecFromPrevQA=0; // initial #rec in out-nc-file.
  currQARec=0;

  frequency_pos=-1;

  // pre-set check-modes: all are used by default
  isCheckMeta=true;
  isCheckTime=true;
  isCheckData=true;

  bufferSize=1500;

  exitCode=0;

  // file sequence: f[first], s[equence], l[ast]
  fileSequenceState='x' ;  // 'x':= undefined

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

  nc->setGlobalAtt( "QA_revision", revision);
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
  if( ! projectTableFile.isExisting(projectTableFile.path) )
  {
     std::string key("70_3");
     if( notes->inq( key, fileStr) )
     {
        std::string capt("no path to the tables, tried ") ;
        capt += projectTableFile.path;

        if( notes->operate(capt) )
        {
          notes->setCheckMetaStr( fail );
          setExit( notes->getExitValue() ) ;
        }
     }
  }

  // tables names usage: both project and standard tables
  // reside in the same path.
  // Naming of the project table:
  if( !projectTableFile.is )
    projectTableFile.setFilename("pt_NONE.csv");
  else if( projectTableFile.extension != ".csv" )
    projectTableFile.setExtension(".csv") ;

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

void
QA::openQA_Nc(InFile &in)
{
  // Generates a new nc file for QA results or
  // opens an existing one for appending data.
  // Copies time variable from input-nc file.

  // name of the file begins with qa_
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
  if( ! (isCheckTime || isCheckData) )
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

  // So, we have to generate a netCDF file from almost scratch;
  if( currQARec == 0 && in.nc.getNumOfRecords() == 1 )
    qaTime.isSingleTimeValue = true;

  // open new netcdf file
  if( qaNcfileFlags.size() )
    nc->create(qaFile.getFile(),  qaNcfileFlags);
  else
    nc->create(qaFile.getFile(),  "Replace");

  bool isNoTime=false;
  if( isCheckTime && pIn->isTime )
  {
    // create a dimension for fixed variables only if there is any
    for( size_t m=0 ; m < varMeDa.size() ; ++m )
    {
      if( varMeDa[m].var->isFixed )
      {
        isNoTime=true;
        break;
      }
    }

    if( !isNoTime )
      qaTime.openQA_NcContrib(nc);
  }
  else
    isNoTime=true;

  if( isNoTime )
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

  // convert ascii formated date to class Date
  qaTime.getDRSformattedDateRange(period, sd);

  Date* fN_left = &period[0];
  Date* fN_right = &period[1];

  // necessary for validity (not sufficient)
  if( *fN_left > *fN_right )
  {
     std::string key("42_4");
     if( notes->inq( key, fileStr) )
     {
       std::string capt("invalid range for period in the filename, found ");
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
     std::string key("16_2");
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
     std::string key("16_3");
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

  if( tV_left_obj )
    delete tV_left_obj;
  if( tV_right_obj )
    delete tV_right_obj;
  if( tB_left_obj )
    delete tB_left_obj;
  if( tB_right_obj )
    delete tB_right_obj;

  return false;
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
  qaData.dataOutputBuffer.clear();
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
