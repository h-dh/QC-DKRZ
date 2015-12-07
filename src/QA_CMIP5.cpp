//#include "qa.h"


DRS_CV::DRS_CV(QA* p)
{
  pQA = p;

  notes = pQA->notes;

  enabledCompletenessCheck=true;

  applyOptions(pQA->optStr);
}

void
DRS_CV::applyOptions(std::vector<std::string>& optStr)
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
DRS_CV::checkFilename(std::string& fName, struct DRS_CV_Table& drs_cv_table)
{
  Split x_filename;
  x_filename.setSeparator("_");
  x_filename.enableEmptyItems();
  x_filename = fName ;

  checkVariableName(x_filename[0]) ;

  checkFilenameGeographic(x_filename);

  std::string frq = pQA->qaExp.getFrequency();

  if( testPeriod(x_filename) && frq.size() && frq != "fx" )
  {
    // no period in the filename
    std::string key("1_6a");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("filename requires a period") ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  checkFilenameEncoding(x_filename, drs_cv_table);

  return;
}

void
DRS_CV::checkFilenameEncoding(Split& x_filename, struct DRS_CV_Table& drs_cv_table)
{
  // fileEncodingName: name of the encoding type
  // fileEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  if( x_filename.size() == 0 )
    return;

  // components of the current path, these are given in the reverse order.
  Split& drs = x_filename;

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";
  std::string t;

  size_t enc_sz = drs_cv_table.fileEncoding.size() ;

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  std::vector<std::vector<size_t> > specialFaultIx ;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;
    specialFaultIx.push_back( std::vector<size_t>() );

    x_e.setSeparator("_");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    // could have a trailing ".nc" item; if yes, then remove this beforehand
    if( drs_cv_table.fileEncoding[ds].rfind(".nc") < std::string::npos )
    {
      std::string& t = drs_cv_table.fileEncoding[ds];
      // is it trailing?
      if( t.substr(t.size()-3) == ".nc" )
        x_e = t.substr(0, t.size()-3) ;
    }
    else
      x_e = drs_cv_table.fileEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->table_DRS_CV.getFile() ;
        capt += ": encoding ";
        capt += hdhC::tf_assign("item", x_e[x]) + " not found in CV";
        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckMetaStr(pQA->fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "variable name" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;

        else if( x_e[x] == "ensemble member" )
          gM[x_e[x]] = getEnsembleMember() ;

        else if( x_e[x] == "gridspec" )
          gM[x_e[x]] = "gridspec" ;

        else if( x_e[x] == "temporal subset" )
          gM[x_e[x]] = n_ast ;

        else if( x_e[x] == "geographical info" )
          gM[x_e[x]] = n_ast ;

        else if( x_e[x] == "version" )
          gM[x_e[x]] = n_ast ;
      }
      else if( x_e[x] == "MIP table" )
        gM[x_e[x]] = pQA->qaExp.getMIP_tableName() ;
      else
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;
    }

    // special for gridspec filenames
    if( drs_cv_table.fileEncodingName[ds] == "GRIDSPEC" )
    {
      for( size_t i=0 ; i < drs.size() ; ++i)
      {
        // frequency
        if( x_e[i] == "frequency" )
        {
          t = gM[ x_e[i] ];
          if( t != "fx" || drs[i] != "fx" )
            specialFaultIx[ds].push_back(i);
        }

        // ensemble member
        if( x_e[i] == "ensemble member" )
        {
          t = gM[ x_e[i] ];
          if( t != "r0i0p0" )
            specialFaultIx[ds].push_back(i);
        }
      }
    }


    // append * twice in case of no geo-info nor period;
    // surplusses are safe
    drs.append("*");
    drs.append("*");

    //count coincidences between path components and the DRS
    for( size_t jx=0 ; jx < x_e.size() ; ++jx)
    {
      t = gM[ x_e[jx] ];

      if( jx == drs.size() )
        break;

      if( drs[jx] == t || t == n_ast )
        ++countCI[ds];
    }

    if( ! specialFaultIx[ds].size() )
      if( countCI[ds] == x_e.size() )
        return;  // check passed; take into account two optional items
  }

  // find the encoding with maximum number of coincidences, which
  // is assumed to correspond to the current encoding type.
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  // find details of the faults
  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::vector<std::string> text;
  std::vector<std::string> keys;

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    t = gM[ x_e[i] ];

    if( hdhC::isAmong(i, specialFaultIx[m]) )
    {
      if( x_e[i] == "frequency" )
      {
        keys.push_back("1_5b");
        text.push_back( "A gridspec file must have frequency fx" );
      }

      if( x_e[i] == "ensemble member" )
      {
        keys.push_back("1_5a");
        text.push_back( "A gridspec file must have ensemble member r0i0p0" );
      }
    }
  }

  std::string txt;
  findFN_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
     text.push_back(txt);
     keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS CV filename:");
    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], "DRS") )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkFilenameGeographic(Split& x_filename)
{
  // the geographical indicator should be the last item
  int i;
  int last = x_filename.size()-1 ;

  for( i=last ; i >= 0 ; --i )
  {
    if( x_filename[i].size() > 1 )
    {
      if( x_filename[i][0] == 'g' && x_filename[i][1] == '-' )
        break;
    }
  }

  if( i == -1 )
    return ;  // no geo-indicator

  Split x_geo(x_filename[i], "-");

  std::vector<std::string> key;
  std::vector<std::string> capt;
  std::string cName("geographical indicator");
  if( i != last )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7a");
    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " should appear last in the filename" ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  if( x_geo.size() < 2 )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7b");

    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " g-XXXX[-YYYY]: syntax fault" ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
    }

    return ;
  }

  // valid stand-alone '-' separated words
  std::vector<std::string> kw;
  kw.push_back("global");
  kw.push_back("lnd");
  kw.push_back("ocn");
  kw.push_back("zonalavg");
  kw.push_back("areaavg");

  size_t kw_pos_zzz=3;

  // entries (1st char): (b)ounding-box, (r)egion, (t)ype, (f)ault
  std::vector<char> seq ;

  for( size_t ix=1 ; ix < x_geo.size() ; ++ix )
  {
    if( ix == 1 )
    {
      // a) bounding box?
      Split x_box;
      x_box.setSeparator("lat");
      x_box.addSeparator("lon");
      x_box.setItemsWithSeparator();
      x_box = x_geo[ix];

      for(size_t n=0 ; n < x_box.size() ; ++n )
      {
        bool isLat, isLon ;
        double val;
        if( x_box[n] == "lat" )
        {
          isLat = true ;
          isLon = false ;
          val=90.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }
        else if( x_box[n] == "lon" )
        {
          isLon = true ;
          isLat = false ;
          val=180.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }

        Split x_term;
        x_term.setSeparator(":alnum:");
        x_term = x_box[n];

        for( size_t jx=0; jx < x_term.size() ; ++jx )
        {
          // a number is expected
          if( hdhC::isDigit(x_term[jx]) )
          {
            if( x_term[jx].find('.') < std::string::npos )
            {
              key.push_back("1_7c");
              capt.push_back(cName + ": numbers should be rounded to the nearest integer");
            }

            if( hdhC::string2Double(x_term[jx]) > val )
            {
              key.push_back("1_7d");
              capt.push_back(cName);
              if(isLat)
                capt.back() += ": latitude value should not exceed 90 degr";
              else if(isLon)
                capt.back() += ": longitude value should not exceed 180 degr";
            }
          }

          // range indicator
          ++jx;
          bool is_SNWE_fault=false;

          if( isLat && ! (x_term[jx] == "S" || x_term[jx] == "N") )
            is_SNWE_fault=true;
          else if( isLon && ! (x_term[jx] == "E" || x_term[jx] == "W") )
            is_SNWE_fault=true;

          if( is_SNWE_fault )
          {
            key.push_back("1_7e");
            capt.push_back(cName + ": invalid bounding-box ");
          }
        }
      }
    }  // ix==1

    if( ix ==1 && seq.size() )
      continue;

    size_t k;
    for(k=0 ; k < kw.size() ; ++k )
      if( kw[k] == x_geo[ix] )
        break;

    if( k < kw_pos_zzz )
      seq.push_back('r') ;
    else if( k < kw.size() )
      seq.push_back('t') ;
    else
      seq.push_back('f') ;

  } // geo_ix

  // a type requires a preceding BB or region
  for( size_t i=0 ; i < seq.size() ; ++i )
  {
    if( seq[i] == 't')
    {
      bool isA = !( i && (seq[i-1] == 'b' || seq[i-1] == 'r') ) ;
      bool isB = !( i && (seq[i-1] == 'f'  ) ) ;
      if( isA && isB )
      {
        key.push_back("1_7g");
        capt.push_back(cName + ": invalid specifier ");
        capt.back() += "g-XXXX[-yyy][-zzz]: given zzz, but missing XXXX";
      }
    }
  }

  if( hdhC::isAmong('f', seq) )
  {
    // 1st index for 'g'
    for( size_t i=1 ; i < x_geo.size() ; ++i )
    {
      size_t j;
      for( j=0 ; j < kw.size() ; ++j )
      {
        if( kw[j] == x_geo[i] )
          break;

        std::string t(x_geo[i].substr(0,3));
        if( t == "lat" || t == "lon" )
          break;
      }

      if( j == kw.size() )
      {
        key.push_back("1_7f");
        capt.push_back(cName + ": invalid specifier");
        capt.back() += hdhC::tf_val(x_geo[i]) ;
        capt.back() += ", valid are";
        capt.back() += hdhC::tf_val( hdhC::getComposedVector(kw) );
      }
    }
  }


  for( size_t ii=0 ; ii < key.size() ; ++ii )
  {
    if( notes->inq( key[ii], pQA->fileStr))
    {
      (void) notes->operate(capt[ii]) ;
      notes->setCheckMetaStr(pQA->fail);
    }
  }

  return ;
}

void
DRS_CV::checkMIP_tableName(Split& x_filename)
{
  // Note: filename:= name_CMOR-MIP-table_... .nc
  if( x_filename.size() < 2 )
    return ;  //corrupt filename

  // table sheet name from global attributes has been checked

  // compare file entry to the one in global header attribute table_id
  if( x_filename[1] != CMOR::tableSheet )
  {
    std::string key("7_8");

    if( notes->inq( key, "MIP") )
    {
      std::string capt("Ambiguous MIP table names, found ") ;
      capt += hdhC::tf_assign("MIP-table(file)", x_filename[1]) ;
      capt += " vs. global ";
      capt += hdhC::tf_att("table_id",CMOR::tableSheet);

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
    }

    // try the filename's MIP table name
    if( CMOR::tableSheet.size() == 0 )
      CMOR::tableSheet = pQA->qaExp.getMIP_tableName(x_filename[1]) ;
  }

  return ;
}

void
DRS_CV::checkProductName(std::string& drs_product,
  std::string prod_choice,
  std::map<std::string, std::string>& gM)
{
  Split x_cp(prod_choice, "|");

  for( size_t i=0 ; i < x_cp.size() ; ++i )
  {
    if( drs_product == hdhC::stripSides(x_cp[i]) )
    {
      // adjustment for a test
      gM[CMOR::n_product] = drs_product ;
      return;
    }
  }

  std::string key("1_3");

  if( notes->inq( key, "DRS") )
  {
    std::string capt("DRS CV fault for path component");
    capt += hdhC::tf_val(CMOR::n_product);
    capt += ", found" ;
    capt += hdhC::tf_val(drs_product) ;
    capt += ", expected one of";
    capt += hdhC::tf_val(prod_choice);

    (void) notes->operate(capt) ;
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
DRS_CV::checkPath(std::string& path, struct DRS_CV_Table& drs_cv_table)
{
  // pathEncodingName: name of the encoding type
  // pathEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  size_t enc_sz = drs_cv_table.pathEncoding.size() ;

  if( path.size() == 0 || enc_sz == 0 )
    return;

  // components of the current path, these are given in the reverse order.
//  path= "/hdh/data/CMIP5/CMIP5/output/MPI-M/MPI-ESM-LR/hysterical/day/artmos/r1i1p1/tas/";
  Split drs(path, "/");

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;

    x_e.setSeparator("/");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    x_e = drs_cv_table.pathEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->table_DRS_CV.getFile() ;
        capt += ": encoding " ;
        capt += hdhC::tf_assign("item", x_e[x]) ;
        capt += " not found in CV";

        if( notes->inq( key, "DRS") )
        {
          (void) notes->operate(capt) ;
          notes->setCheckMetaStr(pQA->fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "variable name" )
          gM[x_e[x]] = pQA->qaExp.fVarname ;

        else if( x_e[x] == "ensemble member" )
          gM[x_e[x]] = getEnsembleMember() ;

        else if( x_e[x] == "MIP table" )
          gM[x_e[x]] = pQA->qaExp.getMIP_tableName() ;

        else if( x_e[x] == "gridspec" )
          gM[x_e[x]] = "gridspec" ;
      }
      else
      {
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;

        if( x_e[x] == "MIP table" && gM[x_e[x]].substr(0,6) == "Table " )
        {
          size_t pos=6;
          if( (pos=gM[x_e[x]].find(" ", pos)) < std::string::npos )
            gM[x_e[x]] = gM[x_e[x]].substr(6, pos-6) ;
        }
      }
    }

    std::string t;
    int ix;
    if( (ix = getPathBegIndex( drs, x_e, gM )) == -1 )
      continue;

    size_t drsBeg = ix;
    size_t i;

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      t = gM[ x_e[j] ];

      if( (i = drsBeg+j) < drs.size() )
      {
        if( drs[i] == t || t == n_ast )
          ++countCI[ds];
        else if( x_e[j] == "version number" && hdhC::isDigit(drs[i].substr(1)) )
          ++countCI[ds];

        // drs may have a different value, e.g. output1
        else if( x_e[j] == CMOR::n_product )
        {
          checkProductName(drs[i], cvMap[CMOR::n_product+"_constr"], gM);
          ++countCI[ds];
        }
      }
      else
        drs.append(n_ast); // who knows
    }

    if( countCI[ds] == x_e.size() )
      return;  // check passed
  }

  // find the encoding with maximum number of coincidences
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  std::vector<std::string> text;
  std::vector<std::string> keys;

  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::string txt;
  findPath_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
    text.push_back(txt);
    keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS CV path:");

    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], pQA->fileStr) )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckMetaStr(pQA->fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkVariableName(std::string& f_vName)
{
  if( f_vName.find('-') < std::string::npos )
  {
    std::string key("1_4c");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt(hdhC::tf_var(f_vName));
      capt += "in the filename should not contain a hyphen" ;

      (void) notes->operate(capt) ;
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
DRS_CV::findFN_faults(Split& drs, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();
  int drsSz = drs.size() ;

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    if( drsSz == x_eSz )
    {
      text = " check failed, suspicion of a missing item in the filename, found" ;
      text += hdhC::tf_val(drs.getStr()) ;

      return;
    }

    if( drs[j] == CMOR::n_output )
    if( !(drs[j] == t || t == n_ast) )
    {
      text = " check failed, found " ;
      text += hdhC::tf_assign(x_e[j],drs[j]) ;
      text += ", expected" ;
      text += hdhC::tf_val(t) ;

      return;
    }
  }

  return ;
}

void
DRS_CV::findPath_faults(Split& drs, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();
  int drsSz = drs.size() ;
  int drsBeg = drsSz - x_eSz ;

  if( drsBeg < 0)
  {
    text = " check failed" ;
    return; // fewer path items than expected
  }

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    int i = drsBeg+j ;

    if( i == -1 )
    {
      text = " check failed, suspicion of a missing item in the path, found" ;
      text += hdhC::tf_val(drs.getStr()) ;
      break;
    }

    if( !( drs[i] == t || t == n_ast) )
    {
      if(x_e[j] == "activity" )
      {
        if( notes->inq( "1_1a", pQA->fileStr, "INQ_ONLY") )
        {
          std::string s( hdhC::Upper()(drs[i]) ) ;
          if( s == t )
            continue;
        }
      }

      if(x_e[j] == "version number" )
      {
        if( drs[i][0] == 'v' && hdhC::isDigit(drs[i].substr(1)) )
          continue;
      }

      text = " check failed, expected " ;
      text += hdhC::tf_assign(x_e[j],t) ;
      text += " found" ;
      text += hdhC::tf_val(drs[drsBeg+j]) ;

      break;
    }
  }

  return ;
}

std::string
DRS_CV::getEnsembleMember(void)
{
  if( ensembleMember.size() )
    return ensembleMember;

  // ensemble member
  int ix;
  size_t g_ix = pQA->pIn->varSz ;  // index to the global atts

  ensembleMember = "r" ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("realization")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ensembleMember += 'i' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("initialization_method")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  ensembleMember += 'p' ;
  if( (ix = pQA->pIn->variable[g_ix].getAttIndex("physics_version")) > -1 )
    ensembleMember += pQA->pIn->variable[g_ix].attValue[ix][0] ;

  return ensembleMember ;
}

int
DRS_CV::getPathBegIndex(
    Split& drs, Split& x_e,
    std::map<std::string, std::string>& gM )
{
  int ix=-1;
  bool isActivity; // applied case-insensivity

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    std::string s(drs[i]);

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      if( x_e[j] == CMOR::n_activity )
      {
        isActivity=true;
        s = hdhC::Upper()(s);
      }
      else
        isActivity=false;

      if( s == gM[ x_e[j] ]  )
      {
        ix = static_cast<int>(i);

        // the match could be a leading false one, requested is the last one
        for( ++i ; i < drs.size() ; ++i)
        {
          s = drs[i];

          if(isActivity)
            s = hdhC::Upper()(s);

          if( s == gM[ x_e[j] ]  )
            ix = static_cast<int>(i);
        }

        return ix;
      }
    }
  }

  return ix;
}

void
DRS_CV::run(void)
{
  DRS_CV_Table& drs_cv_table = pQA->drs_cv_table ;

  // check the path
  checkPath(pQA->pIn->file.path, drs_cv_table) ;

  // compare filename to netCDF global attributes
  checkFilename(pQA->pIn->file.basename, drs_cv_table);

  return;
}

bool
DRS_CV::testPeriod(Split& x_f)
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

  // Does the filename have a trailing date range?
  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  // is it formatted as expected?
  if( testPeriodFormat(x_f, sd) )
    return true;

  // now that we have found two candidates for a date
  // compose ISO-8601 strings
  std::vector<Date> period;
  pQA->qaTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("1_6c");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("invalid period in the filename, found ");
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
  }
  else
  {
    if( pQA->qaTime.time_ix > -1 &&
        ! pQA->pIn->variable[pQA->qaTime.time_ix].isInstant )
    {
      std::string key("3_14");
      if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
      {
        std::string capt(hdhC::tf_var("time_bnds"));
        capt += "is missing" ;

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
    std::string key("1_7f");

    if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
    {
      std::string capt("Warning: period in the filename: ") ;
      capt +="note that StartTime-EndTime is compliant with a CMOR peculiarity";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( pQA->fail );
    }
  }
  else if( testPeriodDatesFormat(sd) ) // format of period dates.
  {
    // period requires a cut specific to the various frequencies.
    std::vector<std::string> text ;
    testPeriodPrecision(sd, text) ;

    if( text.size() )
    {
      std::string key("1_6e");
      std::string capt("period in the filename with wrong precision") ;

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
DRS_CV::testPeriodAlignment(std::vector<std::string>& sd, Date** pDates, bool b[])
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
      std::string key("1_6g");
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
DRS_CV::testPeriodPrecision(std::vector<std::string>& sd,
                  std::vector<std::string>& text)
{
  // Partitioning of files check are equivalent.
  // Note that the format was tested before.
  // Note that desplaced start/end points, e.g. '02' for monthly data, would
  // lead to a wrong cut.

  if( sd[0].size() != sd[1].size() )
  {
    text.push_back(" 1st and 2nd date are different") ;
    return;
  }

  // precision corresponds to the MIP table
  std::vector<size_t> ix;

  // a) yyyy
  if( CMOR::tableSheet == QA_Exp::MIP_tableNames[1] )
  {
    if( sd[0].size() != 4 )
      text.push_back(", expected yyyy, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // b) ...mon, aero, Oclim, and cfOff
  ix.push_back(2);
  ix.push_back(3);
  ix.push_back(4);
  ix.push_back(5);
  ix.push_back(6);
  ix.push_back(7);
  ix.push_back(8);
  ix.push_back(13);
  ix.push_back(17);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( CMOR::tableSheet == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 6 )
        text.push_back(", expected yyyyMM, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // c) day, cfDay
  if( CMOR::tableSheet == QA_Exp::MIP_tableNames[9]
        || CMOR::tableSheet == QA_Exp::MIP_tableNames[14] )
  {
    if( sd[0].size() != 8 )
      text.push_back(", expected yyyyMMdd, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // d) 6hr..., 3hr, and cf3hr
  ix.clear();
  ix.push_back(10);
  ix.push_back(11);
  ix.push_back(12);
  ix.push_back(15);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( CMOR::tableSheet == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 12 )
        text.push_back(", expected yyyyMMddhhmm, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // e) cfSites
  if( CMOR::tableSheet == QA_Exp::MIP_tableNames[16] )
  {
    if( sd[0].size() != 14 )
      text.push_back(", expected yyyyMMddhhmmss, found " + sd[0] + "-" + sd[1]) ;
  }

  return;
}

bool
DRS_CV::testPeriodDatesFormat(std::vector<std::string>& sd)
{
  // return: true means go on for testing the period cut
  // partitioning of files
  if( sd.size() != 2 )
    return false;  // no period; is variable time invariant?

  std::string frequency(pQA->qaExp.getFrequency());
  std::string str;

  if( frequency == "3hr" || frequency == "6hr" )
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
    std::string key("1_6f");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("period in filename of incorrect format, found ");
        capt += sd[0] + "-" +  sd[1];
        capt += " expected " + str ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
     }
  }

  return true;
}

bool
DRS_CV::testPeriodFormat(Split& x_f, std::vector<std::string>& sd)
{
  int x_fSz=x_f.size();

  if( ! x_fSz )
    return true; // trapped before

  // any geographic subset? Even with wrong separator '_'?
  if( x_f[x_fSz-1].substr(0,2) == "g-" )
    --x_fSz;
  else if( x_f[x_fSz-1] == "g" )
    --x_fSz;
  else if( x_fSz > 1 && x_f[x_fSz-2].substr(0,2) == "g" )
    x_fSz -= 2 ;

  if( x_fSz < 1 )
    return true;

  if( x_f[x_fSz-1] == "clim" || x_f[x_fSz-1] == "ave" )
  {
    // Wrong separator for appendix clim or ave, found '_'
    std::string key = "1_6b" ;
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("Wrong separation of filename's period's appendix");
      capt += hdhC::tf_val(x_f[x_fSz-1]);
      capt += ", found underscore";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr( pQA->fail );
    }

    --x_fSz;
  }

  if( x_fSz < 1 )
    return true;

  Split x_last(x_f[x_fSz-1],'-') ;
  int x_lastSz = x_last.size();

  // minimum size of x_last could never be zero

  // elimination of 'ave' or 'clim' separated by a dash
  if( x_last[x_lastSz-1] == "clim" || x_last[x_lastSz-1] == "ave" )
    --x_lastSz ;

  bool isRegular=true;
  if( x_lastSz == 2 )
  {
    // the regular case
    std::vector<int> ix;
    if( hdhC::isDigit(x_last[0]) )
      sd[0]=x_last[0] ;
    else
    {
      ix.push_back(0);
      isRegular=false;
    }

    if( hdhC::isDigit(x_last[1]) )
      sd[1]=x_last[1] ;
    else
    {
      isRegular=false;
      ix.push_back(1);
    }

    if( ix.size() == 2 )
      return true;  // apparently not a period
    else if( ix.size() == 1 )
    {
      // try for clim or ave without any separator with a wrong one
      size_t pos;
      std::string f(x_last[ix[0]]);
      if( (pos=f.rfind("clim")) < std::string::npos
             || (pos=f.rfind("ave")) < std::string::npos )
      {
        if( pos && hdhC::isDigit( f.substr(0, pos-1) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos-1) ;
        }
        else if( pos && hdhC::isDigit( f.substr(0,pos) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos) ;
        }
        else
          isRegular=false;

        if(ix.size())
        {
          // Wrong separator for appendix clim or ave, found '_'
          std::string key("1_6b") ;
          if( notes->inq( key, pQA->fileStr) )
          {
            std::string capt("Wrong separation of filename's period's appendix");
            capt += hdhC::tf_val(x_last[ix[0]]);

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( pQA->fail );
          }
        }
      }
    }
  }
  else
    isRegular=false;

  if( ! isRegular )
  {
    std::string f;
    if( sd[0].size() )
      f = x_last[1];
    else
      f = x_last[0];

    // could be a period with a wrong separator
    std::string sep;
    size_t pos=std::string::npos;

    bool isSep=false;
    for( size_t i=0 ; i < f.size() ; ++i )
    {
      if( !hdhC::isDigit(f[i]) )
      {
        if(isSep)
          return true;  // not a wrong separator, just anything

        isSep=true;
        sep=f[i];
        pos=i;
      }
    }

    if( pos < std::string::npos )
    {
      sd[0]=f.substr(0, pos) ;
      sd[1]=f.substr(pos+1) ;

      // Wrong separator for appendix clim or ave, found '_'
      std::string key("1_6b") ;
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string capt("Wrong separation of filename's period's dates");
        capt += ", found ";
        capt += hdhC::tf_val(sep);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }

  // in case of something really nasty
  if( !( hdhC::isDigit(sd[0]) && hdhC::isDigit(sd[1]) ) )
    return true;

  return false;
}

// class for comparing meta-data from the file to the
// "CMIP5 MOdel Output Requirements", but global attributes
CMOR::CMOR(QA* p)
{
  pQA = p;
  pExp = &p->qaExp;
  notes = p->notes;

  initDefaults();

  applyOptions(p->optStr);
}

void
CMOR::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "pEI" || split[0] == "parentExperimentID"
         || split[0] == "parent_experiment_id" )
     {
       if( split.size() == 2 )
       {
          parentExpID=split[1];
       }
       continue;
     }

     if( split[0] == "pER" || split[0] == "parentExperimentRIP"
         || split[0] == "parent_experiment_rip" )
     {
       if( split.size() == 2 )
       {
          parentExpRIP=split[1];
       }
       continue;
     }
  }

  return;
}

void
CMOR::checkDimSheet(std::vector<std::string>& vs_dimSheet,
  InFile& in, VariableMetaData& vMD,
  std::vector<struct DimensionMetaData>& vs_f_DMD_entries,
  std::map<std::string, size_t>& col,
  std::string dimName, size_t colMax )
{
  // dimName is in the CMOR-dimension column of the dim-sheet.
  // This method is called for each of such a dimension given in a sheet
  // for variables.

  // dimName is the only unique connection between the dimensional
  // table sheet and those table sheets for the variables.
  // Two kinds of names are given: the CMOR variant giving explicit
  // names for dedicated purposes and the 'output name'.
  // The 'output name' is ambivalent by subsuming
  // a CMOR dimension family  to a single type
  // (e.g. plev7, plev17, etc. --> plev)

  // Dimensions are described in the upper part of the standard table,
  // which was opened in the calling method/function.

   struct DimensionMetaData t_DMD ;

   // purpose: a dim in the table is only available as variable
   // in the file
   struct DimensionMetaData  tmp_f_DMD ;
   VariableMetaData          tmp_vMD(pQA) ;

   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // look for a matching entry in the dimension sheet
   for( size_t i=0 ; i < vs_dimSheet.size() ; ++i )
   {
     x_line=vs_dimSheet[i];

     // is it in the table?
     if( x_line[col[n_CMOR_dimension]] == dimName )
     {
       t_DMD.attMap[n_CMOR_dimension]=x_line[col[n_CMOR_dimension]];

       // is there a mapping to a name in column 'coords_attr'?
       if( x_line[col[n_coordinates]].size() )
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_coordinates]];
       else
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_output_dim_name]];

       t_DMD.attMap[n_standard_name] =x_line[col[n_standard_name]];

       t_DMD.attMap[n_long_name] =  hdhC::unique(x_line[col[n_long_name]]);
       if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
         t_DMD.attMap[n_long_name] = hdhC::Lower()(t_DMD.attMap[n_long_name]) ;

       t_DMD.attMap[n_axis]=x_line[col[n_axis]];
       t_DMD.attMap[n_units]=hdhC::unique(x_line[col[n_units]]);
       t_DMD.attMap[n_index_axis]=x_line[col[n_index_axis]];
       t_DMD.attMap[n_coordinates]=x_line[col[n_coordinates]];
       t_DMD.attMap[n_bounds_quest]=x_line[col[n_bounds_quest]];
       t_DMD.attMap[n_valid_min]=x_line[col[n_valid_min]];
       t_DMD.attMap[n_valid_max]=x_line[col[n_valid_max]];
       t_DMD.attMap[n_type]=x_line[col[n_type]];
       t_DMD.attMap[n_positive]=x_line[col[n_positive]];
       t_DMD.attMap[n_value]=x_line[col[n_value]];
       t_DMD.attMap[n_bounds_values]=x_line[col[n_bounds_values]];
       t_DMD.attMap[n_requested]=x_line[col[n_requested]];
       t_DMD.attMap[n_bounds_requested]=x_line[col[n_bounds_requested]];

       t_DMD.attMap[n_tol_on_requests]=hdhC::empty;
       std::string& str0 = x_line[col[n_tol_on_requests]] ;
       if( str0.substr(0, n_tol_on_requests.size()) == n_tol_on_requests )
          t_DMD.attMap[n_tol_on_requests]=x_line[col[n_tol_on_requests]];

       std::string& t_DMD_outname        = t_DMD.attMap[n_output_dim_name] ;

       // find corresponding auxiliary name in the file
       size_t index;
       for( index=0 ; index < vs_f_DMD_entries.size() ; ++index)
       {
          std::string& f_DMD_outname
              = vs_f_DMD_entries[index].attMap[n_output_dim_name] ;

          if( f_DMD_outname == t_DMD_outname )
            break;
       }

       if( index == vs_f_DMD_entries.size() )
         continue;

       // compare findings from the file with those from the table
       checkDimSheetEntries(in, vMD, vs_f_DMD_entries[index], t_DMD) ;

/*
       if( t_DMD.attMap[n_CMOR_dimension] == "plevs" )
       {
         // restore for the project table
         vs_f_DMD_entries[index].size     = plevs_file_size  ;
         vs_f_DMD_entries[index].checksum = plevs_file_checksum ;
       }
*/

       return;
    }
  }

  return;
}

void
CMOR::checkDimSheetEntries(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // Do the dimensional meta-data found in the netCDF file
  // match those in the standard output table?

  if( t_DMD.attMap[n_output_dim_name] != f_DMD.attMap[n_output_dim_name] )
    checkDimSheet_outname(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_standard_name] != f_DMD.attMap[n_standard_name] )
    checkDimSheet_stdName(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_long_name] != f_DMD.attMap[n_long_name] )
    checkDimSheet_longName(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_axis] != f_DMD.attMap[n_axis] )
    checkDimSheet_axis(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_units] != f_DMD.attMap[n_units] )
    checkDimSheet_units(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_axis] == "ok" )
    checkDimSheet_indexAxis(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_coordinates].size() )
    checkDimSheet_coordsAtt(in, vMD, f_DMD, t_DMD);

  // the second condition takes especially into account
  // declaration of a variable across table sheets
  // (Omon, cf3hr, and cfSites)
  if( t_DMD.attMap[n_bounds_quest] == "yes" )
    checkDimSheet_boundsQuest(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_min].size() )
    checkDimSheet_validMin(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_max].size() )
    checkDimSheet_validMax(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_type] != f_DMD.attMap[n_type] )
    checkDimSheet_type(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_positive].size() )
    checkDimSheet_positive(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_value].size() )
    checkDimSheet_value(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_values].size() )
    checkDimSheet_boundsValues(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_requested].size() )
    checkDimSheet_requested(in, vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_requested].size() )
    checkDimSheet_boundsRequested(in, vMD, f_DMD, t_DMD);

  return ;
}

void
CMOR::checkDimSheet_axis(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4e");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_axis ));
    capt += "no match with the request of the CMOR table";

    if( f_DMD.attMap[n_axis].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_axis]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD.attMap[n_axis]);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkDimSheet_boundsQuest(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // standard table requires a boundary variable
  if( f_DMD.var->bounds.size() )
    return ;

  std::string key("4_4p");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "bounds?" ));
    capt += "missing bounds-variable as requested by the CMOR table" ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkDimSheet_boundsRequested(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
//   checkWithTolerance(f_DMD, t_DMD, n_bounds_requested );
   return;
}

void
CMOR::checkDimSheet_boundsValues(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check dimensional values

  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;

  checkWithTolerance(f_DMD, t_DMD, n_bounds_values);

  return ;
}

void
CMOR::checkDimSheet_coordsAtt(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::vector<std::string> dimVar;
  dimVar.push_back("region");
  dimVar.push_back("passage");
  dimVar.push_back("type_description");

  std::string& t_DMD_outname = t_DMD.attMap[n_output_dim_name] ;

  // note: the loop is mostly passed without action
  for(size_t ix=0 ; ix < dimVar.size() ; ++ix )
  {
    if( t_DMD_outname != dimVar[ix] )
      continue;

    // Is coordinates-att set in the corresponding variable?
    std::string s(vMD.var->getAttValue(n_coordinates));

    if( s.find(dimVar[ix]) == std::string::npos )
    {
      std::vector<std::string> dim;
      dim.push_back("basin");
      dim.push_back("line");
      dim.push_back(n_type);

      std::string key("4_10");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt( QA_Exp::getCaptionIntroVar("dims", vMD, n_coordinates ));
        capt += "missing name, expected from entry";
        capt += hdhC::tf_val(dim[ix]);
        capt += "member";
        capt += hdhC::tf_val(dimVar[ix]);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkDimSheet_indexAxis(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  return;
}

void
CMOR::checkDimSheet_longName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4d");

  std::string& t_DMD_long_name = t_DMD.attMap[n_long_name] ;
  std::string& f_long_name = f_DMD.attMap[n_long_name] ;

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "long name" ));
    if( f_long_name.size() )
    {
      capt += "found";
      capt += hdhC::tf_val(f_long_name);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD_long_name) ;

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
CMOR::checkDimSheet_outname(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4b");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "output dimension name" ));
    capt += "missing output dimensions name in the file";

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return true;
}

void
CMOR::checkDimSheet_positive(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string& t_positive = t_DMD.attMap[n_positive] ;
  std::string& f_positive = f_DMD.attMap[n_positive] ;

  if( t_positive == f_positive )
    return;

  std::string key("4_4m");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_positive));
    capt += hdhC::tf_att(n_positive);
    capt += "in the file does not match the request in the CMOR table";
    if( f_DMD.attMap[n_positive].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_positive]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_positive);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkDimSheet_requested(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
   // There could be layers additionally
   // to the 17 mandatory ones for the dimension 'plevs'.
   // Thus, we have to ignore the optional ones.
   // for the comparisons between standard output table and file

  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_requested);
  else
  {
    size_t i=0;
    if( t_DMD.attMap[n_CMOR_dimension] == "plevs" )
      i=17;

    checkWithTolerance(f_DMD, t_DMD, n_requested, i );
  }

  return;
}

void
CMOR::checkDimSheet_stdName(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // dimensions which may have various standard names
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  std::string key("4_4c");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "standard name" ));
    capt += "not as the CMOR table requested";
    if( f_DMD.attMap[n_standard_name].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_standard_name]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD.attMap[n_standard_name]);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkDimSheet_type(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  if( f_DMD.attMap[n_type] == "char" && t_DMD.attMap[n_type] == "character" )
     return;

  std::string key("4_4l");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_type ));
    capt += "not as the CMOR table requested";
    capt += ", found";
    capt += hdhC::tf_val(f_DMD.attMap[n_type]);
    capt += ", expected";
    capt += hdhC::tf_val(t_DMD.attMap[n_type]);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
CMOR::checkDimSheet_units(InFile& in,
     VariableMetaData& vMD,
     struct DimensionMetaData& f_DMD,
     struct DimensionMetaData& t_DMD)
{
  // Special: units related to any dimension but 'time'

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?

  // one or more blanks are accepted
  std::string f_units( hdhC::unique(f_DMD.attMap[n_units]) );
  std::string t_units( hdhC::unique(t_DMD.attMap[n_units]) );

  if( t_units == f_units )
    return;

  // dimensions which may have various units
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  if( t_units.find('?') < std::string::npos )
  {
    // time

    // Note: units of 'time' in standard table: 'days since ?',
    //       in the file: days since a specific date.

    if( (f_units.find("days since") < std::string::npos)
          != (t_units.find("days since") < std::string::npos) )
    {
      std::string key("3_9");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
        capt += "ill-formatted units, found";
        capt += hdhC::tf_val(f_units);
        capt += ", expected <days since ...>";

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }

    return;
  }

  bool hasUnits=false;

  // index axis has no units
  if( t_DMD.attMap[n_index_axis] == "ok" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string& t_odn = t_DMD.attMap[n_output_dim_name] ;

  // no units
  if( t_odn == "basin" || t_odn == "line" || t_odn == "type")
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  if( t_odn == "site" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string key("4_4f");

  if( hasUnits )
  {
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
      capt += "found " + hdhC::tf_assign(n_units, f_units) ;
      capt += ", expected dim-less";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  // regular case: mismatch
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
    capt += hdhC::tf_att(n_units);
    capt += " in the file does not match the request in the table ";
    capt += pExp->varReqTable.basename ;
    if( f_units.size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_units);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_units);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return ;
}

void
CMOR::checkDimSheet_value(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;


  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_value );
  else
    checkWithTolerance(f_DMD, t_DMD, n_value);

  return ;
}

void
CMOR::checkDimSheet_validMin(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  in.nc.getData(ma, f_DMD.var->name);

  if( ma.size() )
  {
    double min=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] < min )
        min=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_min]) );

    if( min < t_val )
    {
      std::string key("4_4j");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_min ));
        capt += "data minimum is lower than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(min));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_min]);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkDimSheet_validMax(InFile& in,
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  in.nc.getData(ma, f_DMD.var->name);

  if( ma.size() )
  {
    double max=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] > max )
        max=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_max]) );

    if( max > t_val )
    {
      std::string key("4_4k");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_max ));
        capt += "data maximum is higher than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(max));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_max]);

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkRequestedAttributes(void)
{
  // check requested attributes of specific variables
  for(size_t i=0 ; i < pQA->drs_cv_table.varName.size() ; ++i)
  {
    // global attributes
    if( pQA->drs_cv_table.varName[i] == pQA->s_global
            && pQA->pIn->variable[pQA->pIn->varSz].name == "NC_GLOBAL" )
      checkReqAtt_global();
    else
    {
      for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
      {
        // check for requested attributes of variables (whether coordinates var
        // or data var doesn't matter).
        if( pQA->drs_cv_table.varName[i] == pQA->pIn->variable[j].name )
        {
          checkReqAtt_variable(pQA->pIn->variable[j]);
          break;
        }
      }
    }
  }

/*
  // check attribute _FillValue and missing_value, if available
  std::string fV("_FillValue");
  std::string mV("missing_value");

  std::vector<std::string> fVmv;
  fVmv.push_back(fV);
  fVmv.push_back(mV);

  for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
  {
    Variable& var = pQA->pIn->variable[j] ;

    for( size_t l=0 ; l < 2 ; ++l )
    {
      int k;
      if( (k=var.getAttIndex(fVmv[l])) > -1 )
      {
        std::vector<float> aV;
        pQA->pIn->nc.getAttValues(aV, fVmv[l], var.name ) ;
        float rV = 1.E20 ;

        if( aV.size() == 0 || aV[0] != rV )
        {
          std::string key("3_11");

          if( notes->inq( key, var.name) )
          {
            std::string capt(
              hdhC::tf_att(var.name, fVmv[l], var.attValue[j][0])) ;
            capt += "does not match requested value=1.E20";

            (void) notes->operate(capt) ;
            notes->setCheckMetaStr( pQA->fail );
          }
        }
      }
    }
  }

  // for data variables, both _FillValue and missing_value
  // should be defined in CORDEX
  for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    bool is_fV = var.isValidAtt(fV) ;
    bool is_mV = var.isValidAtt(mV) ;

    if( (is_fV || is_mV) && (is_fV != is_mV) )
    {
      std::string key("3_12");

      if( notes->inq( key, var.name) )
      {
        std::string capt(hdhC::tf_var(var.name, hdhC::colon)) ;
        capt += "if " + hdhC::tf_att(fV);
        capt += "or " + mV ;
        capt += ", then both should be defined";

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr( pQA->fail );
      }
    }
  }
*/

  return ;
}

void
CMOR::checkReqAtt_global(void)
{
  // check requested attributes
  DRS_CV_Table& drs = pQA->drs_cv_table;

  Variable& glob = pQA->pIn->variable[pQA->pIn->varSz] ;

  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == pQA->s_global )
      break;

  if( ix == drs.varName.size() )
    return;

  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // missing requested global attribute
    if( ! glob.isValidAtt(rqName) )
    {
       std::string key("2_3");

       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("requested ");
         capt += pQA->s_global;
         capt += hdhC::blank;
         capt += hdhC::tf_att(rqName);
         capt += "is missing" ;

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr( pQA->fail );

         continue;
       }
    }

    // requested attribute expects a requested value
    if( rqValue.size() )  // a value is requested
    {
      std::string aV = glob.getAttValue(rqName) ;

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, pQA->s_global) )
        {
           std::string capt(pQA->s_global);
           capt += hdhC::blank;
           capt += hdhC::tf_att(rqName, hdhC::no_blank);
           capt=" missing requested value=";
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }
       else
       {
          // note that several alternatives may be acceptable
          Split x_rqValue(rqValue, "|");
          std::vector<std::string> vs_rqValue(x_rqValue.getItems());

          if( x_rqValue[0] == "YYYY-MM-DDTHH:MM:SSZ" )
          {
            // a valid date format may have T substituted by a blank
            // and Z omitted or the latter be lower case.
            bool is=false;
            if( aV.size() > 18 )
            {
              if( aV[4] == '-' && aV[7] == '-' )
              {
                  if( aV[10] == 'T' || aV[10] == ' ' )
                  {
                    if( aV[13] == ':' && aV[16] == ':' )
                    {
                      if( aV.size() == 20 )
                      {
                        if( !(aV[19] == 'Z' || aV[19] == 'z' ) )
                          is=true;
                      }

                      if( ! hdhC::isDigit( aV.substr(0, 4) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(5, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(8, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(11, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(14, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(17, 2) ) )
                        is=true ;
                    }
                    else
                      is=true;
                  }
                  else
                    is=true;
              }
              else
                is=true;
            }
            else
              is=true;

            if( is )
            {
              std::string key("2_4");
              if( notes->inq( key, pQA->s_global) )
              {
                std::string capt(pQA->s_global);
                capt += hdhC::blank;
                capt += hdhC::tf_att(rqName);
                capt += "does not comply with YYYY-MM-DDThh:mm:ssZ, found: " ;
                capt += aV ;

                (void) notes->operate(capt) ;
                notes->setCheckMetaStr( pQA->fail );
              }
            }
          }

          else if( !hdhC::isAmong(aV, vs_rqValue) )
          {
            // there is a value. is it the requested one?
            std::string key("2_8");
            if( notes->inq( key, pQA->s_global ) )
            {
              std::string capt(pQA->s_global);
              capt += hdhC::blank;
              capt += hdhC::tf_att(hdhC::empty, rqName, aV);
              capt += "does not match requested value";
              capt += hdhC::tf_val(rqValue) ;

              (void) notes->operate(capt) ;
              notes->setCheckMetaStr( pQA->fail );

              continue;
            }
          }
       }
    }
  } // end of for-loop

  return;
}

void
CMOR::checkReqAtt_variable(Variable &var)
{
  // vName := name of an existing variable (already tested)
  // ix    := index of in.variable[ix]

/*
  DRS_CV_Table& drs = pQA->drs_cv_table;

  std::string &vName = var.name;

  // check required attributes
  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == vName )
      break;

  // find required attributes missing in the file
  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // find corresponding att_name of given vName in the file
    int jx=var.getAttIndex(rqName);

    // special: values may be available for cloud amounts

    // missing required attribute
    if( jx == -1 && rqName != "values" )
    {
       std::string key("2_1");

       if( notes->inq( key, vName) )
       {
         std::string capt(hdhC::tf_att(vName,rqName));
         capt += "is missing";

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr( pQA->fail );

         continue;
       }
    }

    std::string &aN = var.attName[jx] ;

    // required attribute expects a value (not for cloud amounts)
    if( rqValue.size() )  // a value is required
    {
      if( ! isCloudAmount && rqName == "values" )
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
           std::string capt(hdhC::tf_att(vName, aN, hdhC::colon));
           capt="missing required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }

       // there is a value. is it the required one?
       else if( aV != rqValue )
       {
         bool is=true;
         if( rqName == pQA->n_long_name)
         {
           // mismatch tolerated, because the table does not agree with CMIP5
           if( rqValue == "pressure" && aV == "pressure level" )
             is=false;
           if( rqValue == "pressure level" && aV == "pressure" )
             is=false;
         }
         else if( (rqName == pQA->n_positive) || (rqName == pQA->n_axis) )
         {
           // case insensitive
           if( rqValue == hdhC::Lower()(aV) )
             is=false;
         }

         std::string key("2_3");
         if( is &&  notes->inq( key, vName ) )
         {
           std::string capt(hdhC::tf_att(vName, aN, aV));
           capt += "does not match required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckMetaStr( pQA->fail );

           continue;
         }
       }
    }
  } // end of for-loop
*/

  return;
}

/*
void
CMOR::checkReqVariableType(void)
{
  // check variable type; uses DRS_CV_Table
  DRS_CV_Table& table = pQA->drs_cv_table ;

  for( size_t i=0 ; i < table.varName.size() ; ++i )
  {
    std::string& tName = table.varName[i];

    for( size_t j=0 ; j < table.attName[i].size() ; ++j )
    {
      std::string& tAttName = table.attName[i][j] ;

      if( tAttName == "VAR_TYPE" )
      {
        bool isA = false ;
        size_t v;
        for(v=0 ; v < pQA->pIn->varSz ; ++v )
        {
          Variable& var = pQA->pIn->variable[v] ;

          if( var.name == tName )
          {
            isA=true;
            break;
          }
        }

        if( v == pQA->pIn->varSz )
        {
          for(v=0 ; v < pQA->pIn->varSz ; ++v )
          {
            Variable& var = pQA->pIn->variable[v] ;

            if( var.isDATA && tName == "DATA_VAR" )
            {
              isA = true;
              break;
            }
            else if( var.isAUX && tName == "AUX_VAR" )
            {
              isA = true;
              break;
            }
          }
        }

        if( isA )
        {
          Variable& var = pQA->pIn->variable[v] ;
          std::string& tAttValue = table.attValue[i][j] ;

          std::string s(pQA->pIn->nc.getVarTypeStr(var.name));

          if( tAttValue != s )
          {
            std::string key("3_2");
            if( notes->inq( key, var.name) )
            {
              std::string capt("table=");
              capt += CMOR::tableSheet;
              capt += ", var=";
              capt += var.name ;
              capt += ": ";

              if( tName == "DATA_VAR" )
                capt = "data ";
              else if( tName == "AUX_VAR" )
                capt = "auxiliary ";

              capt += hdhC::tf_var(var.name);
              capt += "has wrong data type, found";
              capt += hdhC::tf_val(s);
              capt += ", expected";
              capt += hdhC::tf_val(tAttValue);

              (void) notes->operate(capt) ;
              notes->setCheckMetaStr( pQA->fail );
            }
          }
        }
      }
    }
  }

  return;
}
*/

void
CMOR::checkStringValues( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  std::vector<std::string> vs_values;

  pQA->pIn->nc.getData(vs_values, f_DMD.var->name);

  if( vs_values.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(vs_values.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  size_t i;
  for( i=0 ; i < vs_values.size() ; ++i )
    vs_values[i] = hdhC::stripSides(vs_values[i]) ;

  for( i=0 ; i < x_tVal.size() ; ++i )
    if( !hdhC::isAmong(x_tVal[i], vs_values) )
      break;

  if( i == x_tVal.size() )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "mismatch of data values between file and table, expected also";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
CMOR::checkVarReqTable(InFile& in, VariableMetaData& vMD,
             std::vector<struct DimensionMetaData>& vs_f_DMD_entries)
{
   // We have arrived here, because no project table
   // wasn't defined, yet. Or no entry was found.
   // So, we scan through the variable requirements table.

   // return true for the very special case that a tracer
   // variable was not found in table sheet Omon, because
   // it is defined in Oyr

   if( !pExp->varReqTable.is )  // no standard table
      return false;

   std::string str;

//   std::fstream ifs(str0.c_str(), std::ios::in);
   // This class provides the feature of putting back an entire line
   std::string str0;

   ReadLine ifs(pExp->varReqTable.getFile());

   if( ! ifs.isOpen() )
   {
      std::string key("7_4") ;

      if( notes->inq( key, vMD.var->name) )
      {
         std::string capt("could not open table.") ;
         capt += pExp->varReqTable.basename ;

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
      }
   }

   // headings for variables and dimensions
   std::map<std::string, size_t> v_col;
   std::map<std::string, size_t> d_col;

   size_t v_colMax, d_colMax;

   // read headings from the variable requirements table,
   // lines in the dimensions sub-sheet are stored in vs_dim_sheet_line
   std::vector<std::string> vs_dimSheet;

   readHeadline(ifs, vMD, vs_dimSheet, v_col, d_col, v_colMax, d_colMax);

   VariableMetaData tbl_entry(pQA);

   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // find the table sheet, corresponding to the 2nd item
   // in the variable name. The name of the table sheet is
   // present in the first column and begins with "CMOR Table".
   // The remainder is the name.
   // Unfortunately, the variable requirement table is in a shape of a little
   // distance from being perfect.

   bool isFound=false;

   while( ! ifs.getLine(str0) )
   {
     // try to identify the name of the table sheet in str0
     // return true, if not found
     if( findVarReqTableSheet(ifs, str0, vMD) )
       continue; // try next line

     // find entry for the requested variable
     if( findVarReqTableEntry(ifs, str0, vMD, v_col, v_colMax) )
       isFound=true;

     break;
   }

  // We have found an entry
   if( isFound )
   {
     x_line = str0;

     // This was tested in findVarReqTableSheet()
     tbl_entry.priority = x_line.toInt(v_col[n_priority]);
     tbl_entry.attMap[n_name_alt] = x_line[v_col[n_output_var_name]];

     tbl_entry.attMap[n_long_name] = hdhC::unique( x_line[v_col[n_long_name]] );
     if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
        tbl_entry.attMap[n_long_name] = hdhC::Lower()(tbl_entry.attMap[n_long_name]) ;

     tbl_entry.attMap[n_cell_methods] = x_line[v_col[n_cell_methods]];
     tbl_entry.attMap[n_cell_measures] = x_line[v_col[n_cell_measures]];
     tbl_entry.attMap[n_type] = x_line[v_col[n_type]];
     tbl_entry.var->name = x_line[v_col[n_CMOR_variable_name]];
     tbl_entry.var->std_name = x_line[v_col[n_standard_name]];
     tbl_entry.var->units
         = hdhC::unique( x_line[v_col[n_unformatted_units]] );
     if( tbl_entry.var->units.size() )
       tbl_entry.var->isUnitsDefined=true;
     else
       tbl_entry.var->isUnitsDefined=false;

     // check the dimensions of the variable from the table.
     Split x_tableVarDims(x_line[v_col[n_CMOR_dimension]]) ;
     for( size_t x=0 ; x < x_tableVarDims.size() ; ++x )
       tbl_entry.var->dimName.push_back(x_tableVarDims[x]);

     // special for items declared across table sheets
     if( CMOR::tableSheetAlt == "cfSites"
          || CMOR::tableSheetAlt == "cf3hr" )
       tbl_entry.attMap[n_cell_methods]="time: point";

     // netCDF properties are compared to those in the table.
     // Exit in case of any difference.
     checkVarReqTableEntry(vMD, tbl_entry);

     // get dimensions names from nc-file. If not a genuine dimensions, then
     // try the coordinates attribute for sclar variables.
     std::vector<std::string> vs_dims(vMD.var->dimName);
     Split x_coord(vMD.var->getAttValue(n_coordinates));

     for( size_t i=0 ; i < x_coord.size() ; ++i )
     {
       // only scalar variables
       if( ! hdhC::isAmong(x_coord[i], vMD.var->dimName ) )
       {
          // fault was trapped in CF
          if( in.nc.isVariableValid(x_coord[i]) )
            vs_dims.push_back(x_coord[i]) ;
       }
     }

     std::string dName;
     std::vector<std::string> vs_dName;

     for(size_t l=0 ; l < vs_dims.size() ; ++l)
     {
        bool is=false;
        for( size_t k=0 ; k < vs_dName.size() ; ++k )
        {
          if( vs_dName[k] == vs_dims[l] )
          {
             is=true;
             break;
          }
        }

       if(is)
         continue;  // label; already taken into account

       // new instance
       vs_f_DMD_entries.push_back( DimensionMetaData() );

       // the spot where meta-data of variables are taken
       dName = getDimMetaData(in, vMD, vs_f_DMD_entries.back(), vs_dims[l]) ;

       if( dName.size() )
         vs_dName.push_back(dName);
     }

     for(size_t l=0 ; l < x_tableVarDims.size() ; ++l)
       // check basic properties between the file and
       // requests in the table.
       checkDimSheet(vs_dimSheet, in, vMD, vs_f_DMD_entries,
         d_col, x_tableVarDims[l], d_colMax );

     return false;
   }

   // there was no match, but we try alternatives

   // Is it one of those providing an alternative?.
   if( CMOR::tableSheet == "Omon"
         || CMOR::tableSheet == "cf3hr"
            || CMOR::tableSheet == "cfSites" )
     return true;

   if( CMOR::tableSheetAlt.size() )
   {
     // switch back to the original settings to get it right
     // for issuing the warning below.
     CMOR::tableSheet = "Omon" ;
     CMOR::tableSheetAlt = "Oyr" ;
   }

    std::string key("3_1") ;
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroVar(
                          pExp->varReqTable.basename, vMD));
      capt += "not found in ";
      capt += hdhC::tf_assign("sheet",CMOR::tableSheet);

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
   }

   // No variable found in the standard table.
   // Build a project table from netCDF file properties.
   return false;
}

void
CMOR::checkVarReqTableEntry(
    VariableMetaData& vMD,
    VariableMetaData& tbl_entry)
{
  // Do the variable properties found in the netCDF file
  // match those in the table (var-requirements or project)?
  // Exceptions: priority is not defined in the netCDF file.
  std::string t0;

  std::string* currTable;
  std::string captsIntro("table=");
  if( CMOR::tableSheetAlt.size() )
    currTable = &CMOR::tableSheetAlt ;
  else
    currTable = &CMOR::tableSheet ;

  if( tbl_entry.attMap[n_long_name] != vMD.attMap[n_long_name] )
  {
    bool is=true;

    // special consideration for tracers in table sheet Omon
    if( CMOR::tableSheet == "Oyr" && CMOR::tableSheetAlt == "Omon" )
    {
      // the name in Omon is taken from Oyr + 'at surface'
      std::string t(tbl_entry.attMap[n_long_name]);
      t += " at surface";
      if( t == vMD.attMap[n_long_name] )
        is=false;
      else
      {
         t = tbl_entry.attMap[n_long_name];
         t += " at Surface";
         if( t == vMD.attMap[n_long_name] )
           // this for the correct spelling
           is=false;
      }
    }

    if( is )
    {
      std::string key("4_5");

      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroVar(*currTable, vMD, n_long_name));
        capt += "expected" ;

        if( tbl_entry.attMap[n_long_name + "_orig"].size() )
          capt += hdhC::tf_val(tbl_entry.attMap[n_long_name + "_orig"]) ;
        else
          capt += hdhC::tf_val(tbl_entry.attMap[n_long_name]) ;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }

  if( tbl_entry.attMap[n_cell_methods] != vMD.attMap[n_cell_methods] )
  {
    // strip anything within () from the cell-entry
    std::string tbl_cm( tbl_entry.attMap[n_cell_methods] ) ;
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
      if( vMD.attMap[n_cell_methods].find(splt[i]) == std::string::npos )
        is=true;

    if( is )
    {
       std::string key("4_7");

       if( notes->inq( key, vMD.var->name) )
       {
         std::string capt(QA_Exp::getCaptionIntroVar(*currTable, vMD, n_cell_methods));
         capt += "found" ;
         if( vMD.attMap[n_cell_methods].size() )
           capt += hdhC::tf_val(vMD.attMap[n_cell_methods]) ;
         else
           capt += hdhC::tf_val(pQA->notAvailable) ;

         capt += ", expected";
         if( tbl_entry.attMap[n_cell_methods].size() )
           capt += hdhC::tf_val(tbl_entry.attMap[n_cell_methods]) ;
         else
           capt += hdhC::tf_val(pQA->notAvailable) ;

         (void) notes->operate(capt) ;
         notes->setCheckMetaStr(pQA->fail);
         pQA->setExit( notes->getExitValue() ) ;
      }
    }
  }


  // the standard table has type==real. Is it for
  // float only, or also for double? So, in case of real,
  // any non-int type is accepted
  bool isTblTypeReal =
      tbl_entry.attMap[n_type] == "real"
         || tbl_entry.attMap[n_type] == "float"
             || tbl_entry.attMap[n_type] == "double" ;
  bool isNcTypeReal =
      vMD.attMap[n_type] == "real"
         || vMD.attMap[n_type] == "float"
              || vMD.attMap[n_type] == "double" ;

  if( tbl_entry.attMap[n_type].size() == 0 && vMD.attMap[n_type].size() != 0 )
  {
    std::string key("4_11");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroVar(*currTable, vMD, n_type));
      capt += "check discarded, not found in table " ;
      capt += hdhC::tf_assign(CMOR::tableSheet);

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }
  else if( (isTblTypeReal && ! isNcTypeReal)
            || ( ! isTblTypeReal && isNcTypeReal) )
  {
    std::string key("4_8");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroVar(*currTable, vMD, n_type));
      capt += "found" ;
      if( vMD.attMap[n_type].size() )
        capt += hdhC::tf_val(vMD.attMap[n_type]) ;
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      capt += ", expected";

      if( tbl_entry.attMap[n_type].size() )
        capt += hdhC::tf_assign("type", tbl_entry.attMap[n_type]) ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
  }

  return;
}

void
CMOR::checkWithTolerance( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name);

  if( ma.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(ma.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  double tolVal=0;
  if( t_DMD.attMap[n_tol_on_requests].size());
    tolVal = hdhC::string2Double(t_DMD.attMap[n_tol_on_requests]);

  if( !maxSz )
    maxSz = x_tVal.size();

  std::vector<double> tolMin;
  std::vector<double> tolMax;

  for( size_t i=0 ; i < maxSz ; ++i )
  {
    tolMin.push_back(ma[i] - tolVal);
    tolMax.push_back(ma[i] + tolVal);
  }

  // convert table values to double, which is always possible for CMIP5
  size_t i;
  for( i=0 ; i < maxSz ; ++i )
  {
    double tVal = hdhC::string2Double(x_tVal[i]) ;
    if( !( (tolMin[i] <= tVal) && (tVal <= tolMax[i]) ) )
      break;
  }

  if( i == maxSz )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "Mismatch of data values between file and table, found";
    capt += hdhC::tf_val(hdhC::double2String(ma[i]));
    capt += ", expected ";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

bool
CMOR::findVarReqTableEntry(ReadLine& ifs, std::string& str0,
   VariableMetaData& vMD,
   std::map<std::string, size_t>& col, size_t col_max)
{
   // return true: entry is not the one we look for.
   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   size_t colIx=col[CMOR::n_CMOR_variable_name];
   bool retState=false;

   do
   {
     // no entry
     if( str0.substr(0,10) == "CMOR Table" )
       return false;

     // Get the string for the MIP-sub-table corresponding to
     // time table entries read by parseTimeTable().
     if( str0.substr(0,13) == "In CMOR Table" )
       findVarReqTableSubSheet(str0, vMD) ;

     x_line=str0;
     if( x_line[colIx] == vMD.var->name )
     {
       retState=true;
       break;
     }

   } while( ! ifs.getLine(str0) ) ;

   // netCDF variable not found in the table
   ifs.close();

   return retState;  // we have found an entry and the shape matches.
}

bool
CMOR::findVarReqTableSheet(ReadLine& ifs, std::string& str0,
  VariableMetaData& vMD)
{
   // return true, if str0 contains no table name sheet
   size_t pos;
   Split x_col;

   do
   {

     if( str0.substr(0,10) == "CMOR Table" )
     {
        if( (pos=str0.find(',')) < std::string::npos )
        {
          x_col = str0.substr(0,pos) ;

          // The name of the table is the 3rd blank-separated item;
          // it may have an appended ':'
          if( x_col.size() > 2 )
            if( (pos=x_col[2].rfind(hdhC::colon)) < std::string::npos )
              str0=x_col[2].substr(0, pos);

          if( str0 == CMOR::tableSheet )
            return false;  // sub table found
        }
     }

   } while( ! ifs.getLine(str0) ) ;

   return true;  // not found; try next
}

void
CMOR::findVarReqTableSubSheet(std::string& str0, VariableMetaData& vMD)
{
   // get the string for the MIP-sub-table, i.e. table sub-sheet name.
   // This is used in conjunction to the time table (parseTimeTable()).
   size_t p;
   if( (p = str0.find(hdhC::colon)) < std::string::npos )
     // skip also a leading ' ' after the colon
     CMOR::tableSheetSub = str0.substr(p+2) ;

   return;
}

std::string
CMOR::getDimMetaData(InFile& in,
      VariableMetaData& vMD,
      struct DimensionMetaData& f_DMD,
      std::string dName)
{
  // return 0:
  // collect dimensional meta-data into a struct.

  // find the corresponding variable
  size_t i;
  for(i=0 ; i < pQA->pIn->varSz ; ++i )
  {
    Variable& var = pQA->pIn->variable[i];

    if( var.name == dName )
    {
      f_DMD.var = &var ;
      break;
    }

    // is dName associated to a label?
    if( var.isLabel )
    {
      if( var.dimName[0] == dName )
      {
        dName = var.name;
        f_DMD.var = &var ;
        break;
      }
    }
  }

  if( i == pQA->pIn->varSz )
  {
    std::string key("4_1");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt("table=");
      capt += CMOR::tableSheet + ", var=";
      capt += f_DMD.var->name + ", dim=";
      capt += dName ;
      capt += ": ";
      capt += hdhC::tf_val(dName, hdhC::blank);
      capt += "in the dims-sheet of ";
      capt += pExp->varReqTable.basename ;
      capt += " is missing in the file";

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }
    return "";  // nothing was found
  }

  // pre-set
  f_DMD.attMap[n_output_dim_name]=dName;
  f_DMD.attMap[n_coordinates]=dName;

  // get the var type
  f_DMD.attMap[n_type] = in.nc.getVarTypeStr(dName);

  // get the attributes

  for( size_t j=0 ; j < f_DMD.var->attName.size() ; ++j)
  {
    std::string& aName  = f_DMD.var->attName[j];
    std::string& aValue = f_DMD.var->attValue[j][0];

    if( aName == n_axis )
      f_DMD.attMap[n_axis] = aValue ;
    else if( aName == n_long_name )
    {
      f_DMD.attMap[n_long_name] = hdhC::unique(aValue) ;
      if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
        f_DMD.attMap[n_long_name] = hdhC::Lower()(f_DMD.attMap[n_long_name]) ;
    }
    else if( aName == n_standard_name )
      f_DMD.attMap[n_standard_name] = aValue ;
    else if( aName == n_units )
      f_DMD.attMap[n_units] = aValue ;
    else if( aName == n_positive )
      f_DMD.attMap[n_positive] = aValue ;
  }

  // determine the checksum of limited var-presentations of dim
  if( ! f_DMD.var->checksum && ! f_DMD.var->isUnlimited() )
  {
    if( f_DMD.var->type == NC_CHAR )
    {
      std::vector<std::string> vs;
      in.nc.getData(vs, dName);

      bool reset=true;  // is set to false during the first call
      for(size_t i=0 ; i < vs.size() ; ++i)
      {
        vs[i] = hdhC::stripSides(vs[i]);
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(vs[i], &reset) ;
      }
    }
    else
    {
      MtrxArr<double> mv;
      in.nc.getData(mv, dName);

      bool reset=true;
      for( size_t i=0 ; i < mv.size() ; ++i )
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(mv[i], &reset) ;
    }
  }

  return dName;
}

void
CMOR::initDefaults(void)
{

  return;
}

bool
CMOR::readHeadline(ReadLine& ifs,
   VariableMetaData& vMD,
   std::vector<std::string>& vs_dimSheet,
   std::map<std::string, size_t>& v_col,
   std::map<std::string, size_t>& d_col,
   size_t& v_colMax, size_t& d_colMax )
{
   // find the captions for table sheets dims and for the variables

   std::string str0;

   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // This should give the heading of the table sheet for the dimensions.
   // The table sheet is identified by the first column.
   while( !ifs.getLine(str0) )
   {
     if( str0.substr(0,13) == "CMOR table(s)" )
       break ;
   }

   x_line=str0;

   // identify columns of the Taylor table; look for the indexes
   for( d_colMax=0 ; d_colMax < x_line.size() ; ++d_colMax)
   {
      if( d_colMax == 0 )
        d_col[n_CMOR_tables] = d_colMax;
      else if( x_line[d_colMax] == "CMOR dimension" )
        d_col[n_CMOR_dimension] = d_colMax;
      else if( x_line[d_colMax] == "output dimension name" )
        d_col[n_output_dim_name] = d_colMax;
      else if( x_line[d_colMax] == "standard name" )
        d_col[n_standard_name] = d_colMax;
      else if( x_line[d_colMax] == "long name" )
        d_col[n_long_name] = d_colMax;
      else if( x_line[d_colMax] == n_axis )
        d_col[n_axis] = d_colMax;
      else if( x_line[d_colMax] == "index axis?" )
        d_col[n_index_axis] = d_colMax;
      else if( x_line[d_colMax] == n_units )
        d_col[n_units] = d_colMax;
      else if( x_line[d_colMax] == "coords_attrib" )
        d_col[n_coordinates] = d_colMax;
      else if( x_line[d_colMax] == "bounds?" )
        d_col[n_bounds_quest] = d_colMax;
      else if( x_line[d_colMax] == n_valid_min )
        d_col[n_valid_min] = d_colMax;
      else if( x_line[d_colMax] == n_valid_max )
        d_col[n_valid_max] = d_colMax;
      else if( x_line[d_colMax] == n_type )
        d_col[n_type] = d_colMax;
      else if( x_line[d_colMax] == n_positive )
        d_col[n_positive] = d_colMax;
      else if( x_line[d_colMax] == n_value )
        d_col[n_value] = d_colMax;
      else if( x_line[d_colMax] == n_requested )
        d_col[n_requested] = d_colMax;
      else if( hdhC::clearSpaces(x_line[d_colMax]) == n_bounds_values )
        d_col[n_bounds_values] = d_colMax;
      else if( hdhC::clearSpaces(x_line[d_colMax]) == n_bounds_requested )
        d_col[n_bounds_requested] = d_colMax;
   }

   // find the capt for variables
   while( ! ifs.getLine(str0) )
   {
     vs_dimSheet.push_back( str0 );

     if( str0.substr(0,8) == n_priority )
       break;
   } ;

   x_line=str0;

   // now, look for the indexes of the variable's columns
   for( v_colMax=0 ; v_colMax < x_line.size() ; ++v_colMax)
   {
     if( x_line[v_colMax] == "CMOR variable name" )
       v_col[n_CMOR_variable_name] = v_colMax;
     if( x_line[v_colMax] == "output variable name" )
       v_col[n_output_var_name] = v_colMax;
     else if( x_line[v_colMax] == n_priority )
       v_col[n_priority] = v_colMax;
     else if( x_line[v_colMax] == "standard name" )
       v_col[n_standard_name] = v_colMax;
     else if( x_line[v_colMax] == "long name" )
       v_col[n_long_name] = v_colMax;
     else if( x_line[v_colMax] == "unformatted units" )
       v_col[n_unformatted_units] = v_colMax;
     else if( x_line[v_colMax] == n_cell_methods )
       v_col[n_cell_methods] = v_colMax;
     else if( x_line[v_colMax] == n_cell_measures )
       v_col[n_cell_measures] = v_colMax;
     else if( x_line[v_colMax] == n_type )
       v_col[n_type] = v_colMax;
     else if( x_line[v_colMax] == "CMOR dimensions" )
       v_col[n_CMOR_dimension] = v_colMax;
     else if( x_line[v_colMax] == "valid min" )
       v_col[n_valid_min] = v_colMax;
      else if( x_line[v_colMax] == "valid max" )
       v_col[n_valid_max] = v_colMax;
   }

   return true;
}

void
CMOR::run(InFile& in, VariableMetaData& vMD)
{
  // Matching the ncfile inherent meta-data against pre-defined
  // CMOR tables.

//  checkReqVariableType();

  checkRequestedAttributes();

  // Meta data of variables from file or table are stored in struct varMeDa.
  // Similar for dimMeDa for the associated dimensions.

  std::vector<struct DimensionMetaData> vs_f_DMD_entries;

  // Scan through the standard output table, respectivels variable requests.
  // "Any" indicates that no valid table sheet was found
  if( CMOR::tableSheet != "Any" && ! vMD.var->isExcluded )
  {
    bool is;
    if( checkVarReqTable(in, vMD, vs_f_DMD_entries) )
    {
      // very special: a tracer variable was not found
      // in table sheet Omon, because it is defined in Oyr
      if( CMOR::tableSheet == "Omon" )
      {
        CMOR::tableSheetAlt = "Omon" ;
        CMOR::tableSheet = "Oyr" ;

       is = checkVarReqTable(in, vMD, vs_f_DMD_entries) ;

        // switch back to the original table required for the project table entry
        CMOR::tableSheet = "Omon" ;
        CMOR::tableSheetSub =  "Marine Bioge" ;
        CMOR::tableSheetAlt = "Oyr" ;
      }
      else if( CMOR::tableSheet == "cf3hr" )
      {
        CMOR::tableSheetAlt = "cf3hr" ;
        CMOR::tableSheet = "Amon" ;
        std::string saveCellMethods(vMD.attMap[n_cell_methods]);
        vMD.attMap[n_cell_methods]="time: point";

        is = checkVarReqTable(in, vMD, vs_f_DMD_entries) ;

        // switch back to the original table required for the project table entry
        CMOR::tableSheet = "cf3hr" ;
        CMOR::tableSheetSub.clear() ;
        CMOR::tableSheetAlt = "Amon" ;

        if( ! is )
          vMD.attMap[n_cell_methods]=saveCellMethods;
      }
      else if( CMOR::tableSheet == "cfSites" )
      {
        CMOR::tableSheetAlt = "cfSites" ;
        CMOR::tableSheet = "Amon" ;
        std::string saveCellMethods(vMD.attMap[n_cell_methods]);
        vMD.attMap[n_cell_methods]="time: point";

        is = checkVarReqTable(in, vMD, vs_f_DMD_entries) ;

        // switch back to the original table required for the project table entry
        CMOR::tableSheet = "cfSites" ;
        CMOR::tableSheetSub =  "CFMIP 3-ho" ;
        CMOR::tableSheetAlt = "Amon" ;

        if( ! is )
          vMD.attMap[n_cell_methods]=saveCellMethods;
      }
    }
  }

  return;
}


// class with project specific purpose
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

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);
       continue;
     }

     if( split[0] == "tVR"
          || split[0] == "table_variable_requirements" )
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
   if( varReqTable.path.size() == 0 )
      varReqTable.setPath(pQA->tablePath);

   return;
}

void
QA_Exp::checkDataVarNum(void)
{
  // each file must contain only a single output field from a single simulation
  if( pQA->pIn->dataVarIndex.size() == 1 )
    return;

  if( pQA->pIn->dataVarIndex.size() == 0 )
  {
    if( fVarname.size() )
    {
      // unfortunately, no data-variable was found previously
      for( size_t i=0 ; i < pQA->pIn->varSz ; ++i )
      {
        if( pQA->pIn->variable[i].name == fVarname )
        {
          // correction
          pQA->pIn->dataVarIndex.push_back(i);
          pQA->pIn->variable[i].isDATA=true;
          return;
        }
      }
    }

    std::string key("9_1b");
    if( notes->inq( key) )
    {
      std::string capt("no data variable present") ;

      (void) notes->operate(capt) ;
      notes->setCheckMetaStr(pQA->fail);
      pQA->setExit( notes->getExitValue() ) ;
    }

    return;
  }

  std::string key("9_1a");
  if( notes->inq( key) )
  {
    std::string capt("multiple variables, found: ") ;

    for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
    {
      if(i)
        capt += ", ";

      capt += pQA->pIn->variable[pQA->pIn->dataVarIndex[i]].name;
    }

    (void) notes->operate(capt) ;
    notes->setCheckMetaStr(pQA->fail);
    pQA->setExit( notes->getExitValue() ) ;
  }

  return;
}

void
QA_Exp::checkMetaData(InFile& in)
{
  notes->setCheckMetaStr("PASS");

  // is it NetCDF-3 classic?
  checkNetCDF(in);

  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  CMOR cmor(pQA);

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    if( varMeDa[i].var->isDATA )
    {
      cmor.run(in, varMeDa[i] );
    }
  }

  // eventually
  bool is=true;
  for( size_t i=0 ; i < in.variable.size() ; ++i )
  {
     if( in.variable[i].name == fVarname )
     {
        is=false;
        break;
     }
  }

  if( is )
  {
     std::string key("1_3");

     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("variable name");
       capt += hdhC::tf_val(fVarname, hdhC::blank);
       capt += "in the filename does not match any variable in the file." ;

       (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
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
      std::string capt("format does not conform to netCDF classic, found") ;
      capt += hdhC::tf_val(s);

      (void) notes->operate( capt) ;
      notes->setCheckMetaStr(pQA->fail);
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

  //there should be only present a single data variable
  checkDataVarNum();

  for( size_t i=0 ; i< pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    pushBackVarMeDa( &var );  //push next instance
  }

  if( pQA->pIn->dataVarIndex.size() > 1 )
  {
     std::string key("9_1");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("multiple data variables are present, found ");

       for( size_t j=0 ; j < pQA->pIn->dataVarIndex.size() ; ++j )
       {
         Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[j]];
         if(j)
           capt += ", ";

         capt += var.name;
       }

       (void) notes->operate(capt) ;
       notes->setCheckMetaStr( pQA->fail );
     }
  }

  return;
}

std::string
QA_Exp::getCaptionIntroDim(
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD, std::string att)
{
  std::string& t_DMD_outname = t_DMD.attMap[CMOR::n_output_dim_name] ;

  std::string intro("table=");
  intro += CMOR::tableSheet + ", var(file)=";
  intro += f_DMD.var->name + ", CMOR-dim=";
  intro += t_DMD.attMap[CMOR::n_CMOR_dimension] ;

  if( t_DMD_outname == "basin" )
    intro += ", region";
  if( t_DMD_outname == "line" )
    intro += ", passage";
  if( t_DMD_outname == CMOR::n_type )
    intro += ", type_description";

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
}

std::string
QA_Exp::getCaptionIntroVar( std::string table,
    struct VariableMetaData& f_VMD, std::string att)
{
  std::string intro("table=");
  intro += table + ", var=";
  intro += f_VMD.var->name ;

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
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
  if( CMOR::tableSheet.size() )
    return CMOR::tableSheet;

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
      std::string key("7_7");

      if( notes->inq( key, "global") )
      {
        std::string capt("invalid MIP table name in global attribute, found ") ;
        capt += hdhC::tf_att(hdhC::empty, "table_id", x_tableID.getStr()) ;

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
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
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
         notes->setCheckMetaStr( pQA->fail );
         pQA->setExit( notes->getExitValue() ) ;
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
     std::string key("7_12");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("no path to a table, tried ") ;
        capt += varReqTable.path;

        (void) notes->operate(capt) ;
        notes->setCheckMetaStr(pQA->fail);
        pQA->setExit( notes->getExitValue() ) ;
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
      vMD.var->std_name = vMD.var->getAttValue(CMOR::n_standard_name);

      std::string s( vMD.var->getAttValue(CMOR::n_long_name));
      vMD.attMap[CMOR::n_long_name] = hdhC::unique(s);
      if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
        vMD.attMap[CMOR::n_long_name] = hdhC::Lower()(vMD.attMap[CMOR::n_long_name]) ;

      vMD.attMap[CMOR::n_cell_methods] = vMD.var->getAttValue(CMOR::n_cell_methods);
      vMD.attMap[CMOR::n_cell_measures] = vMD.var->getAttValue(CMOR::n_cell_measures);
      vMD.attMap[CMOR::n_type] = pQA->pIn->nc.getVarTypeStr(var->name);
   }

   return;
}

void
QA_Exp::run(std::vector<std::string>& optStr)
{
  // apply parsed command-line args
  applyOptions(optStr);

  fVarname = getVarnameFromFilename(pQA->pIn->file.filename);
  getFrequency();

  // Create and set VarMetaData objects.
  createVarMetaData() ;

  if( inqTables() )
  {
    CMOR::tableSheet = getMIP_tableName() ;

    DRS_CV drsFN(pQA);
    drsFN.run();

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

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;

   if( v )
   {
     var = v;
     isNewVar=false;
   }
   else
   {
     var = new Variable ;
     isNewVar=true;
   }
}

VariableMetaData::~VariableMetaData()
{
  if( isNewVar )
    delete var;

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
   notes = new Annotation(n);

   // this is not a mistaken
   qaData.setAnnotation(n);

   return;
}

void
VariableMetaData::setAnnotation(Annotation *n)
{
   notes = n;

   qaData.setAnnotation(n);

   return;
}
