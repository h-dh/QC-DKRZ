#include "base.h"

Base::Base()
{
  initDefaults();
}

Base::Base(const Base &b)
{
  initDefaults();
  copy(b);
}

Base&
Base::operator=( const Base &b)
{
  if( this == &b )
    return *this;

  copy(b);
  return *this;
}

void
Base::applyOptions()
{
   // options only applicable to Base properties

   Split split;
   split.setSeparator("=");

   // now we apply to the varNames found
   for( size_t j=0 ; j < optStr.size() ; ++j)
   {
     split = optStr[j];
     if( split.size() == 2 )
     {
       if( isVarname( split[0] ) )
       {
         if( getObjName() == "IN" )
           continue;
         else
         {
           Split list(split[1], ",");
           for( size_t i=0 ; i < list.size() ; i++)
             varName.push_back( list[i] );
         }
       }

       if( split[0][0] == 'f' )
       {
         setFilename( split[1] );

         continue;
       }

       if( split[0] == "filling_value" ||
              split[0] == "fillingValue" || split[0] == "fV"
                  || split[0] == "_FillValue" )
       {
         isMissValSet=true;
         Split spltFV(split[1],",");
         for( size_t i=0 ; i < spltFV.size() ; ++i)
           fillingValue.push_back( spltFV.toDouble(i) ) ;
         continue;
       }

     }

     // skip ident number
     if( split.isNumber(0) )
        continue;

     // these are applied to each GD obj
//     for( size_t i=0 ; i < pGM.size() ; ++i)
/*
     {
       if( split[0] == "back_rotation"
            || split[0] == "backRotation")
       {
         isBackRotation=true;
         pGM[i]->setBackRotation();
         continue;
       }
*/

/*
       if( split[0] == "gmt_multi_segment"
            || split[0] == "gmtMultiSegment" )
       {
         pGM[i]->setOutputGMT_MultiSegment();
         continue;
       }

       if( split[0] == "oF" )
       {
         pGM[i]->setPrintFilename(split[1]);
         continue;
       }

       if( split[0] == "print_cells" || split[0] == "printCells" )
       {
         pGM[i]->setPrintWithGeoCoord();
         continue;
       }

       if( split[0].substr(0,3) == "reg"
            || split[0].substr(0,3) == "Reg" )
       {
         regionFile = split[1];
         continue;
       }
*/
//     }
   }

   return;
}

void
Base::clearVariable( Variable &v )
{
  v.name.clear();
  v.id=-1;
  v.pGM=0;
  v.pDS=0;
  v.pSrcBase=0;
  v.pIn=0;
  v.pNc=0;
  return;
}

void
Base::copy(const Base &b)
{
  objName = b.objName;
  thisID=b.thisID;
  filenameItems=b.filenameItems;

  srcStr.clear();
  for( size_t i=0 ; i < b.srcStr.size() ; ++i)
    srcStr.push_back(b.srcStr[i]);

  for( size_t i=0 ; i<b.optStr.size() ; ++i )
    optStr.push_back( b.optStr[i] );

  //see rules for variable and srcName in attach()
  for( size_t i=0 ; i<b.variable.size() ; ++i )
  {
    variable.push_back( *new Variable );
    variable[i].name= b.variable[i].name;
  }
  for( size_t i=0 ; i<b.obsolete_srcVarName.size() ; ++i )
    obsolete_srcVarName.push_back( obsolete_srcVarName[i]);

  isBackRotation=b.isBackRotation;
  isMissValSet=b.isMissValSet;
  explicitFillingValue=b.explicitFillingValue;

  for( size_t i=0 ; i<b.regioStr.size() ; ++i )
    regioStr.push_back(b.regioStr[i]);

  // index mapped to varName for the next vectors
  for( size_t i=0 ; i<b.fillingValue.size() ; ++i )
    fillingValue.push_back( b.fillingValue[i]);

  // used to point to those source-gM(s) that will be used
  for( size_t i=0 ; i<b.pSrcGD.size() ; ++i )
    pSrcGD.push_back( b.pSrcGD[i] );

  notes=b.notes;
  cF=b.cF;
  pIn=b.pIn;
  fDI=b.fDI;
  pOper=b.pOper;
  pOut=b.pOut;
  qA=b.qA;
  tC=b.tC;

  // point to the source base
  pSrcBase=b.pSrcBase;
  pNc=b.pNc;  // only set in InFile objects

  return;
}

bool
Base::exceptionHandling(std::string key,
  std::string capt, std::string text,
  std::vector<std::string> &checkType,
  std::string vName )
{
   if( notes )
   {

//     bool sendEmail=false;
//     std::string table="*";
//     std::string task="L1";
//     notes->push_front(key , vName, sendEmail, table, "L1", "");

     if( ! notes->inq(key, vName ) )
       return false;  // if discarded

     for( size_t i=0 ; i < checkType.size() ; ++i )
     {
       if( checkType[i] == "meta" )
         notes->setCheckMetaStr("FAIL");
       else if( checkType[i] == "time" )
         notes->setCheckTimeStr("FAIL");
       else if( checkType[i] == "data" )
         notes->setCheckDataStr("FAIL");
       else if( checkType[i] == "cf" )
         notes->setCheckCF_Str("FAIL");
     }

     return notes->operate(capt, text);
   }

   return false ;
}

void
Base::exceptionError(std::string str)
{
  // Occurrence of an error usually stops the run at once.
  // But, the calling program unit is due to exit.
  static bool doInit=true;
  if( doInit )
  {
    xcptn.strError = getObjName() ;
    xcptn.strError += "_error" ;

     // base-name if available, i.e. after initialisation of the InFile obj
    if( filenameItems.is )
    {
      xcptn.strError += "_";
      xcptn.strError += filenameItems.basename ;
    }
    xcptn.strError += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsError )
    xcptn.ofsError = new std::ofstream(xcptn.strError.c_str());

  *xcptn.ofsError << str << std::endl;

  return ;
}

void
Base::exceptionWarning(std::string str)
{
  // a warning does not stop the run

  static bool doInit=true;
  if( doInit )
  {
    // This happens only once. All error and warning
    // calls on the global scale and in any class-related
    // objects refer to this.

    xcptn.strWarning = getObjName() ;
    xcptn.strWarning += "_warning" ;

    if( filenameItems.is )
    {
      xcptn.strWarning += "_";
      xcptn.strWarning += filenameItems.basename ;
    }
    xcptn.strWarning += ".txt";

    doInit = false;
  }

  // open file for writing
  if( ! xcptn.ofsWarning )
    xcptn.ofsWarning = new std::ofstream(xcptn.strWarning.c_str());

  if( xcptn.ofsWarning->is_open() )
  {
    // write message
    xcptn.ofsWarning->write(str.c_str(), str.size()+1);
    *xcptn.ofsWarning << std::endl;
  }

  return ;
}

void
Base::finally(int errCode, std::string note)
{
  // print a message to std::cout. This will be further handled by
  // the calling entity.
  if( note.size() )
    std::cout << note << ";" << std::endl;

  // distinguish from a sytem crash (segmentation error)
  std::cout << "NormalExecution;" << std::endl;

  // in case that just a message is issued, but the program
  // is going to continue.
  if( errCode < 0)
    return;

  if( errCode > 0)
    exit(errCode);

  return;
}

std::vector<std::string>
Base::getVarname( std::string &s)
{
  // There might be a varname specification or not.
  // no varname found: return true

  std::vector<std::string> names;
  std::string list;

  std::vector<std::string> tokens;
  tokens.push_back( ":v=" ) ;
  tokens.push_back( ":varname=" ) ;
  tokens.push_back( ":vname=" ) ;
  tokens.push_back( ":variable" ) ;

  bool isEmpty=true;
  size_t pos0=0;
  for( size_t i=0 ; i < tokens.size() ; ++i )
  {
    if( (pos0 = s.find( tokens[i] ) ) < std::string::npos )
      // e.g.: s: "...:vname=...:..." and tokens: ":vname=..."
      isEmpty=false;
    else if( s.substr(0,tokens[i].size()-1) == tokens[i].substr(1) )
      // e.g.: s: "vname=..." and tokens: "vname=..."
      isEmpty=false;
  }

  if( isEmpty )
    return names;

  // the assignment sign
  size_t pos1=s.find("=",pos0);
  if( pos1 == std::string::npos )
  {
     std::ostringstream ostr(std::ios::app);
     ostr << "Base::getVarname(): invalid assignment";

     exceptionError( ostr.str() );
     std::string note("E8: no rules for finding a variable name.");
     finally(8, note);
  }
  else
    ++pos1 ;

  // the end of the assignment
  size_t pos2=s.find(":",pos1);
  if( pos2 == std::string::npos )
     // the end of the string
     pos2=s.size();

  list=s.substr(pos1, pos2-pos1);

  // expand the list
  Split spl(list, ",");
  for( size_t i=0 ; i < spl.size() ; ++i)
      names.push_back( spl[i] ) ;

  return names;
}

std::vector<std::string>
Base::getVarname( std::string &s, std::vector<std::string> &alias)
{
  // There might be a varname specification or not.
  // no varname found: return true
  // The reference 'alias' is special for operations,
  // e.g. v=x=var1,y=var2,... . If given, then var1 is the
  // variable returned via the vector and x,y,... are members
  // stored in alias.

  std::vector<std::string> list;
  std::vector<std::string> names;

  list = getVarname( s ) ;

  // expand the list
  for( size_t i=0 ; i < list.size() ; ++i)
  {
    Split spl(list[i], "=");

    if( spl.size() == 2 )
    {
      alias.push_back( spl[0] ) ;
      names.push_back( spl[1] ) ;
    }
    else if( spl.size() == 1 )
    {
      alias.push_back( "" ) ;
      names.push_back( spl[0] ) ;
    }
    else
    {
      std::ostringstream ostr(std::ios::app);
      ostr << "Base::getVarname(): invalid assignment\n";
      ostr << " of variable names." ;

      exceptionError( ostr.str() );
      std::string note("E9: invalid assignment of variable names.");
      finally(9, note);
    }
  }

  return names;
}

void
Base::help(void)
{
  std::cerr << "Description of options:\n" ;
  std::cerr << "Options for class Base applying to derived classes:\n" ;
  std::cerr << "CellStatistics, InFile, Oper, and OutFile.\n" ;
  std::cerr << "Option strings for these classes may enclose\n" ;
  std::cerr << "other option strings of class designators\n";
  std::cerr << "QA, TC, and FD, which may be embedded with\n" ;
  std::cerr << "e.g. QA@ where @ is any character not occurring\n";
  std::cerr << "in the enclosing string (the last may be omitted,\n" ;
  std::cerr << "but, may be repeated in another embedded string).\n\n";
  std::cerr << "Options: The []-part is optional, patterns like\n" ;
  std::cerr << " '..a_b..' may also be written as '..aB..','\n" ;
  std::cerr << " alternatives are indicated below by '|'\n" ;
  std::cerr << "   back_rotation\n" ;
  std::cerr << "   f[ilename]=string\n" ;
  std::cerr << "   gmt_multi_segment\n" ;
  std::cerr << "   filling_value=figure | mV=figure\n" ;
  std::cerr << "   oF=output-filename-in-gM\n" ;
  std::cerr << "   print_cells\n" ;
  std::cerr << std::endl;
  return;
}

void
Base::initDefaults(void)
{
  // pre-settings
  isAllocate=false;
  isArithmeticMean=false ;
  isBackRotation=false;
  isMissValSet=false;

  xcptn.ofsError=0;
  xcptn.ofsWarning=0;

  return;
}

bool
Base::isVarname( std::string &s)
{
  // There might be a varname specification or not.
  // no varname found: return true, if a varname
  // is specified

  if( s.find("v") < std::string::npos )
    return true ;
  if( s.find("varname") < std::string::npos )
    return true;

  if( s.find("vname") < std::string::npos )
    return true ;
  if( s.find("variable") < std::string::npos )
    return true;

  return false ;
}

void
Base::makeVariable (NcAPI *pnc, std::string name, int id)
{
  variable.push_back( *new Variable );
  variable.back().name=name;

  variable.back().pGM=0;
  variable.back().pDS=0;
  variable.back().id=-1;
  variable.back().pSrcBase=0;
  variable.back().pIn=0;
  variable.back().pNc=pnc;
  variable.back().setID(id);

  varNameMap[name] = variable.size()-1 ;

  return;
}

void
Base::setFilename(std::string f)
{
  filenameItems = hdhC::setFilename(f);
  return;
}

void
Base::setSrcVariable(Variable &var, Base *p)
{
  for( size_t j=0 ; j < p->variable.size() ; ++j )
  {
    if( var.name !=  p->variable[j].name )
      continue;

    var.id       = p->variable[j].id ;
    var.pGM      = p->variable[j].pGM ;
    var.pDS      = p->variable[j].pDS ;
    var.pSrcBase = p ;
    var.pIn      = p->variable[j].pIn ;
    var.pNc      = p->variable[j].pNc ;
  }
}

/*
void
Base::setVarnameAndGD(NcAPI *pnc, std::string s)
{
  makeVariable(pnc, s);
}
*/

void
Base::setVarProps(void)
{
  // Establish relation between the gM_Unlimited variable name(s)
  // of the current object to those of the source(s). Settings
  // for an operation are indicated by operndStr.size() > 0.

  // An operation will create new or modify data.
  // No operation is effective for OutFile, where data remains untouched
  // but renaming of variables and/or selection of multiple variables
  // is feasible.

  if( operandStr.size() == 0 )
    setVarPropsNoOperation();
  else
    setVarPropsForOperation();

  // apply srcVariables to set variables of *this
  for( size_t i=0 ; i < varName.size() ; ++i )
  {
      if( i == variable.size() )
      {
        // new obj from the source
        variable.push_back( *new Variable) ;
        variable.back().pGM      = srcVariable[i].pGM ;
        variable.back().pDS      = srcVariable[i].pDS ;
      }
      else // connect the current new pGM into the stream of sources
      {
        variable.back().pGM      = 0 ;
        variable.back().pDS      = 0 ;
      }

      variable.back().name     = varName[i] ;
      variable.back().id       = srcVariable[i].id ;
      variable.back().pSrcBase = srcVariable[i].pSrcBase ;
      variable.back().pIn      = srcVariable[i].pIn ;
      variable.back().pNc      = srcVariable[i].pNc ;
  }

  return;
}

void
Base::setVarPropsForOperation( void )
{
  // Notation:
  //     vN: number of current variables
  //     sN: number of names of sources summing sN[i0], sN[i1], ...

  //   Notation: args in <operation>: a_ij is linked to obj[i]v[j].
  //   sN == 0           -->  a_ij = obj[i][0]  with x=0,1,...
  //   sN == num of ops  -->  a_ij = obj[i]v[j]
  //   sN > num of ops   -->  error
  // connect each alias to the corresponding variable

  // number of specified source varname(s)
  std::vector<std::string> sVN;
  std::vector<std::string> alias;
  std::vector<std::string> srcVarName;

  // Find matches for aliased srcVarNames and variables.
  // Loop through source objects.
  for( size_t j=0 ; j < pSrcBase.size() ; ++j )
  {
     Base *p=pSrcBase[j];

     // get source varname(s) and aliases
     srcVarName.clear();
     alias.clear();

     for( size_t k=0 ; k < p->optStr.size() ; ++k )
     {
       std::vector<std::string> s0;
       std::vector<std::string> a0;
       s0 = getVarname( p->optStr[k], a0);

       for( size_t k1=0 ; k1 < s0.size() ; ++k1 )
       {
         if( s0[k1].size() > 0 )
         {
           srcVarName.push_back( s0[k1] ) ;
           if( a0[k1].size() > 0 )
             alias.push_back( a0[k1] );
         }
       }
     }

     // look for matches of operandStr and aliases
     for( size_t i=0 ; i < operandStr.size() ; ++i )
     {
       for( size_t j=0 ; j < alias.size() ; ++j )
       {
          if( alias[j] != operandStr[i] )
            continue;

          for( size_t k=0 ; k < p->variable.size() ; ++k )
          {
             if( p->variable[k].name == srcVarName[j] )
             {
                srcVariable.push_back( *new Variable) ;
                srcVariable.back().name = p->variable[k].name ;
//                varName.push_back( p->variable[k].name );

                setSrcVariable(srcVariable.back(), p);

                opMap[operandStr[i]] = &srcVariable.back();
             }
          }
       }
    }
  }

  if( opMap.size() > 0 )
    return;

  // No source varname specified. Then try whether there is
  // the same number of operandStr in the declaration
  // for the operation as of number of source objects.
  if( operandStr.size() == pSrcBase.size() )
  {
    for( size_t i=0 ; i < pSrcBase.size() ; ++i )
    {
       Base *p=pSrcBase[i];
       srcVariable.push_back( *new Variable) ;
       srcVariable.back().name = p->variable[0].name ;
       setSrcVariable(srcVariable.back(), p);

       opMap[operandStr[i]] =&srcVariable.back();
    }

    return;
  }

  // cannot connect alias to the corresponding variable
  std::ostringstream ostr(std::ios::app);
  ostr << "Base::setVarPropsForOperation(): " ;
  ostr << "no rules to link alias to variables in operation.\n";

  ostr << "Operation:";
  for(size_t j=0 ; j < optStr.size() ;++j)
    ostr << optStr[j] << " " ;

  ostr << "\nSource objects:\n";
  for(size_t j=0 ; j < pSrcBase.size() ;++j)
  {
    Base *p=pSrcBase[j];
    for(size_t i=0 ; i < optStr.size() ;++i)
      ostr << p->optStr[i] << " " ;
  }

  exceptionError( ostr.str() );
  std::string note("E6: no rules to link alias to any variable in operation.");

  finally(6, note);

  return;
}

void
Base::setVarPropsNoOperation( void )
{
  // Selection of source variables and renaming takes place.

  std::vector<std::vector<std::string> > srcVarName;

  // number of specified source varname(s)
  size_t totalSVN=0;

  for( size_t i=0 ; i < pSrcBase.size() ; ++i )
  {
     Base *p=pSrcBase[i];

     srcVarName.push_back( *new std::vector<std::string> );

     for( size_t j=0 ; j < p->optStr.size() ; ++j )
       srcVarName[i] = getVarname(p->optStr[j]) ;

     // try for matching '.*'
     if( srcVarName[i].size() == 1 && srcVarName[i][0] == ".*" )
     {
       srcVarName[i].clear() ;
       std::string t0;
       for( size_t j=0 ; j < p->variable.size() ; ++j )
         srcVarName[i].push_back( p->variable[j].name );
     }

     totalSVN += srcVarName[i].size() ;
  }

  // Clear varName, if matching for ".*"
  // Users would likely use the default, i.e. nothing
  if( varName.size() == 1 && varName[0] == ".*" )
    varName.clear() ;

  if( varName.size() == 0 && totalSVN == 0 )
  {
    // Nothing was specified, so set all you can get.
    // Getting multiple varnames from multiple sources is ok.
    for( size_t i=0 ; i < pSrcBase.size() ; ++i )
    {
      Base *p=pSrcBase[i];
      for( size_t j=0 ; j < p->variable.size() ; ++j )
      {
        srcVariable.push_back( *new Variable) ;
        srcVariable.back().name = p->variable[j].name ;
        varName.push_back( p->variable[j].name );

        setSrcVariable(srcVariable.back(), p);
      }
    }

    return;
  }

  // ------------------------------------

  // vN == 0 && sN  > 0        --> vN[i] = sN[i]
  // This is also for swapping variables in the OutFile
  if( varName.size() == 0 && totalSVN > 0 )
  {
     // assign srcVarName(s) to varName(s)
     for( size_t i=0 ; i < srcVarName.size() ; ++i )
     {
        Base *p=pSrcBase[i];

        for( size_t j=0 ; j < srcVarName[i].size() ; ++j )
        {
          // find the one you want to
          for( size_t k=0 ; k < p->variable.size() ; ++k )
          {
            if( p->variable[k].name == srcVarName[i][j] )
            {
              srcVariable.push_back( *new Variable) ;
              srcVariable.back().name = p->variable[k].name ;
              varName.push_back( p->variable[k].name );

              setSrcVariable(srcVariable.back(), p);
            }
          }
        }
     }

    return;
  }

  // ------------------------------------

  // select varname and rename implicitly, respectively.
  if( varName.size() > 0 && totalSVN == 0 )
  {
     if( varName.size() == pSrcBase.size() )
     {
       // vN  > 0 && sN == 0 && vN == numOfObj --> first of each object
       for( size_t i=0 ; i < pSrcBase.size() ; ++i )
       {
         Base *p=pSrcBase[i];

          // find the one you want to
          if( p->variable.size() > 0 )
          {
             srcVariable.push_back( *new Variable) ;
             srcVariable.back().name = p->variable[0].name ;

             setSrcVariable(srcVariable.back(), p);
          }
       }
       return;
     }
     else
     {
       // vN  > 0 && sN == 0 && vN != numOfObj
       // --> as many as available from the first, and then
       // from the second until the number of varNames is reached.
       size_t count=0;
       for( size_t i=0 ; i < pSrcBase.size() ; ++i )
       {
         Base *p=pSrcBase[i];

          // find the one you want to
         for( size_t j=0 ; j < p->variable.size() ; ++j )
         {
            srcVariable.push_back( *new Variable) ;
            srcVariable.back().name = p->variable[i].name ;

            setSrcVariable(srcVariable.back(), p);

            ++count;
            if( count == varName.size() )
              return;
         }
       }
     }
     return;
  }

  // ------------------------------------

  // vN > 0 && sN > 0
  //     --> rename explicitly (partially)
  if( varName.size() > 0 && totalSVN > 0 )
  {
     // Rename variables.
     // varName[i]  corresponds to srcVarName[i]
     // sequentially from left to right in the sequence of objects,
     // e.g.: v1,v2,v3,v4 <--> b0:v1, b1:v1,v2, b3:v1

     // Note: The for-loop is for the case: varname.size==totalSVN.
     //       Reasonable args are in the responsibility of the user
     size_t id=0;
     size_t num=0;
     for( size_t i=0 ; i < varName.size() ; ++i)
     {
/*
       if( srcVarName[thisID].size() == num )
       {
         num=0;
         ++id;
       }
*/

       if( srcVarName.size() == id )
       {
         std::ostringstream ostr(std::ios::app);
         ostr << "Base::setVarPropsNoOperation(): ";
         ostr << "no rules for this setting." << std::endl;
         exceptionError( ostr.str() );
         std::string note("E7: No rules for finding a variable name.");
         finally(7, note);
       }

       srcVariable.push_back( *new Variable );
       srcVariable.back().name=srcVarName[id][num];
       setSrcVariable(srcVariable.back(), pSrcBase[id]);
       ++num;
     }

     // continue, if varName.size < totalSVN.
/*
     if( srcVarName[thisID].size() == num )
     {
        ++id;
        num=0;
     }
*/

     for( ; id < srcVarName.size() ; ++id)
     {
       for( ; num < srcVarName[id].size() ; ++num)
       {
         srcVariable.push_back( *new Variable );
         srcVariable.back().name=srcVarName[id][num];
         varName.push_back( srcVarName[id][num] );

         setSrcVariable(srcVariable.back(), pSrcBase[id]);
       }
       num=0;
     }

     return;
  }

  // Here we arrived in case of error.
  std::ostringstream ostr(std::ios::app);
  ostr << "Base::setVarPropsNoOperation(): ";
  ostr << "no rules for this setting.";
  exceptionError( ostr.str() );
  std::string note("E8: no rules for finding a variable name.");
  finally(8, note);

  return;
}

