#include "annotation.h"

Annotation::Annotation(Annotation *n)
{
  initDefaults();

  if( n )
     copyInit(n);
}

void
Annotation::applyOptions(void)
{
  Split split;
  split.setSeparator("=");
  split.setIgnore(" ");

     // the first loop for items with higher precedence
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     split = optStr[i] ;

     if( split[0] == "cL"
         || split[0] == "checkList"
            || split[0] == "check_list" )
     {
          checkList=split[1] ;
          continue;
     }

     if( split[0] == "d" || split[0] == "display" )
     {
        isDisplay=true;
        continue;
     }

     if( split[0] == "note" )
     {
       if( split.size() == 2 )
       {
          BraceOP groups;
          groups.set( split[1] );

          std::string str0;
          while ( groups.next(str0) )
            // empty string for txt
            setConfVector( "", str0 ) ;

          continue;
       }
     }

     if( split[0] == "nA"
          || split[0] == "noteAlways" || split[0] == "note_always")
     {
       if( split.size() == 2 )
       {
          setUseAlways( split[1], "always" );
          continue;
       }
     }

     if( split[0] == "nLL"
          || split[0] == "noteLevelLimit"
             || split[0] == "note_level_limit")
     {
       if( split.size() == 2 )
       {
         if( split[i].size() == 1 )
         {
           // only a number
           std::string s("L");
           s += split[i] ;
           setUseAlways( s, "limit" );
         }
         else
           setUseAlways( split[1], "limit" );

          continue;
       }
     }

     if( split[0] == "nCR"
          || split[0] == "no_check_results" || split[0] == "noCheckResults")
     {
        isCheckResultsWasPrinted=true;
        continue;
     }

     if( split[0] == "oP"
          || split[0] == "output_PASS" || split[0] == "outputPASS")
     {
        isOutputPASS=true;
        continue;
     }

  }

  return;
}

void
Annotation::config(void)
{
  // note: first items of options may be already defined
  //       in QA.applyOPtions()

  // get flag configuration from file
  readConf();

  // basic default settings
  if( isUseDefault )
  {
    options.push_back("ST,L1");  // warning
    if( levelLimit )
    {
      std::string s("PT,");
      s += levelLimitStr;
      options.push_back(s);  // severe, not emergency

      // defaults before any table was defined
      s = levelLimitStr;
      options.push_back(s);
    }
    else
    {
      options.push_back("PT,L1");  // severe, not emergency

      // defaults before any table was defined;
      // also for inquiries not defined by the chek-list
      options.push_back("L1");
    }

    // three place holders corresponding to options
    descript.push_back( "" );
    descript.push_back( "" );
    descript.push_back( "" );
  }

  parse();

  return;
}

void
Annotation::copyInit(Annotation *n)
{
  currTable = n->currTable;
  currTableAcronym = n->currTableAcronym;
  isCheckResultsWasPrinted = n->isCheckResultsWasPrinted;

  for( size_t i=0 ; i < n->code.size() ; ++i )
     code.push_back( n->code[i] ) ;
  for( size_t i=0 ; i < n->count.size() ; ++i )
     count.push_back( n->count[i] ) ;
  for( size_t i=0 ; i < n->level.size() ; ++i )
     level.push_back( n->level[i] ) ;
  for( size_t i=0 ; i < n->table.size() ; ++i )
     table.push_back( n->table[i] ) ;
  for( size_t i=0 ; i < n->task.size() ; ++i )
     task.push_back( n->task[i] ) ;
  for( size_t i=0 ; i < n->text.size() ; ++i )
     text.push_back( n->text[i] ) ;
  for( size_t i=0 ; i < n->var.size() ; ++i )
     var.push_back( n->var[i] ) ;

  for( size_t i=0 ; i < n->value.size() ; ++i )
  {
    value.push_back( std::vector<std::string>() );

    for( size_t j=0 ; j < n->value[i].size() ; ++j )
       value[i].push_back( n->value[i][j] ) ;
  }

  for( size_t i=0 ; i < n->permittedFlagBegin.size() ; ++i )
     permittedFlagBegin.push_back( n->permittedFlagBegin[i] ) ;

  for( size_t i=0 ; i < n->xRecord_0.size() ; ++i )
  {
    xRecord_0.push_back( std::vector<size_t>() );

    for( size_t j=0 ; j < n->xRecord_0[i].size() ; ++j )
       xRecord_0[i].push_back( n->xRecord_0[i][j] ) ;
  }

  for( size_t i=0 ; i < n->xRecord_1.size() ; ++i )
  {
    xRecord_1.push_back( std::vector<size_t>() );

    for( size_t j=0 ; j < n->xRecord_1[i].size() ; ++j )
       xRecord_1[i].push_back( n->xRecord_1[i][j] ) ;
  }

  return;
}

void
Annotation::eraseAnnotation(std::string str, std::string name)
{
  if( !findAnnotation(str, name) )
    return;

  // Note: composition: level-flag.
  std::map<std::string, std::string>::iterator it;

  if( name.size() )
    str += "_" + name ;

  for( it=mp.begin() ; it != mp.end() ; ++it )
  {
    // flag found
    if( str == it->first.substr(0,str.size()) )
      mp_count[it->first] = 0;
  }

  return;
}

bool
Annotation::findAnnotation(std::string str, std::string name)
{
  if ( mp.begin() == mp.end() )
    return false;

  if( name.size() )
     str += "_" + name ;

  std::map<std::string, std::string>::iterator it;

  for( it=mp.begin() ; it != mp.end() ; ++it )
    if( str == it->first.substr(0,str.size()) )
      return true;

  return false;
}

bool
Annotation::findIndex(std::string &key, bool isOnly)
{
  // scan over explicit flags and explicit name

  // isOnly==true: don't mark for effVectors;
  //               will not be applied by operate()

  // scan already known indexes
  for( size_t i=0 ; i < effIndex.size() ; ++i)
  {
    if( code[effIndex[i]] == key )
    {
      if( effVar[i] == currName )
      {
         currIndex = effIndex[i] ;

         return isMultipleTags ? false : true ;
      }
    }
  }

  // scan over explicit flags and explicit name
  for( size_t i=0 ; i < code.size() ; ++i)
  {
     if( code[i] == key )
     {
        if( currName == var[i] )
        {
          if( currTableAcronym == table[i] || table[i] == "*" )
          {
             currIndex = i;

             if( ! isOnly )
             {
               effIndex.push_back( i );
               effVar.push_back( currName );
             }

             return false;
          }
        }
     }
  }

  // scan over explicit flags, but wild-card name
  for( size_t i=0 ; i < code.size() ; ++i)
  {
     if( code[i] == key )
     {
        if( var[i] == "*" )
        {
          if( currTableAcronym == table[i] || table[i] == "*" )
          {
             currIndex = i;

             if( ! isOnly )
             {
               effIndex.push_back( i );
               effVar.push_back( currName );
             }

             return false;
          }
        }
     }
  }

  // for more general settings, apply to all flags and all variables
  for( size_t i=0 ; i < code.size() ; ++i)
  {
     if( code[i] == "*" )
     {
        if( currName == var[i] || var[i] == "*" )
        {
          if( currTableAcronym == table[i] || table[i] == "*" )
          {
             push_back( key, currName, level[i],
                 table[i], task[i], "", value[i], xRecord_0[i], xRecord_1[i]);

             // adjust indexes of the effective vectors
//             for( size_t j=0 ; j < effIndex.size() ; ++j )
//                ++effIndex[j] ;  //shift contained index

//             currIndex = 0;
              currIndex = code.size() -1 ;

             if( ! isOnly )
             {
               effIndex.push_back( currIndex );
               effVar.push_back( currName );
             }

             return false;
          }
        }
     }
  }

  return false;
}

std::vector<std::string>
Annotation::getAnnotation(std::string tag)
{
  std::vector<std::string> vs;

  if ( mp.begin() == mp.end() )
    return vs;

  std::string str;

  std::map<std::string, std::string>::iterator it;

  for( it=mp.begin() ; it != mp.end() ; ++it )
  {
    size_t sz;
    if( (sz=it->first.find('|')) == std::string::npos )
      sz = it->first.size();

    str = it->first.substr(0,sz) ;

    if( str.find(tag) < std::string::npos )
       vs.push_back(str);
  }

  return vs;
}

std::string
Annotation::getCheckResults(void)
{
  // brief summery of the QA findings, which are currently
  // collected in vectors, but are eventually merged.

  // default: omission
  if( checkCF_Str.size() == 0 )
    checkCF_Str = "OMIT" ;
  if( checkMetaStr.size() == 0 )
    checkMetaStr = "OMIT" ;
  if( checkTimeStr.size() == 0 )
    checkTimeStr = "OMIT" ;
  if( checkDataStr.size() == 0 )
    checkDataStr = "OMIT" ;

  std::string out("CF_conv: ");

  out += checkCF_Str ;
  out += " meta_data: " ;
  out += checkMetaStr ;
  out += " time_values: " ;
  out += checkTimeStr ;
  out += " data: " ;
  out += checkDataStr ;

  return out;
}

int
Annotation::getExitValue(void)
{
  if( isExit[4] )  // stop session
    return 4;
  if( isExit[3] )  // stop immediately and lock
    return 3;
  if( isExit[2] )  // stop after meta-data check and lock
    return 2;
  if( isExit[1] )  // issue warning
    return 1;

  return 0;
}

bool
Annotation::init(void)
{
  if( isInit )
    return false;

  isInit=true;

  applyOptions();

  config();

  return true;
}

void
Annotation::initDefaults(void)
{
  setObjName("X");
  ofsNotes=0;

  recErrCountLimit=5;

  isCheckResultsWasPrinted = false;
  isDisplay=false;
  isOutputPASS=false;
  isUseDefault=true;

  countMultipleTags=0;
  levelLimit=0;

  mail_level = 0;
  numOfExits = 5;

  for( size_t i=0 ; i < numOfExits ; ++i )
    isExit[i]=false;

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  isInit=false;

  return;
}

bool
Annotation::inq( std::string key, std::string name, std::string mode)
{
  // SYNTAX mode: [INQ::ONLY[ | INQ::NMT]]
  // with  INQ_ONLY  simulation without triggering an action
  //          NO_MT  no multiple tags
  //       LIST_TXT  disable the default text from the check-list table
  currName=name;

  bool isOnly=false;
  isMultipleTags=true;
  isListText=false;

  if( mode.size() )
  {
    Split splt(mode," |");
    for( size_t i=0 ; i < splt.size() ; ++i )
    {
       if( splt[i] == "INQ_ONLY" )
         isOnly=true;
       else if( splt[i] == "NO_MT" )
         isMultipleTags=false;
       else if( splt[i] == "LIST_TXT" )
         isListText=true;
    }
  }

  // find handling setting for key; sets variable currIndex
  // return value true, for a redo of non-multiple tags
  if( findIndex(key, isOnly) )
    return false;

  // configured to discard?
  if( task[currIndex].find("D") < std::string::npos )
  {
    bool isCommon=true;

    // check any specified constraint
    if( constraintValue.size() )
    {
      isCommon=false;

      for(size_t i=0 ; i < value[currIndex].size() ; ++i )
      {
        if( value[currIndex][i] == constraintValue )
        {
          constraintValue.clear();

          return false;  // discard any kind of notification
        }
      }

      return true;  // constraint was set but did not match
    }

    // check excluded record
    for(size_t i=0 ; i < xRecord_0[currIndex].size() ; ++i )
    {
      isCommon=false;
      for(size_t j=xRecord_0[currIndex][i] ; j < xRecord_1[currIndex][i] ; ++j )
      {
        if( j == currentRecord )
          return false;  // discard any kind of notification
      }
    }

    if( isCommon )
      return false;
  }

  return true;   // default exception handling
}

/*
void
Annotation::linkObject(IObj *p)
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
*/

bool
Annotation::operate(std::string headline,
      std::string passedText,
      std::string mail_subject, std::string mail_body )
{
   bool isReturn=false;

   std::string tag(code[currIndex]) ;
   if( currName.size() )
      tag  += "_" + currName ;

   if( isMultipleTags )
   {
      // the annex will be erased before printing
      tag += "|";
      tag += hdhC::itoa( countMultipleTags++ );
   }

   if( mp.count(tag) == 0 )
   {
     mp[tag] = code[currIndex] ;
     mp_lev[tag] = level[currIndex] ;

     if( headline.size() )
       mp_capt[tag] = headline;
     else
       mp_capt[tag] = text[currIndex];
   }

   if( mp_count.count(tag) == 0 )
     mp_count[tag]=1 ;
   else
     mp_count[tag] = mp_count[tag]+1 ;

   // levels
   if( level[currIndex] == "L1" )
   {
     isExit[1]=true;
     isReturn=true;
   }
   else if( level[currIndex] == "L2" )
   {
     // Stop after meta-data check
     // or if in time or data mode immediately
     isExit[2]=true;
     isReturn=true;
   }
   else if( level[currIndex] == "L3" )
   {
     // Stop immediately
     isExit[3]=true;
     isReturn=true;
   }
   else if( level[currIndex] == "L4" )
   {
     //Emergency.
     isExit[4]=true;
     isReturn=true;
   }

   // print notes
   if( isListText )
     passedText = text[currIndex] ;


   // a caption only for the 'classical' output into the QA_RESULTS DRS
   std::string capt(code[currIndex]);
   capt += ": ";
   capt += mp_capt[tag] ;

   if( !isDisplay )
     printNotes( tag, capt, passedText );

   // prepare email
   if( task[currIndex].find("EM") < std::string::npos )
   {
      if( mail_body.size() )
      {
         mail_out_body += "\n" ;
         mail_out_body += mail_body ;
      }

      if( mail_subject.size() && getExitValue() > mail_level )
      {
         mail_out_subject = "SUBJECT-BEG" ;
         mail_out_subject += mail_subject ;
         mail_out_subject += "SUBJECT-END" ;

         mail_level = getExitValue() ;
      }
   }

   return isReturn ;
}

void
Annotation::parse(void)
{
  Split csl;
  csl.setSeparator(',');
  csl.setIgnore(" ");

  std::vector<std::string> tmpCodes;
  std::vector<std::string> vars;
  std::string              checkListText;
  std::string              str0;

  std::vector<std::string> dedicatedValue;
  std::vector<size_t>      xRec_0;
  std::vector<size_t>      xRec_1;

  for( size_t i=0 ; i < options.size() ; ++i )
  {
     csl=options[i];

     if( csl.size() == 0 )
       continue;  // default behaviour

     tmpCodes.clear();  // reset
     vars.clear();

     if( dedicatedValue.size() )
       dedicatedValue.clear();

     if( xRec_0.size() )
     {
       xRec_0.clear();
       xRec_1.clear();
     }

     bool isPT=false;
     bool isST=false;
     bool isEmail=false;
     bool isL1=false;
     bool isL2=false;
     bool isL3=false;
     bool isEmergency=false;
     bool isD=false;

     checkListText = descript[i];

     // extract error flags, tables, and tasks;
     // both will be applied to each variable below or if the variable
     // is omitted, then to all variables.
     for( size_t j=0 ; j < csl.size() ; ++j )
     {
        str0 = csl[j] ;

        if( str0 == "*" )
          continue;

        if( str0 == "L1" )
        {
          isL1=true;
          continue;
        }

        if( str0 == "L2" )
        {
          isL2=true;
          continue;
        }

        if( str0 == "L3" )
        {
          isL3=true;
          continue;
        }

        if( str0 == "L4" )
        {
          isEmergency=true;
          continue;
        }

        if( str0 == "D" )
        {
          isD=true;
          continue;
        }

        if( str0 == "EM" )
        {
          isEmail=true;
          continue;
        }

        if( str0 == "PT" )
        {
          isPT=true;
          continue;
        }

        if( str0 == "VR" )
        {
          isST=true;
          continue;
        }

        // any begin with a digit indicates a flag as well as beginning with
        // a vector member of permitted FlagBegin whose size is smaller than that of str0.
        if( hdhC::isDigit( str0[0]) )
        {
          tmpCodes.push_back( str0 ) ;
          continue;
        }
        else
        {
          if( str0.find('=') == std::string::npos)  // not a record
          {
            bool isCont=false;
            for(size_t ifb=0 ; ifb < permittedFlagBegin.size() ; ++ifb)
            {
              size_t sz = permittedFlagBegin[ifb].size();
              if( str0.size() > sz && str0.substr(0,sz) == permittedFlagBegin[ifb] )
              {
                tmpCodes.push_back( str0 ) ;
                isCont=true;
                break;
              }
            }

            if( isCont )
               continue;
          }

        }

        if( str0.size() > 2 && str0[0] == 'V' )
        {
          size_t pos;

          if( (pos=str0.find('=')) < std::string::npos )
            dedicatedValue.push_back( str0.substr(pos+1) );
          continue;
        }

        if( str0.size() > 2 && str0[0] == 'R' )
        {
          size_t pos;

          if( (pos=str0.find('=')) < std::string::npos )
          {
            Split recs( str0.substr(pos+1), "-");
            if( recs.size() == 0 )
              continue;  // silently dropped
            else
            {
              xRec_0.push_back( static_cast<size_t>(
              hdhC::string2Double(str0.substr(pos+1))) ) ;

              std::string t(str0.substr(pos+1));
              if( (pos=t.find('-')) < std::string::npos )
              {
                 xRec_1.push_back( static_cast<size_t>(
                 hdhC::string2Double(t.substr(pos+1))) +1 ) ;
              }
              else
                xRec_1.push_back(xRec_0.back() + 1) ;
            }
          }
          continue;
        }

        // only variables
        if( str0.substr(0,4) == "var=" )
          vars.push_back( str0.substr(4) );
        else
          vars.push_back( str0 );
     }

     // any variable specified? No? Then, set '*' for all variables
     if( vars.size() == 0 )
       vars.push_back("*");

     if( tmpCodes.size() == 0 )
       tmpCodes.push_back("*");  // apply to all kinds of error flags

     // now scan for all the variables and update vectors
     for( size_t j=0 ; j < vars.size() ; ++j )
     {
       // here, a variable is specified
       for( size_t cc=0 ; cc < tmpCodes.size() ; ++cc )
       {
          // index over both types of tables: tt==0 --> ST, tt==1 --> PT
          // push the corresponding vectors
          var.push_back(vars[j]);
          code.push_back(tmpCodes[cc]);

          if( isPT )
            table.push_back("PT") ;
          else if( isST )
            table.push_back("VR") ;
          else
            table.push_back("*") ;

          task.push_back("");
          if( isD )
            task.back() = "D";

          if( isEmail )
            task.back() += ",EM" ;

          if( isL1 )
            level.push_back("L1");
          else if( isL2 )
            level.push_back("L2");
          else if( isL3 )
            level.push_back("L3");
          else if( isEmergency )
            level.push_back("L4");
          else
            level.push_back("");

          count.push_back( 0 );
          text.push_back( checkListText );

          value.push_back( std::vector<std::string>() );
          for( size_t i=0 ; i < dedicatedValue.size() ; ++i )
            value.back().push_back( dedicatedValue[i] ) ;

          xRecord_0.push_back( std::vector<size_t>() );
          xRecord_1.push_back( std::vector<size_t>() );
          for( size_t i=0 ; i < xRec_0.size() ; ++i )
          {
            xRecord_0.back().push_back( xRec_0[i]) ;
            xRecord_1.back().push_back( xRec_1[i]) ;
          }

/*
std::cout << "v: " << vars[j]
          << "\tl: " << level.back()
          << "\tt: " << task.back() << std::endl ;
*/

       }  // loop over flags
     }  // loop over variables
  }  // loop over all assignments in the configuration

  options.clear();  // no longer needed
  return;
}

void
Annotation::print(void)
{
  printEMail();
  printFlags();
  printCheckResult();

  return;
}

void
Annotation::printCheckResult(void)
{
  if( isCheckResultsWasPrinted )
    return;

  isCheckResultsWasPrinted = true;

  // brief summery of the QA findings, which are currently
  // collected in vectors, but are eventually merged.

  std::string out( "CHECK-BEG" );

  out += getCheckResults() ;

  out += "CHECK-END";  // mark of the end of an output line
  std::cout << out << std::endl;

  return;
}

void
Annotation::printEMail(void)
{
   if( mail_out_body.size() )
   {
     std::cout << "EMAIL-BEG" ;

     if( mail_out_subject.size() )
        std::cout << mail_out_subject ;

     std::cout << mail_out_body ;

     std::cout << "EMAIL-END" << std::endl ;
   }
   return;
}

void
Annotation::printFlags(void)
{
  std::string out ;

  if( isDisplay )
  {
    out ="----------------------------\n" ;

    if( isOutputPASS )
    {
      out += "path: " + file.path;
      out += "\nfile: " + file.filename;
      if( mp.begin() == mp.end() )
      {
        out += ":\tPASS\n" ;
        std::cout << out << std::endl;
        return;
      }
      else
      {
        out += ":\tFAIL\n" ;
        std::cout << out << std::endl;
      }
    }
  }

  std::map<std::string, std::string>::iterator it;

  for( it=mp.begin() ; it != mp.end() ; ++it )
  {
    const std::string& f=it->first;
    const std::string& s=it->second;

    if( mp_count[f] == 0 )
       continue;  // overruled by another one, so skip

    out.clear();

    if( ! isDisplay )
      out = "FLAG-BEG" ;

    if( mp_lev[f].size() )
      out += mp_lev[f] + '-' ;

    out += s;
    out += ':' ;

    if( mp_capt.count(f) )
    {
      if( ! isDisplay )
         out += "CAPT-BEG" ;
      else
         out += ' ';

      // remove surrounding blanks
      std::string str(hdhC::stripSurrounding(mp_capt[f]));

      // convert the first char of the caption to upper case.
      if( str[0] == '!' ) // suppression
        out += str.substr(1);
      else
      {
        str[0] = toupper(str[0]);
        out += str;
      }
    }

    // any account for accumulated 'R' flags?
    // Still the caption.
    if( s[0] == 'R' )
    {
       out += " (total: " ;
       out += mp_count[f] ;
       out += ")" ;
    }

    int last=out.size() -1 ;
    if( last > -1 &&
        !(out[last] == '.' || out[last] == '!' ||out[last] == '?' ) )
      out += "." ;

    if( ! isDisplay )
      out += "CAPT-END" ;
    else
      out +="\n";

    if( mp_txt.count(f) && mp_txt[f].size() )
    {
      if( ! isDisplay )
      {
        out += "TXT-BEG" ;
        out += mp_txt[f];
        out += "TXT-END" ;
      }
      else
        out += mp_txt[f] + "\n";
    }

    if( ! isDisplay )
      out += "FLAG-END";  // mark of the end of an output line

    std::cout << out << std::endl;
  }

  return;
}

void
Annotation::printHeader(std::ofstream *ofs)
{
//   std::string str( where ) ;
   std::string str;
   str +="\nPath: " ;
   str += file.path ;
   str += "\nFile: " ;
   str += file.filename;

   *ofs << str << std::endl ;

   return ;
}

void
Annotation::printNotes(std::string &tag, std::string &caption,
   std::string str, bool enableStopAtErr)
{
  // str == message

  // Occurrence of an error usually stops the run immediately.
  // But, the calling program unit is due to exit.
  if( ofsNotes == 0 )
  {
    if( file.path.size() == 0 )
      file.path = "undefined_path" ;
    if( file.filename.size() == 0 )
      file.filename = "undefined_filename" ;

    // compose the header of issued messages
    std::string strNotes = "qa_note_" ;
    strNotes += file.filename + ".txt";

    // open file for writing
    if( ! ofsNotes )
      ofsNotes = new std::ofstream(strNotes.c_str());

    printHeader(ofsNotes);
  }

  // prevent error message flooding
  if( count[currIndex]++ > recErrCountLimit )
    return;

  if( count[currIndex] > recErrCountLimit )
  {
    *ofsNotes << code[currIndex] << ": more ..." << std::endl;

    return;
  }

  // write message
  *ofsNotes << "\n" << caption << "\n" << str << std::endl;

  // output the pure text with '\n' replaced by ';'
  size_t sz=str.size();
  for( size_t i=0 ; i < sz ; ++i )
    if( str[i] == '\n' )
       str[i] = ';' ;

   mp_txt[tag] = str;

  return ;
}

void
Annotation::push_back(std::string pf_code, std::string pf_var,
      std::string pf_level,
      std::string pf_table, std::string pf_task, std::string pf_text,
      std::vector<std::string> &pf_value,
      std::vector<size_t> &pf_xRec_0, std::vector<size_t> &pf_xRec_1)
{
   code.push_back(pf_code) ;
   count.push_back(0) ;
   level.push_back(pf_level) ;
   table.push_back(pf_table) ;
   task.push_back(pf_task) ;
   text.push_back(pf_text) ;
   value.push_back(pf_value) ;
   var.push_back(pf_var) ;
   xRecord_0.push_back(pf_xRec_0);
   xRecord_1.push_back(pf_xRec_1);

   return;
}

void
Annotation::push_front(std::string pf_code, std::string pf_var,
      std::string pf_level,
      std::string pf_table, std::string pf_task, std::string pf_text,
      std::vector<std::string> &pf_value,
      std::vector<size_t> &pf_xRec_0, std::vector<size_t> &pf_xRec_1)
{

   if( code.size() == 0 )
   {
     code.push_back(pf_code) ;
     count.push_back(0) ;
     level.push_back(pf_level) ;
     table.push_back(pf_table) ;
     task.push_back(pf_task) ;
     text.push_back(pf_text) ;
     value.push_back(pf_value) ;
     var.push_back(pf_var) ;
     xRecord_0.push_back(pf_xRec_0);
     xRecord_1.push_back(pf_xRec_1);

     return;
   }

   // testing idendity
   for( size_t i=0 ; i < code.size() ; ++i )
     if( code[i] == pf_code )
       return;

   std::vector<std::string>::iterator it_code  = code.begin();
   std::vector<size_t>::iterator      it_count = count.begin();
   std::vector<std::string>::iterator it_level = level.begin();
   std::vector<std::string>::iterator it_table = table.begin();
   std::vector<std::string>::iterator it_task  = task.begin();
   std::vector<std::string>::iterator it_text  = text.begin();
   std::vector<std::vector<std::string> >::iterator it_value = value.begin();
   std::vector<std::string>::iterator it_var   = var.begin();
   std::vector<std::vector<size_t> >::iterator it_xRecord_0 = xRecord_0.begin();
   std::vector<std::vector<size_t> >::iterator it_xRecord_1 = xRecord_1.begin();

   code.insert(it_code, pf_code) ;
   count.insert(it_count, 0) ;
   level.insert(it_level, pf_level) ;
   table.insert(it_table, pf_table) ;
   task.insert(it_task, pf_task) ;
   text.insert(it_text, pf_text) ;
   var.insert(it_var, pf_var) ;
   value.insert(it_value, pf_value) ;
   xRecord_0.insert(it_xRecord_0, pf_xRec_0) ;
   xRecord_1.insert(it_xRecord_1, pf_xRec_1) ;

   return;
}

void
Annotation::readConf(void)
{
  std::string notesConf= tablePath ;
  if( tablePath[tablePath.size()-1] != '/' )
    notesConf += '/' ;
  notesConf += checkList ;

  ReadLine ifs( notesConf ) ;
  ifs.skipBashComment();
  ifs.clearSurroundingSpaces();
  ifs.setBreakpoint('&');  // wil be toggled

  // at first, read settings from file
//  std::ifstream ifs(notesConf.c_str(), std::ios::in);

  std::string str0;

  if( !ifs.isOpen() )  // file does not exist
     return ;

  std::string txt;
  BraceOP groups;

  while( !ifs.getLine(txt) )
  {
    if( txt.size() == 0 )
      continue;

    if(txt.find("PERMITTED_FLAG_BEGIN") < std::string::npos)
    {
       size_t pos;
       if( (pos=txt.find("=")) < std::string::npos)
       {
         Split splt(txt.substr(++pos), ",") ;
         for(size_t i=0 ; i< splt.size() ; ++i)
            permittedFlagBegin.push_back(splt[i]);
       }

       continue;
    }

    // one for all
    size_t pos_x=0;
    size_t pos;
    if( useAlways.size()
          || (pos_x=txt.find("NOTE_ALWAYS=")) < std::string::npos
            || (pos=txt.find("NOTE_ALWAYS")) < std::string::npos )
    {
      pos = 12;
      if( pos_x < std::string::npos )
         ++pos ;  // for the assignment '=' char

      if( useAlways.size() )
        groups.set(useAlways) ;
      else
        groups.set(txt.substr(pos+10)) ;  // size of "NOTE_ALWAYS"

      // clear previous settings
      options.clear();
      descript.clear();

      while ( groups.next(txt) )
        setConfVector( "", txt ) ;

      isUseDefault =false;
      break; // leave the while loop
    }

    // regular entries my be arbitrarily split across several lines
    ifs.unsetBreakpoint() ;
    ifs.getLine(str0) ;
    ifs.setBreakpoint('&') ;

    if( levelLimit ||
          txt.find("NOTE_LEVEL_LIMIT") < std::string::npos )
    {
      if( levelLimit == 0 )
      {
         // coding for case-insensitive leading 'L'
        levelLimit = static_cast<size_t>( hdhC::string2Double(txt) );
        levelLimitStr = "L";
        levelLimitStr += hdhC::double2String(levelLimit);

        continue;
      }

      // replace level value by the limitation value
      size_t pos ;
      while( (pos=str0.find("L", pos)) < std::string::npos )
      {
         size_t lval=static_cast<size_t>(
                        hdhC::string2Double(str0.substr(pos,2) ) );

         if( lval > levelLimit )
           str0.replace(pos,2,levelLimitStr);

         ++pos; // avoids an infinite loop
      }
    }

    if( txt.size() && txt[txt.size()-1] != '.' )
      txt += '.' ;

    groups.set( str0 );
    while ( groups.next(str0) )
      setConfVector( txt, str0 ) ;
  }

  ifs.close();
  return;
}

void
Annotation::setConfVector(std::string txt, std::string str0)
{
  //parse directives supplied by BraceOP

  if( str0.size() == 0 )
    return; // no brace contents

  options.push_back( str0 );
  descript.push_back( txt );

  return ;
}

void
Annotation::setTable(std::string t, std::string acronym)
{
  // this method sets the current table name and it clears
  // all references to the other table.

  if( t.size() == 0 )
    return;

  if( currTable.size() == 0 )
    currTable = t;
  else
    return;

  currTableAcronym = acronym ;

  std::vector<std::string>::iterator it_code  = code.end()-1;
  std::vector<size_t>::iterator      it_count = count.end()-1;
  std::vector<std::string>::iterator it_level = level.end()-1;
  std::vector<std::string>::iterator it_table = table.end()-1;
  std::vector<std::string>::iterator it_task  = task.end()-1;
  std::vector<std::string>::iterator it_text  = text.end()-1;
  std::vector<std::vector<std::string> >::iterator it_value = value.end()-1;
  std::vector<std::string>::iterator it_var   = var.end()-1;
  std::vector<std::vector<size_t> >::iterator it_xRecord_0 = xRecord_0.end()-1;
  std::vector<std::vector<size_t> >::iterator it_xRecord_1 = xRecord_1.end()-1;

  for(  ; it_table >= table.begin() ; )
  {
     if( *it_table != currTableAcronym && *it_table != "*" )
     {
        code.erase(it_code);
        count.erase(it_count);
        level.erase(it_level);
        table.erase(it_table);
        task.erase(it_task);
        text.erase(it_text);
        value.erase(it_value);
        var.erase(it_var);
        xRecord_0.erase(it_xRecord_0);
        xRecord_1.erase(it_xRecord_1);
     }

     --it_code;
     --it_count;
     --it_level;
     --it_table;
     --it_task;
     --it_text;
     --it_value;
     --it_var;
     --it_xRecord_0;
     --it_xRecord_1;
  }

  return;
}

void
Annotation::setUseAlways(std::string s, std::string mode)
{
  if( mode == "always" )
    useAlways=s;
  else if( mode == "limit" )
  {
    levelLimit=static_cast<size_t>(hdhC::string2Double(s));

    // insensitive to a leading 'L'
    levelLimitStr = "L";
    levelLimitStr += hdhC::double2String(levelLimit);
  }

  return;
}
