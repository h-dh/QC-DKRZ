#include "qa_PT.h"

ProjectTable::ProjectTable(QA *p0, InFile *p1, std::string &s0, std::string &s1)
{
   qa  = p0;
   pIn = p1;

   path=s0;
   table=s1;

   excludedAttributes.push_back("comment");
   excludedAttributes.push_back("history");
   excludedAttributes.push_back("associated_files");
}

void
ProjectTable::check(void)
{
  for( size_t i=0 ; i < pIn->dataVarIndex.size() ; ++i )
  {
     Variable& var = pIn->variable[pIn->dataVarIndex[i]];
     if( check(var) )
       write(var);
  }

  return;
}

bool
ProjectTable::check(Variable &dataVar)
{
  // return value is true, when there is not a project table, yet.

  // Search the project table for an entry
  // matching the varname and frequency (account also rotated).

  // Open project table. Mode:  read
  std::string str0(path);
  str0 += "/" + table ;

  std::ifstream ifs(str0.c_str(), std::ios::in);
  if( !ifs.is_open() )  // file does not exist
     return true;  // causes writing a new entry

  std::string t_md;

  std::string t0;
  std::string t1;
  std::string prjVarName(dataVar.name);
  if( id_1st.size() )
    prjVarName += id_1st ;

  bool isNew=true;
  while( getline(ifs, str0) )
  {
    if( str0.size() )
      continue;  // skip blank lines

    size_t p1, p2;  // positions of ','
    if( (p1=str0.find(',')) < std::string::npos )
    {
      t0 = str0.substr(0, p1);
      if( t0 != prjVarName )
        continue;  // did not match var-name

      // looking for a second item
      if( (p2=str0.find(',', ++p1)) == std::string::npos )
        continue;  // corrupt line

      t1 = str0.substr(p1, p2-p1);
    }
    // else  // would be a corrupt table

    if( id_2nd.size() && t1 != id_2nd )
       continue;

    // found a valid entry
    t_md = hdhC::stripSurrounding(str0) ;

    isNew=false;

    // read aux-lines
    while( getline(ifs, str0) )
    {
      t0 = hdhC::stripSurrounding(str0) ;
      if( t0.substr(0,4) != "aux=" )
        goto BREAK ;  // found the end of the entry

      t_md += '\n';
      t_md += t0 ;
    }
  }

  if( isNew )
    return true;  // entry not found

BREAK:

  // close the project table
  ifs.close();

  // get meta data info from the file
  std::string f_md;
  getMetaData(dataVar, f_md);

  // Comparison of meta-data from the project table and the file, respectively.
  // Note that the direct comparison fails because of different spaces.

  // Deviating distribution of auxiliaries and attributes is tolerated.
  // Use index-vector for book-keeping.
  // Meaning: x=split, t=file, t=table, a=attribute, eq=(att-name,att-value)

  Split splt_xt(t_md, '\n');
  Split splt_xf(f_md, '\n');
  size_t xt_sz=splt_xt.size();
  size_t xf_sz=splt_xf.size();

  std::vector<std::string> xf;
  for( size_t i=0 ; i < xf_sz ; ++i )
    xf.push_back( hdhC::stripSurrounding(splt_xf[i]) );

  std::vector<std::string> xt;
  for( size_t i=0 ; i < xt_sz ; ++i )
    xt.push_back( hdhC::stripSurrounding(splt_xt[i]) );

  // simple test for identity
  if( xt_sz == xf_sz )
  {
    bool is=true;
    for( size_t i=0 ; i < xt_sz ; ++i )
    {
       if( xt[i] != xf[i] )
       {
         is=false ;
         break;
       }
    }

    if( is )
       return false;
  }

  // store indexes
  std::vector<int> xf_ix;
  for( size_t i=0 ; i < xf_sz ; ++i )
    xf_ix.push_back( i );

  std::vector<int> xt_ix;
  for( size_t i=0 ; i < xt_sz ; ++i )
    xt_ix.push_back( i );

  // At first, test whether total auxiliary entries are identical.
  for( size_t i=0 ; i < xt_sz ; ++i )
  {
    for( size_t j=0 ; j < xf_sz ; ++j )
    {
       // only test strings for indexes still to be checked
       if( xt_ix[i] > -1 && xf_ix[i] > -1 && xf[j] == xt[i] )
       {
         xt_ix[i] = -1;
         xf_ix[j] = -1;
         break;
       }
    }
  }

  Split xf_a;  // meta-data from file
  xf_a.setSeparator(',');
  Split xt_a;  // from the table
  xt_a.setSeparator(',');

  Split xf_eq;
  xf_eq.setSeparator('=');
  Split xt_eq;
  xt_eq.setSeparator('=');

  bool isMissAux;
  for( size_t ixt=0 ; ixt < xt_ix.size() ; ++ixt )
  {
    int jt=xt_ix[ixt];

    if( jt < 0 )
      continue;  // passed already a check

    isMissAux=true;

    //split at comma
    xt_a = xt[jt] ;

    for( size_t ixf=0 ; ixf < xf_ix.size() ; ++ixf )
    {
      int jf=xf_ix[ixf];

      if( jf < 0 )
        continue;  // passed already a check

      //split at comma
      xf_a = xf[jf] ;

      // compare the names of the auxiliaries
      if( xf_a[0] != xt_a[0] )
        continue;

      isMissAux=false;

      // scan the attributes
      for( size_t ita=1 ; ita < xt_a.size() ; ++ita )
      {
        // split at '='
        xt_eq = xt_a[ita] ;

        bool isAttMissing=true;

        for( size_t ifa=1 ; ifa < xf_a.size() ; ++ifa )
        {
          // split at '='
          xf_eq = xf_a[ifa] ;

          if( xt_eq[0] == xf_eq[0] ) // found identical attribute names
          {
            isAttMissing=false;
            if( xt_eq[1] == xf_eq[1] )  // found identical values
              break;  // try the next attribute

            // different values --> annotation
            std::string key("51_3");
            if( notes->inq( key, dataVar.name ) )
            {
              std::string capt;
              if( xt_a[0].substr(0,4) == "aux=" )
              {
                capt = "auxiliary=";
                capt += xt_a[0].substr(4) ;
              }
              else
              {
                capt = "variable=";
                capt += xt_a[0] ;
              }

              if( xt_eq[0] == "values" )
                capt += ": changed layout";
              else
              {
                capt += ", att=";
                capt += xt_eq[0];
                capt += ": changed value" ;
              }
              capt += " as to the project table";

              std::string text(xt_eq[0]) ;
              text += " (table)" ;
              if( xt_eq[1].size() )
              {
                text += '=' ;
                text += xt_eq[1] ;
              }
              else
                text += ": not available" ;

              text += "\n" ;
              text += xf_eq[0] ;
              text += " (file)" ;
              if( xf_eq[1].size() )
              {
                text += '=' ;
                text += xf_eq[1] ;
              }
              else
                text += ": not available" ;

              (void) notes->operate(capt, text) ;
              notes->setCheckMetaStr( "FAIL" );
            }

            break;  // try next
          }
        }

        if( isAttMissing )
        {
          std::string key("51_2");
          if( notes->inq( key, dataVar.name ) )
          {
            std::string capt;
            if( xt_a[0].substr(0,4) == "aux=" )
            {
              capt = "auxiliary=";
              capt += xt_a[0].substr(4) ;
            }
            else
            {
              capt = "variable=";
              capt += xt_a[0] ;
            }

            if( xt_eq[0] == "values" )
              capt += ": no value";
            else
            {
              capt += ", att=";
              capt += xt_eq[0];
              capt += ": missing" ;
            }
            capt += " compared to the project table";

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( "FAIL" );
          }
        }
      }
    }

    if( isMissAux )
    {
      std::string key("50_1");
      if( notes->inq( key, dataVar.name ) )
      {
        std::string capt;
        if( xt_a[0].substr(0,4) == "aux=" )
        {
          capt = "auxiliary=";
          capt += xt_a[0].substr(4) ;
        }
        else
        {
          capt = "variable=";
          capt += xt_a[0] ;
        }

        capt += ": missing compared to the project table" ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( "FAIL" );
      }
    }
  }


  // test for missing attributes in the table (reversed for-loops)
  bool isAddAux=true;
  for( size_t ixf=0 ; ixf < xf_ix.size() ; ++ixf )
  {
    int jf=xf_ix[ixf];

    if( jf < 0 )
      continue;  // passed already a check

    isAddAux=true;

    //split at comma
    xf_a = xf[jf] ;

    for( size_t ixt=0 ; ixt < xt_ix.size() ; ++ixt )
    {
      int jt=xt_ix[ixt];

      if( jt < 0 )
        continue;  // passed already a check

      //split at comma
      xt_a = xt[jf] ;

      // compare the names of the auxiliaries
      if( xf_a[0] != xt_a[0] )
        continue;

      // found identical auxiliaries
      isAddAux=false;

      // scan the attributes
      for( size_t ifa=1 ; ifa < xf_a.size() ; ++ifa )
      {
        // split at '='
        xf_eq = xf_a[ifa] ;
        bool isAddAtt=true;

        for( size_t ita=1 ; ita < xt_a.size() ; ++ita )
        {
          // split at '='
          xt_eq = xt_a[ita] ;

          if( xt_eq[0] == xf_eq[0] ) // found identical attribute names
          {
            isAddAtt=false;
            break;
          }
        }

        if( isAddAtt )
        {
          // additional attribute of auxiliary in the file
          std::string key("51_1");
          if( notes->inq( key, dataVar.name ) )
          {
            std::string capt;
            if( xt_a[0].substr(0,4) == "aux=" )
            {
              capt = "auxiliary=";
              capt += xf_a[0].substr(4) ;
            }
            else
            {
              capt = "variable=";
              capt += xf_a[0] ;
            }

            if( xt_eq[0] == "values" )
              capt += ": additional values compared to the project table";
            else
            {
              capt += ", att=";
              capt += xf_eq[0];
              capt += " is a new attribute compared to the project table" ;
            }

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( "FAIL" );
          }
        }
      }
    }

    if( isAddAux )
    {
      std::string key("50_2");
      if( notes->inq( key, dataVar.name) )
      {
        std::string capt("additional auxiliary=");
        capt += xf_a[0].substr(4) ;
        capt += " compared to the project table" ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( "FAIL" );
      }
    }
  }

  return false;
}

void
ProjectTable::checkType(Variable &var, std::string &s)
{
   // Get the checksum of all non-unlimited variables
   // The purpose of checksums of auxiliary variables is to
   // ensure consistency between follow-up experiments.
   // Note: targets have NC_DOUBLE and all other have NC_FLOAT

   // collect all coordinate vars.
   std::vector<std::string> cv;
   size_t k;

   for( size_t i=0 ; i < pIn->variable.size() ; ++i)
   {
     for( size_t j=0 ; j < pIn->variable[i].dimName.size() ; ++j)
     {
       for( k=0 ; k < cv.size() ; ++k)
         if( pIn->variable[i].dimName[j] == cv[k] )
             break;

       if( k == cv.size() )
          cv.push_back( pIn->variable[i].dimName[j] );
     }
   }

   // test the type of the variable
   std::string annotTxt;

   if( var.isDATA )
   {
     if( var.type != NC_FLOAT )
     {
        // dimless variables are excluded from the check.
        if( var.dimName.size() )
          annotTxt="float";
     }
   }
   else
   {
     for( k=0 ; k < cv.size() ; ++k)
     {
        if( cv[k] == var.name )
        {
          if( var.type != NC_DOUBLE )
          {
            annotTxt="double";
            break;
          }
        }
     }
   }

   std::string type(pIn->nc.getVarTypeStr( var.name ) ) ;

   if( annotTxt.size() )  // wrong type
   {
     std::string key("32_1");
     if( notes->inq( key, "PT") )
     {
       std::string capt("variable=");
       capt += var.name ;
       capt += ": data type " ;
       capt += annotTxt;
       capt += " is required";

       std::string text("type (file)=");
       text += type ;

       (void) notes->operate(capt, text) ;
       notes->setCheckMetaStr( "FAIL" );
     }
   }

   s += ',';
   s += type;

   return;
}

void
ProjectTable::getAtts(Variable &var, std::string &s)
{
   size_t sz = var.attName.size();

   if( sz )
     s += ',';

   for(size_t j=0 ; j < sz ; ++j )
   {
     std::string &aN = var.attName[j] ;

     bool isCont=false;
     for( size_t x=0 ; x < excludedAttributes.size() ; ++x )
     {
       if( aN == excludedAttributes[x] )
       {
         isCont=true;
         break;
       }
     }

     if( isCont )
       continue;

     s += var.attName[j] ;
     s += '=';

     if( var.attValue[j].size() )
     {
        // convert comma --> space
       std::string &t = var.attValue[j][0] ;
       size_t pos;
       while( (pos=t.find(',')) < std::string::npos )
          t[pos]=' ';

       s += t ;
     }

     if( j < (sz-1) )
       s += ',';
   }

   // get checksum of values
   if( ! ( var.isUnlimited() || var.isDATA) )
     getValues(var, s);

   return;
}

void
ProjectTable::getMetaData(Variable &dataVar, std::string &md)
{
  // Put all meta-data to string.

  // beginning of extraction of meta data
  // varname and frequency
  md = "\n" + dataVar.name;
  if( id_1st.size() )
     md += id_1st ;
  md += "," ;

  if( id_2nd.size() )
    md += id_2nd + "," ;

  // dimensions
  md += "dims=";
  for( size_t i=0 ; i < dataVar.dimName.size() ; ++i )
  {
    if( i )
      md += ' ' ;
    md += dataVar.dimName[i] ;
  }

  // get attributes of the data variable
  getAtts(dataVar, md);
  checkType(dataVar, md);

  // get meta-data of auxiliaries
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].isDATA )
       continue;

    md += "\n  aux=" ;
    md += pIn->variable[i].name;

    getAtts(pIn->variable[i], md);
    checkType(pIn->variable[i], md);
  }

  return;
}

void
ProjectTable::getValues(Variable &var, std::string &s)
{
   // Get the checksum of the limited variables.
   // The purpose of checksums of auxiliary variables is to
   // ensure consistency between follow-up experiments.
   // Note: targets have NC_DOUBLE and all other have NC_FLOAT

   uint32_t ck=0;
   std::string str;
   std::vector<std::string> vs;

   if( var.isUnlimited() )
     return;

   // get data and determine the checksum
   if( var.type == NC_CHAR )
   {
     pIn->nc.getData(vs, var.name);
     bool reset=true;  // is set to false during the first call
     for(size_t l=0 ; l < vs.size() ; ++l)
     {
       vs[l] = hdhC::stripSurrounding(vs[l]);
       ck = hdhC::fletcher32_cmip5(vs[l], &reset) ;
     }
   }
   else
   {
      ck=0;
      MtrxArr<double> mv;
      pIn->nc.getData(mv, var.name );

      if( mv.size() > 0 )
        ck = hdhC::fletcher32_cmip5(mv.arr, mv.size()) ;
   }

   std::ostringstream ostr;
   ostr << ",values=" << ck ;

   s += ostr.str() ;

   return;
}

bool
ProjectTable::lockFile(std::string &fName )
{
  //test for a lock
  std::string lockFile(fName);
  lockFile += ".lock" ;

  std::string test("/bin/bash -c \'test -e ") ;
  test += lockFile ;
  test += '\'' ;

  std::string lock("/bin/bash -c \'touch ");
  lock += lockFile ;
  lock += "  &> /dev/null\'" ;

  // see 'man system' for the return value, here we expect 0,
  // if file exists.

  size_t count=0;

  // THIS WILL lock until the lock-file is removed
  while ( ! system( test.c_str() ) )
  {
    sleep( 1 ) ;  // in seconds
    ++count;

    if( count == 1800 )  // 1/2 hr
    {
      std::string key("72_2");
      if( notes->inq( key, "PT") )
      {
         std::string capt("project table is locked for 1/2 hour") ;
         std::string text("Check running applications.") ;

         if( notes->operate(capt, text) )
           qa->setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( system( lock.c_str() ) )
  {
     std::string key("72_1");
     if( notes->inq( key, "PT") )
     {
        std::string capt("could not set a project table lock") ;
        std::string text("could not create lock-file to protect project table.") ;

        if( notes->operate(capt, text) )
          qa->setExit( notes->getExitValue() ) ;
     }

     return true;
  }

  return false;
}

void
ProjectTable::setExcludedAttributes(std::vector<std::string> &v)
{
   for( size_t i=0 ; i < v.size() ; ++i )
     excludedAttributes.push_back(v[i]) ;
   return;
}

bool
ProjectTable::unlockFile(std::string &fName )
{
  //test for a lock
  std::string lockFile(fName);
  lockFile += ".lock" ;

  std::string unlock("/bin/bash -c \'rm -f ");
  unlock += lockFile ;
  unlock += '\'' ;

  // see 'man system' for the return value, here we expect 0,
  // if file exists.

  size_t retries=5;

  for( size_t r=0 ; r < retries ; ++r )
  {
    if( ! system( unlock.c_str() ) )
      return false;
    sleep(1);
  }

  return true;
}

void
ProjectTable::write(Variable &dataVar)
{
  // store meta data info
  std::string md;

  getMetaData(dataVar, md);

  std::string pFile(path);
  pFile += '/' ;
  pFile += table;

  lockFile(pFile);

  // open the file for appending.
  std::fstream oifs;

  oifs.open(pFile.c_str(),
          std::ios::in | std::ios::out );

  if( !oifs) // does not exist
    oifs.open(pFile.c_str(),
          std::ios::out | std::ios::trunc | std::ios::in);

  if (! oifs.is_open() )
  {
    std::string key("70_2");
    if( notes->inq( key, "PT") )
    {
      std::string capt("could not create a project table") ;

      std::string text("Could not create project table=") ;
      text += pFile ;
      text += "\nPlease, check the setting in the configuration file." ;

      if( notes->operate(capt, text) )
      {
        notes->setCheckMetaStr( qa->fail );
        qa->setExit( notes->getExitValue() ) ;
      }
    }
  }
  else
  {
    // append
    oifs.seekp(0, std::ios::end);

    oifs << md << std::flush;
    oifs.close();
  }

  unlockFile(pFile);

  return;
}
