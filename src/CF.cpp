CF::CF()
{
  initDefaults();
}

void
CF::applyOptions(void)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split.size() == 0 )
       continue;  // this is a fault

     if( split[0] == "fR"
          || split[0] == "follow_recommendations" )
     {
        followRecommendations=true;
        continue;
     }

     if( split[0] == "cF" )
     {
       if( split.size() == 2 )
       {
          cFVersion=split[1] ;
          continue;
       }
     }

     if( split[0] == "cFAT"
          || split[0] == "area_table" )
     {
       if( split.size() == 2 )
       {
          size_t p;
          if( (p=split[1].rfind('/')) < std::string::npos )
          {
             tablePath = split[1].substr(0, p);
             std_name_table = split[1].substr(p+1);
          }
          else
            area_table=split[1];
          continue;
       }
     }

     if( split[0] == "cFSN"
          || split[0] == "standard_name_table" )
     {
       if( split.size() == 2 )
       {
          size_t p;
          if( (p=split[1].rfind('/')) < std::string::npos )
          {
             tablePath = split[1].substr(0, p);
             std_name_table = split[1].substr(p+1);
          }
          else
            std_name_table=split[1];
          continue;
       }
     }

     if( split[0] == "cFSRN"
          || split[0] == "region_table" )
     {
       if( split.size() == 2 )
       {
          size_t p;
          if( (p=split[1].rfind('/')) < std::string::npos )
          {
             tablePath = split[1].substr(0, p);
             std_name_table = split[1].substr(p+1);
          }
          else
            region_table=split[1];
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
   }

   return;
}

void
CF::attributeSpellCheck(void)
{
  double editDistance, eDMin;
  size_t eDMin_ix;

  // test for some typos; also for global attributes
  for( size_t i=0 ; i < pIn->variable.size() ; ++i )
  {
    Variable& var = pIn->variable[i];

    for( size_t j=0 ; j < var.attName.size() ; ++j)
    {
      // would accept same names when case is ignored; this is trapped elsewhere.
      std::string t(hdhC::Lower()(var.attName[j])) ;
      bool is=false;
      for( size_t jj=0 ; jj < var.attName.size() ; ++jj )
      {
        if( j != jj )
        {
          if( t == hdhC::Lower()(var.attName[jj]) )
          {
            is=true;
            break;
          }
        }
      }
      if( is )
        continue;

      size_t k;
      eDMin=100.;
      for( k=0 ; k < CF::attName.size() ; ++k)
      {
        std::string s=CF::attName[k];
        editDistance = wagnerFischerAlgo(var.attName[j], CF::attName[k]);

        if( editDistance < eDMin )
        {
           eDMin = editDistance ;
           eDMin_ix = k;
        }

        if( editDistance == 0 )
          break;
      }

      if( eDMin < 0.5 && ! var.isValidAtt(CF::attName[eDMin_ix]) )
      {
        if( notes->inq(bKey + "0h", var.name) )
        {
          std::string capt("is " ) ;
          if( var.name == n_NC_GLOBAL )
          {
            capt += "global " + n_attribute ;
            capt += captVal(var.attName[j]) ;
          }
          else
            capt += captAtt(var.name,var.attName[j], no_colon) ;
          capt += "misspelled? Did you mean " ;
          capt += CF::attName[eDMin_ix] + "?";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        // correction for internal use
        std::map<std::string, int>:: iterator it
            = var.attNameMap.find(var.attName[j]);

        var.attName[j] = CF::attName[eDMin_ix] ;

        if (it != var.attNameMap.end())
        {
          int value = it->second;
          var.attNameMap.erase(it);
          var.attNameMap[var.attName[j]] = value;
        }

      }

    }
  }

  return;
}

std::string
CF::captAtt(std::string v, std::string a,
    std::string& m1, std::string& m2, std::string& m3)
{
  bool isColon=true;
  bool isBlank=true;
  bool isUpper=false;

  std::vector<std::string*> ps;
  ps.push_back(&a);
  ps.push_back(&m1);
  ps.push_back(&m2);
  ps.push_back(&m3);

  int count=0;

  for( size_t i=0 ; i < ps.size() ; ++i )
  {
    if( *ps[i] == no_colon )
    {
      ++count;
      isColon=false;
    }
    else if( *ps[i] == no_blank )
    {
      ++count;
      isBlank=false;
    }
    else if( *ps[i] == s_upper )
    {
      ++count;
      isUpper=true;
    }
  }

  if( count == 4 )
    return captAtt(s_empty, v, isColon, isBlank, isUpper );

  return captAtt(v, a, isColon, isBlank, isUpper );
}

std::string
CF::captAtt(std::string v, std::string a, std::string& m1, std::string& m2)
{
  bool isColon=true;
  bool isBlank=true;
  bool isUpper=false;

  std::vector<std::string*> ps;
  ps.push_back(&a);
  ps.push_back(&m1);
  ps.push_back(&m2);

  int count=0;

  for( size_t i=0 ; i < ps.size() ; ++i )
  {
    if( *ps[i] == no_colon )
    {
      ++count;
      isColon=false;
    }
    else if( *ps[i] == no_blank )
    {
      ++count;
      isBlank=false;
    }
    else if( *ps[i] == s_upper )
    {
      ++count;
      isUpper=true;
    }
  }

  if( count == 3 )
    return captAtt(s_empty, v, isColon, isBlank, isUpper );

  return captAtt(v, a, isColon, isBlank, isUpper );
}

std::string
CF::captAtt(std::string v, std::string a, std::string& m1)
{
  bool isColon=true;
  bool isBlank=true;
  bool isUpper=false;

  std::vector<std::string*> ps;
  ps.push_back(&a);
  ps.push_back(&m1);

  int count=0;

  for( size_t i=0 ; i < ps.size() ; ++i )
  {
    if( *ps[i] == no_colon )
    {
      ++count;
      isColon=false;
    }
    else if( *ps[i] == no_blank )
    {
      ++count;
      isBlank=false;
    }
    else if( *ps[i] == s_upper )
    {
      ++count;
      isUpper=true;
    }
  }

  if( count == 2 )
    return captAtt(s_empty, v, isColon, isBlank, isUpper );

  return captAtt(v, a, isColon, isBlank, isUpper );
}

std::string
CF::captAtt(std::string v, std::string a)
{
  bool isColon=true;
  bool isBlank=true;
  bool isUpper=false;

  if( a == no_colon )
     return captAtt(s_empty, v, false, isBlank, isUpper );
  else if( a == no_blank )
     return captAtt(s_empty, v, isColon, false, isUpper );
  else if( a == s_upper )
     return captAtt(s_empty, v, isColon, isBlank, true );

  return captAtt(v, a, isColon, isBlank, isUpper );
}

std::string
CF::captAtt(std::string a)
{
  return n_attribute + blank + a ;
}

std::string
CF::captAtt(std::string& v, std::string& a, bool isColon, bool isBlank, bool isUpper)
{
  std::string s;

  if( isUpper )
  {
    if( v.size() )
      s = "Attribute <" + v + ">:" + a;
    else
      s = "Attribute " + a;
  }
  else
  {
    if( v.size() )
      s = n_attribute + " <" + v + ">:" + a ;
    else
      s = n_attribute + blank + a ;
  }

  if(isColon)
    s += ":";

  if(isBlank)
    s += " ";

  return s ;
}

std::string
CF::captVal(std::string v, bool isTrailingBlank)
{
  std::string s(" <" + v + ">" );
  if(isTrailingBlank)
    s += " ";

  return s ;
}

std::string
CF::captVar(std::string v, std::string& m1, std::string& m2)
{
  bool isColon=true;
  bool isBlank=true;

  if( m1 == no_colon || m2 == no_colon )
     isColon=false;
  if( m1 == no_blank || m2 == no_blank )
     isBlank=false;

  return captVar(v, isColon, isBlank );
}

std::string
CF::captVar(std::string v, std::string& m1)
{
  bool isColon=true;
  bool isBlank=true;

  if( m1 == no_colon )
     isColon=false;
  if( m1 == no_blank )
     isBlank=false;

  return captVar(v, isColon, isBlank );
}

std::string
CF::captVar(std::string v, bool isColon, bool isBlank)
{
  std::string s( n_variable + " <" + v + ">" );
  if(isColon)
    s += ":";

  if(isBlank)
    s += " ";

  return s ;
}

void
CF::checkCoordinateFillValueAtt(Variable& var)
{
   bool isF = var.isValidAtt(n_FillValue) ? true : false ;
   bool isM = var.isValidAtt(n_missing_value) ? true : false ;

   if( isF || isM )
   {
      if( notes->inq(bKey + "12c", var.name) )
      {
        std::string capt("coordinate " + captVar(var.name, false));
        capt += "should not have " ;
        if( isF && isM)
        {
          capt += n_attribute + "s" ;
          capt += captVal(n_FillValue)  + "and" + captVal(n_missing_value) ;
        }
        else if( isF )
          capt += captAtt(n_FillValue, no_colon) ;
        else
          capt += captAtt(n_missing_value, no_colon) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
   }

   return;
}

void
CF::checkCoordinateValues(Variable& var, bool isFormTermAux)
{
  // checks only meta-data declarations; _FV values in the data section
  // are tested below. Chap9 features are tested elsewhere
  if( cFVal < 16 && var.coord.isCoordVar )
    checkCoordinateFillValueAtt(var) ;

  // no check of an unlimited coord var for efficiency
//  if( var.isUnlimited() )
//     return;

  if( var.isNoData )
  {
    if( notes->inq(bKey + "0d", var.name) )
    {
      std::string capt ;
      if( !var.coord.isCoordVar )
        capt = "auxiliary " ;

      capt += "coordinate " + captVar(var.name);
      capt += "No data" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return;
  }

  signed char x_sc=0;
  char x_c=0;
  short x_s=0;
  int x_i=0;
  float x_f=0.;
  double x_d=0.;
  unsigned char x_uc=0;
  unsigned short x_us=0;
  unsigned int x_ui=0;
  long long x_ll=0;
  unsigned long long x_ull=0;

  if( var.type == NC_BYTE )
    checkCoordinateValues(var, isFormTermAux, x_sc) ;
  else if( var.type == NC_CHAR )
    checkCoordinateValues(var, isFormTermAux, x_c) ;
  else if( var.type == NC_SHORT )
    checkCoordinateValues(var, isFormTermAux, x_s) ;
  else if( var.type == NC_INT )
    checkCoordinateValues(var, isFormTermAux, x_i) ;
  else if( var.type == NC_FLOAT )
    checkCoordinateValues(var, isFormTermAux, x_f) ;
  else if( var.type == NC_DOUBLE )
    checkCoordinateValues(var, isFormTermAux, x_d) ;
  else if( var.type == NC_UBYTE )
    checkCoordinateValues(var, isFormTermAux, x_uc) ;
  else if( var.type == NC_USHORT )
    checkCoordinateValues(var, isFormTermAux, x_us) ;
  else if( var.type == NC_UINT )
    checkCoordinateValues(var, isFormTermAux, x_ui) ;
  else if( var.type == NC_INT64 )
    checkCoordinateValues(var, isFormTermAux, x_ll) ;
  else if( var.type == NC_UINT64 )
    checkCoordinateValues(var, isFormTermAux, x_ull) ;

  return;
}

template <typename T>
void
CF::checkCoordinateValues(Variable& var, bool isFormTermAux, T x)
{
  // all coordinate variables must be strictly monotonic

  // Exceptions:
  // variables named by a formula_terms attribute
  // and auxiliary coordinate variables
  bool testMonotony=true;
  if( !var.coord.isCoordVar || isFormTermAux )
    testMonotony=false;

  size_t num ;
  MtrxArr<T> mv;

  if( var.isUnlimited() )
  {
    if( (num = pIn->nc.getNumOfRecords()) )
      pIn->nc.getData(mv, var.name, 0, num );
  }
  else
    pIn->nc.getData(mv, var.name );

  if( mv.validRangeBegin.size() )
  {
    // note: [mv.vRB, mv.vRE[:  first contiguous stride without _FV
    // take into account that data may begin/end with _FV
    int i=-1;
    // a single stride
    if(mv.validRangeBegin[0] > 0)
      i=0;
    else if(mv.validRangeEnd[0] < mv.size())
      i=mv.validRangeEnd[0];

    if( i > -1 )
    {
      std::string capt0;
      if( var.isCoordinate() )
      {
        if( !var.coord.isCoordVar )
          capt0 = "auxiliary " ;

        capt0 = "coordinate " ;
      }
      else if( isFormTermAux )
        capt0 += "Formula terms " ;

      capt0 += captVar(var.name) + "Data should not " ;

      std::vector<size_t> vs_ix ;

      // Inf or NaN
      if( mv.valExcp->countInf || mv.valExcp->countNaN )
      {
        std::string capt;

        if( notes->inq(bKey + "12a", var.name) )
        {
          if( mv.valExcp->countInf )
          {
            capt += "be <Inf> found at index " ;
            capt += mv.indicesStr(mv.valExcp->firstInf_ix) ;
          }

          if( mv.valExcp->countNaN )
          {
            if( mv.valExcp->countInf )
              capt += ", ";
            capt += "<NaN> found at index " ;
            capt += mv.indicesStr(mv.valExcp->firstNaN_ix) ;
          }

          (void) notes->operate(capt0 + capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      // any kind of missing value
      if( mv.valExcp->exceptionCount.size() )
      {
        std::string capt ;

        int ec=0;
        for(size_t e=0 ; e < mv.valExcp->exceptionCount.size() ; ++e)
        {
          if( mv.valExcp->exceptionCount[e] )
          {
            if(ec++)
              capt += ", ";

            if( mv.valExcp->exceptionMode[e] == '=')
            {
              if( var.isValidAtt(n_FillValue) )
              {
                capt += "be ";
                capt += hdhC::sAssign(n_FillValue,
                                      var.getAttValue(n_FillValue)) ;
                capt += " found" ;
              }
              else if( var.isValidAtt(n_missing_value) )
              {
                capt += "be " ;
                capt += hdhC::sAssign(n_missing_value,
                                      var.getAttValue(n_missing_value)) ;
                capt += " found" ;
              }
              else
              {
                capt += "be default " ;
                capt += hdhC::sAssign(n_FillValue,
                                      hdhC::double2String(mv[i])) ;
                capt += " found" ;
              }
            }

            else if( mv.valExcp->exceptionMode[e] == '<')
            {
              capt += "fall below " ;
              capt += hdhC::sAssign(n_valid_min, var.getAttValue(n_valid_min)) ;
              capt += " found" ;
            }
            else if( mv.valExcp->exceptionMode[e] == '>')
            {
              capt += "exceed " ;
              capt += hdhC::sAssign(n_valid_max, var.getAttValue(n_valid_max)) ;
              capt += " found" ;
            }
            else if( mv.valExcp->exceptionMode[e] == 'R')
            {
              capt += "be out of " + n_valid_range ;
              int j = var.getAttIndex(n_valid_range);
              capt += "=<"+ var.attValue[j][0] ;
              capt += ", " + var.attValue[j][1] + ">" ;
              capt += " found value" ;
            }
          }
        }

        if( ec && notes->inq(bKey + "12d", var.name) )
        {
          capt += " at index " + mv.indicesStr(i) ;

          (void) notes->operate(capt0+capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      return;
    }
  }

  if( !testMonotony || mv.size() < 2 )
    return;

  if( notes->findAnnotation(bKey + "12d", var.name) )
    return; // test below would fail anyway

  bool isMonotone=true;
  bool isDifferent=true;

  size_t i=0;
  size_t mon_ix=0;
  size_t dif_ix=0;

  // more than a single value
  if( var.dimName.size() > 1 )
    return;

  double sig;
  for( size_t j=0 ; j < mv.validRangeBegin.size() ; ++j )
  {
    // start at [1], because of the difference below
    i=mv.validRangeBegin[j] + 1 ;

    if( !j )
      sig=(mv[i]-mv[i-1]) / fabs(mv[i]-mv[i-1]);

    for( ; i < mv.validRangeEnd[j] ; ++i)
    {
      double v0=sig*mv[i-1];
      double v1=sig*mv[i];

      if( v1 > v0 )
        continue;

      if( v1 < v0 )
      {
        if(!mon_ix)
          mon_ix = i;

        isMonotone=false;
      }
      else if( v1 == v0 )
      {
        if(!dif_ix)
          dif_ix = i;

        isDifferent=false;
      }
    }
  }

  if( ! isMonotone )
  {
    i = mon_ix;

    if( notes->inq(bKey + "12b", var.name) )
    {
      // no considering about matrix indexing
      std::string capt("coordinate " + captVar(var.name, false));
      capt += "has to be strictly monotonic around index " ;
      capt += mv.indicesStr(i) + ", found values" ;

      if(i>1)
        capt += captVal(hdhC::double2String(mv[i-2]), false);

      capt += ", " + captVal(hdhC::double2String(mv[i-1]), false);

      if( i < mv.size() )
        capt += "," + captVal(hdhC::double2String(mv[i]));

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  else if( ! isDifferent && notes->inq(bKey + "12b", var.name) )
  {
    std::vector<size_t> vs_ix ;
    std::string capt("coordinate " + captVar(var.name, false));
    capt += "has to be strictly monotonic at indices " ;

    capt += mv.indicesStr(dif_ix-1) ;
    capt += " and " ;
    capt += mv.indicesStr(dif_ix) ;
    capt += ", found equal values" ;

    capt += captVal(hdhC::double2String(mv[dif_ix-1]), false);
    capt += "," + captVal(hdhC::double2String(mv[dif_ix]), false);

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  return;
}

void
CF::checkGroupRelation(void)
{
  if( pIn->varSz == 0 )  // a file only with global attributes
     return;

  std::vector<std::string>& aG = associatedGroups;

  if( aG.size() )
  {
    for( size_t i=0 ; i < pIn->varSz ; ++i )
    {
      Variable& var = pIn->variable[i];

      if( var.dimName.size() == 1 && hdhC::isAmong(var.dimName[0], aG) )
        var.addAuxCount();
    }
  }

  // find unrelated scalar variables
  if( pIn->varSz > 1 )
  {
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    if( var.isCoordinate() )
    {
      bool is=true;

      if( associatedGroups.size() && hdhC::isAmong(var.name, associatedGroups) )
        is=false;

      if( is && var.boundsOf.size() )
        is=false;

      if( is )
      {
        // is it specified in a coordinates attribute?
        for( size_t p=0 ; p < ca_vvs.size() ; ++p )
        {
          if( hdhC::isAmong( var.name, ca_vvs[p]) )
          {
            is=false;
            break;
          }
        }

        // is it specified in a dimList?
        if( is )
        {
          for( size_t p=0 ; p < pIn->varSz ; ++p )
          {
            if(i==p)
              continue;

            Variable& var_p = pIn->variable[p] ;

            if( var_p.dimName.size() && hdhC::isAmong( var.name, var_p.dimName) )
            {
              is=false;
              break;
            }
          }
        }

        // isXYZinCoordAtt checks a rule not described by CF, but it is practice.
        // If all spatial coordinates are specified
        // by the coordinate attribute, then the label may be
        // omitted in a coordinates attribute.
        if( is )
        {
          if( isXYZinCoordAtt() )
            break; // only once for all scalars
          else if( ! isCompressAux(var) )
          {
            if( notes->inq(bKey + "0c") )
            {
              std::string capt("warning: " );
              if( var.isScalar )
                capt += "Scalar c";
              else
                capt += "C";

              capt += "oordinate " ;
              capt += captVar(var.name, false) ;
              capt += "is not related to any other " + n_variable ;

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }
      }
    }
  }
  }

  // dimension not in use
  for( size_t i=0 ; i < dimensions.size() ; ++i )
  {
    bool is=true;

    for( size_t j=0 ; j < pIn->varSz ; ++j )
    {
      Variable& var = pIn->variable[j];

      if( hdhC::isAmong(dimensions[i], var.dimName ) )
      {
        is=false;
        break;
      }
    }

    if( is )
    {
      for( size_t j=0 ; j < pIn->varSz ; ++j )
      {
        if( pIn->variable[j].isValidAtt(n_compress, dimensions[i]))
        {
          is=false;
          break;
        }
      }
    }

    if( is )
    {
      // is a dimension specified by any coordinate attribute
      for( size_t j=0 ; j < pIn->varSz ; ++j )
      {
        if( pIn->variable[j].isValidAtt(n_coordinates, dimensions[i]))
        {
          is=false;
          break;
        }
      }
    }

    if( is && notes->inq(bKey + "0b") )
    {
      std::string capt("warning: Unused " + n_dimension + captVal(dimensions[i])) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
}

void
CF::checkSN_Modifier(Variable& var)
{
  // Is it a valid modifier?

  std::vector<std::string> valid_mod;
  valid_mod.push_back("detection_minimum");
  valid_mod.push_back(n_number_of_observations);
  valid_mod.push_back("standard_error");
  valid_mod.push_back("status_flag");

  if( var.snTableEntry.remainder.size() == 0
        && !var.snTableEntry.hasBlanks )
     return;

  Split x_remainder( var.snTableEntry.remainder );

  int countValidMod=0;

  size_t sz(x_remainder.size()) ;

  for( size_t i=0 ; i < sz ; ++i )
    if( hdhC::isAmong(x_remainder[i], valid_mod) )
      ++countValidMod ;

  if( countValidMod )
  {
    if( x_remainder.size() == 1 && ! var.snTableEntry.hasBlanks )
      return; // fine

    if( countValidMod > 1 && sz > 1 )
    {
      if( notes->inq(bKey + "33c", var.name) )
      {
        std::string capt(captAtt(var.name,
                hdhC::sAssign(n_standard_name,var.std_name),no_colon)) ;
        capt += "with too many modifiers" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( (countValidMod == 0 || var.snTableEntry.hasBlanks)
          && notes->inq(bKey + "33b", var.name) )
  {
    std::string capt(captAtt(var.name,
                  hdhC::sAssign(n_standard_name, var.std_name), no_colon));
    capt += "contains " ;
    if( countValidMod == 0 && var.snTableEntry.remainder.size()
            && var.snTableEntry.hasBlanks )
      capt += "blanks or undefined modifier";
    else if ( var.snTableEntry.hasBlanks )
      capt += "blanks";
    else
      capt += "undefined modifier";

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  return ;
}

bool
CF::cmpUnits( std::string s, std::string ref)
{
//  if( s == ref )
//    return true;

  ut_unit* from = ut_parse(unitSystem, s.c_str(), UT_ASCII);
  bool is=false;

  if( from )
  {
    ut_unit* to = ut_parse(unitSystem, ref.c_str(), UT_ASCII);

    if( to )
    {
      cv_converter* converter = ut_get_converter(from, to);

      if( converter )
      {
        cv_free(converter);
        is=true;
      }
    }

    if(to == from)
      from=0;

    if(to)
      ut_free(to);
  }

  if(from)
    ut_free(from);

  return is;
}

bool
CF::entry(void)
{
  // get properties from the file according to CF-1.x chapters
  chap();

  // find groups of related variables
  checkGroupRelation();

  // coordinates, axis, positive
  finalAtt() ;

  findAmbiguousCoords();

  if( isCheck )
  {
    if(followRecommendations )
      chap_reco();
  }

  // a few post-check considerations about combinations of annotations
  postAnnotations();

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( var.isDataVar() )
      var.isDATA = true;
    else
      var.isAUX = true;
  }

  return true;
}

void
CF::finalAtt(void)
{
   final_dataVar();

   // check requirements of axes
   finalAtt_axis();
   finalAtt_coordinates();
   finalAtt_positive();

   return;
}

void
CF::finalAtt_axis(void)
{
  // CF-1.4: only coordinate vars may have attached an axis attribute.
  // CF-1.6: aux coord vars AND coordinate vars may have attached an axis attribute.
  // CF-1.6: A data variable may have both coordinate and auxiliary coordinate variables,
  //         but only one of them may have an axis attribute.
  //         However, it is recommended that one of them has an axis attribute.

  // the checks start from a data variables point of view
  for( size_t j=0 ; j < pIn->varSz ; ++j )
  {
    Variable& var = pIn->variable[j] ;

    if( var.isValidAtt(n_axis) )
    {
      // legal values of axis are T,Z,Y,X (case insensitive)
      std::string axis( var.getAttValue(n_axis, lowerCase) );

      if( ! (axis == "x" || axis == "y" || axis == "z" || axis == "t")  )
      {
        if( notes->inq(bKey + "4a", var.name) )
        {
          std::string capt(captVar(var.name));
          capt += "Axis is not X, Y, Z, or T (case insensitive), found" ;
          capt += captVal(var.getAttValue(n_axis));

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      if( var.isDataVar() )
      {
        if( notes->inq(bKey + "5f", var.name) )
        {
          std::string capt("data " + captVar(var.name, false));
          capt += "should not have " + captAtt(n_axis);

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
      else if( var.coord.isCoordVar || cFVal > 15 )
      {
        if( axis == "x" )
            ++var.coord.indication_X;
        else if( axis == "y" )
          ++var.coord.indication_Y;
        else if( axis == "z" )
          ++var.coord.indication_Z;
        else if( axis == "t" )
          ++var.coord.indication_T;

        // axis must be consistent with the coordinate variable
        std::string capt("Found ");

        if( axis != "x" && var.coord.isX )
          capt +=  hdhC::sAssign(n_axis, var.getAttValue(n_axis))
                    + ", expected X";
        else if( axis != "y" && var.coord.isY )
          capt += hdhC::sAssign(n_axis, var.getAttValue(n_axis))
                    + ", expected Y";
        else if( axis != "z" && var.coord.isZ )
          capt += hdhC::sAssign(n_axis, var.getAttValue(n_axis))
                    + ", expected Z";
        else if( axis != "t" && var.coord.isT )
          capt += hdhC::sAssign(n_axis, var.getAttValue(n_axis))
                    + ", expected T";

        if( capt.size() > 7 )
        {
          if( notes->inq(bKey + "4b", var.name) )
          {
            (void) notes->operate(captVar(var.name) + capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  char xs[]={'X', 'Y', 'Z', 'T'};

  // relations between data variables and associated coordinate variables
  // as well as auxiliary coordinate variables.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( !var.isDataVar() || var.coord.isCoordVar || var.isCoordinate() )
       continue;

    // collection of indices of all var-reps of the current variable's dims
    // and of existing variables declared in the coordinates attribute.
    std::vector<int> coll_ix;
    std::vector<int> coll_dim_ix;
    std::vector<int> coll_ix_all;

    // dim-list
    int ix;
    for( size_t l=0 ; l < var.dimName.size() ; ++l )
    {
       if( (ix=pIn->getVarIndex(var.dimName[l])) > -1 )
       {
          coll_dim_ix.push_back(ix);
          coll_ix_all.push_back(ix);
       }
    }

    // coordinates attribute list
    int jx;
    if( (jx=ca_ix[i]) > -1 )
    {
       for(size_t l=0 ; l < ca_vvs[jx].size() ; ++l )
         if( (ix=pIn->getVarIndex(ca_vvs[jx][l])) > -1 )
           coll_ix_all.push_back( ix );
    }

    // unique
    for(size_t c=0 ; c < coll_ix_all.size() ; ++c )
    {
      if( ! hdhC::isAmong(coll_ix_all[c], coll_ix) )
        coll_ix.push_back( coll_ix_all[c] );
    }

    // take into account the four axis
    std::vector<size_t> ix_AuxCoordAxis[4];
    std::vector<size_t> ix_CoordAxis[4];
    std::vector<size_t> ix_Axis[4];
    std::vector<size_t> ix_CoordVar[4];

    for( size_t c=0 ; c < coll_ix.size() ; ++c )
    {
      size_t ix=coll_ix[c];
      Variable& var_ix = pIn->variable[ix] ;

      int l=-1;
      if( var_ix.coord.isX )
        l=0;
      else if( var_ix.coord.isY )
        l=1;
      else if( var_ix.coord.isZ )
        l=2;
      else if( var_ix.coord.isT )
        l=3;

      if( l > -1 && var_ix.coord.isCoordVar )
          ix_CoordVar[l].push_back(ix);

      std::string axis( var_ix.getAttValue(n_axis) );
      for( l=0 ; l < 4 ; ++l )
        if( toupper(axis[0]) == xs[l] )
          break;

      if( l < 4 )
      {
        ix_Axis[l].push_back(ix);

        if( var_ix.coord.isCoordVar )
          ix_CoordAxis[l].push_back(ix);
        else
          ix_AuxCoordAxis[l].push_back(ix);
      }
    }

    for( size_t l=0 ; l < 4 ; ++l )
    {
      if( ix_CoordAxis[l].size() > 1 )
      {
        if( notes->inq(bKey + "5g", var.name) )
        {
          std::string capt( captVar(var.name, false) );
          capt += "must not depend on more than one coordinate " + n_variable ;
          capt += " with the same axis, found:";

          for( size_t k=0 ; k < ix_CoordAxis[l].size() ; ++k )
          {
            if( k )
              capt += "," ;
            capt += captVal(pIn->variable[ ix_CoordAxis[l][k] ].name, false) ;
          }

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      if( cFVal < 16 && ix_AuxCoordAxis[l].size() )
      {
        for( size_t k=0 ; k < ix_AuxCoordAxis[l].size() ; ++k )
        {
          size_t ix = ix_AuxCoordAxis[l][k];
          if( notes->inq(bKey + "5h", pIn->variable[ix].name) )
          {
            std::string capt(cFVersion + ": Auxiliary coordinate ");
            capt += captVar(pIn->variable[ix].name, false);
            capt += "must not have " + captAtt(n_axis, no_colon);

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }

      else if( cFVal > 15 &&
                  ix_AuxCoordAxis[l].size() && ix_CoordAxis[l].size() )
      {
        if( notes->inq(bKey + "5e", var.name) )
        {
          std::string capt(captVar(var.name,false) );
          capt += "should not have a coordinate " + n_variable ;
          capt += " and an auxiliary coordinate " + n_variable ;
          capt += " both with " + captAtt(n_axis, no_colon) + "=<";
          capt += xs[l] ;
          capt += ">";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }

    if( followRecommendations )
    {
      // recommendation: horizontal coordinate variable
      // should have an axis attribute
      for( size_t l=0 ; l < 2 ; ++l )
      {
        if( ix_CoordVar[l].size() == 1 && ! ix_CoordAxis[l].size() )
        {
          size_t ix = ix_CoordVar[l][0];

          // cases with a false axis value would match this
          if( pIn->variable[ix].getAttIndex(n_axis) == -1 )
          {
            std::string tag(bKey + "5d");

            if( notes->inq(tag, pIn->variable[ix].name) )
            {
              std::string capt("reco: Horizontal coordinate ");
              capt += captVar(pIn->variable[ix].name, false) ;
              capt += "should have " + captAtt(n_axis,no_colon);
              capt += "=";
              capt += xs[l];

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }
      }
    }
  }

  return;
}

void
CF::finalAtt_coordinates(void)
{
  // cf_conventions-1.4.pdf:
  // All of a variable's spatio temporal dimensions that are not latitude, longitude,
  // vertical, or time dimensions are required to be associated with the relevant
  // latitude, longitude, vertical, or time coordinates via the coordinates
  // attribute of the variable.

  // A) find missing auxiliary coordinate names in the coordinates attribute
  // B) for dta var with non-var-representative dimension

  // C) coordinate var must not have coordinates attribute
  // D) coordinates attribute contains non-existing variable

  std::vector<size_t> dv_ix ;  // indices to the data variables
  std::vector<size_t> cv_ix ;  // indices to the coordinate variables
  std::vector<size_t> acv_ix ;  // indices to the auxiliary coordinate variables

  // Prelude: separate data var, coord. var, and aux vars from each other
  std::vector<size_t> tmp_dv_ix ;

  for( size_t ix = 0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix] ;

     if( var.isDataVar() && var.boundsOf.size() == 0 )
       dv_ix.push_back(ix);
  }

  for( size_t ix = 0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix] ;

     if( var.isValidAtt(n_sample_dimension ) )
        continue;
     if( var.isValidAtt(n_cf_role ) )
        continue;
     if( var.isValidAtt(n_instance_dimension ) )
        continue;

     if( var.coord.isCoordVar )
       cv_ix.push_back(ix);
     else if( var.isCoordinate() || var.isLabel )
     {
      // Exception in order to avoid a false guess: all non-data variables with
      // a coordinates attribute and the same set of dimensions like data variables
      // are data variables. This is the only way to tell a pressure data var
      // from a pressure auxiliary coordinate.

      if( var.isValidAtt(n_coordinates) )
      {
        for( size_t d = 0 ; d < dv_ix.size() ; ++d )
        {
          Variable& var_dv = pIn->variable[dv_ix[d]] ;

          if( var_dv.dimName.size() == var.dimName.size() )
          {
            size_t i;
            for( i=0 ; i < var_dv.dimName.size() ; ++i )
              if( ! hdhC::isAmong(var.dimName[i], var_dv.dimName) )
                break;

            if( i == var_dv.dimName.size() )
            {
              var.addDataCount();
              var.clearCoord();
              dv_ix.push_back(ix);

              break;
            }
          }
        }
      }

      if( !var.isDataVar() )
        acv_ix.push_back(ix);
    }
  }

  finalAtt_coordinates_A(dv_ix, cv_ix, acv_ix );
  finalAtt_coordinates_B(dv_ix, cv_ix, acv_ix );
  finalAtt_coordinates_C(dv_ix, cv_ix, acv_ix );

  return;
}

void
CF::finalAtt_coordinates_A(std::vector<size_t>& dv_ix,
  std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix )
{
  // A) Find auxiliary coordinate name missing in the coordinates attribute.
  //    For each aux-var do:
  //    a) get the dim-list
  //    b) for each data var depending on the aux-var-dim-list
  //       (there could be other dims)
  //    c) is there a coordinates attribute?
  //    d) is aux-var contained in the coordinates att list?

  for( size_t av = 0 ; av < acv_ix.size() ; ++av )
  {
     Variable& var_av = pIn->variable[acv_ix[av]] ;

     // loop over the data var list
     for( size_t dv=0 ; dv < dv_ix.size() ; ++dv )
     {
        Variable& var_dv = pIn->variable[dv_ix[dv]] ;

        // is the auxiliary listed in the coordinates attribute?
        int ca_jx ;
        if( (ca_jx=ca_ix[ dv_ix[dv] ]) > -1 )
        {
          if( hdhC::isAmong(var_av.name, ca_vvs[ca_jx]) )
            continue;
        }

        if( var_av.dimName.size() )
        {
          // count number of aux-var dims also being data var dims
          size_t count = 0;

          for( size_t i = 0 ; i < var_av.dimName.size() ; ++i )
              if( hdhC::isAmong(var_av.dimName[i], var_dv.dimName) )
                  ++count;

          size_t sz = var_av.dimName.size() ;
          if( var_av.isLabel && sz )
            --sz;

          if( count == sz )
          {
            // b) yes
            // c) is there any coordinates att?
            if(  ca_jx > -1 )
            {
              // aux-var is missing in coordinates att
              if( ! hdhC::isAmong(var_av.name, ca_vvs[ca_jx]) )
              {
                bool isNoException=true;
                int j;
                if( (j=var_dv.getAttIndex(n_grid_mapping)) > -1 )
                  if( var_dv.attValue[j][0] == var_av.name )
                      isNoException = false;

/*
                if(isNoException
                      && isChap6_labelSubstAuxCoord(var_av, ca_vvs[ca_jx]) )
                    isNoException = false;
*/

                bool withT=false;
                if( var_av.name == timeName )
                  withT=true;

                if(isNoException && isXYZinCoordAtt(withT) )
                    isNoException=false;

                if( isNoException && notes->inq(bKey + "5i", var_dv.name) )
                {
                  std::string capt("auxiliary coordinate ");
                  capt += captVar(var_av.name, no_colon) + "is not named in " ;
                  capt += captAtt(var_dv.name,n_coordinates,no_colon, no_blank) ;

                  (void) notes->operate(capt) ;
                  notes->setCheckCF_Str( fail );
                }
              }
            }

            else
            {
              // data var requires a coordinates att which is missing

              //exception: compressed data
              bool isNotCompressed=true;

              if( compressIx > -1 )
              {
                Variable& var_cmpr = pIn->variable[compressIx] ;

                if( var_dv.dimName.size() == 1 &&
                    var_dv.dimName[0] == var_cmpr.name )
                        isNotCompressed=false;
              }

              if( isNotCompressed && !var_av.coord.isCoordVar )
              {
                if( ! (cFVal > 15 && var_dv.type != NC_INT
                        && var_dv.isValidAtt(n_sample_dimension) ) )
                {
                  std::string tag(bKey + "5c");
                  if( notes->inq(tag, var_dv.name, NO_MT) )
                  {
                    std::string capt("data " + captVar(var_dv.name, no_colon) );
                    capt += "depends on auxiliary coordinate " ;

                    // try for a dimless-Z
                    std::string* save = &var_av.name ;  // if not dimless-Z
                    for(size_t l=0 ; l < pIn->varSz ; ++l )
                    {
                      if( pIn->variable[l].coord.isZ_DL )
                      {
                        // found variable name is dim of the data variable
                        if( hdhC::isAmong(pIn->variable[l].name, var_dv.dimName))
                          save = 0;
                        else
                          save = &pIn->variable[l].name ;
                        break;
                      }
                    }

                    if( save )
                    {
                      capt += captVar(*save, no_colon, no_blank);
                      capt += ", thus requiring " + captAtt(n_coordinates, no_colon);

                      (void) notes->operate(capt) ;
                      notes->setCheckCF_Str( fail );
                    }
                  }
                }
              }
            }
          }
        }
     }
  }

  return;
}

void
CF::finalAtt_coordinates_B(std::vector<size_t>& dv_ix,
            std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix )
{
  // this case is not covered by A)
  // B) for data var with depending on a non-var-rep dims
  //   a) check data var dim-list for non-vars
  //   b) find aux-var depending on it
  //   c) is aux-var contained in the coordinates att list of data var

  for( size_t dv=0 ; dv < dv_ix.size() ; ++dv )
  {
    Variable& var_dv = pIn->variable[dv_ix[dv]] ;
    int u = ca_ix[ dv_ix[dv] ] ;

    for( size_t j=0 ; j < var_dv.dimName.size() ; ++j )
    {
      int i;

      if( (i=pIn->getVarIndex(var_dv.dimName[j])) == -1 )
      {
        // a) is given
        if( acv_ix.size() )
        {
          size_t av;
          for( av=0 ; av < acv_ix.size() ; ++av )
          {
            Variable& var_av = pIn->variable[acv_ix[av]];

            // auxiliary coord depending on the non-var dim
            if( hdhC::isAmong(var_dv.dimName[j], var_av.dimName) )
            {
              if( u == -1 )
              {
                std::string tag(bKey + "5c");
                if( notes->inq(tag, var_dv.name, NO_MT) )
                {
                  std::string capt("data " + captVar(var_dv.name, no_colon) );
                  capt += "depends on auxiliary coordinate ";
                  capt += captVar(var_av.name, no_colon, no_blank);
                  capt += ", thus " + captAtt(n_coordinates,no_colon);
                  capt += "is required";

                  (void) notes->operate(capt) ;
                  notes->setCheckCF_Str( fail );
                }
              }
              else
              {
                if( ! hdhC::isAmong(var_av.name, ca_vvs[u] )
                      && ! isChap6_labelSubstAuxCoord(var_av, ca_vvs[u]))
                {
                  if( !(isXYZinCoordAtt() && var_av.isLabel) )
                  {
                    std::string tag(bKey + "5i");
                    if( notes->inq(tag, var_dv.name ) )
                    {
                      std::string capt("auxiliary coordinate ");
                      capt += captVar(var_av.name, no_colon) + "is not named in ";
                      capt += captAtt(var_dv.name, n_coordinates, no_colon, no_blank) ;

                      (void) notes->operate(capt) ;
                      notes->setCheckCF_Str( fail );
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return;
}

void
CF::finalAtt_coordinates_C(std::vector<size_t>& dv_ix,
            std::vector<size_t>& cv_ix, std::vector<size_t>& acv_ix )
{
  // C) coordinate var must not have coordinates attribute
  std::string ca_val;

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix] ;

    bool isAnyCoord = var.isCoordinate() ;

    if( ca_ix[ix] > -1 )
    {
      if( var.coord.isCoordVar || isAnyCoord )
      {
        if( notes->inq(bKey + "5b") )
        {
          std::string capt;
          if( !var.coord.isCoordVar )
             capt = "auxiliary ";

          capt += "coordinate " + captVar(var.name, no_colon);
          capt += "should not have " + captAtt(n_coordinates, no_colon);

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }


      // D) coordinates att contains non-existing variable. This is done in a
      // method, which is also used for the compress-att.
      std::vector<std::string>& vs = ca_vvs[ca_ix[ix]];
      for( size_t j=0 ; j < vs.size() ; ++j )
      {
        int vCa ;
        if( (vCa=pIn->getVarIndex(vs[j])) == -1 )
        {
          if( notes->inq(bKey + "5a") )
          {
            std::string capt(captAtt(var.name, n_coordinates, no_colon));
            capt += "names a non-existing " ;
            capt += captVar(vs[j], no_colon, no_blank) ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  return;
}

void
CF::finalAtt_positive(void)
{
   std::string ca_val;

   for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
   {
     Variable& var = pIn->variable[ix] ;

     // check the correct usage of the positive attribute
     if( var.isCoordinate() && !var.coord.isZ && var.isValidAtt(n_positive) )
     {
       if( notes->inq(bKey + "43b", var.name) )
       {
          std::string capt(captAtt(var.name, n_positive, no_colon));
          capt += "is only allowed for vertical " + n_coordinates ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
       }
     }

   }

   return;
}

void
CF::final_dataVar(void)
{
  // ambiguous (weired) setting of attributes may result in a state
  // pretending that there is no data variable

  bool is=true;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].isDataVar() )
    {
      is=false;
      break;
    }
  }

  if( is && notes->inq(bKey + "0f") )
  {
    std::string capt("Warning: No data variable in the file" ) ;

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  return;
}

int
CF::finally(int eCode)
{
//  notes->print();
  int ec= notes->getExitValue();
  eCode = (eCode > ec ) ? eCode : ec ;
  return  eCode;
}

void
CF::findAmbiguousCoords(void)
{
   // test for variables with ambiguous coordinate info .
   int count[4];

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      if( var.isDataVar() || var.isMapVar )
         continue;

      count[0] = var.coord.indication_X;
      count[1] = var.coord.indication_Y;
      count[2] = var.coord.indication_T;
      count[3] = var.coord.indication_Z;

      char type[] = { 'X', 'Y', 'T', 'Z'} ;

      //sort
      for( size_t s0=0 ; s0 < 3 ; ++s0 )
      {
        for( size_t s1=s0+1 ; s1 < 4 ; ++s1 )
        {
           if( count[s0] < count[s1] )
           {
              int swap_count=count[s0];
              char swap_type=type[s0];
              count[s0] = count[s1];
              type[s0] = type[s1];
              count[s1] = swap_count;
              type[s1] = swap_type;
           }
        }
      }

      // analyse
      if( count[0] && (count[0] - var.indication_DV) > 1 )
      {
         if( count[0] > count[1] )
         {
            if( type[0] == 'X' )
               var.coord.isX = true;
            else if( type[0] == 'Y' )
               var.coord.isY = true;
            else if( type[0] == 'Z' )
               var.coord.isZ = true;
            else if( type[0] == 'T' )
               var.coord.isT = true;
         }
         else
         {
            bool is=true;
            // this doesn't work when there are three equally high ranked counts
            if( type[0] == 'X' && var.coord.isX )
               is = !is;
            if( type[1] == 'Y' && var.coord.isY )
               is = !is;
            if( type[2] == 'Z' && var.coord.isZ )
               is = !is;
            if( type[3] == 'T' && var.coord.isT )
               is = !is;

            if( is && notes->inq(bKey + "0i", var.name) )
            {
              std::string capt(captVar(var.name) ) ;
              capt += "Could not detect unambiguous state of ";

              if( !var.coord.isCoordVar )
                capt += "auxiliary ";
              capt += "coordinate variable" ;

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
         }
      }
   }

   return;
}

bool
CF::findLabel(Variable& var)
{
   if( var.isLabel )
     return var.isLabel;

   // evidences for a label:
   // a) variable is char type
   // b) the last dimension is used only once for all variables

   if( var.type != NC_CHAR || var.dimName.size() == 0 )
     return (var.isLabel=false);

   std::string& lastDimName = var.dimName[var.dimName.size()-1];

   for( size_t i=0 ; i < pIn->variable.size() ; ++i )
   {
     if( pIn->variable[i].name == var.name )
       continue;

     if( hdhC::isAmong(lastDimName, pIn->variable[i].dimName) )
       return (var.isLabel=false);
   }

   return (var.isLabel=true) ;
}

void
CF::findCellMeasures(Variable& var)
{
   // find valid variable declared by a cell_measures attribute
   int j ;

   if( (j=var.getAttIndex(n_cell_measures)) > -1 )
   {
     // get a list of strings clear of :-tailed key-words
     std::vector<std::string> vs( getKeyWordArgs(var.attValue[j][0], "arg", ':') ) ;

     for( size_t v=0 ; v < vs.size() ; ++v )
     {
       for( size_t i = 0 ; i < pIn->varSz ; ++i )
       {
         Variable& vari = pIn->variable[i] ;

         if( vari.name == vs[v] )
         {
            var.push_aux(vari.name);

            var.addDataCount();
            vari.addAuxCount();
            return ;
         }
       }
     }
   }

   return ;
}

void
CF::getAssociatedGroups(void)
{
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    int j;
    if( (j=var.getAttIndex(n_sample_dimension)) > -1 )
    {
      if( var.dimName.size() == 1 )
        associatedGroups.push_back(var.dimName[0]);
    }

    if( (j=var.getAttIndex(n_instance_dimension)) > -1 )
    {
      associatedGroups.push_back(var.attValue[j][0]);
    }
  }

  return;
}

void
CF::getDims(void)
{
  if( dimensions.size() )
     return  ;

  dimensions = pIn->nc.getDimNames() ; // all dimension names

  for( size_t k=0 ; k < dimensions.size() ; ++k )
  {
    size_t ix;
    for( ix=0 ; ix < pIn->varSz ; ++ix )
    {
      Variable& var = pIn->variable[ix] ;

      // variable representation of dimensions (coordinate vars)
      if( var.dimName.size() == 1 && var.dimName[0] == dimensions[k] )
      {
        varRepDims_ix.push_back(k);
        break;
      }
    }
  }

  for( size_t k=0 ; k < dimensions.size() ; ++k )
  {
    size_t ix;
    for( ix=0 ; ix < pIn->varSz ; ++ix )
    {
      Variable& var = pIn->variable[ix] ;
      size_t sz = var.dimName.size();

      // exclude char-lengths
      if( findLabel(var) && sz )
        --sz;

      // exclude vertices of bounds vars
      if( var.boundsOf.size() && sz )
        --sz;

      if( sz )
      {
        size_t j;
        for(j=0 ; j < sz ; ++j)
        {
          if( var.dimName[j] == dimensions[k] )
          {
            effDims_ix.push_back(k);
            break;
          }
        }

        if( j < sz )
          break;
      }
    }
  }

  return  ;
}

std::vector<std::string>
CF::getKeyWordArgs(std::string& str, std::string mode, char tail)
{
   // mode:s
   //      "char"           all key-words tailed by "char"
   //      "arg" [default]  all words not a key:
   //      "1st"            first of each pair
   //      "2nd"            second of each pair

   // return corresponding list

   std::vector<std::string> vs;

   if( str.size() == 0 )
      return vs;

   bool isArg= ( mode == "arg" ) ? true : false;
   bool is1st= ( mode == "1st" ) ? true : false;
   bool is2nd= ( mode == "2nd" ) ? true : false;

   // syntax: 'key: arg'
   Split x_str(str);

   // check key-word
   for( size_t i=0 ; i < x_str.size() ; ++i )
   {
      if( isArg )
      {
         // e.g., "myKey:" where tail == ":"
         size_t last=x_str[i].size() - 1 ;

         if( x_str[i][last] != tail )
           vs.push_back( x_str[i] ) ;
      }
      else if( is1st && ! i%2 )
        vs.push_back( x_str[i] ) ;
      else if( is2nd && i%2 )
        vs.push_back( x_str[i] ) ;
      else
      {
        size_t last=x_str[i].size() - 1 ;

        if(x_str[i][last] == tail)
           vs.push_back( x_str[i] ) ;
      }
    }

    return vs;
}

void
CF::getSN_TableEntry(void)
{
   // http://cf-pcmdi.llnl.gov/documents/cf-standard-names/cf-standard-name-table.xml

   // index to standard_names
   std::vector<int> zx;

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;
      var.snTableEntry.found=false;
      var.snTableEntry.hasBlanks=false;

      int j;
      if( (j=var.getAttIndex(n_standard_name)) > -1 )
      {
        var.std_name = var.attValue[j][0] ;
        var.snTableEntry.std_name=var.std_name;  // initially, just for a test
        zx.push_back(i) ;
      }
      else if( (j=var.getAttIndex(n_long_name)) > -1 )   // try long_name
      {
        // however, long-name should have no blanks
        var.snTableEntry.std_name = var.attValue[j][0];
        if( var.snTableEntry.std_name.find(" ") == std::string::npos )
          zx.push_back(i) ;
      }

      if( (j=var.getAttIndex(n_units)) > -1 )
        var.units = var.attValue[j][0] ;
   }

   // search the cf-standard-name-table.xml;
   // for efficiency: sort standard_names: if these are all CF conform, then
   // the table will be read only once.
   size_t v_swap;
   int sz=static_cast<int>(zx.size());

   for( int i=0 ; i < sz ; ++i )
   {
     for( int j=i+1 ; j < sz ; ++j )
     {
        std::string &var_i = pIn->variable[ zx[i] ].snTableEntry.std_name;
        std::string &var_j = pIn->variable[ zx[j] ].snTableEntry.std_name;

        if( var_j <  var_i )
        {
          v_swap = zx[i] ;
          zx[i] = zx[j];
          zx[j] = v_swap;
        }
     }
   }

   // note in the structure whether a name was found and the standard name,
   // moving any modifier into the struct member remainder.
   // Also works for blank-separated (instead of '_') standard names.
   scanStdNameTable(zx) ;

   return;
}

void
CF::getVarStateEvidences(Variable& var)
{
  // bounds and boundsOf are always auxiliaries; addAuxCount is done inside
  (void) isBounds(var) ;

  // look for attributes inidicating whether data or aux var
  int j;

  if( (j=var.getAttIndex(n_ancillary_variables)) > -1 )
  {
    var.addDataCount() ;

    // the named variable is also type data var
    int j2;
    if( (j2=pIn->getVarIndex(var.attValue[j][0])) > -1 )
      pIn->variable[j2].addDataCount() ;
  }
  if( var.isValidAtt(n_axis) )
    var.addAuxCount() ;
  if( var.isValidAtt(n_cell_methods) )
    var.addDataCount() ;
  if( var.isValidAtt(n_cf_role) )
    var.addAuxCount() ;
  if( var.isValidAtt(n_climatology) )
    var.addAuxCount() ;
  if( var.isValidAtt(n_coordinates) )
    var.addDataCount() ;
  if( var.isValidAtt(n_flag_values) )
    var.addDataCount() ;
  if( var.isValidAtt(n_flag_masks) )
    var.addDataCount() ;
  if( var.isValidAtt(n_grid_mapping) )
    var.addDataCount() ;
  if( var.isValidAtt(n_instance_dimension) )
    var.addAuxCount() ;
  if( var.isValidAtt(n_sample_dimension) )
    var.addAuxCount() ;
  if( var.isValidAtt(n_positive) )
    var.addAuxCount() ;

  if( var.dimName.size() > 2 )
    var.addDataCount();

  // sub-set of dims of a varRep is larger than the one of the
  // corresponding data variable
  size_t sz;
  if( (sz=var.dimName.size()) )
  {
    for( size_t i=0 ; i < pIn->varSz ; ++i )
    {
      Variable& vari = pIn->variable[i] ;

      // a sub-set of dims?
      if( sz < vari.dimName.size() )
      {
        if( hdhC::isAmong(var.dimName, vari.dimName, true) )
        {
          var.addAuxCount();
          break;
        }
      }
    }
  }

  // support point for aux: var is named by coordinates attributes
  for( size_t i=0 ; i < ca_vvs.size() ; ++i )
    if( hdhC::isAmong(var.name, ca_vvs[i]) )
      var.addAuxCount();

  return;
}

void
CF::hasBounds(Variable& var)
{
  // this checks whether a variable declared by a bounds or climatology attribute
  // is supplied.

  int j;
  std::string* type[2];
  type[0] = &n_bounds;
  type[1] = &n_climatology;

  for( size_t i=0 ; i < 2 ; ++i )
  {
    if( (j=var.getAttIndex( *type[i]) ) > -1 )
    {
       int i2;
       if( (i2=pIn->getVarIndex( var.attValue[j][0] )) == -1 )
       {
         if( notes->inq(bKey + "71g") )
         {
           std::string capt(captAtt(var.name, *type[i], no_colon) ) ;
           capt += "declares non-existing " ;
           if(i)
              capt += n_climatology + blank;
           else
              capt += "boundary " ;
           capt += captVar(var.attValue[j][0], no_colon, no_blank);

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
       else
       {
         pIn->variable[i2].addAuxCount();
         var.push_aux(var.attValue[j][0]);
       }
    }
  }

  return  ;
}

void
CF::inqAuxVersusPureCoord(void)
{
   // Are auxiliary coord vars consistent with coordinate variabels?
   // If not, then the coordinate variables direction is reset, because the
   // auxiliary coord is more reliable in most cases.
  std::vector<int> vs_aux_ix;

  int iCV_X=0;
  int iCV_Y=0;
  int iCV_Z=0;
  int iCV_T=0;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( var.coord.isCoordVar )
    {
      if( var.coord.isX || var.coord.isX )
        ++iCV_X;
      else if( var.coord.isY || var.coord.isY )
        ++iCV_Y;
      else if( var.coord.isZ || var.coord.isZ )
        ++iCV_Z;
      else if( var.coord.isT || var.coord.isT )
        ++iCV_T;
    }

    if( ! var.coord.isCoordVar )
    {
      if( var.coord.isX || var.coord.isY || var.coord.isZ || var.coord.isT )
        vs_aux_ix.push_back( static_cast<int>(i) );
    }
  }

  for( size_t i=0 ; i < vs_aux_ix.size() ; ++i )
  {
    Variable& aux = pIn->variable[vs_aux_ix[i]] ;

    if( aux.isScalar )
    {
      // a scalar aux is regarded as coordinate variable if no
      // other coordinate variable of the same direction exists.
      if( aux.coord.isX && iCV_X == 1 )
         aux.coord.isCoordVar = true;
      if( aux.coord.isY && iCV_Y == 1 )
         aux.coord.isCoordVar = true;
      if( aux.coord.isZ && iCV_Z == 1 )
         aux.coord.isCoordVar = true;
      if( aux.coord.isT && iCV_T == 1 )
         aux.coord.isCoordVar = true;
    }
  }

  return;
}

bool
CF::isBounds(Variable& var)
{
  // return true when var is bounds or climatology variable
  // declared by another variable.

  // known by a previous call
  if( var.boundsOf.size() )
    return true;

  std::string* str[2];
  str[0] = &n_bounds;
  str[1] = &n_climatology;
  std::string t;

  bool is=false;

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var2 = pIn->variable[ix] ;

    if( var2.name == var.name )
       continue;

    for( size_t n=0 ; n < 2 ; ++n )
    {
      // the value of the attribute
      t = var2.getAttValue(*(str[n])) ;

      if( t == var.name )
      {
        // test whether var2 is bounds and has bounds
        if( var.isValidAtt(*(str[n])) )
        {
          if( notes->inq(bKey + "71i", var2.name) )
          {
            std::string capt(captVar(var.name, no_colon)) ;
            capt += "should not be and have " ;
            capt += *(str[n]) + " at the same time" ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }

        if( n == 1 )
           var.isClimatology=true;

        var.boundsOf = var2.name ;
        var2.bounds = var.name ;

        var.addAuxCount() ;
        var2.addAuxCount() ;

        is=true;
      }
    }
  }

  return is ;
}

bool
CF::init()
{
  notes->init();  // safe

  applyOptions();

  return true;
}

void
CF::initDefaults()
{
  setObjName("CF");

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  cFVal=14;  // the default for checking
  followRecommendations=false;
  isFeatureType = false;
  isCF14_timeSeries=false;
  isCheck=false;

  timeIx=-1;
  compressIx=-1;

  bKey="CF_";
  fail="FAIL";

  blank = " " ;
  no_blank="no_blank";
  no_colon="no_colon";
  s_upper="upper";
  s_lower="lower";

  n_ancillary_variables="ancillary_variables";
  n_area="area";
  n_attribute="attribute";
  n_axis="axis";
  n_bounds="bounds";
  n_calendar="calendar";
  n_cell_measures="cell_measures";
  n_cell_methods="cell_methods";
  n_cf_role="cf_role";
  n_climatology="climatology";
  n_compress="compress";
  n_Conventions="Conventions";
  n_coordinates="coordinates";
  n_dimension="dimension";
  n_featureType="featureType";
  n_FillValue="_FillValue";
  n_flag_masks="flag_masks";
  n_flag_values="flag_values";
  n_formula_terms="formula_terms";
  n_global="global";
  n_grid_mapping="grid_mapping";
  n_instance_dimension="instance_dimension";
  n_long_name="long_name";
  n_latitude="latitude";
  n_leap_month="leap_month";
  n_leap_year="leap_year";
  n_longitude="longitude";
  n_missing_value="missing_value";
  n_month_lengths="month_lengths";
  n_NC_GLOBAL="NC_GLOBAL";
  n_number_of_observations="number_of_observations";
  n_positive="positive";
  n_sample_dimension="sample_dimension";
  n_standard_name="standard_name";
  n_time="time";
  n_units="units";
  n_valid_min="valid_min";
  n_valid_max="valid_max";
  n_valid_range="valid_range";
  n_variable="variable";

  n_grid_latitude="grid_" + n_latitude;
  n_grid_longitude="grid_" + n_longitude;

  NO_MT="NO_MT";

  ut_set_error_message_handler( ut_ignore );
  unitSystem = ut_read_xml(NULL);
}

bool
CF::isChap6_labelSubstAuxCoord(Variable& acv, std::vector<std::string>& ca_vs)
{
  // acv:   aux coordinate variable
  // ca_vs: names in a coordinates attribute

  // Note: Example 6.1 of Chap6 (CF-1.4) shows both the lon-lat pair and the label
  //       in the coordinates attribute. But according to the examples
  //       of Appendix H (CF-1.6) only one of the two is sufficient.

  if( acv.dimName.size() == 0 )
    return false;

  Variable* pLabel;
  Variable* pAux;

  // a label may parametrise coordinate variables
  for( size_t v=0 ; v < ca_vs.size() ; ++v )
  {
    if( ca_vs[v] == acv.name )
      continue;

    size_t i;
    for( i=0 ; i < pIn->varSz ; ++i )
      if( pIn->variable[i].name == ca_vs[v] )
        break;

    if( i == pIn->varSz )
      continue;

    if( pIn->variable[i].isLabel && pIn->variable[i].dimName.size() == 2 )
    {
      pLabel = &pIn->variable[i];
      pAux = &acv;
    }
    else if( acv.isLabel && acv.dimName.size() == 2 )
    {
      pAux = &pIn->variable[i];
      pLabel = &acv;
    }
    else
      continue;

    // condition:
    // char  var_ca(dim, label_dim)
    // type  acv(any, dim, any)
    for( size_t d=0 ; d < acv.dimName.size() ; ++d )
      if( pAux->dimName[d] == pLabel->dimName[0] )
        return true;
  }

  return false;
}

bool
CF::isChap9_specialLabel(Variable& label, Variable& dv)
{
   if( cFVal < 16 || label.isScalar )
     return false;

   if( hdhC::isAmong(label.dimName[0], associatedGroups) )
     return true;

/*
   // a label may delegate its "dimensionality" to an index variable,
   // indicated by attribute sample_dimension=label_name
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      if( var.name == label.name )
        continue;

      // condition 1
      int j;
      if( (j=var.getAttIndex(n_sample_dimension)) > -1 )
      {
         // char  label(label_dim)
         // int   var(label_dim)
         //         var:sample_dimension="dim"
         // float data_var(...,dim, ...)
         if( var.dimName.size() == 1 && var.dimName[0] == label.dimName[0] )
         {
            // the sample_dimension attribute names the dimension,
            // which must be shared by dv
            if( hdhC::isAmong(var.attValue[j][0], dv.dimName) )
               return true;
         }
      }

      // condition 2
      else if( (j=var.getAttIndex(n_instance_dimension)) > -1 )
      {
         // char  label(label_dim,n)
         // int   var(dim)
         //         var:instance_dimension="label_dim"
         // float data_var(...,dim, ...)
         if( var.attValue[j][0] == label.dimName[0] )
         {
            // the dim of the variable with sample_dimension must be shared by dv
            if( var.dimName.size() == 1
                  && hdhC::isAmong(var.dimName[0], dv.dimName) )
                return true;
         }
      }
   }
*/
   return false;
}

bool
CF::isCompressAux(Variable& var)
{
  std::string vs_cmpr;
  int j;
  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( var.name == pIn->variable[i].name )
      continue;

    if( (j=pIn->variable[i].getAttIndex(n_compress)) > -1 )
      vs_cmpr += pIn->variable[i].attValue[j][0] + blank;
  }

  if( vs_cmpr.size() )
  {
    Split x_vs_cmpr(vs_cmpr);
    for( size_t i=0 ; i < x_vs_cmpr.size() ; ++i )
      if( x_vs_cmpr[i] == var.name )
        return true;
  }

  return false;
}

bool
CF::isCompressEvidence(Variable& var, bool* isIntType)
{
   // evidence of a compress index variable is given when
   // 1) the type of the variable is int-alike
   // 2) coordinate vars lon and lat are not referenced any further
   // 3) no attributes: axis, positive, units, standard_name,
   //    flag_values, flag_masks, _FillValue
   // 4) lon and lat must be coordinate variables

   // 1)
   if( (*isIntType=pIn->nc.isIndexType(var.name)) )
   {
     std::vector<std::string> vs_notAnAtt;
     vs_notAnAtt.push_back(n_units);
     vs_notAnAtt.push_back(n_standard_name);
     vs_notAnAtt.push_back(n_FillValue);
     vs_notAnAtt.push_back(n_axis);
     vs_notAnAtt.push_back(n_positive);
     vs_notAnAtt.push_back(n_flag_values);
     vs_notAnAtt.push_back(n_flag_masks);

     // 3)
     for( size_t i=0 ; i < vs_notAnAtt.size() ; ++i )
        if( var.isValidAtt( vs_notAnAtt[i] ) )
           return false;
   }
   else
     return false;

   size_t count=0;
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& coordVar = pIn->variable[i];

     if( coordVar.coord.isCoordVar
           && (coordVar.coord.isX || coordVar.coord.isY) )
     {
       ++count;

       // 2)
       for( size_t j=0 ; j < pIn->varSz ; ++j )
       {
          if( i == j )
             continue;

          Variable& rVar = pIn->variable[j] ;

          if( hdhC::isAmong(coordVar.name, rVar.dimName) )
            return false;
       }
     }
   }

   // 4
   if( count != 2 )
      return false;

   return true; // looks like a compress index variable
}

bool
CF::isLatitude(void)
{
   for( size_t i=0 ; i < pIn->varSz ; ++i )
     if( pIn->variable[i].coord.isY )
       return true;

   return false;
}

bool
CF::isLongitude(void)
{
   for( size_t i=0 ; i < pIn->varSz ; ++i )
     if( pIn->variable[i].coord.isX )
       return true;

   return false;
}

bool
CF::isXYZinCoordAtt(bool withT)
{
  // not CF, but common practice.

  // If all spatial coordinates are specified
  // by the coordinate attribute of var, then a label may be
  // omitted in a coordinates attribute.

  for( size_t p=0 ; p < ca_vvs.size() ; ++p )
  {
    std::vector<std::string>& ca_vs = ca_vvs[p];

    bool is[] = { false, false, false, false};
    for(size_t k=0 ; k < ca_vs.size() ; ++k )
    {
      int v;
      if( (v=pIn->getVarIndex(ca_vs[k])) > -1 )
      {
        if( !is[0] && pIn->variable[v].coord.isX )
          is[0] = true;
        else if( !is[1] && pIn->variable[v].coord.isY )
          is[1] = true;
        else if( !is[2] && pIn->variable[v].coord.isZ )
          is[2] = true;
        else if( !is[3] && pIn->variable[v].coord.isT )
          is[3] = true;
      }
    }

    if( is[0] && is[1] && is[2] )
    {
      if( withT && !is[3] )
        return false;

      return true;
    }
  }

  return false;
}

void
CF::linkObject(IObj *p)
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

bool
CF::parseUnits( std::string s)
{
   ut_unit* u = ut_parse(unitSystem, s.c_str(), UT_ASCII);

   return u ? true : false ;
}

void
CF::postAnnotations(void)
{
  std::vector<std::vector<std::string> > tag;
  std::vector<std::string> newTag;
  std::vector<std::string> capt;
  std::vector<std::string> clearTag;

  // definition of tasks; there are different modes:
  // a) replacing all tags by a new one
  // b) erase all tags, but the first one;
  //    assign empty newTag and capt.
  // c) erase a clear-tag, when any specified tag is found;
  //    clearTag is defined by newTag.size() > 0 && newTag == capt

  tag.push_back(std::vector<std::string>()) ;
  tag.back().push_back(bKey + "432b");
  tag.back().push_back(bKey + "432d");
  newTag.push_back(bKey + "432j");
  capt.push_back("Suspecting a misplaced formula_terms attribute");

  tag.push_back(std::vector<std::string>()) ;
  tag.back().push_back(bKey + "26a");
  tag.back().push_back(bKey + "81e");
  newTag.push_back("NC_3_12");
  capt.push_back("NC_3_12");

  tag.push_back(std::vector<std::string>()) ;
  tag.back().push_back(bKey + "432b");
  tag.back().push_back(bKey + "33e");
  newTag.push_back(s_empty);
  capt.push_back(s_empty);

  tag.push_back(std::vector<std::string>()) ;
  tag.back().push_back(bKey + "56a");
  tag.back().push_back(bKey + "5c");
  newTag.push_back(s_empty);
  capt.push_back(s_empty);

  tag.push_back(std::vector<std::string>()) ;
  tag.back().push_back(bKey + "74e");
  tag.back().push_back(bKey + "73e");
  newTag.push_back(s_empty);
  capt.push_back(s_empty);

  std::vector<std::string> annot;
  size_t pos;

  for( size_t t=0 ; t < tag.size() ; ++t )
  {
    bool is=false;
    size_t sz=newTag[t].size() ;

    bool isC = sz && newTag[t] == capt[t] ;
    if( isC )
      tag[t].push_back(newTag[t]); // could contain duplicates

    // collect items
    std::vector<std::string> vs;
    std::vector<std::string> vs2;

    for( size_t i=0 ; i < tag[t].size() ; ++i )
    {
      // same tag for various variables possible
      vs2 = notes->getAnnotation( tag[t][i] );

      if( vs2.size() == 0 )
      {
        if( isC )
          continue; // try the next tag of the collection
        else
        {
          is=true; // all or nothing, thus nothing for this tag collection
          break;
        }
      }

      for(size_t j=0 ; j < vs2.size() ; ++j )
        vs.push_back(vs2[j]);
    }

    if(is || ! vs.size())
      continue;

    // avoid multiples for a given variable
    // caused by added case-C items above
    annot = hdhC::getUniqueVector(vs);

    // extract variable names; multiples are possible
    for( size_t i=0 ; i < annot.size() ; ++i )
      if( (pos=annot[i].rfind('_')) < std::string::npos )
        annot[i] = annot[i].substr(++pos);

    // unique
    std::vector<std::string> items(hdhC::getUniqueVector(annot));

    for(size_t k=0 ; k < items.size() ; ++k )
    {
      size_t count=0;

      if( isC )
        notes->eraseAnnotation(newTag[t], items[k]);  // case c
      else
      {
        for( size_t i=0 ; i < annot.size() ; ++i )
        {
          if( annot[i] == items[k] )
          {
            if( sz || count++ )  // all || all but the first one
              notes->eraseAnnotation(tag[t][i], items[k]);
          }
        }
      }

      if( sz && !isC && notes->inq(newTag[t], items[k])  )
      {
        (void) notes->operate(captVar(items[k]) + capt[t]) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  return;
}

bool
CF::run(void)
{
   init();

   getDims() ;

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      // no data?
      var.isNoData=pIn->nc.isEmptyData(var.name);

      getVarStateEvidences(var);

      if( var.isValidAtt(n_compress) )
      {
        compressIx=var.getVarIndex();
        var.isCompress=true;
      }

      // find coordinate variables
      chap5_1(var);

      if( findLabel(var) )
         var.addAuxCount();

      findCellMeasures(var) ;
   }

   // get index and contents of coordinates attributes
   int sz=pIn->varSz;
   for( int i=0 ; i < sz ; ++i )
   {
      Variable& var = pIn->variable[i];
      vIx[ var.name ] = i ;

      ca_ix.push_back(-1) ;

      int j;
      if( (j=var.getAttIndex(n_coordinates)) > -1 )
      {
        ca_pij.push_back( std::pair<int, int> (i,j) );
        Split x_ca(var.attValue[j][0]);
        ca_vvs.push_back( x_ca.getItems() );
        ca_ix.back() = static_cast<int>(ca_pij.size() - 1);
      }
   }

   getSN_TableEntry();  //read table and set also variables std_name and units.

   getAssociatedGroups();

   // spell-checking of attribute names provided in the file vs.
   // the CF specific ones.
   attributeSpellCheck();

   // which CF Conventions are going to be checked?
   if( ! chap2_6_1() )
     return false;  // undefined Convention is specified

   // dimensionless vertical coordinate? Could set var.isChecked=true
   chap4_3_2() ;  // preponed to make processing easier

   // a) Indexes to variables and attributes.
   // b) It is difficult to find faults in CF-1.4 time-series files
   //    for a z-coordinate. So, detection of a time-series file helps.
   std::string firstDim;

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     // some checks are brought forward for convenience
     if( var.dimName.size() == 0
           || (var.type == NC_CHAR && var.dimName.size() == 1 ) )
       var.isScalar = true;

     // find auxiliaries of any kind. Only a start, further
     // pushes may come later.
     for( size_t j=0 ; j < var.dimName.size() ; ++j )
     {
       int k;
       if( (k=pIn->getVarIndex(var.dimName[j])) > -1 )
       {
         pIn->variable[k].addAuxCount();
         var.push_aux(var.dimName[j]);
       }
     }

     // find old-fashion type of time-series, i.e. all variables
     // depend on the same single dimension
     if( i == 0 && var.dimName.size() == 1 )
       firstDim=var.dimName[0] ;

     if( firstDim.size() )
     {
       if( var.dimName.size() == 0 )
         firstDim.clear();
       else if( var.dimName.size() > 1 )
         firstDim.clear();
       else if( var.dimName[0] != firstDim )
         firstDim.clear();
     }

     // no data?
     if( var.isNoData && var.isDataVar() )
     {
       if( notes->inq(bKey + "0e", var.name) )
       {
         std::string capt(captVar(var.name));
         capt += "No data" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
   }

   if( firstDim.size() )
     isCF14_timeSeries = true;

   return entry();
}

bool
CF::scanStdNameTable(std::vector<int> &zx)
{
  // zx: vector of indices to variables with standard_name and
  // sorted according to the standard_names

  if( zx.size() == 0 )  // no standard name
     return false;

  if( std_name_table.size() == 0 )  // name of the table
     return false;

  std::string file(tablePath + "/");

  ReadLine ifs( file + std_name_table ) ;
  ifs.clearSurroundingSpaces();

  if( ! ifs.isOpen() )
    // could not open
    return false ;

  std::string testName[zx.size()];
  for( size_t i=0 ; i < zx.size() ; ++i )
  {
     Variable& var = pIn->variable[zx[i]];

     // remember the original test name
     testName[i] = var.snTableEntry.std_name;

     bool isCont=false;

     // any identical standard_names in the race?
     for( size_t k=0 ; k < i ; ++k )
     {
       Variable& var_k = pIn->variable[zx[k]];

       if( var.snTableEntry.std_name == testName[k] )
       {
         var.snTableEntry.std_name  = var_k.snTableEntry.std_name ;
         var.snTableEntry.found     = var_k.snTableEntry.found ;
         var.snTableEntry.alias     = var_k.snTableEntry.alias;
         var.snTableEntry.amip      = var_k.snTableEntry.amip;
         var.snTableEntry.grib      = var_k.snTableEntry.grib;
         var.snTableEntry.remainder = var_k.snTableEntry.remainder ;
         var.snTableEntry.canonical_units = var_k.snTableEntry.canonical_units;
         isCont=true;
         break;
       }
     }

     if( isCont )
       continue;

     Split x_sn_name(testName[i]) ;

     if( x_sn_name.size() == 1 )
       scanStdNameTable(ifs, var, testName[i]) ;
     else
     {
       // try successively the blank separated sub-strings;
       // start with the longest string possible
       int j;
       std::string s;
       for( j=static_cast<int>(x_sn_name.size()-1) ; j > -1 ; --j )
       {
         s = x_sn_name[0];
         for( int l=1 ; l <= j ; ++l )
           s += "_" + x_sn_name[l] ;

         scanStdNameTable(ifs, var, s) ;

         if( var.snTableEntry.found )
         {
            s = testName[i].substr(s.size()) ;
            var.snTableEntry.remainder = hdhC::stripSurrounding(s) ;

            // if sn is right, but there is something with the modifier
            if( x_sn_name[0] != var.snTableEntry.std_name )
              var.snTableEntry.hasBlanks = true ;

            break;
         }
       }
     }
  }

  ifs.close();

  return true;
}

bool
CF::scanStdNameTable(ReadLine& ifs, Variable& var, std::string testName)
{
  // Find entries, aliases and units.
  // Units are only avaialble for entries.

  // Table: a block of entries comes first followed by a block of alias entries.
  //        alphabetically ordered.
  // Provided standard_name structs are ordered to ensure to minimise rewindings,
  // therefore the intricate algorithm structure.

  std::string beg_entry("entry id=");
  std::string end_entry("/entry");

  std::string beg_alias("alias id=");
  std::string end_alias("/alias");

  std::string n_grib("grib");
  std::string n_amip("amip");
  std::string canonical_units("canonical_units");

  Split x_line;
  x_line.setSeparator("<>\"");

  std::string line;
  int countRewinds=0;

  while ( true )
  {
    while( ! ifs.getLine(line) )
    {
      // look for standard name entries
      x_line = line ;

      if( x_line[1] == testName )
      {
        if( x_line[0] == beg_entry)
        {
           var.snTableEntry.std_name = testName ;
           var.snTableEntry.found=true;

           // exploit entry block
           while( ! ifs.getLine(line) )
           {
              if( line.find(end_entry) < std::string::npos )
                break;

              x_line = line ;
              if( x_line.size() > 1 )
              {
                if( x_line[0] == canonical_units )
                  var.snTableEntry.canonical_units = x_line[1] ;
                else if( x_line[0] == n_grib )
                  var.snTableEntry.grib = x_line[1] ;
                else if( x_line[0] == n_amip )
                  var.snTableEntry.amip = x_line[1] ;
              }
           }

           return true;
        }
        else if( line.find(beg_alias) < std::string::npos )
        {
           // try again later with the real entry
           var.snTableEntry.alias=testName;
           --countRewinds;

           if( ifs.getLine(line) )  // only the next line
              break; // should not happen

           x_line = line ;
           if( x_line.size() > 1 )
             testName=x_line[1] ;
        }
      }
    }

    // standard_name not in the table
    ifs.rewind();

    if( ++countRewinds == 2 )
       break;
  }

  return false;
}

void
CF::setCheck(std::string &s)
{
   isCheck =true;

   // note: cFVal=14 by default

   if( s.find("1.3") < std::string::npos )
   {
     if( notes->inq(bKey + "261c") )
     {
        std::string capt("CF-1.3 is not implemented,");
        capt += " using CF-1.4";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
   }
   else if( s.find("1.4") < std::string::npos )
     ; // cFVal=14 by default
   else if( s.find("1.5") < std::string::npos )
     cFVal=15;
   else if( s.find("1.6") < std::string::npos )
     cFVal=16;
   else if( s.find("1.7") < std::string::npos )
   {
     cFVal=16;  // fall back

     if( notes->inq(bKey + "261e") )
     {
        std::string capt("CF-1.7 is not implemented,");
        capt += " falling back to CF-1.6" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
   }
   else
   {
     std::string tag(bKey+"261b");
     if( notes->inq(tag) )
     {
       std::string capt("unknown Conventions=" + s);

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }

     isCheck=false;
     return;
   }

   notes->setCheckCF_Str("PASS");

   return;
}

bool
CF::timeUnitsFormat(Variable& var, bool isAnnot)
{
  if( !isCheck )
    return false;

  if( var.isValidAtt(n_cf_role) )
    return false;

  std::string units( var.getAttValue(n_units));

  if( isAnnot && var.coord.isCoordVar && units.size() == 0 )
    return false;

/*
  //  "days since 1990-1-1 0-0-0" passes ut_parse()

  ut_unit* p = ut_parse(unitSystem, units.c_str(), UT_ASCII);

  if( p )
  {
    ut_free(p);  // valid format
    return true;
  }
*/

  // using split ignores multiple blanks
  // convert dateTtimeZone to 'date time Zone'
  size_t countColon=0;  // time separator
  size_t countDash=0;   // date separator

  std::string str0;
  for( size_t i=0 ; i < units.size() ; ++i )
  {
    if( units[i] == '-' )
       ++countDash;
    else if( countDash == 2 )
    {
      if( countColon == 2 )
      {
         if( ! ( units[i] == '.' || hdhC::isDigit(units[i]) ) )
         {
            str0 += ' ';
            countColon=9999; // the rest declares the time-zone
         }
      }
      else if( units[i] == 'T' )
      {
         str0 += ' ';
         continue;
      }
      else if( units[i] == 't' )
      {
         str0 += ' ';
         continue;
      }
      else if( units[i] == ':' )
         ++countColon;
      else if( units[i] == 'Z' || units[i] == 'z')
         str0 += ' ';  // separate the trivial time zone indicator
    }

    str0 += units[i] ;
  }

  Split x_units(str0);

  size_t sz ;
  if( (sz=x_units.size()) < 2 )
    return false;  // to short; even for invalid time units

  // check the value of units
  bool isInvalidFreq=true;
  bool isInvalidKey=true;
  bool isInvalidDate=true;
  bool isInvalidTime=false;  // could be omitted
  bool isInvalidTZ=false;    // could be omitted

  bool isProcTime=true;  // could be omitted
  bool isProcTZ=true;    // could be omitted

  std::vector<std::string> period;
  period.push_back("year");
  period.push_back("month");
  period.push_back("day");
  period.push_back("hour");
  period.push_back("minute");
  period.push_back("second");
  period.push_back("d");
  period.push_back("hr");
  period.push_back("h");
  period.push_back("min");
  period.push_back("sec");

  for(size_t itm=0 ; itm < sz ; ++itm )
  {
    if( isInvalidFreq )
    {
      for( size_t i=0 ; i < period.size() ; ++i )
      {
        if( (x_units[itm] == period[i]) || (x_units[itm] == (period[i] + "s")) )
        {
          isInvalidFreq=false;
          break;
        }
      }

      if( !isInvalidFreq )
        continue;
    }

    if( isInvalidKey )
    {
      if( x_units[itm] == "since" )
      {
        isInvalidKey=false;
        continue;
      }
    }

    if( isInvalidDate )
    {
      // split the date; a negative date is ignored
      Split dt(x_units[itm], "-");
      size_t countFields=0;

      if( dt.size() )
      {
        for( size_t j=0 ; j < dt.size() ; ++j )
          if( hdhC::isDigit(dt[j]) )
            ++countFields;

        if( countFields == 3 )
        {
          if( isAnnot && followRecommendations
              && (dt[0] == "0" || dt[0] == "00")
                 && (dt[1] == "1" || dt[1] == "01")
                   && (dt[2] == "1" || dt[2] == "01") )
            chap4_4a_reco(var);

          isInvalidDate=false;
          continue;
        }
      }
    }

    if( isProcTime )  // test the time string
    {
      size_t countFields=0;

      // a hypen is never valid
      if( x_units[itm].find('-') < std::string::npos )
      {
        isInvalidTime = true;
        isProcTime=false;
      }
      else
      {
        Split tm(x_units[itm], ":");

        if( tm.size() )
        {
          for( size_t j=0 ; j < tm.size() ; ++j )
          {
            if( j < 2 )
            {
              if( hdhC::isDigit(tm[j]) )
                ++countFields;
            }
            else if( j == 2 )
            {
              // special: seconds without an optionally appended TZ character
              Split x_sec(tm[j], ".");

              if( x_sec.size() == 1 && hdhC::isDigit(x_sec[0]) )
                ++countFields;
              else if( x_sec.size() == 2
                        && hdhC::isDigit(x_sec[0]) && hdhC::isDigit(x_sec[1]) )
                ++countFields;
            }
          }
        }

        if( countFields == tm.size() )
          isProcTime=false;
        else if(countFields)
        {
          isInvalidTime = true;
          isProcTime=false;
        }
      }

      continue ;
    }

    if( isProcTZ )  // time zone other than Z
    {
       if( x_units[itm] == "Z" )
       {
          isProcTZ=false;
          continue;
       }

       bool is=false;

       if( x_units[itm][0] == '-' )
         str0=x_units[itm].substr(1);
       else
         str0=x_units[itm];

       if( str0[0] == ':' )  // invalid begin
         isInvalidTZ=true;
       else
       {
         Split tz(str0, ":");
         size_t tzSz=tz.size();

         for( size_t j=0 ; j < tzSz ; ++j )
         {
           if( ! hdhC::isDigit(tz[j]) )
             isInvalidTZ=true;
           else if( tzSz == 1 )
           { // n, nn, nnnn  are valid
             if( tz[j].size() > 4 )
               isInvalidTZ=true;
           }
           else if( tzSz == 2 )
           { // n:n, nn:n, n:nn, nn:nn are valid
             if( tz[0].size() > 2 || tz[1].size() > 2 )
               isInvalidTZ=true;
           }
           else
             is=true; // sound TZ
         }
       }

       if( is || isInvalidTZ )
          isProcTZ=false;
    }
  }

  bool isInvalid = isInvalidFreq || isInvalidKey || isInvalidDate
                   || isInvalidTime || isInvalidTZ ;

  // annotation section
  if( ! isInvalid )
    return true;

  // too many failures: probably not a time unit
  size_t countBad=0;
  size_t countGood=0;

  if( isInvalidFreq )
    ++countBad;
  else
    ++countGood;

  if( isInvalidKey )
    ++countBad;
  else
    ++countGood;

  if( isInvalidDate )
    ++countBad;
  else
    ++countGood;

  if( isInvalidTime )
    ++countBad;
  else if( !isProcTime )
    ++countGood;

  if( isInvalidTZ )
    ++countBad;
  else if( !isProcTZ )
    ++countGood;

  if( isAnnot && countGood > 2 && countBad )
  {
    std::string annot("Expected: frequency since date [time [zone]]") ;
    annot += "\nFound: ";
    annot += var.name + ":units (file)=";
    annot += units ;

    std::string text;

    if( isInvalidFreq )
    {
        if( text.size() == 0 )
          text = annot;
        text += "\nMissing frequency" ;
    }

    if( isInvalidKey )
    {
        if( text.size() == 0 )
          text = annot;
        text += "\nMissing key-word=since" ;
    }

    // time could be omitted, indicated by isProcTime
    if( isInvalidDate && isProcTime )
    {
        if( text.size() == 0 )
          text = annot;
        text += "\nMissing reference date" ;
    }
    else if( isInvalidDate )
    {
        if( text.size() == 0 )
          text = annot;
        text += "\nInvalid date component" ;
    }
    else if( isInvalidTime )
    {
        if( text.size() == 0 )
          text = annot;
        text += "\nInvalid time component" ;
    }

    if( isInvalidTZ )
    {
        if( text.size() == 0 )
          text = annot;
        text +="\nInvalid time zone component" ;
    }

    if( text.size() )
    {
      if( notes->inq(bKey + "44b", var.name) )
      {
        std::string capt(captAtt(var.name, n_units, no_colon)) ;
        capt += "is not CF conform, found" + captVal(var.units,false);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( countGood && countGood > countBad )
    return true; // probably a time units format

  return false; //probably not
}

std::string
CF::units_lon_lat(Variable& var, std::string units)
{
  // Latitude / Longitude Coordinate; desig==lat||lon
  if( units.size() == 0 )
    units = var.getAttValue(n_units) ;

  if( units.size() == 0 )
     return units;  // empty string

  std::string coord;

  std::string unitsBase="degree";

  std::vector <std::string> def_units;
  def_units.push_back( s_empty );  // rotated (single)
  def_units.push_back( "s" );  // rotated (plural)

  def_units.push_back( "_north" );  // lat
  def_units.push_back( "s_north" );
  def_units.push_back( "_N" );
  def_units.push_back( "s_N" );
  def_units.push_back( "N" );
  def_units.push_back( "sN" );

  def_units.push_back( "_east" );  //lon
  def_units.push_back( "s_east" );
  def_units.push_back( "_E" );
  def_units.push_back( "s_E" );
  def_units.push_back( "E" );
  def_units.push_back( "sE" );

  std::string s;

  for( size_t j=0 ; j < def_units.size() ; ++j)
  {
    s = unitsBase + def_units[j] ;
    if( units == s )
    {
      // just find the corresponding
      if( j < 2 )
      {
        coord = "rot" ;
      }
      else if( j < 7 )
      {
        coord = n_latitude ;
        if( var.is_1st_Y )
        {
          ++var.coord.indication_Y;
          var.is_1st_Y=false;
        }
      }
      else
      {
        coord = n_longitude ;
        if( var.is_1st_X )
        {
          ++var.coord.indication_X;
          var.is_1st_X=false;
        }
      }

      break;
    }
  }

  if( coord == "rot" )
  {
    // identification requires also a standard_name
    std::string sn( var.getAttValue(n_standard_name, lowerCase)) ;
    if( sn.size() )
    {
      if( sn == n_grid_longitude )
      {
        coord = sn ;
        if( var.is_1st_rotX )
        {
          ++var.coord.indication_X;
          var.coord.isX=true;
          var.is_1st_rotX=false;
        }
      }
      else if( sn == n_grid_latitude )
      {
        coord = sn ;
        if( var.is_1st_rotY )
        {
          ++var.coord.indication_Y;
          var.coord.isY=true;
          var.is_1st_rotY=false;
        }
      }
    }
  }

  return coord;
}

double
CF::wagnerFischerAlgo(std::string& s, std::string& t)
{
  // http://en.wikipedia.org/wiki/Wagner-Fischer_algorithm

  // For all i and j, d[i,j] will hold the Levenshtein distance between
  // the first i characters of s and the first j characters of t.
  // Note that d has (m+1)  x(n+1) values.
  size_t i_sz= s.size()+1 ;
  size_t j_sz= t.size()+1 ;

  int d[i_sz][j_sz];

  // the distance of any first string to an empty second string
  for( size_t i=0 ; i < i_sz ; ++i )
    d[i][0] = i ;

  // the distance of any second string to an empty first string
  for( size_t j=0 ; j < j_sz ; ++j )
    d[0][j] = j ;

  int cost[3];
  int swp;

  for( size_t i=1 ; i < i_sz ; ++i )
  {
    for( size_t j=1 ; j < j_sz ; ++j )
    {
      if( s[i-1] == t[j-1] )
        d[i][j] = d[i-1][j-1] ;
      else
      {
        cost[0]=d[i-1][j] ;
        cost[1]=d[i][j-1] ;
        cost[2]=d[i-1][j-1] ;

        for( size_t l0=0 ; l0 < 2 ; ++l0 )
        {
          for( size_t l1=l0+1 ; l1 < 3 ; ++l1 )
          {
             if( cost[l1] < cost[l0] )
             {
                swp = cost[l0] ;
                cost[l0] = cost[l1] ;
                cost[l1] = swp ;
             }
          }
        }

        d[i][j] = cost[0]+1 ;
      }
    }
  }

  return static_cast<double>(d[i_sz-1][j_sz-1]) / static_cast<double>(t.size()) ;
}

void
CF::chap(void)
{
   // dimensionless vertical coordinate? Could set var.isChecked=true
   // chap4_3_2() ;  // was checked before

   chap2();  // NetCDF Files and Components
   chap3();  // Description of the Data
   chap4();  // Coordinate Types
   chap5();  // Coordinate Systems
   chap6();  // Labels
   chap7();  // Data Representative of Cells
   chap8();  // Reduction of Dataset Size
   chap9();  // Discrete Sampling Geometries

   inqAuxVersusPureCoord();

/*
   // any indications from a cross-check of coord properties for
   // undetermined coordinate vars?
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& vari = pIn->variable[i] ;

      if( ! vari.coord.isCoordVar )
        continue;
      if( vari.isCoordinate() )
        continue;

      for( size_t j=0 ; j < pIn->varSz ; ++j )
      {
         if( i==j )
            continue;

         Variable& varj = pIn->variable[j] ;

         // Is varj an auxiliary depending on vari and its dimensions, respectively?
         // It is possible that still an ambiguity remains
         if( varj.coord.isX )
         {
            for( size_t k=0 ; k < varj.dimName.size() ; ++k )
            {
               if( varj.dimName[k] == vari.name )
               {
                  vari.coord.isX = true;
                  break;
               }
            }
         }

         if( varj.coord.isY )
         {
            for( size_t k=0 ; k < varj.dimName.size() ; ++k )
            {
               if( varj.dimName[k] == vari.name )
               {
                  vari.coord.isY = true;
                  break;
               }
            }
         }

         if( varj.coord.isZ )
         {
            for( size_t k=0 ; k < varj.dimName.size() ; ++k )
            {
               if( varj.dimName[k] == vari.name )
               {
                  vari.coord.isZ = true;
                  break;
               }
            }
         }

         if( varj.coord.isT )
         {
            for( size_t k=0 ; k < varj.dimName.size() ; ++k )
            {
               if( varj.dimName[k] == vari.name )
               {
                  vari.coord.isT = true;
                  break;
               }
            }
         }
      }
   }
*/

   for( size_t i=0 ; i < pIn->varSz ; ++i )
     pIn->variable[i].disableAmbiguities();


   return;
}

void
CF::chap_reco(void)
{
   chap2_reco();
   chap3_reco();
   chap5_reco();

   return ;
}

void
CF::chap2(void)
{
  if( ! notes->inq(bKey + "2", s_empty, "INQ_ONLY" ) )
     return ;  // checks are selected to be discarded by 'D'

  chap2_1();   // filename extension
  chap2_2() ;  // data types
  chap2_3() ;  // naming
  chap2_4() ;  // dimensions
  chap2_5_1(); // missing data
  chap2_6() ;  // conventions, file contents

  return;
}

void
CF::chap2_reco(void)
{
   chap2_3_reco();    // naming
   chap2_4_reco();    // dimensions
   chap2_6_2_reco();  // title and history

   return;
}

void
CF::chap2_1(void)
{
  if( !isCheck )
     return;

  // filename extension
  if( pIn->filename.substr( pIn->filename.size() - 3) != ".nc" )
  {
    if( notes->inq(bKey + "21a") )
    {
      std::string capt("the extension of the filename should be '.nc'") ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
}

void
CF::chap2_2(void)
{
  // no string type
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    if( var.type == NC_STRING )
    {
      if( notes->inq(bKey + "22a", var.name) )
      {
        std::string capt(captVar(var.name, no_colon)) ;
        capt += "should not be type NC_STRING";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    for( size_t j=0 ; j < var.attName.size() ; ++j )
    {
      if( var.attType[j] == NC_STRING )
      {
        if( notes->inq(bKey + "22b", var.name) )
        {
          std::string capt(captAtt(var.name, var.attName[j], no_colon)) ;
          capt += "should not be type NC_STRING";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  return;
}

void
CF::chap2_3(void)
{
   if( !isCheck )
     return;

   // naming convention
   std::vector<std::string> fault;

   // dimensions
   for( size_t i=0 ; i < dimensions.size() ; ++i )
   {
     if( ! hdhC::isAlpha(dimensions[i][0]) )
     {
       fault.push_back(n_dimension) ;
       fault.back() += captVal(dimensions[i]) ;
       fault.back() += "should begin with a letter" ;
     }

     for( size_t j=1 ; j < dimensions[i].size() ; ++j )
     {
       if( ! ( hdhC::isAlpha(dimensions[i][j])
                 || hdhC::isDigit(dimensions[i][j])
                     || dimensions[i][j] == '_') )
       {
         fault.push_back(n_dimension) ;
         fault.back() += captVal(dimensions[i]) ;
         fault.back() += "should be composed of letters, digits, and underscores" ;
         break;
       }
     }
   }

   // variables
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     if( ! hdhC::isAlpha(var.name[0]) )
     {
       fault.push_back(captVar(var.name, false)) ;
       fault.back() += "should begin with a letter" ;
     }

     for( size_t j=1 ; j < var.name.size() ; ++j )
     {
       if( ! ( hdhC::isAlpha(var.name[j])
                   || hdhC::isDigit(var.name[j])
                        || var.name[j] == '_') )
       {
         fault.push_back(captVar(var.name, false)) ;
         fault.back() += "should be composed of letters, digits, and underscores" ;
         break;
       }
     }

     // attributes
     for( size_t j=0 ; j < var.attName.size() ; ++j )
     {
       // _FillValue contradicts the CF naming
       if( ! hdhC::isAlpha(var.attName[j][0])
          && var.attName[j] != n_FillValue )
       {
         fault.push_back(captAtt(var.name, var.attName[j], no_colon, s_upper)) ;
         fault.back() += "should begin with a letter" ;
       }

       for( size_t k=1 ; k < var.attName[j].size() ; ++k )
       {
         if( ! ( hdhC::isAlphaNum(var.attName[j][k])
                   || var.attName[j][k] == '_') )
         {
           fault.push_back(captVar(var.name)) ;
           fault.back() += n_attribute + " name=" + var.attName[j] ;
           fault.back() += " should not contain character " ;
           fault.back() += var.attName[j][k] ;
           break;
         }
       }
     }
   }

   for( size_t i=0 ; i < fault.size() ; ++i )
   {
     if( notes->inq(bKey + "23a") )
     {
       std::string capt(fault[i]) ;

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
   }

   return;
}

void
CF::chap2_3_reco(void)
{
   // no two dimensions, variables, nor attributes
   // should be identical when case is ignored

   std::string case_ins;
   std::string case_sens;
   std::vector<std::string*> type;
   std::vector<std::string*> myAtt;
   std::vector<std::string> what;

   // dimensions
   std::vector<std::string> dims( dimensions );
   size_t sz=dims.size();

   for( size_t i=0 ; i < sz ; ++i )
   {
     case_ins = hdhC::Lower()(dims[i]);

     for( size_t j=i+1 ; j < sz ; ++j )
     {
       case_sens = hdhC::Lower()(dims[j]);

       if( case_sens == case_ins )
       {
          type.push_back(&n_dimension) ;
          what.push_back(captVal(dims[i]) + "and" + captVal(dims[j],false));
       }
     }
   }

   sz=pIn->varSz;
   for( size_t i=0 ; i < sz ; ++i )
   {
     Variable& var = pIn->variable[i];

     case_ins = hdhC::Lower()(var.name);

     // variables
     for( size_t j=i+1 ; j < sz ; ++j )
     {
       case_sens = hdhC::Lower()(pIn->variable[j].name);

       if( case_sens == case_ins )
       {
          type.push_back(&n_variable) ;
          what.push_back(captVal(var.name)
               + "and" + captVal(pIn->variable[j].name, false));
       }
     }

     // attributes
     size_t asz=var.attName.size();
     for( size_t ai=0 ; ai < asz ; ++ai )
     {
       case_ins = hdhC::Lower()(var.attName[ai]);

       // variables
       for( size_t aj=ai+1 ; aj < asz ; ++aj )
       {
         case_sens = hdhC::Lower()(var.attName[aj]);

         if( case_sens == case_ins )
         {
            type.push_back(&var.name) ;
            what.push_back(captVal(var.attName[ai])
                 + "and" + captVal(var.attName[aj],false));
         }
       }
     }
   }

   std::string tag(bKey + "23b");

   for( size_t i=0 ; i < what.size() ; ++i )
   {
      if( notes->inq(tag) )
      {
        std::string capt("Reco ");

        if( *(type[i]) == n_dimension )
          capt += "for dimensions: " ;
        else if( *(type[i]) == n_variable )
          capt += "for variables: " ;
        else
          capt += "for attributes of " + captVar(*(type[i])) ;

        capt += "Avoid same names when case is ignored, found";
        capt += what[i];

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
   }

   return;
}

void
CF::chap2_4(void)
{
   if( !isCheck )
     return;

   // the dimensions of a variable must have different names

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     bool is=false;

     Variable& var = pIn->variable[i];

     int sz = var.dimName.size();

     for( int j=0 ; j < sz - 1 ; ++j )
     {
       for( int k=j+1 ; k < sz ; ++k )
       {
         if( var.dimName[j] == var.dimName[k] )
         {
            is=true ;
            break;
         }
       }

       if( is )
         break;
     }

     if( is )
     {
       if( notes->inq(bKey + "24a", var.name) )
       {
         std::string capt(captVar(var.getDimNameStr(true)));
         capt += "Dimensions should have different names" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
   }

   return;
}

void
CF::chap2_4_reco(void)
{
   // order of dimensions: T, Z, Y, X
   std::vector<int> dIx;

   // 2nd check: anything else should be on the left side of the dim string
//   size_t any_ixMax; // the corresponding recommended rule is not feasable
                       // when T is unlimited
   size_t tzyx_ixMin;
   size_t sz;

   for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
   {
      Variable& var = pIn->variable[ix];

      if( (sz=var.dimName.size()) < 2 )
        continue;

      tzyx_ixMin=sz;

      for( size_t i=0 ; i < sz ; ++i )
      {
         if( pIn->varNameMap.count(var.dimName[i]) )
         {
           Variable& dvar
              = pIn->variable[ pIn->varNameMap[pIn->variable[ix].dimName[i]] ];

           if( dvar.coord.isT )
           {
             dIx.push_back(static_cast<int>(0));
             if( tzyx_ixMin > i )
               tzyx_ixMin = i ;
           }
           else if( dvar.coord.isZ || dvar.coord.isZ_DL )
           {
             dIx.push_back(static_cast<int>(1));
             if( tzyx_ixMin > i )
               tzyx_ixMin = i ;
           }
           else if( dvar.coord.isY )
           {
             dIx.push_back(static_cast<int>(2));
             if( tzyx_ixMin > i )
               tzyx_ixMin = i ;
           }
           else if( dvar.coord.isX )
           {
             dIx.push_back(static_cast<int>(3));
             if( tzyx_ixMin > i )
               tzyx_ixMin = i ;
           }
         }
      }

      // the sequence of found indices must ascend
      for(size_t j=1 ; j < dIx.size() ; ++j)
      {
        if( dIx[j-1] > dIx[j] )
        {
          std::string tag(bKey + "24b");
          if( notes->inq(tag, var.name) )
          {
            std::string capt("reco: The order of " + n_dimension + "s of ");
            capt += captVar(var.getDimNameStr(true), false);
            capt += "should be T,Z,Y,X" ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }

          break;
        }
      }

      dIx.clear();

      // recommendation conforming to the COARDS subset of CF:
      // any variable other than TZYX should be added "to the left"

/*
      if( any_ixMax > tzyx_ixMin )
      {
        if( notes->inq(bKey + "24c", var.name) )
        {
          std::string capt("reco: ");
          capt += var.name + ": other " + n_dimension + "s than T,Z,Y,X should be on the left" ;
          std::string text("found ");
          text += var.getDimNameStr(true);

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }
      }
*/
   }

   return;
}

void
CF::chap2_5_1(void)
{
  double minVal, maxVal, range[2];

   // missing data
   for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
   {
      Variable& var = pIn->variable[ix];

      int jxMV     = var.getAttIndex(n_missing_value) ;

      if( jxMV > -1 && cFVal == 14)
      {
        if( notes->inq(bKey + "251c", NO_MT) )
        {
          std::string capt("reco for CF-1.4: " + captAtt(n_missing_value));
          capt += " is deprecated";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      int jxFV     = var.getAttIndex(n_FillValue) ;
      int jxVRange = var.getAttIndex(n_valid_range) ;
      int jxVMin   = var.getAttIndex(n_valid_min);
      int jxVMax   = var.getAttIndex(n_valid_max);

      if( jxVMin > -1 )
        minVal=hdhC::string2Double(var.attValue[jxVMin][0]) ;
      if( jxVMax > -1 )
        maxVal=hdhC::string2Double(var.attValue[jxVMax][0]) ;
      if( jxVRange > -1 )
      {
        range[0] = hdhC::string2Double(var.attValue[jxVRange][0]) ;
        range[1] = hdhC::string2Double(var.attValue[jxVRange][1]) ;
      }

      std::vector<std::string> mfName;
      std::vector<std::string> mfvStr;
      std::vector<double> mfv;

      if( jxFV > -1 && jxMV > -1 )
      {
        mfName.push_back(n_missing_value);
        mfName.push_back(n_FillValue);

        mfvStr.push_back(var.attValue[jxMV][0]);
        mfv.push_back(hdhC::string2Double(mfvStr.back()) );

        mfvStr.push_back(var.attValue[jxFV][0]);
        mfv.push_back(hdhC::string2Double(mfvStr.back()) );

        if( mfvStr[0] == mfvStr[1] )
           mfName[0] = s_empty ;
      }
      else if( jxMV > -1 )
      {
         mfName.push_back(n_missing_value);
         mfName.push_back(s_empty);
         mfvStr.push_back(var.attValue[jxMV][0]);
         mfvStr.push_back(s_empty);
         mfv.push_back(hdhC::string2Double(mfvStr[0]) );
      }
      else if( jxFV > -1 )
      {
         mfName.push_back(s_empty);
         mfName.push_back(n_FillValue);
         mfvStr.push_back(s_empty);
         mfvStr.push_back(var.attValue[jxFV][0]);
         mfv.push_back( 0. );
         mfv.push_back(hdhC::string2Double(mfvStr[1]) );
      }

      for( size_t k=0 ; k < mfvStr.size() ; ++k )
      {
         if( followRecommendations && mfName[k].size() == 0 )
           continue;  // not provided

         //_FillValue/missing_value should not be within a specified valid_range
         if( jxVRange > -1 )
         {
           if( mfv[k] >= range[0] && mfv[k] <= range[1] )
           {
             if( notes->inq(bKey + "251a", var.name) )
             {
               std::string capt("reco: ");
               capt += captAtt(var.name,hdhC::sAssign(mfName[k], mfvStr[k]),
                               s_upper, no_colon);
               capt += "should not be within ";
               capt += hdhC::sAssign(n_valid_range,
                      var.attValue[jxVRange][0] + ", "
                          + var.attValue[jxVRange][1]);

               (void) notes->operate(capt) ;
               notes->setCheckCF_Str( fail );
             }
           }

           else if( jxVMin > -1 && jxVMax > -1 )
           {
             if( minVal < mfv[k] && maxVal > mfv[k] )
             {
               if( notes->inq(bKey + "251a", var.name) )
               {
                 std::string capt("reco: ");
                 capt += captAtt(var.name, mfName[k], no_colon, s_upper) ;
                 capt += "should not be within the range specified by ";
                 capt += n_valid_min + " and " + n_valid_max  ;

                 std::string text("Found: " + hdhC::sAssign(mfName[k], mfvStr[k]));
                 text += ", [" + hdhC::sAssign(n_valid_min, var.attValue[jxVMin][0]) ;
                 text += ", " + hdhC::sAssign(n_valid_max, var.attValue[jxVMax][0]) + "]" ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }

           else if( jxVMax > -1 )
           {
             if( maxVal > mfv[k] )
             {
               if( notes->inq(bKey + "251a", var.name) )
               {
                 std::string capt("reco: ");
                 capt += captAtt(var.name, mfName[k], no_colon, s_upper) ;
                 capt += " should not be smaller than the " ;
                 capt += n_valid_max +" value" ;

                 std::string text("Found: " + hdhC::sAssign(mfName[k], mfvStr[k]));
                 text += ", " + hdhC::sAssign(n_valid_max, var.attValue[jxVMax][0]) ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }
           else if( jxVMin > -1 )
           {
             if( minVal < mfv[k] )
             {
               if( notes->inq(bKey + "251a", var.name) )
               {
                 std::string capt("reco for ") ;
                 capt += captAtt(var.name, mfName[k], no_colon) ;
                 capt += " should not be larger than the ";
                 capt += n_valid_min + " value" ;

                 std::string text("Found: " + hdhC::sAssign(mfName[k], mfvStr[k]));
                 text += ", " + hdhC::sAssign(n_valid_min, var.attValue[jxVMin][0]) ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }
         }
      }

      if( jxVRange > -1 && ( jxVMin > -1 || jxVMax > -1) )
      {
         std::string  text[2];
         std::string* ptext[2];
         std::string* p_vm[2];
         std::string* p_vr[2];
         bool is[2];

         text[0] = "Maximum" ;
         text[1] = "Minimum" ;
         ptext[0] = &n_valid_max;
         ptext[1] = &n_valid_min;
         p_vm[0] = &var.attValue[jxVMax][0];
         p_vm[1] = &var.attValue[jxVMin][0];
         p_vr[0] = &var.attValue[jxVRange][1];
         p_vr[1] = &var.attValue[jxVRange][0];
         is[0] = jxVMax > -1 && range[1] != maxVal;
         is[1] = jxVMin > -1 && range[0] != minVal;

         for( size_t i=0 ; i < 2 ; ++i)
         {
           if( is[i] && notes->inq(bKey + "251b", var.name) )
           {
             std::string capt("warning: " + text[i] + " of ");
             capt += captAtt(var.name,hdhC::sAssign(n_valid_range, *(p_vr[i])),
                             no_colon);
             capt += "and " ;
             capt += hdhC::sAssign(*(ptext[i]), *(p_vm[i]) ) ;
             capt += " are different";

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }
      }
  }

  return;
}

void
CF::chap2_6(void)
{
   if( !isCheck )
     return;

   // standardised attributes and types

   // from CF-1.4 Appendix A
   // attribute names are stored in CF::attName
   // attribute types are stored in CF::attType
   //      with S: string, N: numeric, D: data type

   // note that there is no explicit rq_chap2_6_2; it is already
   // checked here.

   // indices of variables and attributes
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     for( size_t j=0 ; j < pIn->variable[i].attName.size() ; ++j)
     {
       std::string &aName = pIn->variable[i].attName[j];

       if( pIn->variable[i].attValue[j][0].size() == 0 && notes->inq(bKey + "0g") )
       {
         std::string capt(captAtt(var.name, aName, no_colon) + "is empty");

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }

       for( size_t k=0 ; k < CF::attName.size() ; ++k )
       {
         if( aName == CF::attName[k] )
         {
            if( CF::attType[k] == 'D' )
            {
               if( var.attType[j] != var.type )
               {
                 if( notes->inq(bKey + "26a", var.name) )
                 {
                   std::string capt(captVar(var.name, false));
                   capt += " and " + captAtt(aName);
                   capt += "have to be the same type";

                   std::string text("type of ");
                   text += var.name + ":";
                   text += hdhC::sAssign(aName, pIn->nc.getTypeStr(var.attType[j])) ;
                   text += "\nType of " ;
                   text += hdhC::sAssign(n_variable, pIn->nc.getTypeStr(var.type)) ;

                   (void) notes->operate(capt, text) ;
                   notes->setCheckCF_Str( fail );
                 }
               }
            }

            else if( CF::attType[k] == 'S' )
            {
               if( var.attType[j] != NC_CHAR )
               {
                 if( notes->inq(bKey + "26b", var.name) )
                 {
                   std::string capt(captAtt(var.name, aName, no_colon));
                   capt += "should be character type, found ";
                   capt += pIn->nc.getTypeStr(var.attType[j]) ;

                   (void) notes->operate(capt) ;
                   notes->setCheckCF_Str( fail );
                 }
               }
            }

            else if( CF::attType[k] == 'N' )
            {
               if( ! pIn->nc.isNumericType(var.name, CF::attName[k] ) )
               {
                 if( notes->inq(bKey + "26c", var.name) )
                 {
                   std::string capt(captAtt(var.name, aName, no_colon));
                   capt += "should be numeric type, found ";
                   capt += pIn->nc.getTypeStr(var.attType[j]) ;

                   (void) notes->operate(capt) ;
                   notes->setCheckCF_Str( fail );
                 }
               }
            }

            break;  // try next attribute
         }
       }
     }
   }

   return;
}

bool
CF::chap2_6_1(void)
{
  // find the CF Conventions ID
  std::string glob_cv;
  std::string aName;

  int j;
  if( (j=pIn->getVarIndex(n_NC_GLOBAL)) > -1 )
  {
    std::string n_att(n_Conventions);
    glob_cv = pIn->variable[j].getAttValue(n_att, lowerCase) ;
  }

  if( glob_cv.size() == 0 )
  {
    if( notes->inq(bKey + "261a") )
    {
      std::string capt("missing global ");
      capt += captAtt(s_empty, n_Conventions, no_colon) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }
  else if( glob_cv.substr(0,3) != "cf-" )
  {
    std::string tag(bKey+"261b");
    if( notes->inq(tag) )
    {
      std::string capt("unknown global ") ;
      capt += captAtt(s_empty, n_Conventions, no_colon, no_blank);
      capt += hdhC::sAssign(glob_cv);

      // test whether "CF-" is missing
      if( glob_cv.size() > 2 )
      {
        if( hdhC::isDigit(glob_cv[0])
            && glob_cv[1] == '-' && hdhC::isDigit(glob_cv[2]) )
        {
           glob_cv[1] = '.' ;  // correction for internal use
           capt += ", expected " ;
           glob_cv = "CF-" + glob_cv ;
           capt += glob_cv ;
        }
      }

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  if( cFVersion.size() )
    setCheck(cFVersion); // option-provided convention supersedes
  else if( glob_cv.size() )
  {
    setCheck(glob_cv);

    if( cFVal == 14 )
      cFVersion="CF-1.4" ;
    else if( cFVal == 16 )
      cFVersion="CF-1.6" ;
  }
  else
  {
    cFVersion="CF-1.4";
    setCheck(cFVersion); // by default
  }

  return true;
}

void
CF::chap2_6_2_reco(void)
{
  // title and history should be global atts. In case there are none,
  // check whether they exist per variable.
  bool isNotTitle=true;
  bool isNotHistory=true;

  std::string t("title");
  std::string h("history");
  std::string *p[] = { 0, 0};

  for( int ix=pIn->variable.size()-1 ; ix > -1  ; --ix )
  {
     Variable& var = pIn->variable[ix];

     if( var.name == n_NC_GLOBAL)
     {
       isNotTitle   = ! var.isValidAtt(t);
       isNotHistory = ! var.isValidAtt(h);

       if( !isNotTitle && !isNotHistory )
          return; // both are global atts

       continue;
     }

     if( var.isValidAtt(t) )
       p[0] = &t;
     if( var.isValidAtt(h) )
       p[1] = &h;

     for( size_t k=0 ; k < 2 ; ++k )
       if( p[k] )
          p[k]=0;
  }

  return;
}

void
CF::chap3(void)
{
  if( ! notes->inq(bKey + "3", s_empty, "INQ_ONLY" ) )
     return ;  // checks are selected to be discarded by 'D'

  chap3_3() ; // standard_name; include look-up to xml table
  chap3_4();  // ancillary_variables
  chap3_5();  // flags

  return;
}

void
CF::chap3_reco(void)
{
  // all variables should use either the long_name or the standard_name
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix];

     // these are exceptions
     if( var.boundsOf.size() || var.isMapVar || var.isCompress || var.isLabel )
       continue;

     if( ! (var.isValidAtt(n_standard_name) || var.isValidAtt(n_long_name) ) )
     {
        // there is an exception for grid_mapping variables
        if( ! var.isValidAtt(n_grid_mapping +"_name") )
        {
          if( notes->inq(bKey + "32a", var.name) )
          {
            std::string capt("reco for " + captVar(var.name));
            capt += "Use "  + n_standard_name + " or " + n_long_name;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
     }
  }

  chap3_5_reco() ;  // flags
  return;
}

void
CF::chap3_3(void)
{
   // standard_name and some more, comparable to
   // http://cf-pcmdi.llnl.gov/documents/cf-standard-names/cf-standard-name-table.xml,
   // which was read in getSN_TableEntry().

   // note that units, amip and/or grib are only checkable for a valid standard_name.

   // any annotation for a standard_name
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      if( var.std_name.size() == 0 )
        continue;

      // take into account that a standard name may contain blanks (error)
      // or a modifier may be appended
      if( ! var.snTableEntry.found )
      {
        if( notes->inq(  bKey + "33a", var.name) )
        {
          std::string capt(captAtt(var.name,hdhC::sAssign(n_standard_name,
                                      var.std_name), no_colon));
          capt += "is not CF conform" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue; // disable the following units check
      }

      if( var.snTableEntry.remainder.size() || var.snTableEntry.hasBlanks )
        checkSN_Modifier(var) ;

      // units convertable to the canonical one. Note that udunits can not
      // discern degrees_north and degrees_east. This will be done in chap4_1.

      // special: standard_name with modifier=number_of_observations requires units=1
      if( var.units.size() && var.snTableEntry.canonical_units.size() )
      {
        bool ordCase = cmpUnits( var.units, var.snTableEntry.canonical_units) ;
        bool spcCase = var.snTableEntry.remainder == n_number_of_observations ;
        bool dimlessZ=false;
        if( var.units == "level" && var.snTableEntry.canonical_units == "1" )
           dimlessZ = true;

        if( !ordCase && !spcCase && !dimlessZ )
        {
          if( notes->inq(bKey + "33e", var.name) )
          {
            std::string capt(captAtt(var.name,hdhC::sAssign(n_units, var.units),
                                     no_colon));
            capt += "is not CF compatible with " ;
            capt += hdhC::sAssign(n_standard_name, var.snTableEntry.std_name) ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }

        if(spcCase && var.units.size() && var.units != "1" )
        {
          if( notes->inq(bKey + "33d", var.name) )
          {
            std::string capt(captAtt(var.name, n_standard_name) );
            capt += "The ";
            capt += hdhC::sAssign("modifier", n_number_of_observations);
            capt += " requires units=1, found";
            capt += captVal(var.units, false) ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }

      // amip
      int j;
      if( (j=var.getAttIndex("amip", lowerCase)) > -1 )
      {
        Split x_amip(var.snTableEntry.amip );
        std::vector<std::string> vs(x_amip.getItems());

        if( !hdhC::isAmong(var.attValue[j][0], vs) )
        {
          if( notes->inq(bKey + "33f", var.name) )
          {
            std::string capt(captVar(var.name));
            capt += hdhC::sAssign("AMIP code", var.attValue[j][0]) ;
            capt += " is not CF compatible, expected " ;
            capt += var.snTableEntry.amip ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }

      // grib
      if( (j=var.getAttIndex("grib", lowerCase)) > -1 )
      {
        Split x_grib(var.snTableEntry.grib );
        std::vector<std::string> vs(x_grib.getItems());

        if( !hdhC::isAmong(var.attValue[j][0], vs) )
        {
          if( notes->inq(bKey + "33g", var.name) )
          {
            std::string capt(captVar(var.name));
            capt += hdhC::sAssign("GRIP code", var.attValue[j][0]);
            capt += " is not CF compatible, expected " ;
            capt += var.snTableEntry.grib;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
   }

   return;
}

void
CF::chap3_4(void)
{
   if( !isCheck )
     return;

   // ancillary_variables
   // there is nothing to check; the attribute just expresses relationships
   // between non-coordinate variables.

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     int j;
     if( (j=var.getAttIndex(n_ancillary_variables)) > -1 )
     {
       std::string &str = var.attValue[j][0];
       Split x_av(str);

       // scan over declared variable names
       for( size_t jj=0 ; jj < x_av.size() ; ++jj )
       {
         int ii = pIn->getVarIndex(x_av[jj]);

         if( ii == -1 )
         {
           if( notes->inq(bKey + "34a", var.name) )
           {
             std::string capt(captAtt(var.name, n_ancillary_variables, no_colon));
             capt += "declares " + captVar(x_av[jj],no_colon, no_blank) ;
             capt += ", which is not in the file" ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }
       }
     }
   }

   return;
}

void
CF::chap3_5(void)
{
   if( !isCheck )
     return;

   // flags
   std::vector<std::string> f_name;
   f_name.push_back(n_flag_values);
   f_name.push_back(n_flag_masks);
   f_name.push_back("flag_meanings");

   std::vector<std::string> *pf[3] ;
   pf[0]= 0 ;  //flag_values
   pf[1]= 0 ;  //flag_masks
   pf[2]= 0 ;  //flag_meanings

   Split x_f_meanings;

   // variables
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     bool flagFound=false;

     for( size_t j=0 ; j < var.attName.size() ; ++j )
     {
       std::string &aName = var.attName[j];

       if( aName == f_name[0] )
       {
         pf[0] = &var.attValue[j] ;
         flagFound=true;
       }
       else if( aName == f_name[1] )
       {
         pf[1] = &var.attValue[j] ;
         flagFound=true;
       }
       else if( aName == f_name[2] )
       {
         pf[2] = &var.attValue[j] ;
         x_f_meanings = var.attValue[j][0] ;
         flagFound=true;
       }
     }

     // valid for both flag_values and flag_masks
     if( flagFound )
     {
        // comparison of types:
        // flag_values and flag_masks, resp., must have the same type
        // as the corresponding variable. This was checked in chap26

        if(pf[0])
        {
          // the values of flag_values must be mutually exclusive
          // note that f_type still holds the type of flag_masks
          bool is=false;
          int sz=static_cast<int>(pf[0]->size());
          for( int l0=0 ; l0 < sz-1 ; ++l0 )
          {
            for( int l1=l0+1 ; l1 < sz ; ++l1 )
            {
              // note that pf[0] is just a pointer to a vector of a vector
              if( (*pf[0])[l0] == (*pf[0])[l1] )
              {
                 is=true;
                 break;
              }
            }

            if( is )
               break;
          }

          if( is )
          {
            if( notes->inq(bKey + "35e", var.name) )
            {
              std::string capt(captVar(var.name));
              capt += "Values of flag_values have to be mutually exclusive";

              std::string text("found ");
              text += var.name + ":flag_values=" ;
              for( int l=0 ; l < sz ; ++l )
              {
                 if( l )
                   text += ", ";
                 text += (*pf[0])[l] ;
              }

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }

        if(pf[1])
        {
          // a variable with flag_mask att must be compatible with bit field
          // expression (char, byte, short, int)

          // note that f_type still holds the type of flag_masks

          if( ! (var.type == pIn->nc.isIndexType(var.name)
                     || var.type == NC_CHAR ) )
          {
            if( notes->inq(bKey + "35h", var.name) )
            {
              std::string capt("type of " + captVal(var.name));
              capt += "has to be bit-field-expression compatible";

              std::string text("type of ");
              text += var.name + "is" ;
              text += pIn->nc.getVarTypeStr(var.name) ;

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
            }
          }

          // the flag_masks att values must be non-zero; we don't care the correct type,
          // but we can't use the values in pf[1]
          std::vector<int> v_i;
          pIn->nc.getAttValues(v_i, n_flag_masks, var.name);

          for( size_t l=0 ; l < v_i.size() ; ++l )
          {
             if( v_i[l] == 0 )
             {
               if( notes->inq(bKey + "35f", var.name) )
               {
                 std::string capt(captAtt(var.name, n_flag_masks, no_colon));
                 capt += "should be non-zero" ;

                 (void) notes->operate(capt) ;
                 notes->setCheckCF_Str( fail );
               }

               break;
             }
          }
        }

        if( x_f_meanings.size() == 0 && pf[0]->size() )
        {
          // if flag_values is present, then the flag_meanings att must be specified
          if( notes->inq(bKey + "35c", var.name) )
          {
            std::string capt(captAtt(var.name, "flag_values"));
            capt += "Missing " + captAtt("flag_meanings", no_colon);

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }

        // number of items for flag_values and flag_mask, resp., must equal
        // the number of flag_meanings
        for( size_t k0=0 ; k0 < 2 ; ++k0 )
        {
          if(pf[k0] == 0 )
            continue;

          for( size_t k1=k0+1 ; k1 < 3 ; ++k1 )
          {
            if(pf[k1] == 0 )
              continue;

            size_t sz = k1 == 2 ? x_f_meanings.size() : pf[k1]->size() ;

            if( pf[k0]->size() != sz )
            {
              if( notes->inq(bKey + "35a", var.name) )
              {
                std::string capt(captVar(var.name));
                capt += "Different numbers of items in ";
                capt += n_attribute + "s " ;
                capt += hdhC::sAssign(f_name[k0],hdhC::itoa(pf[k0]->size())) + " and ";
                capt += hdhC::sAssign(f_name[k1],hdhC::itoa(sz)) ;

                (void) notes->operate(capt) ;
                notes->setCheckCF_Str( fail );
              }
            }

            k0=3;
            break;
          }
        }

        if( cFVal > 14 )
        {
           // flag_meanings must consist of alphanumerics and _,-,.,+,@
           bool is=false;
           std::string &str = (*pf[2])[0] ;
           char c;
           for(size_t j=0 ; j < str.size() ; ++j )
           {
              c= str[j];
              if( hdhC::isAlphaNum(c) )
                 continue;
              else if( c == '_' )
                 continue;
              else if( c == '-' )
                 continue;
              else if( c == '.' )
                 continue;
              else if( c == '+' )
                 continue;
              else if( c == '@' )
                 continue;
              else if( c == ' ' )
                 continue;

              is=true;
              break;
           }

           if( is && notes->inq(bKey + "35g", var.name) )
           {
              std::string t;
              t = c;
              std::string capt(cFVersion + " for " + captVar(var.name) );
              capt += captAtt(s_empty, "flag_meanings", s_upper, no_colon) ;
              capt += "has to consist of alpha-numerics," ;
              capt += " '_', '-', '.', '+', and '@'. Found " + t ;

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
          }
        }

        x_f_meanings.clear();
        pf[0]=0;
        pf[1]=0;
        pf[2]=0;
     }
   }

   return;
}

void
CF::chap3_5_reco(void)
{
   // flags
   // when flag_masks and flag_values are both defined, the boolean AND
   // of each entry in flag_values with its corresponding entry in flag_masks
   // should equal the flag_entry vallue

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     size_t m_ix, v_ix;

     if( (v_ix=var.isValidAtt(n_flag_values))
           && (m_ix=var.isValidAtt(n_flag_masks)) )
     {

        std::vector<unsigned int> v_uc;
        pIn->nc.getAttValues(v_uc, n_flag_values, var.name );

        std::vector<unsigned int> m_uc;
        pIn->nc.getAttValues(m_uc, n_flag_masks, var.name );

        if( m_uc.size() == v_uc.size() )
        {
           unsigned int res;
           for( size_t j=0 ; j < m_uc.size() ; ++j )
           {
              res = v_uc[j] & m_uc[j] ;
              if( res != v_uc[j] )
              {
                 if( notes->inq(bKey + "35i", var.name) )
                 {
                    std::string capt("reco for " + captVar(var.name));
                    capt += "The boolean AND of each flag_values and flag_masks ";
                    capt += "should equal the flag_values entry";

                    (void) notes->operate(capt) ;
                    notes->setCheckCF_Str( fail );
                 }
                 break;
              }
           }
        }
     }
   }

   return;
}

void
CF::chap4(void)
{
  // Coordinate Types
  if( ! notes->inq(bKey + "4", s_empty, "INQ_ONLY" ) )
     return ;  // checks are selected to be discarded by 'D'

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix];

     // check (aux) coordinate vars, eventually the data
     if( !var.isChecked && chap4(var) )
     {
        bool is = var.coord.isCoordVar ? true : false ;

        // aux coord vars for chap9 features are tested elsewhwere
        if( cFVal > 15 && !is )
          continue ;

        // false: disabled monotony test
        checkCoordinateValues(var);
     }
  }

  return;
}

bool
CF::chap4(Variable& var)
{
  if( var.isCompress )
     return false;

  hasBounds(var);

  // identifying bounds before checking coordinate types
  // simplifies things.
  if( isBounds(var) )
    return false;

// Coordinate Types: functions return true if identified

  if( chap4_4(var) )  // time
    return true;

  if( chap4_1(var) )  // lat/lon or rotated coord
    return true;

  // note that Z with dimensions for height in units=m is the most difficult
  if( chap4_3(var) )  // vertical coord, also dimensionless
    return true;

  return false;
}

bool
CF::chap4_1(Variable& var)
{
  std::string units(var.getAttValue(n_units)) ;
  std::string sn( var.getAttValue(n_standard_name, lowerCase) );
  std::string axis( var.getAttValue(n_axis) );

  // possible return values: lon, lat, rlon, rlat, size==0
  std::string coordType( units_lon_lat(var, units) );

  bool isType[2]={false, false};

  if( coordType == n_longitude || coordType == "rlon" )
    isType[0]=true;
  else if( coordType == n_latitude || coordType == "rlat" )
    isType[1]=true;

  bool isAx[2]={false, false};  // optional, but if, then axis=X or Y

  bool isSN[2]={false, false};

  int count[2]={0, 0};

  if( isType[0] )
    count[0] = 1;
  else if( isType[1] )
    count[1] = 1;

  // no standard_name? try the long-name
  if( sn.size() == 0 )
    sn = var.getAttValue(n_long_name) ;

  if( sn.size() )
  {
    if( sn == n_longitude )
    {
      isSN[0]=true;
      ++count[0] ;
    }
    else if( sn == n_latitude )
    {
      isSN[1]=true;
      ++count[1] ;
    }
    else
    {
      --count[0];
      --count[1];
    }
  }

  // axis is optional, but if provided, then X/Y/Z
  if( axis.size() )
  {
    if( axis == "Y" || axis == "y" )
    {
      isAx[1] = true;
      ++count[1] ;
    }
    else if( axis == "X" || axis == "x" )
    {
      isAx[0] = true;
      ++count[0] ;
    }
  }

  for( size_t i=0 ; i < 2 ; ++i )
  {
    if( isType[i] && isSN[i] )
      count[i] += 2 ;  // weighted
    if( isType[i] && isAx[i] )
      ++count[i] ;
    if( isSN[i] && isAx[i] )
      ++count[i] ;
  }

  // decision whether X, Y or neither
  int ix = (count[0] > count[1]) ? 0 : 1 ;

  if( count[ix] < 2 )
    return false;

  var.addAuxCount(count[ix]);
  var.isChecked=true;

  if( ix )
    var.coord.isY=true;
  else
    var.coord.isX=true;

  if( !isCheck )
    return true;

  if( var.units.size() == 0 )
  {
    if( isSN[ix] )
    {
      if( notes->inq(bKey + "41b", var.name) )
      {
        std::string capt(captVar(var.name, no_colon) + "was rated a " );
        if( sn == n_grid_latitude || sn == n_grid_longitude )
          capt += "rotational " ;

        if( sn == n_longitude || sn == n_grid_longitude )
          capt += n_longitude ;
        else if( sn == n_latitude || sn == n_grid_latitude)
          capt += n_latitude ;

        capt += " coordinate, but " + captAtt(n_units) + "is missing";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( !isSN[ix] && isType[ix] )
  {
    if( coordType == "rlon" || coordType == "rlat" )
    {
      if( notes->inq(bKey + "41c", var.name) )
      {
        std::string capt(captVar(var.name) + "was rated a rotational " );
        if( isType[ix] )
          capt += n_longitude ;
        else
          capt += n_latitude ;

        capt += " coordinate, but the " + n_standard_name + " is missing";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( units.size() && isSN[ix] )
  {
    bool is=false;
    if( isType[ix] )
    {
      std::string sn_coordType( units_lon_lat(var, var.snTableEntry.canonical_units) );
      if( sn_coordType != coordType )
        is=true;
    }
    else
      is=true;

    if( is && notes->inq(bKey + "41a", var.name) )
    {
      std::string capt ;
      if( isSN[ix] )
      {
        capt = captAtt(var.name,
                       hdhC::sAssign(n_standard_name, var.std_name), no_colon) ;
      }
      else
        capt = captAtt(var.name,n_axis+captVal(axis), no_colon) ;

      capt +=  "is not compatible with ";
      capt += hdhC::sAssign(n_units, var.units) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return true;
}

bool
CF::chap4_3(Variable& var)
{
  // vertical coordinate

  // the case of a dimensionless vertical coordinate was checked elsewhere
  if( chap4_3_1(var) )  // n-dimensional (n>1)  or scalar?
    return true;

  return false;
}

bool
CF::chap4_3_1(Variable& var)
{
  // dimensional vertical coordinate.

  // 1) units must be available
  // 2) units==pressure or attribute positive is present
  // Optionally:
  //   3) appropriate standard_name
  //   4) axis == Z

  std::string units( var.getAttValue(n_units) ) ;
  std::string axis( var.getAttValue(n_axis, lowerCase) ) ;
  std::string positive( var.getAttValue(n_positive, lowerCase) ) ;
  std::string& canon_u = var.snTableEntry.canonical_units;

  bool isP=false;
  bool isM=false;
  bool isAx=false;

  int countZ=0;
  int countLimit=1;

  if( units.size() ) // required
  {
    if( (isP=cmpUnits(units, "Pa")) )
      ++countZ;
    else if( (isM=cmpUnits( units, "m")) )
      ++countZ;
  }

  // std_name with appropriate units; optional
  if( canon_u.size() )
  {
    if( cmpUnits(canon_u, "Pa") )
    {
      ++countZ;
      isP=true ;
    }
    else if( cmpUnits(canon_u, "m") )
    {
      ++countZ;
      isM=true ;
    }
  }

  bool isPos ;
  if( (isPos=(positive == "up" || positive == "down") ) )
    ++countZ;

  if( positive.size() )
    ++countZ;

  if( (isAx= axis == "z") )
    ++countZ;
  else if( axis.size() )
    --countZ;

  // some combinations indicating a vertical dimension
  if( isPos && (isP || isM) ) // positive and a kind of units
    ++countZ;
  if( isPos && isAx )  // positive and axis
      ++countZ;
  if( isAx && (isP || isM) )  // axis and units
      ++countZ;

  if(var.isDataVar() )
    --countZ;

  // finally
  if( isM )
    ++countLimit;

  if( countZ == countLimit )
  {
    if( var.coord.isCoordVar )
      ++countZ;

    for( size_t i=0 ; i < ca_vvs.size() ; ++i )
      if( static_cast<int>(i) != var.id && hdhC::isAmong(var.name, ca_vvs[i]) )
        ++countZ;
  }

  if( countZ > countLimit )
  {
    var.coord.isZ=true;
    var.isChecked=true;
    var.addAuxCount(countZ);
  }
  else
    return false;

  if( isCheck && units.size() == 0 )
  {
    if( !var.coord.isZ_DL )
    {
      // was is rated a dimless Z without fomrula_terms attribute?
      if( notes->inq(bKey + "43d", var.name) )
      {
        std::string capt(captVar(var.name, false));
        capt += "was rated a Z-coordinate, but ";
        capt += n_units + " are missing";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  // check properties and validity
  if( isCheck && !isP && !isPos )
  {
    if( positive.size() )
    {
      if( notes->inq(bKey + "43a", var.name) )
      {
        std::string capt(captAtt(var.name, n_positive, no_colon));
        capt += "should be Up|Down, found" ;
        capt += captVal(var.getAttValue(n_positive), false) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
    else if( notes->inq(bKey + "43c", var.name) )
    {
      std::string capt(captVar(var.name));
      capt += captAtt(s_empty,n_positive, s_upper, no_colon);
      capt += "is required for a " + n_dimension ;
      capt += "al Z-coordinate with non-pressure " + n_units ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return var.coord.isZ;
}

void
CF::chap4_3_2(void)
{
  // dimensionless vertical coordinate?

  // valid standard names
  std::vector<std::string> valid_sn;
  valid_sn.push_back("atmosphere_ln_pressure_coordinate");
  valid_sn.push_back("atmosphere_sigma_coordinate");
  valid_sn.push_back("atmosphere_hybrid_sigma_pressure_coordinate");
  valid_sn.push_back("atmosphere_hybrid_height_coordinate");
  valid_sn.push_back("atmosphere_sleve_coordinate");
  valid_sn.push_back("ocean_sigma_coordinate");
  valid_sn.push_back("ocean_sigma_coordinate");
  valid_sn.push_back("ocean_sigma_z_coordinate");
  valid_sn.push_back("ocean_double_sigma_coordinate");

  //variables in formula_terms; order doesn't matter
  std::vector<std::string> valid_ft;
  valid_ft.push_back("p0: lev:");
  valid_ft.push_back("sigma: ps: ptop:");
  valid_ft.push_back("a: ap: b: ps: p0:");
  valid_ft.push_back("a: b: orog:");
  valid_ft.push_back("a: b1: b2: ztop: zsurf1: zsurf2:");
  valid_ft.push_back("sigma: eta: depth:");
  valid_ft.push_back("s: eta: depth: a: b: depth_c:");
  valid_ft.push_back("sigma: eta: depth: depth_c: nsigma: zlev:");
  valid_ft.push_back("sigma: depth: z1: z2: a: href: k_c:");

  std::vector<int> ix ;  // collects all the candidates
  std::vector<int> ij_fT;
  std::vector<int> ij_sN;
  std::vector<int> ij_aV;

  std::vector<int> ix_sN ;
  std::vector<int> ix_aV ; // value of an attribute is appropriate
  std::vector<int> jx_sN ;
  std::vector<int> jx_aV ; // value of an attribute is appropriate

  // two ways to identify a dimless vertical coord.
  // a) a formula_terms attribute,
  // b) a standard name indicating a valid dimless method

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
     Variable& var = pIn->variable[i] ;
     int j;

     if( (j=var.getAttIndex(n_formula_terms)) > -1 )
     {
        ix.push_back(i);

        ij_fT.push_back(j);
        ij_sN.push_back(-1);
        ij_aV.push_back(-1);
     }

     if( (j=var.getAttIndex(n_standard_name)) > -1 )
     {
        if( hdhC::isAmong(var.attValue[j][0], valid_sn) )
        {
          ix_sN.push_back(i);
          jx_sN.push_back(j);
        }
     }

     Split x_av;
     for( size_t v=0 ; v < var.attName.size() ; ++v )
     {
        if( var.attName[v] == n_cell_measures
              || var.attName[v] == n_cell_methods )
          continue;

        x_av = var.attValue[v][0] ;
        size_t x_av_sz = x_av.size() ;

        if( x_av_sz )
        {
          bool is=true;

          for( size_t x=0 ; x < x_av_sz ; ++x )
          {
            size_t tail = x_av[x].size()-1;
            if( x_av[x++][tail] != ':' )
            {
              is=false;
              break;
            }

            tail = x_av[x].size()-1;
            if( x_av[x][tail] == ':' )
              is=false;
          }

          if( is )
          {
            ix_aV.push_back(i);
            jx_aV.push_back(v);
            break;
          }
        }
     }
  }

  // the sequence of tests must be: i), ii)

  // i) try attribute with appropriate format
  for( size_t i=0 ; i < ix_aV.size() ; ++i )
  {
    size_t j;
    for( j=0 ; j < ix.size() ; ++j )
    {
      if( ix_aV[i] == ix[j] )
      {
        if( ij_aV[j] == -1 )
          ij_aV[j] = jx_aV[i];

        break;
      }
    }

    if( j == ix.size() )
    {
      ix.push_back(ix_aV[i]) ;
      ij_fT.push_back(jx_aV[i]) ;
      ij_sN.push_back(-1) ;
    }
  }

  // ii) found appropriate standard name
  for( size_t i=0 ; i < ix_sN.size() ; ++i )
  {
    size_t j;
    for( j=0 ; j < ix.size() ; ++j )
    {
      if( ix_sN[i] == ix[j] )
      {
        if( ij_sN[j] == -1 )
          ij_sN[j] = jx_sN[i];

        break;
      }
    }

    if( j == ix.size() )
    {
      ix.push_back(ix_sN[i]) ;
      ij_fT.push_back(-1) ;
      ij_sN.push_back(jx_sN[i]) ;
    }
  }

  for( size_t i=0 ; i < ix.size() ; ++i)
  {
    Variable& var = pIn->variable[ix[i]];

    if( chap4_3_2(var, valid_sn, valid_ft, ij_fT[i], ij_sN[i]) )
    {
      var.coord.isZ    = true;
      var.coord.isZ_DL = true;
      var.addAuxCount();
      var.isChecked=true;
    }
  }

  return;
}

bool
CF::chap4_3_2(Variable& var,
   std::vector<std::string>& valid_sn,
   std::vector<std::string>& valid_ft,
   int att_ft_ix, int att_sn_ix )
{
  // dimensionless vertical coordinate
  int valid_ft_ix=-1;
  int valid_sn_ix=-1;

  int att_units_jx ;
  std::string units;
  if( (att_units_jx=var.getAttIndex(n_units)) > -1 )
  {
    // tolerated units for COARDS compatibility: level, layer, sigma_level;
    // would change deprecated units forms to "1"
    units = var.attValue[att_units_jx][0];
    chap4_3_2_deprecatedUnits(var, units) ;
  }

  // cross-check of standard_name vs. formula_terms
  if( chap4_3_2_checkSNvsFT(var, valid_sn, valid_ft, valid_sn_ix,
                   att_ft_ix, att_sn_ix, units) )
    return false; // no formula_terms case

  // formula_terms by vector of pairs: 1st=key: 2nd=param_varName
  std::vector<std::pair<std::string, std::string> > att_ft_pv ;

  chap4_3_2_getParamVars(var, valid_sn, valid_ft, valid_ft_ix, valid_sn_ix,
                   att_ft_ix, att_ft_pv) ;

  // sn and ft from the same valid algorithm?
  if( att_sn_ix > -1 && valid_ft_ix > -1 && valid_ft_ix != valid_sn_ix)
  {
    if( notes->inq(bKey + "432e", var.name) )
    {
      std::string capt(captVar(var.name));
      capt += "The attributes " + n_formula_terms + " and " ;
      capt += n_standard_name + " are not compatible";

      std::string text("found: " + var.name + ":") ;
      text += hdhC::sAssign(n_standard_name, var.attValue[att_sn_ix][0]) + " and " ;
      text += var.name + ":" + hdhC::sAssign(n_formula_terms, var.attValue[att_ft_ix][0]) ;

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  if( isCheck && att_units_jx > -1 )
  {
    if( units.size() && units != "1" )
    {
      if( notes->inq( bKey + "432b", var.name) )
      {
        std::string capt(captVar(var.name));
        capt += "Dimensionless vertical coordinate must not have " ;
        capt += n_units + captVal(units, false) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  std::vector<std::string> fTerms;
  Split x_fTerms( valid_ft[valid_ft_ix] );
  for( size_t i=0 ; i < x_fTerms.size() ; ++i )
     fTerms.push_back( x_fTerms[i] );

  // after return, fTerms contains associated variables.
  chap4_3_2_verify_FT(var,valid_ft_ix,
          valid_ft[ valid_ft_ix ], att_ft_ix, fTerms, att_ft_pv);

  size_t ft_sz = fTerms.size();

  if( ft_sz )
  {
     size_t j;
     for( j=0 ; j < ft_sz ; ++j )
       if( fTerms[j] == var.name )
         break;
     if( j == ft_sz )
       var.isChecked = true;  // data is not necessary

     for( j=0 ; j < fTerms.size() ; ++j )
     {
       int i ;

       if( (i=pIn->getVarIndex(fTerms[j])) > -1 )
       {
         pIn->variable[i].addAuxCount();

         // parameter variables should not have any _FillValue within the data body;
         // this is not mentioned in the CF doc.
         if( !pIn->variable[i].isChecked )
         {
           checkCoordinateValues(pIn->variable[i], true); // no monotony test

           pIn->variable[i].isChecked=true;
         }
       }
     }
  }

  return true;
}

bool
CF::chap4_3_2_checkSNvsFT( Variable& var,
   std::vector<std::string>& valid_sn,
   std::vector<std::string>& valid_ft,
   int& valid_sn_ix,
   int& att_ft_ix, int& att_sn_ix, std::string& units)
{
  // return true for no formula_terms.

  // note that in the case of a non_existing ft att, all the atts are searched for
  // a valid ft signature. So, att_ft_ix may return an index.

  // cross-check of standard_name vs. formula_terms (case: dimless vertical coord)
  // return true if no formula_term proceeding is possible
  if( att_sn_ix > -1 )
  {
    std::string str( hdhC::Lower()(var.attValue[att_sn_ix][0]) );

    for( size_t j=0 ; j < valid_sn.size() ; ++j )
    {
      if( str == valid_sn[j] )
      {
        valid_sn_ix = j ;
        break;
      }
    }
  }

  // if there is any appropriate value for a formula_terms attribute, but
  // the name of the attribute is different, then the att name was different
  if( att_ft_ix > -1 && var.attName[att_ft_ix] != n_formula_terms )
  {
    if( notes->inq(bKey + "432f", var.name) )
    {
      std::string capt(captVar(var.name));
      capt += "The signature of a " + n_formula_terms + blank + n_attribute;
      capt += " was found, but the name of the attribute is" ;
      capt += captVal(var.attName[att_ft_ix]) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  if( att_sn_ix == -1 )
  {
    if( att_ft_ix > -1 && notes->inq(bKey + "432d", var.name) )
    {
      std::string capt(captAtt(var.name, n_formula_terms, no_colon));
      capt += "is available, but ";
      capt += n_standard_name + " is missing";

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }
  else
  {
    if( att_ft_ix == -1 )
    {
      if( notes->inq(bKey + "432a", var.name) )
      {
        std::string capt(captVar(var.name));
        capt += hdhC::sAssign(n_standard_name, var.std_name) ;
        capt += " suggests a missing " + captAtt(n_formula_terms, no_colon) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      if( units.size() == 0 || units == "1" ) // another evidence of dim-less Z
      {
        var.coord.isZ = true;
        var.coord.isZ_DL = true;
        var.addAuxCount();
      }
    }
  }

  // no formula_terms attribute is given
  if( att_ft_ix == -1)
     return true;  // apparently no hybrid case

  return false;
}

void
CF::chap4_3_2_deprecatedUnits(Variable& var, std::string &units)
{
   // units level, layer and sigma_level are deprecated

  if( units == "level" || units == "layer" || units == "sigma_level" )
  {
    if( followRecommendations )
    {
      std::string tag(bKey+"31a");
      if( notes->inq(tag, var.name) )
      {
        std::string capt("reco: ");
        capt += captAtt(var.name,
                        hdhC::sAssign(n_units, units), no_colon, s_upper);
        capt += "is deprecated";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    units = "1" ;
  }

  return ;
}

void
CF::chap4_3_2_getParamVars( Variable& var,
   std::vector<std::string>& valid_sn,
   std::vector<std::string>& valid_ft,
   int& valid_ft_ix, int& valid_sn_ix,
   int att_ft_ix,
   std::vector<std::pair<std::string, std::string> >& att_ft_pv)
{
  // test for a valid ft att
  std::string& att_ft = var.attValue[att_ft_ix][0];
  Split x_att_ft(att_ft);

  // the check of the attribute appears cumbersome, because a faulty composition of
  // formula_terms should be recognised in detail (and annotated).
  std::vector<int> key_ix;
  std::vector<int> pv_ix;

  int sz=x_att_ft.size() ;
  for(int i=0 ; i < sz ; ++i )
  {
    std::string &s0 = x_att_ft[i] ;
    if( s0[s0.size()-1] == ':' )
      key_ix.push_back(i);
    else
      pv_ix.push_back(i);
  }

  // build a pair: 1st=key, 2nd=paramVar
  size_t kp_sz = key_ix.size() ;
  if( key_ix.size() < pv_ix.size() )
     kp_sz = pv_ix.size() ;

  for( size_t i=0 ; i < kp_sz ; ++i )
  {
    att_ft_pv.push_back( std::pair<std::string, std::string> (s_empty,s_empty) );

    if( i < key_ix.size() )
      att_ft_pv.back().first = x_att_ft[key_ix[i]] ;

    if( i < pv_ix.size() )
      att_ft_pv.back().second = x_att_ft[pv_ix[i]] ;
  }

  // just for passing the result
  bool isFault=false;
  if( key_ix.size() != pv_ix.size() )
    isFault=true;
  else
  {
    // test the sequence
    for( size_t i=0 ; i < key_ix.size() ; ++i )
    {
        if( !i &&  key_ix[i] != 0 )
          isFault=true;

        if( key_ix[i]+1 != pv_ix[i] )
          isFault=true;
    }
  }

  if( isFault )
    att_ft_pv.push_back( std::pair<std::string, std::string> (s_empty,s_empty) );

  // formula_terms attribute was found, now get the indices in the valid vectors.
  // This is more tricky for found_ft_ix than for found_sn_ix above.
  // Take into account the term: strings and
  // find the number of existing variables for each valid_ft element.
  std::vector<int> num;
  std::vector<Split> x_valid_ft;

  for( size_t i=0 ; i < valid_ft.size() ; ++i )
  {
     num.push_back( 0 );
     x_valid_ft.push_back( Split(valid_ft[i]) ) ;
  }

  // compare to the parameters provided by CF (appendix D)
  for( size_t i=0 ; i < att_ft_pv.size() ; ++i )
  {
     for( size_t j=0 ; j < valid_ft.size() ; ++j )
     {
       for( size_t k=0 ; k < x_valid_ft[j].size() ; ++k )
       {
         if( x_valid_ft[j][k] == att_ft_pv[i].first )
         {
            for( size_t iv=0 ; iv < pIn->varSz ; ++iv )
               if( pIn->variable[iv].name == att_ft_pv[i].second )
                 ++num[j];
         }
       }
     }
  }

  // sort indices according to the highest number
  size_t v_swap;
  std::vector<int> zx;
  sz=static_cast<int>(num.size());
  for( int i=0 ; i < sz ; ++i )
    zx.push_back(i);

  sz=static_cast<int>(zx.size());
  for( int i=0 ; i < sz-1 ; ++i )
  {
    for( int j=i+1 ; j < sz ; ++j )
    {
       if( num[zx[j]] >  num[zx[i]] )
       {
         v_swap = zx[i] ;
         zx[i] = zx[j];
         zx[j] = v_swap;
       }
    }
  }

  if( num[zx[0]] && num[zx[0]] == num[zx[1]] )
  {
     // could not identify unambiguously the valid_ft index. Thus, if one
     // of them matches valid_sn_ix, then take it.
     if( zx[0] == valid_sn_ix )
        valid_ft_ix = zx[0] ;
     else if( zx[1] == valid_sn_ix )
        valid_ft_ix = zx[1] ;
  }
  else
    valid_ft_ix = zx[0] ;

  return ;
}

void
CF::chap4_3_2_verify_FT(
  Variable& var,
  int valid_ft_ix,
  std::string &valid_ft,
  int att_ft_ix,
  std::vector<std::string> &fTerms,
  std::vector<std::pair<std::string, std::string> >& att_ft_pv)
{
  // formula_term contains 'terms:' and names of variables, perhaps omitting
  // particular terms.

  // Vector fTerms contains a list of term: strings.
  // Note that it will contain existing var-names on return.

  std::vector<std::string> valid_units;
  valid_units.push_back("Pa 1");
  valid_units.push_back("1 Pa Pa");
  valid_units.push_back("1 Pa 1 Pa Pa");
  valid_units.push_back("m 1 m");
  valid_units.push_back("1 1 1 m m m");
  valid_units.push_back("1 m m");
  valid_units.push_back("1 m m 1 1 m");
  valid_units.push_back("1 m m m 1 m");
  valid_units.push_back("1 m m m 1 m 1");

  Split x_fUnits(valid_units[valid_ft_ix]);

  std::vector<std::string> assoc;
  std::vector<std::pair<std::string, std::string> > paramVarUnits;

  // additional empty pair indicating a faulty sequence in ft
  size_t pvSz = att_ft_pv.size() ;
  if( pvSz )
    if( att_ft_pv[pvSz-1].first.size() == 0 && att_ft_pv[pvSz-1].second.size() == 0 )
      --pvSz;

  if( isCheck )
  {
    for( size_t j=0 ; j < pvSz ; ++j )
    {
      int i;

      if( (i=pIn->getVarIndex(att_ft_pv[j].second)) == -1 )
      {
        if( notes->inq(bKey + "432g", var.name) )
        {
          std::string capt(captAtt(var.name, n_formula_terms, no_colon));
          capt += "declares undefined " + n_variable + " by" ;
          capt += captVal(att_ft_pv[j].first + blank + att_ft_pv[j].second) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue;
      }

      // no variable declared by formula_terms is allowed
      // to have _FillValue occurrences.
      if( att_ft_pv[j].second == pIn->variable[i].name )
      {
        for( size_t k=0 ; k < fTerms.size() ; ++k )
        {
          if( fTerms[k] == att_ft_pv[j].first )
          {
            // check the units of the parameters
            int iu = pIn->variable[i].getAttIndex(n_units);
            if( iu == -1 || pIn->variable[i].attValue[iu][0] == "1" )
            {
              if( x_fUnits[k] != "1" && notes->inq(bKey + "432h", var.name) )
              {
                  std::string capt(captAtt(var.name,n_formula_terms));
                  capt += "Auxiliary " ;
                  capt += captVar(pIn->variable[i].name, n_units, no_colon) ;
                  capt += "require" + captVal(x_fUnits[k], false) ;
                  capt += ", but found " + n_dimension + "less" ;

                  (void) notes->operate(capt) ;
                  notes->setCheckCF_Str( fail );
              }
            }
            else
            {
              if( cmpUnits(pIn->variable[i].attValue[iu][0], x_fUnits[k]) )
                paramVarUnits.push_back(
                  std::pair<std::string, std::string>
                  (pIn->variable[i].name, pIn->variable[i].attValue[iu][0]) );
              else
              {
                if( notes->inq(bKey + "432h", var.name) )
                {
                  std::string capt(captAtt(var.name, n_formula_terms));
                  capt += "Auxiliary " + n_variable + captVal(pIn->variable[i].name) ;
                  capt += "has " ;
                  capt += hdhC::sAssign(n_units, pIn->variable[i].attValue[iu][0]) ;
                  if( x_fUnits[k] == "1" )
                    capt += ", but " + n_dimension + "less is required";
                  else
                    capt += ", but" + captVal(x_fUnits[k]) + "is required";

                  (void) notes->operate(capt) ;
                  notes->setCheckCF_Str( fail );
                }
              }
            }

            break;
          }
        }
      }
    }

    // here after having looped through the fT att parameters
    if( paramVarUnits.size() > 1 )
    {
       for( size_t k=1 ; k < paramVarUnits.size() ; ++k )
       {
         if( paramVarUnits[k].second != paramVarUnits[0].second )
         {
           if( notes->inq(bKey + "432i", var.name) )
           {
             std::string capt("warning for ") ;
             capt += captAtt(var.name, n_formula_terms);
             capt += "Units of auxiliary " + n_variable + "s";
             capt += captVal(paramVarUnits[0].first) + "and";
             capt += captVal(paramVarUnits[k].first);
             capt += "should be identical, found";
             capt += captVal(paramVarUnits[0].second) + "and" ;
             capt += captVal(paramVarUnits[k].second) ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }

           break;
         }
       }
    }

    for(size_t i=0 ; i < att_ft_pv.size() ; ++i )
    {
      if( att_ft_pv[i].first.size() == 0 || att_ft_pv[i].second.size() == 0 )
      {
        // found a syntax error in formula_terms
        if( notes->inq(bKey + "432c", var.name) )
        {
          std::string t;
          Split x_valid_ft(valid_ft);
          for( size_t k=0 ; k < x_valid_ft.size() ; ++k )
          {
            if(k)
              t += blank;
            t += x_valid_ft[k] + " v" + hdhC::itoa(k);
          }

          std::string capt(captAtt(var.name, n_formula_terms, no_colon)) ;
          capt += "with syntax error, expected " + t ;
          capt += ", found " + captVal(var.attValue[att_ft_ix][0]) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        break;
      }
    }
  }

  fTerms.clear();
  for(size_t i=0 ; i < att_ft_pv.size() ; ++i )
  {
    if( att_ft_pv[i].second.size() )
    {
       // any non-existing variable in the formula_terms string?
       int j;
       if( (j=pIn->getVarIndex(att_ft_pv[i].second)) > -1 )
         fTerms.push_back(att_ft_pv[i].second);
    }
  }

  // fTerms contains now only associated variables.

  return ;
}

bool
CF::chap4_4(Variable& var)
{
  bool isUnits;
  bool isName=false;

  // test for time units and verify a correct format
  if( (isUnits=timeUnitsFormat(var)) )
  {
    isName = true; // works for any name

    // calendar and related stuff
    if( chap4_4_1(var) )
      var.addAuxCount();
  }
  else
  {
    if( var.name == n_time
         || var.getAttValue(n_standard_name, lowerCase) == n_time )
      isName = true ;
    else
    {
      std::vector<std::string> timeAtt;
      timeAtt.push_back(n_calendar);
      timeAtt.push_back(n_month_lengths);
      timeAtt.push_back(n_leap_year);
      timeAtt.push_back(n_leap_month);

      // particular for some attributes
      if( isCheck )
      {
        for( size_t j=0 ; j < timeAtt.size() ; ++j )
        {
          if( var.isValidAtt( timeAtt[j]) )
          {
            if( notes->inq(bKey + "44d", var.name) )
            {
              std::string capt("warning for "+ captVar(var.name));
              capt += captAtt(timeAtt[j], s_upper, no_colon);
              capt += "may only be attached to the time coordinate";

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }
      }
    }
  }

  if( isName )
  {
    timeName = var.name;
    timeIx = var.getVarIndex();

    var.coord.isT=true;
    var.isChecked=true;
    var.addAuxCount();

    if( !isUnits )
    {
      std::string tag(bKey + "44a");
      if( notes->inq(tag, var.name) )
      {
        std::string capt(captVar(var.name, false));
        capt += "was rated a T-coordinate, but ";
        capt += captAtt(n_units) + "is missing";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    return true ;
  }

  return false;
}

void
CF::chap4_4a_reco(Variable& var)
{
   // having called this method means that a ref-date 0-1-1 was found
   // note that omission of calendar results in proleptic gregorian
   for( size_t i=0 ; i < var.attName.size() ; ++i )
   {
      if( var.attName[i] == n_calendar
         && hdhC::Upper()(var.attValue[i][0]) == "NONE" )
        return;
   }

   if( notes->inq(bKey + "44c", var.name) )
   {
      std::string capt(captVar(var.name) + "Indication of climatological time by ");
      capt += "reference time in the year 0-1-1 is deprecated" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
   }

   return;
}

bool
CF::chap4_4_1(Variable& var)
{
  // calendar and related stuff
  std::vector<std::string> def_cal;

  def_cal.push_back("gregorian");
  def_cal.push_back("standard");
  def_cal.push_back("proleptic_gregorian");
  def_cal.push_back("noleap");
  def_cal.push_back("365_day");
  def_cal.push_back("all_leap");
  def_cal.push_back("366_day");
  def_cal.push_back("360_day");
  def_cal.push_back("julian");
  def_cal.push_back("none");

  std::string v_cal( var.getAttValue(n_calendar, lowerCase) );

  std::string str;

  if( v_cal.size() == 0 )
  {
     if( ! var.isValidAtt(n_month_lengths) )
       v_cal = def_cal[0] ;  // by default
  }
  else if( v_cal == "360" || v_cal == "365" || v_cal == "366" )
  {
     str=v_cal;
     v_cal += "_day" ;
  }
  else if( v_cal.find("days") < std::string::npos )
  {
     str=v_cal;
     v_cal = v_cal.substr(0, v_cal.size()-1) ;
  }

  if( str.size() )
  {
    if( notes->inq(bKey + "441g", var.name) )
    {
      std::string capt(captAtt(var.name,
                               hdhC::sAssign(n_calendar, str), no_colon));
      capt += "is misspelled, did you mean" + captVal(v_cal,false) + "?" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  bool isTime = v_cal.size();

  if( !isCheck )
    return isTime;

  if( v_cal.size() )
  {
     if( hdhC::isAmong(v_cal, def_cal) )
        return isTime;
  }

  // calendar is non-standard or not defined.

  // --> month_lengths is required with 12 values
  int ml;
  if( (ml=var.getAttIndex(n_month_lengths)) > -1 )
  {
    if( var.attValue[ml].size() != 12 )
    {
      if( notes->inq(bKey + "441a", var.name) )
      {
        std::string capt(captAtt(var.name, n_month_lengths, no_colon)) ;
        capt += "requires 12 values, found" ;
        std::string str(hdhC::itoa(var.attValue[ml].size())) ;
        capt += captVal(str) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }
  else if( v_cal.size() && notes->inq(bKey + "441d", var.name) )
  {
    std::string capt(captVar(var.name)) ;
    capt += "Non-standard calendar" + captVal(v_cal);
    capt += "requires " + captAtt(n_month_lengths, no_colon) ;

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  // if att=leap_month is defined, then leap_year is required
  bool isLMV=false;

  int i;
  if( (i=var.getAttIndex(n_leap_month)) > -1 )
  {
    isLMV=true;

    std::string capt;

    // value must be in the range 1-12
    if( var.attValue[i][0].size() )
    {
      double dv=hdhC::string2Double( var.attValue[i][0] ) ;
      int iv = static_cast<int>(dv);

      if(! (iv > 0 && iv < 13) )
      {
        capt = captAtt(var.name, n_leap_month, no_colon) ;
        capt += "requires a value in the range 1-12, found";
        capt += captVal(var.attValue[i][0], false);
      }
    }
    else  // empty attribute
    {
      capt = captAtt(var.name, n_leap_month, no_colon);
      capt += "requires a value in the range 1-12";
    }

    if( capt.size() && notes->inq(bKey + "441c", var.name) )
    {
      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  // chap4_4_1_reco()
  if( followRecommendations && isLMV && !var.isValidAtt(n_leap_year) )
  {
    if( notes->inq(bKey + "441b", var.name) )
    {
      std::string capt("warning: " + captAtt(var.name, n_leap_month, s_upper));
      capt += "is ignored because " + n_leap_year + " is not specified" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return isTime;
}

void
CF::chap5(void)
{
  // coordinate systems
  if( ! notes->inq(bKey + "5", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  // find coordinate variables
  // chap5_1(Variable&) ; // applied in run() for convenience

  // find 'coordinates' attribute(s)
  chap5_2() ;

  // reduced horizontal grid
  // chap5_3() ;  see chap8_2()

  // time series of station data
//  if( cFVal < 16 )
//    chap5_4() ;

  // chap5_5 (trajectories): checked implicitely

  // grid mapping
  chap5_6();

  // scalar coordinate was checked in run()

  return ;
}

void
CF::chap5_reco(void)
{
  size_t ix;

  for( ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix] ;

    // the name of multi-coord-var should not match the name of any of
    // its dimensions.

    if( var.isScalar )
    {
      for( size_t i=0 ; i < dimensions.size() ; ++i )
      {
         if( var.name == dimensions[i] )
         {
           if( notes->inq(bKey + "57a", var.name) )
           {
             std::string capt("reco: Scalar ");
             capt += captVar(var.name, false) ;
             capt += "should not match the name of " + n_dimension ;
             capt += captVal(dimensions[i]) ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
        }
      }
    }
    else
    {
      size_t sz=var.dimName.size();
      if( var.isLabel && sz > 1 )
        --sz;

      if( sz > 1 )
      {
        for( size_t i=0 ; i < sz ; ++i )
        {
          if( var.name == var.dimName[i] )
          {
            if( notes->inq(bKey + "5c", var.name) )
            {
              std::string capt("reco: Multi-" + n_dimension + "al ");
              capt += captVar(var.name, false) ;
              capt += "should not match the name of any of its " + n_dimension + "s" ;

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }
      }
    }


  }

  return;
}

void
CF::chap5_1(Variable& var)
{
  // Coordinate variables: a single dimension and identical name for
  // dimension and variable, e.g. time(time).

  if( var.dimName.size() == 1 && var.dimName[0] == var.name )
  {
     var.coord.isCoordVar=true;
//     var.countAux += 10;
     var.addAuxCount(10);
  }

  return ;
}

void
CF::chap5_2(void)
{
  // two-dimensional coordinates lon/lat
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    if( var.isChecked )
      continue ;

    int j = ca_ix[ix] ;
    if( j == -1 )
      continue;

    std::vector<std::string>& vs = ca_vvs[j];
    int ii;
    for(size_t k=0 ; k < vs.size() ; ++k )
    {
      // It is permissible to have coordinate variables included.
      if( (ii=pIn->getVarIndex(vs[k])) > -1 )
      {
        pIn->variable[ii].addAuxCount();
        var.push_aux(pIn->variable[ii].name);

        if( ! pIn->variable[ii].coord.isCoordVar )
        {
          std::string su( units_lon_lat(var) );

          if( su == n_latitude )
            ++pIn->variable[ii].coord.indication_Y;
          else if( su == n_longitude )
            ++pIn->variable[ii].coord.indication_X;
          else if( su == n_grid_latitude )
            ++pIn->variable[ii].coord.indication_Y;
          else if( su == n_grid_longitude )
            ++pIn->variable[ii].coord.indication_X;
          else
          {
             // a standard name could help
             if( pIn->variable[ii].getAttValue(n_standard_name, lowerCase)
                     == n_latitude )
               ++pIn->variable[ii].coord.indication_Y;
             else if( pIn->variable[ii].getAttValue(n_standard_name, lowerCase)
                     == n_longitude )
               ++pIn->variable[ii].coord.indication_X;
          }
        }
      }
    }
  }

  // Any aux coord without coordinates attribute?
  // Look for 2-D vars with lon/lat units.
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var2 = pIn->variable[ix];

    if( var2.coord.isCoordVar )
       continue;

    size_t sz;
    if( (sz=var2.dimName.size()) == 2 && !var2.isUnlimited() )
    {
      int cnt=0;
      int ii;
      for( size_t i=0 ; i < sz ; ++i )
        if( (ii=pIn->getVarIndex(var2.dimName[i])) > -1
            && ( pIn->variable[ii].coord.isY
                  || pIn->variable[ii].coord.isY ) )
              ++cnt;

      if( cnt == 2 )
      {
         var2.addAuxCount() ;

         std::string su( units_lon_lat(var2) );
         if( su == n_latitude || su == n_grid_latitude )
           ++var2.coord.indication_Y;
         else if( su == n_longitude || su == n_grid_longitude )
           ++var2.coord.indication_X;

         continue ;
      }
    }
  }

  return;
}

/*
void
CF::chap5_4(void)
{
  // time series of station data (superseded for CF-1.6
  // there is a single eff dimension (besides the bounds vertices)
  // and lon and lat auxiliaries depend only on this dimension.

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    if( var.isChecked || var.coord.isCoordVar )
      continue;

    for( size_t k=0 ; k < pureDims_ix.size() ; ++k )
    {
      if( var.dimName.size() == 1
            && var.dimName[0] == dimensions[ pureDims_ix[k] ] )
      {
        std::string su( units_lon_lat(var) );

        if( su == "lat" || su == "rlat" )
          ++var.coord.indication_Y ;
        else if( su == "lon" || su == "rlon" )
          ++var.coord.indication_X ;
        else
          continue;  // skip next line

        var.addAuxCount();
      }
    }
  }

  return ;
}
*/

void
CF::chap5_6(void)
{
  // data var, grid_mapping attribute value == grid_mapping variable
  std::vector<std::pair<std::string, std::string> > dv_gmv;

  std::string mapCoord[2];

  // scan for attribute 'grid_mapping'
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

//    if( var.isChecked )
//      continue;

    // look for a data variable with a grid_mapping attribute
    int j;

    if( (j=var.getAttIndex(n_grid_mapping)) > -1 )
    {
       std::string& str = var.attValue[j][0] ;

       if( str.size() == 0 )
         continue;

       size_t j;
       for( j=0 ; j < dv_gmv.size() ; ++j )
          if( dv_gmv[j].second == str )
             break;

       if( j == dv_gmv.size() )
         dv_gmv.push_back(
            std::pair<std::string, std::string> (var.name, str)) ;

       int retVal;
       bool isMapping=true;

       if( ! (retVal=chap5_6_gridMappingVar(var, str, "latitude_longitude")) )
       {
//         mapCoord[1]=n_latitude;
//         mapCoord[0]=n_longitude;
         isMapping=false;
       }
       else if( ! (retVal=chap5_6_gridMappingVar(var, str, "rotated_latitude_longitude")) )
       {
         mapCoord[0] = n_grid_latitude ;
         mapCoord[1] = n_grid_longitude ;
       }
       else if( ! (retVal=chap5_6_gridMappingVar(var, str, s_empty)) )
       {
         mapCoord[0] = "projection_y_coordinate" ; // value for most methods
         mapCoord[1] = "projection_x_coordinate" ;
       }
       else
       {
         if( retVal == 1 )
         {
           if( str.size() && notes->inq(bKey + "56c") )
           {
             std::string capt(captAtt(var.name, n_grid_mapping, no_colon));
             capt += "names non-existing ";
             capt += captVar(var.getAttValue(n_grid_mapping), no_colon, no_blank);

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }

         else if( retVal == 2 && notes->inq(bKey + "56f") )
         {
           std::string capt(captVar(str));
           capt += "Missing " ;
           capt += captAtt(n_grid_mapping +"_name", no_colon);

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }

         else if( retVal > 2 && notes->inq(bKey + "56g") )
         {
           // index==retVal-3 takes back an action done in chap5_6_gridMappingVar()
           std::string gmn(n_grid_mapping +"_name");

           std::string capt("grid mapping " + captVar(str, false) );
           capt += "with undefined " + gmn ;
           capt += captVal(pIn->variable[retVal-3].getAttValue(gmn));

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }

         continue;
       }

       // coordinates attribute issues
       if( isMapping )
         chap5_6_attProps(var, mapCoord) ;
    }
  }

  // look for an existing grid-mapping variables which has not been declared
  // by a grip_mapping attribute of a data variable.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    if( var.isChecked )
       continue;

    // exclude previously found grid mapping variables
    size_t j;
    for( j=0 ; j < dv_gmv.size() ; ++j )
       if( dv_gmv[j].second == var.name )
         break;

    if( j == dv_gmv.size() )
    {
      if( var.isValidAtt(n_grid_mapping + "_name") )
      {
        if( notes->inq(bKey + "56e", var.name) )
        {
          std::string capt("found a detached grid-mapping ") ;
          capt += captVar(var.name, no_colon, no_blank);
          capt += ", but an " + captAtt(n_grid_mapping) + "is missing" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  return;
}

int
CF::chap5_6_gridMappingVar(Variable& var, std::string &s, std::string gmn)
{
   // Param s: the grid_mapping attribute of the data variable.
   //     gmn: name of a particular grid mapping method

   // return: 0: found grid_mapping variable with valid grid_mapping_name
   //         1: found no grid_mapping variable
   //         2: found grid_mapping variable with missing grid_mapping_name
   //    3 + ix: found grid_mapping variable with invalid grid_mapping_name


   // find grid_mapping variable gmn
   // and check existence of grid_mapping_name att
   int ix = pIn->getVarIndex(s);  // index of the grid mapping variable

   int retVal=1;

   if( ix > -1 )
   {
     Variable& var_gmv = pIn->variable[ix] ;
     var_gmv.isMapVar=true;
     var_gmv.addAuxCount();
     var.push_aux(var_gmv.name);

     retVal=2;

     if( var_gmv.isValidAtt(n_grid_mapping + "_name") )
     {
        // the grid mapping variabel must have attribute grid_mapping_name
        std::string str( var_gmv.getAttValue(n_grid_mapping +"_name") );

        if( str.size() )
        {
          if( gmn.size() == 0 )
          {
            // all methods defined in CF
            std::vector<std::string> vs_gmn;
            vs_gmn.push_back("albers_conical_equal_area");
            vs_gmn.push_back("azimuthal_equidistant");
            vs_gmn.push_back("lambert_azimuthal_equal_area");
            vs_gmn.push_back("lambert_conformal_conic");
            vs_gmn.push_back("lambert_cylindrical_equal_area");
            vs_gmn.push_back("latitude_longitude");
            vs_gmn.push_back("mercator");
            vs_gmn.push_back("orthographic");
            vs_gmn.push_back("polar_stereographic");
            vs_gmn.push_back("rotated_latitude_longitude");
            vs_gmn.push_back("stereographic");
            vs_gmn.push_back("transverse_mercator");
            vs_gmn.push_back("vertical_perspective");

            if( hdhC::isAmong(str, vs_gmn) )
              gmn = str;
          }

          retVal = (str == gmn) ? 0 : (3+ix);
        }
        else
          retVal = 0 ;  // caught elsewhere
     }

     // recommendation: grid mapping variable should not have dimensions
     if( followRecommendations && var_gmv.dimName.size() )
     {
        if( notes->inq(bKey + "56d", var_gmv.name, NO_MT) )
        {
          std::string capt("reco: Grid_mapping-") ;
          capt += hdhC::sAssign(n_variable, var_gmv.name) ;
          capt += " should not have " + n_dimension + "s" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
     }
   }

   return retVal;
}

void
CF::chap5_6_attProps(
  Variable& dataVar,       // variable with a grid_mapping attribute
  std::string mCV[] )  // required std_names of coord vars
{
   // only called when not latitude_longitude, i.e. there is a mapped grid-lon/grid-lat
   // and a true lon/lat.

   // the coordinates att of the data var must exist
   if( ! dataVar.isValidAtt(n_coordinates) )
   {
     if( notes->inq(bKey + "56a", dataVar.name) )
     {
        std::string capt(n_grid_mapping + blank + captVar(dataVar.name));
        capt += "Missing " + captAtt(n_coordinates) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
   }

   // each name in mCV should have a corresponding standard_name
   // in a coordinate variable.
   bool is_sn[] = {true, true};

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     // a matching standard_name between a variable and the one of
     // the grid mapping procedure is used to identify the associated
     // coordinate variable.
     Variable& var = pIn->variable[i] ;

     std::string curr_sn( var.getAttValue(n_standard_name, lowerCase) );
     if( curr_sn.size() )
     {
       for( size_t l=0 ; l < 2 ; ++l )
       {
         if( curr_sn == mCV[l] )
         {
           is_sn[l]=false;
           if( l == 0 )
           {
             var.coord.isY=true;
             ++var.coord.indication_Y;
           }
           else
           {
             var.coord.isX=true;
             ++var.coord.indication_X;
           }

           break;
         }
       }

       if( !is_sn[0] && !is_sn[1] )
         break;
     }
   }

   if( !isCheck )
     return;

   // required standard_names not supported by any variable's standard-name
   for( size_t l=0 ; l < 2 ; ++l )
   {
     if( is_sn[l] )
     {
       // standard_name requirements
       if( notes->inq(bKey + "56b") )
       {
         std::string capt("grid mapping: missing ");
         capt += hdhC::sAssign(n_standard_name, mCV[l]) ;
         capt += " for any " + n_variable ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
   }

   return;
}

void
CF::chap6(void)
{
  // labels
  // a) char-type
  // b) and dim-size > 0 && with a dim which is unique across all variables
  if( ! notes->inq(bKey + "6", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  std::vector<size_t> label_ix;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
     Variable& var = pIn->variable[i];

     if( var.type == NC_CHAR )
     {
        var.isLabel=true;
        label_ix.push_back(i);
     }
  }

  if( label_ix.size() == 0 )
     return;

  // auxiliary coordinates declared by labels
  std::vector<std::string> vs_lbl_aux;
  std::vector<bool> isNoException;

  for( size_t i=0 ; i < label_ix.size() ; ++i )
  {
    Variable& label = pIn->variable[label_ix[i]];
    vs_lbl_aux.push_back(s_empty);
    isNoException.push_back(true);

    for( size_t j=0 ; j < pIn->varSz ; ++j )
    {
      Variable& var = pIn->variable[j];

      if( label.name == var.name )
        continue;

      if( var.coord.isCoordVar || ! var.isCoordinate() )
        continue;

      if( var.isScalar && label.isScalar )
        vs_lbl_aux[i] += blank + var.name ;
      else if( var.dimName.size() == 1 && label.dimName.size() > 1 )
      {
        if( var.dimName[0] == label.dimName[0] )
          vs_lbl_aux[i] += blank + var.name ;
      }
    }
  }

  // declared here because of the scope
  std::vector<std::string> vs_region_table;

  // any label not named in a coordinates attribute?
  for( size_t l=0 ; l < label_ix.size() ; ++l )
  {
    if( !isNoException[l] )
      continue;

    Variable& label = pIn->variable[label_ix[l]];

    for( size_t i=0 ; i < pIn->varSz ; ++i )
    {
      if( i == label_ix[l] )
         continue;

      Variable& var = pIn->variable[i];

      int j;

      if( (j=var.getAttIndex(n_grid_mapping)) > -1 )
        if( var.attValue[j][0] == label.name )
           isNoException[l] = false;
    }

    for( size_t c=0 ; c < ca_pij.size() ; ++c)
    {
      // A label must have one or two dimensions.
      // The maximum string-length cannot be checked

      // If the label is not a scalar auxiliary type one , then the first dimension
      // must match one of the dims of the data variable
      if( label.dimName.size() == 2 )
      {
        Variable& dv = pIn->variable[ca_pij[c].first] ;

        if( ! hdhC::isAmong(label.dimName[0], dv.dimName)
              && ! isChap9_specialLabel(label, dv) )
        {
          if( notes->inq(bKey + "6a", label.name, NO_MT) )
          {
             std::string capt("non-scalar label " + captVar(label.name, no_colon));
             capt += "does not share a " + n_dimension ;
             capt += " with associated " + captVar(dv.name, no_colon, no_blank);

             std::string text("label: ");
             text += label.getDimNameStr(true);
             text += "\n " + n_variable + ": ";
             text += dv.getDimNameStr(true);

             (void) notes->operate(capt, text) ;
             notes->setCheckCF_Str( fail );
          }
        }
      }
    }

    // check standardized-region-names. Note that it is possible that
    // neither a standard_name nor any data is provided
    bool isRegion=false;
    int j = label.getAttIndex(n_standard_name) ;
    if( j > -1 && label.attValue[j][0] == "region" )
      isRegion=true;

    std::vector<std::string> vs_region_file;

    if( ! pIn->nc.isNoRecords(label.name) )
    {
      pIn->nc.getData(vs_region_file, label.name);

      for( size_t f=0 ; f < vs_region_file.size() ; ++f)
      {
        if( vs_region_file[f].size() )
          vs_region_file[f] = hdhC::Lower()(vs_region_file[f]);
        else
          vs_region_file.clear();
      }
    }

    // only read once
    if( ! vs_region_table.size() && region_table.size() )
    {
      std::string file(tablePath + "/");
      ReadLine ifs( file + region_table ) ;

      if( ifs.isOpen() )
      {
        std::string line;
        while( ! ifs.getLine(line) )
          vs_region_table.push_back(line);

        ifs.close();
      }
    }

    // any standardized-region-name available?
    if( vs_region_file.size() )
    {
      if( vs_region_table.size() )
      {
        bool isMatch;

        // a string from the table matches one in the file
        if( (isMatch=hdhC::isAmong(vs_region_file, vs_region_table))
                && ! isRegion )
        {
          if( notes->inq(bKey + "6b", label.name) )
          {
              std::string capt(captVar(label.name));
              capt += "A label taken from table ";
              capt += std_name_table ;
              capt += " requires " + n_standard_name + "=region";

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
          }
        }

        else if( !isMatch && isRegion )
        {
          if( notes->inq(bKey + "6c", label.name) )
          {
            std::string capt("label " + captVar(label.name));
            capt += captAtt(s_empty, n_standard_name, s_upper, no_colon) ;
            capt += captVal("region") ;
            capt += "requires labels taken from";
            capt += captVal(std_name_table, false) ;
            capt += ", found" ;
            for(size_t f=0 ; f < vs_region_file.size() ; ++f )
            {
              if( !hdhC::isAmong(vs_region_file[f], vs_region_table) )
              {
                if(f)
                  capt += ",";
                capt += captVal(vs_region_file[f], false) ;
              }
            }

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }

    // vs_region_file is empty
    else if(isRegion)
    {
      if( notes->inq(bKey + "6d", label.name) )
      {
          std::string capt("label " + captVar(label.name));
          capt += captAtt(s_empty,hdhC::sAssign(n_standard_name, "region"),
                          no_colon, s_upper) ;
          capt += "is specified, but data, i.e. labels, are missing";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
      }
    }

  }

  return ;
}

void
CF::chap7(void)
{
  // cells
  if( ! notes->inq(bKey + "7", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  // cell boundaries
  chap7_1() ;

  // cell measures
  chap7_2() ;

  // cell methods
  chap7_3() ;

  // climatological statistics
  chap7_4a() ;

  return ;
}

void
CF::chap7_1(void)
{
  if( !isCheck )
    return ;

  // this pertains to both cell boundaries and climatologies
  std::string units;

  // if method isBounds() detects bounds, then it sets:
  // var.boundsOf = pIn->variable[...].name ;
  // pIn->variable[...].bounds = var.name ;

  // look for some indices
  // first: variable.boundsOf, i.e. is bound
  // second: variable.bounds, i.e. has bounds
  std::vector<std::pair<int, int> > ix;

  // Is a variable declared as bounds|climatology of another variable? Yes, then
  // corresponding variable members are 'boundsOf' and 'bounds', repectively.
  int sz = static_cast<int>(pIn->varSz);

  for( int i=0 ; i < sz ; ++i )
  {
    // find coordinate variables with a bound or climatology declaration.
    if( isBounds(pIn->variable[i]) )
       ix.push_back( std::pair<int, int>
                     (i, pIn->getVarIndex(pIn->variable[i].boundsOf) ) );
  }

  // analyse matching pairs
  for( size_t i=0 ; i < ix.size() ; ++i )
  {
    Variable& var_is   = pIn->variable[ix[i].first];
    Variable& var_has  = pIn->variable[ix[i].second];

    // the bounds-var must have the same dimensions as its associated
    // variable plus a trailing one indicating the maximum number of vertices.

    // potential faults:
    // the additional dim for vertices is missing,
    // the sub-set of dims is different from the associated variable,
    // too many dims, if auxiliary coordinate and coordinate variable are mistaken.

    // note: a misplaced climatology attribute will be detected later, thus
    // skip such a case.
    if( var_is.isClimatology )
    {
      if( var_has.name != timeName )
        continue;
    }

    size_t sz_is = var_is.dimName.size() ;
    size_t sz_has = var_has.dimName.size() ;
    size_t sz_scalar=0;

    if( sz_has == 0 )
    {
       if( pIn->nc.isDimValid(var_has.name)
              && pIn->nc.getDimSize(var_has.name) == 1 )
          sz_scalar=1;
    }

    bool isMissingVerticesDim=false;
    bool hasTooManyVerticesDim=false;
    bool isNotIdenticalSubSet=false;

    size_t sz = ( sz_is < sz_has ) ? sz_is : sz_has ;

    size_t v;
    for( v=0 ; v < sz ; ++v )
        if( var_is.dimName[v] != var_has.dimName[v] )
          break;

    if( !( v == sz && (v+1+sz_scalar) == sz_is ) )
    {
      if( v < sz )
        isNotIdenticalSubSet=true;
      else if( sz_is <= sz_has )
        isMissingVerticesDim=true;
      else if( sz_is > sz_has )
        hasTooManyVerticesDim=true;
    }

    if( isNotIdenticalSubSet )
    {
      if( notes->inq(bKey + "71a", var_is.name) )
      {
        std::string capt;
        if( var_is.isClimatology )
          capt = n_climatology;
        else
          capt = n_bounds ;
        capt += "-" + captVar(var_is.getDimNameStr(true), false);
        capt += "should share the set of " + n_dimension ;
        capt += "s of the associated " +  n_variable;
        capt += captVal(var_has.getDimNameStr(true)) ;

        std::string text("Found: ");
        text += var_is.getDimNameStr(true);
        text += " vs. " + var_has.getDimNameStr(true);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    else if( isMissingVerticesDim )
    {
      if( notes->inq(bKey + "71b", var_is.name) )
      {
        std::string capt;
        if( var_is.isScalar )
          capt = "scalar ";

        if( var_is.isClimatology )
          capt += n_climatology;
        else
          capt += n_bounds ;
        capt += "-" + captVar(var_is.getDimNameStr(true), false);
        capt += "is missing the ";

        if( !var_is.isScalar )
          capt += "additional ";
        capt += n_dimension + " for the vertices" ;

        std::string text("Found: ");
        text += var_is.getDimNameStr(true);
        text += " vs. " + var_has.getDimNameStr(true);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    else if( hasTooManyVerticesDim )
    {
      if( notes->inq(bKey + "71d", var_is.name) )
      {
        std::string capt;
        if( var_is.isClimatology )
          capt = n_climatology;
        else
          capt = n_bounds ;
        capt += "-" + captVar(var_is.getDimNameStr(true));
        capt += "Dimensions are incompatible with the associated " ;
        capt += captVar(var_has.getDimNameStr(true), no_colon, no_blank) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // a boundary variable must have a numeric type
    if( var_is.type == NC_CHAR )
    {
      if( notes->inq(bKey + "71c", var_is.name) )
      {
        std::string capt;
        if( var_is.isClimatology )
          capt = n_climatology;
        else
          capt = n_bounds ;
        capt += blank + captVar(var_is.name, false);
        capt += "should be numeric type" ;

        std::string text("Found: type=" );
        text += pIn->nc.getVarTypeStr(var_is.name);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    bool isFault[] = { false, false} ;

    if( var_is.isValidAtt(n_units) )
    {
       std::string units_is( var_is.getAttValue(n_units) );
       std::string units_has( var_has.getAttValue(n_units) );

       if( units_is != units_has )
         isFault[0]=true;
    }

    if( var_is.isValidAtt(n_standard_name) )
    {
       std::string sn_is( var_is.getAttValue(n_standard_name) );
       std::string sn_has( var_has.getAttValue(n_standard_name) );

       if( sn_is != sn_has )
         isFault[1]=true;
    }

    for( size_t f=0 ; f < 2 ; ++f )
    {
       if( isFault[f] )
       {
         std::string tag;
         std::string* att;
         if( f )
         {
           tag = "71e";
           att = &n_standard_name ;
         }
         else
         {
           tag = "71f";
           att = &n_units ;
         }

         if( notes->inq(bKey + tag , var_is.name) )
         {
           std::string capt(
                captAtt(var_is.name,
                        hdhC::sAssign(*att, var_is.getAttValue(*att)), no_colon)) ;
           capt +=  + "and " ;
           capt += captAtt(var_has.name,
                           hdhC::sAssign(*att, var_has.getAttValue(*att)), no_colon) ;
           capt += "are different" ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
    }
  }

  if( followRecommendations )
  {
    std::string* s[2];
    s[0] = &n_FillValue;
    s[1] = &n_missing_value;

    for( size_t i=0 ; i < ix.size() ; ++i )
    {
      Variable& var_is  = pIn->variable[ix[i].first];

      for( size_t i=0 ; i < 2 ; ++i )
      {
        if( var_is.isValidAtt( *s[i]) )
        {
          if( notes->inq(bKey + "71h", var_is.name) )
          {
            std::string capt("reco: ");
            capt += "Variable" + captVal(var_is.name) ;
            capt += "should not have " ;
            capt += captAtt( *s[i], no_colon) ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  return;
}

void
CF::chap7_2(void)
{
  if( !isCheck )
    return ;

  // cell_measures
  std::string cm;
  int meas_ix; // index of the measure variable

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    int jx = var.getAttIndex(n_cell_measures) ;

    if( jx == -1 )  // cell_measures not available
      continue;

    // syntax: 'key: variable'
    // only two key-words (simultaneously the name of variable)
    // are defined: area and volume.
    std::string cm(var.getAttValue(n_cell_measures));

    // check key-words
    std::vector<std::string> cm_key( getKeyWordArgs(cm, "key", ':') );

    if( cm_key.size() == 0 )
    {
       // missing key-word
      if( notes->inq(bKey + "72b", var.name) )
      {
        std::string capt(captAtt(var.name, n_cell_measures, no_colon)) ;
        capt += "requires missing name area: or volume:" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      var.attValue[jx][0]="CF failed" ;

      continue ;
    }
    else
    {
      if( ! (cm_key[0] == "area:" || cm_key[0] == "volume:") )
      {
        // wrong key-word
        if( notes->inq(bKey + "72b", var.name) )
        {
          std::string capt(captAtt(var.name, n_cell_measures, no_colon)) ;
          capt += "with invalid " + hdhC::sAssign("name", cm_key[0]);
          capt += ", required is area: or volume:";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }

    // check cell_measures variable
    // get variable name given in the cell_measures attribute
    std::vector<std::string> cm_arg( getKeyWordArgs(cm, "arg", ':') );

    if( cm_arg.size() )
    {
      if( (meas_ix = pIn->getVarIndex(cm_arg[0])) == -1 )
      {
        if( notes->inq(bKey + "72e", var.name) )
        {
          std::string capt(captAtt(var.name, n_cell_measures, no_colon));
          capt += "names a non-existing " + n_variable ;
          capt += " by" + captVal(cm, false) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue;
      }

      Variable& mvar = pIn->variable[meas_ix];

      mvar.addAuxCount();
      var.push_aux(mvar.name);

      // must have units
      bool is=false;
      if( mvar.units.size() == 0 )
        is=true;
      else
      {
        // a measure variable must have units that are consistent with
        // the measure type, i.e., m2 for area and m3 for volume.
        std::string xpctdU;

        if( cm_key[0].find(n_area) < std::string::npos )
          xpctdU = "m2";
        else if( cm_key[0].find("volume") < std::string::npos )
          xpctdU = "m3";

        if( xpctdU.size() && ! cmpUnits( mvar.units, xpctdU) )
          is=true;
      }

      if(is && notes->inq(bKey + "72c", mvar.name) )
      {
        std::string capt(captVar(mvar.name, no_colon) + "requires missing ") ;
        if( cm_key[0].find(n_area) < std::string::npos )
          capt += hdhC::sAssign(n_units, "m2");
        else if( cm_key[0].find("volume") < std::string::npos )
          capt += hdhC::sAssign(n_units, "m3");

        if( mvar.units.size() )
          capt += ", found " + captVal(mvar.units);

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      // The dimensions of the measure variable should be the same as or a subset of
      // the dimensions of the variable to which they are related, but their order
      // is not restricted.
      for( size_t im=0 ; im < mvar.dimName.size() ; ++im )
      {
        if( ! hdhC::isAmong(mvar.dimName[im], var.dimName) )
        {
          if( notes->inq(bKey + "72a", mvar.name) )
          {
            std::string capt("dimensions of measure-");
            capt += captVar(mvar.getDimNameStr(true), false);
            capt += "are not a (sub-)set of ";
            capt += captVar(var.getDimNameStr(true), false);

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }

          break;
        }
      }
    }
  }

  return;
}

void
CF::chap7_3(void)
{
  if( !isCheck )
    return ;

  // cell_methods
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    // Note that key-words used for climatological statistics
    // are checked in chap7_4b()

    int jx ;
    if( (jx=var.getAttIndex(n_cell_methods)) == -1 )
      continue;

    // note that the loop is in fact gratuitous
    std::string cm ;
    for( size_t i=0 ; i < var.attValue[jx].size() ; ++i )
    {
       if( i )
          cm += blank ;
       cm += var.attValue[jx][i] ;
    }

    cm = hdhC::stripSurrounding(cm);

    std::string s0;
    size_t last = cm.size()-1;
    bool isColonFail=true;

    if( cm.find(':') < std::string::npos )
       isColonFail = false;

    // ':' must not be the last character
    if( cm[last] == ':' )
      isColonFail = true;

    bool isSepFail=false;
    int bracketOpen=0;
    int bracketClose=0;

    // look for typos and repair
    size_t pos=0 ;
    while( cm[pos] == ':' )
    {
       isColonFail=true;
       ++pos;
    }

    for( ; pos < cm.size() ; ++pos )
    {
      // a) is each 'key:' separated by blank(s) from the next item?
      // b) is each '(' separated by blank(s) from the previous item?
      // c) is each ')' separated by blank(s) from the next item?
      //    note that ')' may be the last character.

      if( cm[pos] == ':' )
      {
        if( pos < last &&  ! (cm[pos+1] == ' ' || cm[pos+1] == ')' ) )
        {
          s0 += ": ";
          isSepFail=true;
          continue;
        }
        else if( cm[pos-1] == ' ' )
        {
          isColonFail=true;

          // try to repair the trivial case "key :word", but annotate
          if( pos > 1 && cm[pos-2] != ':' )
          {
             s0[s0.size()-1] = ':' ;  // swap : and ' '
             s0 += ' ' ;
             continue;  // skip current colon at pos
          }
        }
      }
      else if( cm[pos] == '(' )
      {
        ++bracketOpen;

        if( cm[pos-1] != ' ' )
        {
          s0 += ' ';
          isSepFail=true;
        }
      }
      else if( cm[pos] == ')' )
      {
        ++bracketClose;

        if( pos != last && cm[pos+1] != ' ' )
        {
          isSepFail=true;

          s0 += cm[pos] ;
          s0 += ' ';
          continue;
        }
      }

      s0 += cm[pos] ;
    }

    if( bracketClose != bracketOpen )
    {
      if( notes->inq(bKey + "732e", var.name) )
      {
        std::string capt(captAtt(var.name, n_cell_methods));
        capt += "Comment without a closing paranthesis" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // parse 'wordLists', i.e., sequences of
    // dim1: [name:2: [dim3 ...] method [....].
    // Any dimN: after a non-colon item indicates the start
    // of another wordList, except the phrase within parenthesis.

    Split x_cm(s0);

    if( ! isColonFail )
    {
      // no begin with name:
      if( x_cm[0].size() && x_cm[0][x_cm[0].size()-1] != ':' )
        isColonFail = true;

      // an end with name:
      size_t pos;
      if( x_cm.size() )
         pos = x_cm.size()-1;

      if( x_cm[pos].size() && x_cm[pos][x_cm[pos].size()-1] == ':' )
        isColonFail = true;
    }

    if( isSepFail )  // only once
    {
      if( notes->inq(bKey + "73a", var.name) )
      {
         std::string capt(captAtt(var.name,n_cell_methods, no_colon));
         capt += "requires blank separated word-list, found" ;
         capt += captVal(cm);

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }

      var.attValue[jx][0].clear() ;
      return;
    }

    if( isColonFail )
    {
      if( notes->inq(bKey + "73b", var.name) ) // only once
      {
        std::string capt(captAtt(var.name, n_cell_methods, no_colon));
        capt += "requires format name: method, found" + captVal(cm) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      var.attValue[jx][0].clear() ;
      return;
    }

    if( bracketClose != bracketOpen )
       return;

    if( followRecommendations )
    {
      // special: cm="lat: lon: ..."  --> "area: ..."
      int i0 = pIn->getVarIndex( x_cm[0].substr(0, x_cm[0].size()-1) ) ;
      for( size_t i=0 ; i < x_cm.size()-1 ; ++i )
      {
         int i1 = pIn->getVarIndex( x_cm[i+1].substr(0, x_cm[i+1].size()-1) ) ;

         if( i0 > -1 && i1 > -1 )
         {
           if( (pIn->variable[i0].coord.isX || pIn->variable[i0].coord.isY )
                 && (pIn->variable[i1].coord.isX || pIn->variable[i1].coord.isY ) )
           {
             if( notes->inq(bKey + "731a", var.name) )
             {
               std::string capt("Reco for ");
               capt += captAtt(var.name, n_cell_methods) ;
               capt += "Names" + captVal(pIn->variable[i0].name + ":") + "and" ;
               capt += captVal(pIn->variable[i1].name + ":") ;
               capt += "should be replaced by area:";

               (void) notes->operate(capt) ;
               notes->setCheckCF_Str( fail );
             }

             // replace by area:
             cm.clear();
             for( size_t j=0 ; j < x_cm.size() ; ++j )
             {
                if( j )
                   cm += blank;

                if( j == i )
                {
                  cm += "area:" ;
                  ++j;
                }
                else
                  cm += x_cm[j] ;
             }

             var.attValue[jx][0].clear();
             x_cm = cm ;

             break;
           }
         }

         i0 = i1;
      }
    }

    // now the total cell_method str is decomposed at first into a separated list and
    // then each list item into a part for dimensions, methods, and comment-bracket
    std::vector<std::string> cm_name;
    std::vector<std::string> cm_method;
    std::string cm_comment;
    bool isNewList=true;

    for( size_t i=0 ; i < x_cm.size() ; ++i )
    {
       size_t lst=x_cm[i].size();
       if(lst)
          --lst;

       // get comments
       if( x_cm[i][0] == '(' )
       {
          cm_comment = x_cm[i++] ;
          for( ; i < x_cm.size() ; ++i )
          {
             cm_comment += blank + x_cm[i] ;

             if( x_cm[i].size() && x_cm[i][x_cm[i].size()-1] == ')' )
               break;
          }

          isNewList=true;
       }

       // get methods
       else if( isNewList && x_cm[i][lst] == ':')
       {
          cm_name.push_back( x_cm[i] );
          cm_method.push_back(s_empty);
          isNewList=false;
       }

       // get name:
       else if( x_cm[i][lst] == ':')
          cm_name.back() += blank + x_cm[i] ;
       else
       {
          if( isNewList )
             cm_method.back() += blank + x_cm[i] ;
          else
          {
             cm_method.back() = x_cm[i] ;
             isNewList=true;
          }
       }
    }

    // Exploit wordLists

    // A given dimension name should only occur once in a cell_methods string,
    // except time:
    std::vector<std::string> multipleDims;

    int sz = static_cast<int>(cm_name.size());
    for( int l=0 ; l < sz-1 ; ++l )
    {
      for( int j=l+1 ; j < sz ; ++j )
      {
        if( cm_name[l] == cm_name[j] )
        {
           if( !hdhC::isAmong(cm_name[l], multipleDims) )
             multipleDims.push_back( cm_name[l] );
        }
      }
    }

    if( cm_comment.size() )
      if( chap7_3_cellMethods_Comment(cm_comment, var) )
         return ;

    std::string time_colon( timeName + ":" );

    for( size_t i=0 ; i < cm_name.size() ; ++i )
    {
       if( chap7_3_cellMethods_Name(cm_name[i], var) )
          return;
       if( chap7_3_cellMethods_Method(cm_method[i], var) )
          return;

       if( cm_name[i] != time_colon )
         if( chap7_3_3(cm_method[i], var, "where") )
           return;
    }

    bool isClim=false;
    if( timeIx > -1 )
      // cm-syntax for climatologies
      isClim = chap7_4b(var, cm_name, cm_method) ;

    // all names representing non-point variables should have bounds
    chap7_3_inqBounds(var, cm_name, cm_method, isClim );

    if( var.attValue[jx][0].size() == 0 )
    {
      std::string cm_new;

      for( size_t i=0 ; i < cm_name.size() ; ++i )
      {
        if( i )
          cm_new += blank ;

        cm_new += cm_name[i] ;

        if(cm_method[i].size() )
          cm_new += blank + cm_method[i] ;
      }

      if( cm_comment.size() )
        cm_new += " (" + cm_comment + ")" ;

      var.attValue[jx][0] += cm_new  ;
    }

    if( followRecommendations )
    {
//      chap7_3b_reco(var, cm_name);
      chap7_3_4b(var, cm_name, cm_method);
    }
  }

  return;
}

bool
CF::chap7_3_cellMethods_Comment(std::string &par, Variable& var)
{
  //return true for failure

  // Additional information may be specified within parenthesis.
  // a) matching () was checked previously
  // b) check the only standardised one, syntax: (interval: value unit)
  Split x_par(par, " ()");

  if( x_par.size() )
  {
    if( x_par[0] == "comment:" )
    {
      if( par.find("interval:") == std::string::npos )
      {
        if( notes->inq(bKey + "732a", var.name) )
        {
           std::string capt(captAtt(var.name, n_cell_methods) );
           capt += "Key-word" + captVal("comment:") ;
           capt += "should be omitted because of absent standardised information" ;

           std::string text("found=" + par);

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
        }
      }
      else
      {
        if( notes->inq(bKey + "732d", var.name) )
        {
           std::string capt(captAtt(var.name, n_cell_methods));
           capt += "Swapped order of information, ";
           capt += "required is (interval: val units comment: text)" ;

           std::string text("found=" + par);

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
        }
      }

      return true ;
    }

    if( x_par[x_par.size()-1] == "comment:" )
    {
      if( notes->inq(bKey + "732c", var.name) )
      {
         std::string capt(captAtt(var.name, n_cell_methods, no_colon));
         capt += "provides incomplete non-standardised information" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }

      return true ;
    }

    if( x_par[0] != "interval:" )
      return false;  // comment by non-standardised information
  }

  bool is=true;

  if( x_par.size() > 2 && hdhC::isDigit(x_par[1]) )
  {
    if( parseUnits(x_par[2]) )
      is=false;
  }

  if( is )
  {
     if( notes->inq(bKey + "732b", var.name) )
     {
        std::string capt(captAtt(var.name, n_cell_methods));
        capt += "Interval information requires (interval: value units), found";
        capt += captVal(par, false);

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }

     return true;
  }

  return false;
}

bool
CF::chap7_3_cellMethods_Method(std::string &str0, Variable& var)
{
  // str0 contains only the method item
  std::vector<std::string> term;
  term.push_back("maximum");
  term.push_back("mean");
  term.push_back("median");
  term.push_back("mid_range");
  term.push_back("minimum");
  term.push_back("mode");
  term.push_back("point");
  term.push_back("standard_deviation");
  term.push_back("sum");
  term.push_back("variance");

  // method str0 must begin with one of the terms
  Split x_methods(str0);

  if( x_methods.size() == 0 )
    // should not happen, caught elsewhere
    return true;

  if( !hdhC::isAmong(x_methods[0], term) )
  {
    if( notes->inq(bKey + "73d", var.name) )
    {
      std::string capt(captAtt(var.name, n_cell_methods, no_colon));
      capt += "with invalid " + hdhC::sAssign("method", x_methods[0]) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return true;
  }

  // check valid mode
  if( x_methods.size() > 1 )
  {
    if( ! (x_methods[1] == "where"
            || x_methods[1] == "over"
              || x_methods[1] == "within" ) )
    {
      if( notes->inq(bKey + "733a", var.name) )
      {
        std::string capt(captAtt(var.name, n_cell_methods));
        capt += "Invalid condition before";
        capt += captVal(x_methods[1], false) + ", expected where|over|within" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      return true;
    }
  }

  return false;
}

bool
CF::chap7_3_cellMethods_Name(std::string &name, Variable& var)
{
  // valid names:  dimension of the variable, a scalar coordinate variable,
  //               , a standard name indicating the axis, or area.

  Split x_name(name, " :");

  for( size_t x=0 ; x < x_name.size() ; ++x )
  {
    size_t ix;

    // dim of a var
    if( hdhC::isAmong(x_name[x], var.dimName) )
    {
      continue;
    }

    for( ix=0 ; ix < pIn->varSz ; ++ix )
    {
      // a scalar coord or standard_name (of any variable)
      if( pIn->variable[ix].isScalar && pIn->variable[ix].name == x_name[x] )
      {
         pIn->variable[ix].addAuxCount();
         var.push_aux(pIn->variable[ix].name);
         break;
      }
      else if( pIn->variable[ix].getAttValue(n_standard_name) == x_name[x] )
        break;
    }

    if( ix < pIn->varSz )
      continue;

    // No specific coordinate: area or a valid standard name that is neither
    // a dimensions nor a coordinate variable.
    if( chap7_3_4a(x_name[x]) )
      continue;

    if( notes->inq(bKey + "73c", var.name) )
    {
      std::string capt(captAtt(var.name, n_cell_methods));
      capt += "Invalid name" + captVal(x_name[x]+":");

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return false;
}

void
CF::chap7_3_inqBounds(Variable& var,
  std::vector<std::string>& cm_name,
  std::vector<std::string>& cm_method, bool isClim )
{
  // cell_methods:
  // All names representing non-point variables should have bounds.

  // Get list of unique names without method  point.
  // Note that a cm_name item may contain more than a single dimension,
  // but there is only a single correspponding method item.
  static std::vector<std::string> name;

  for( size_t l=0 ; l < cm_name.size() ; ++l )
  {
    Split x_cm_name(cm_name[l], ": ");  // the dimensions in cell_methods
    for( size_t x=0 ; x < x_cm_name.size() ; ++x )
    {
      if( cm_method[l].find("point") == std::string::npos )
      {
        size_t n;
        for(n=0 ; n < name.size() ;++n)
        {
          if( name[n] == x_cm_name[x] )
            break;
        }

        if( n == name.size() )
        {
          // exclude no-coordinate cases area:, longitude:, and latitude:
          int j;
          if( (j=pIn->getVarIndex(x_cm_name[x])) == -1 )
          {
            // find name of lon/lat by finding standard_names (or long_name)
            std::vector<std::string> vs;
            if( x_cm_name[x] == "area" )
            {
              vs.push_back(n_longitude);
              vs.push_back(n_latitude);
            }
            else if( x_cm_name[x] == n_longitude )
              vs.push_back(n_longitude);
            else if( x_cm_name[x] == n_latitude )
              vs.push_back(n_latitude);

            if( vs.size() )
            {
              for(size_t v=0 ; v < pIn->varSz ; ++v)
              {
                Variable& var_t = pIn->variable[v];
                for( size_t k=0 ; k < vs.size() ; ++k)
                {
                  int m;
                  if( var_t.std_name.size() )
                  {
                    if( vs[k] == var_t.std_name )
                      name.push_back(var_t.name);
                  }
                  else if( (m=var_t.getAttIndex(n_long_name)) > -1 )
                  {
                    std::string& s = var_t.attValue[m][0] ;
                    if( vs[k] == s )
                      name.push_back(var_t.name);
                  }
                }
              }
            }
          }
          else
            name.push_back( x_cm_name[x] );
        }
      }
    }
  }

  for( size_t l=0 ; l < name.size() ; ++l )
  {
    int ix ;

    if( (ix=pIn->getVarIndex(name[l])) > -1 )
    {
      Variable& var_n = pIn->variable[ix];

      if( !var_n.bounds.size() && notes->inq(bKey + "73e", var.name, NO_MT) )
      {
        std::string capt("reco: Variable" + captVal(var_n.name) ) ;
        capt += "named by " + captAtt(var.name, n_cell_methods, no_colon);
        capt += "should have attached ";

        if( isClim )
          capt += n_climatology ;
        else
          capt += "bounds" ;
        capt += "-" + n_variable;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  return;
}

void
CF::chap7_3b_reco(Variable& var, std::vector<std::string> &cm_name )
{
  // cell_methods

  // data variables (exception of bounds)
  // should have cell_methods attributes

// the following tests is disabled, because of:
// CF-1.6, p.40: "... it is recommended that every data variable include for
//each of its dimensions and each of its scalar coordinate variables
//the cell_methods information of interest
// [!!!](unless this information would not be meaningful).
//It is especially recommended that cell_methods be explicitly specified
//for each spatio-temporal dimension and each spatio-temporal scalar
//coordinate variable."

//Question: what does meaningful mean?

/*
  if( var.boundsOf.size() )
    return;

  if( var.dimName.size() == 0 )
    return;

  std::vector<size_t> missing;
  std::string item;

  // scan over dimensions of a variable and scalar variables
  // supplied by attribute coordinates
  std::vector<std::string> items;

  // dimensions
  for( size_t i=0 ; i < var.dimName.size() ; ++i )
    items.push_back( var.dimName[i] );

  // scalar coordinate variables
  int jx = ca_ix[ vIx[var.name] ] ;

  if( jx > -1 )
  {
    for( size_t i=0 ; i < ca_vvs[jx].size() ; ++i )
    {
      int j;
      if( (j=pIn->getVarIndex(ca_vvs[jx][i])) > -1 )
      {
          if( pIn->variable[j].isScalar && pIn->variable[j].isCoordinate() )
            items.push_back( ca_vvs[jx][i] );
      }
    }
  }

  for( size_t i=0 ; i < item.size() ; ++i )
  {
    bool is=true;
    for( size_t l=0 ; l < cm_name.size() ; ++l )
    {
      Split x_cm_name(cm_name[l], ": ");

      for( size_t x=0 ; x < x_cm_name.size() ; ++x )
      {
        if( x_cm_name[x] == items[i] || x_cm_name[x] == n_area )
        {
          is=false;
          break;
        }
      }

      if( is )
      {
        // the current dimension is neither listed in cell_methods nor is
        // it a scalar coord var.
        int v ;
        if( (v=pIn->getVarIndex(var.dimName[i]) > -1 ) )
        {
          if( pIn->variable[v].isCoordinate() )
             missing.push_back(jx);
        }
      }
    }
  }

  if( missing.size() )
  {
    if( notes->inq(bKey + "73g", var.name) )
    {
      std::string capt("reco for " + captAtt(var.name, n_cell_methods, no_colon));
      capt += "should specify an item for each " + n_dimension ;
      capt += ", also for " ;

      for( size_t l=0 ; l < missing.size() ; ++l )
      {
         if( l )
           capt += ", " ;

         capt += captVal(var.dimName[missing[l]]) ;
      }

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }
*/

  return;
}

bool
CF::chap7_3_3(std::string &method, Variable& var, std::string mode)
{
  // return true for failure
  // mode: where or over
  std::string where("where");
  std::string over("over");

  // find the word after the mode string, i.e. 'where' or 'over'
  Split x_method(method);
  std::string type;

  // prevent to check a climatology case
  if( mode == where && method.find(where) == std::string::npos )
     return false;

  size_t iw;
  for( iw=0 ; iw < x_method.size() ; ++iw )
  {
     if( x_method[iw] == mode )
     {
       if( (iw+1) < x_method.size() )
         type=x_method[++iw] ;

       break;
     }
  }

  if( type.size() == 0 )
  {
    std::string key(bKey);

    std::string capt(captAtt(var.name, n_cell_methods));
    if( iw == x_method.size() )
    {
      key += "733a";
      capt += "Undefined condition" + captVal(mode) ;
    }
    else
    {
      key += "733b";
      capt += "Condition" + captVal(mode) + "without a type";
    }

    if( notes->inq(key, var.name) )
    {
      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return true;  // no where |over clause specified
  }

  // only read once
  std::vector<std::string> vs_areaType;

  if( ! vs_areaType.size() && region_table.size() )
  {
    std::string file(tablePath + "/");
    ReadLine ifs( file + area_table ) ;

    if( ifs.isOpen() )
    {
      std::string line;
      while( ! ifs.getLine(line) )
        vs_areaType.push_back(line);

      ifs.close();
    }
  }

  // two conventions are available (chap 7.3.3). The one with a
  // type-variable takes precedence

  // typevar: a string-valued auxiliary variable with
  // a standard name of area_type
  int  tv_ix;
  int  sn_ix;
  bool isAreaTypeVar=false;

  // is the second convention given?
  if( (tv_ix = pIn->getVarIndex(type)) > -1 )
  {
     Variable& var_tv = pIn->variable[tv_ix];

     isAreaTypeVar = true;

     std::vector<std::string> txt;

     if( var_tv.isLabel )
     {
       if( var_tv.coord.isCoordVar  )
          txt.push_back("is not an auxiliary coordinate variable") ;

       if( (sn_ix=var_tv.getAttIndex(n_standard_name)) == -1
               || var_tv.attValue[sn_ix][0] != "area_type" )
         txt.push_back("has no " + n_standard_name + "=area_type") ;
     }
     else
       txt.push_back("is not string-valued") ;

     for( size_t i=0 ; i < txt.size() ; ++i )
     {
        if( notes->inq(bKey + "733d", var_tv.name) )
        {
          std::string capt(captAtt(var.name, n_cell_methods + ", 2nd convention"));
          capt += "Condition" + captVal(mode) ;
          capt += "declares area-type-";
          capt += captVar(var_tv.name, no_colon, no_blank) + ", which ";
          capt += txt[i] ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        return true;
     }
  }
  else
  {
     // check for the first convention
     if( !hdhC::isAmong(type, vs_areaType) )
     {
        if( notes->inq(bKey + "733c", var.name) )
        {
          std::string capt(captAtt(var.name, n_cell_methods + ", 1st convention"));
          capt += "The" + captVal(mode, false) ;
          capt += "-condition declares invalid area_type" + captVal(type,false) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        return true;
     }
  }

  if( isAreaTypeVar )
  {
    // check more rules for the second convention
    Variable& tVar = pIn->variable[tv_ix] ;

    if( mode == over )
    {
      // only a single string-valued item is allowed; note that
      // the last dimension is for the maximum string size.
      if( tVar.dimName.size() > 1 )
      {
        if( pIn->nc.getDimSize(tVar.dimName[0]) > 1 )
        {
          if( notes->inq(bKey + "733e", var.name) )
          {
            std::string capt(captAtt(var.name, n_cell_methods + ", 2nd convention"));
            capt += "Variable " + hdhC::sAssign("type2", tVar.name) ;
            capt += " in <where type1 over type2> should not have more than a single area-type string" ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }

          return true;
        }
      }
    }

    // for both where and over:
    // does the type-var provide valid area-type strings?
    std::vector<std::string> vs;
    std::vector<std::string> tvFail;
    pIn->nc.getData(vs, tVar.name);

    for(size_t l=0 ; l < vs.size() ; ++l)
    {
      vs[l] = hdhC::stripSurrounding(vs[l]);

      if( !hdhC::isAmong(vs[l], vs_areaType) )
        tvFail.push_back(vs[l]);
    }

    if( tvFail.size() )
    {
      if( notes->inq(bKey + "733f", var.name) )
      {
         std::string capt(captAtt(var.name, n_cell_methods + ", 2nd convention"));
         capt += "Type2-" + captVar(tVar.name, no_colon) ;
         if( tvFail.size() > 5 )
            capt += "with more than five invalid area_type strings";
         else
         {
           capt += "with invalid area_type=" ;
           capt += tvFail[0];
           for( size_t l=1 ; l < tvFail.size() ; ++l)
             capt += ", " + tvFail[l] ;
         }

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );

         return true;
      }
    }

    // the type-var is an auxiliary coordinate variable
    tVar.coord.isAny=true;

    int j = ca_ix[ vIx[var.name] ];
    if( j > -1 && !hdhC::isAmong(tVar.name, ca_vvs[j]) )
    {
      if( notes->inq(bKey + "5i", var.name) )
      {
         std::string capt("auxiliary " + captVar(tVar.name, false));
         capt += "name is not named in " + captAtt(n_coordinates) ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }

      return true;
    }
  }

  if( mode == where && method.find(over) < std::string::npos )
     return chap7_3_3(method, var, over) ;

  return false ;
}

bool
CF::chap7_3_4a(std::string& name)
{
   // no specific coordinate: area or valid standard name instead of
   // coordinate variable, scalar coordinate variable or dimension.
   // Return true if such a case is given.

   // name must not be the name of a dimension
   if( hdhC::isAmong(name, dimensions) )
     return false;

   // name must not be the name of a scalar variable
   for( size_t i=0 ; i < pIn->variable.size() ; ++i )
     if( pIn->variable[i].isScalar && pIn->variable[i].name == name )
       return false;

   if( name == n_area )
     return true;

   // special: a two common valid standard_name
   if( name == n_longitude || name == n_latitude )
        return true;

   // rule: name must not be the name of a dimension or a scalar variable
   // collection of correct names; these have been sorted out before.

  if( std_name_table.size() == 0 )  // cf-standard-name-table.xml
     return false;

  std::string file(tablePath + "/");

  ReadLine ifs( file + std_name_table ) ;
  ifs.clearSurroundingSpaces();

  if( ! ifs.isOpen() )
    // could not open
    return false ;

  Variable var;
  var.snTableEntry.found=false;

  scanStdNameTable(ifs, var, name) ;
  ifs.close();

  if( var.snTableEntry.found )
    return true;

  return false;
}

void
CF::chap7_3_4b(Variable& var,
  std::vector<std::string> &cm_name,
  std::vector<std::string> &cm_method )
{
  // cell_methods when there are no coordinates.
  // Except for entries whose cell_methods attribute is point, all numeric variables
  // and scalar coordinate vars named by cell_methods should have
  // bounds or climatology attributes.
  std::string dim;

  for( size_t l=0 ; l < cm_name.size() ; ++l )
  {
    if( cm_method[l].find("point") < std::string::npos )
      continue;

    std::vector<std::string> vs( pIn->nc.getDimNames());

    Split x_cm_name(cm_name[l], ": ");  // the dimensions in cell_methods

    for( size_t x=0 ; x < x_cm_name.size() ; ++x )
    {
      int ix = pIn->getVarIndex(x_cm_name[x]) ;

      if( ix == -1 &&
            ( x_cm_name[x] == n_area
                || x_cm_name[x] == n_longitude
                    || x_cm_name[x] == n_latitude) )
      {
        std::string meaning;
        std::string *kind;

        int i_lat, i_lon;
        i_lat = i_lon = -1 ;

        // test dimension names
        for( size_t k=0 ; k < vs.size() ; ++k )
        {
          if( vs[k] == n_longitude )
            i_lon = k;

          else if( vs[k] == n_latitude )
            i_lat = k;
        }

        // test coordinate variables
        for( size_t k=0 ; k < pIn->varSz ; ++k )
        {
          if( pIn->variable[k].name == n_longitude )
            i_lon = k;

          else if( pIn->variable[k].name == n_latitude )
            i_lat = k;

          else if( pIn->variable[k].coord.isX )
            i_lon = k;

          else if( pIn->variable[k].coord.isY )
            i_lat = k;
        }

        if( x_cm_name[x] == n_area )
        {
          if( i_lon == -1 && i_lat == -1 )
            meaning=n_global;
          else if( i_lon == -1 )
            meaning="zonal";
          else if( i_lat == -1 )
            meaning="meridional";

          if( meaning.size() )
            kind=&n_area;
        }

        else if( i_lon == -1 && x_cm_name[x] == n_longitude )
        {
          meaning="meridional";
          kind=&n_longitude;
        }

        else if( i_lat == -1 && x_cm_name[x] == n_latitude )
        {
          meaning="zonal";
          kind=&n_latitude;
        }

        if( meaning.size() && notes->inq(bKey + "734a", var.name) )
        {
          std::string capt("reco for " + captAtt(var.name, n_cell_methods)) ;
          capt += hdhC::sAssign("Name", *kind +":") + " for " + meaning ;
          capt += " scope should have size-one-dimensioned " + n_variable;

          if( meaning == n_global )
            capt += "s " + n_longitude + " and " + n_latitude ;
          else
            capt += " " + *kind ;

          capt += " with bounds" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  return;
}

void
CF::chap7_4a(void)
{
  if( !isCheck )
    return ;

  // index to the climatology variable, i.e. the bounds
  size_t cvIx;

  // climatological statistics. Try for multiple occurrences
  for( cvIx=0 ; cvIx < pIn->varSz ; ++cvIx )
  {
    Variable& var = pIn->variable[cvIx];

    // only time may have a climatology att
    if( var.isValidAtt(n_climatology) )
    {
      if( var.name == timeName )
        break ;
      else
      {
        if( notes->inq(bKey + "74c", var.name) )
        {
          std::string capt("only " + captVar(timeName, false));
          capt += "may have " + captAtt(n_climatology, no_colon) ;
          capt += ", found in " + captVar(var.name,no_colon, no_blank);

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  if( cvIx == pIn->varSz )
    return;  // no climatology

  // When there is a climatology, then at least one variable must have
  // a corresponding cell_methods attribute.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
    if( pIn->variable[i].isValidAtt(n_cell_methods) )
      return;

  if( notes->inq(bKey + "74b", pIn->variable[cvIx].name) )
  {
    std::string capt("a " + n_climatology + blank + n_variable) ;
    capt += " is available, but no data " + n_variable ;
    capt += " specifies " + captAtt(n_cell_methods);

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  return;
}

bool
CF::chap7_4b(Variable& var,
  std::vector<std::string> &cm_name,
  std::vector<std::string> &method)
{
  // return true for a valid syntax

  // a climatology must have at least two time: dimensions; find the indices
  std::vector<size_t> t_ix;
  for(size_t i=0 ; i < cm_name.size() ; ++i )
    if( cm_name[i] == (timeName + ":") )
       t_ix.push_back(i);

  int j;
  if( (j=pIn->variable[timeIx].getAttIndex(n_climatology)) > -1 )
  {
    int i = pIn->getVarIndex(pIn->variable[timeIx].attValue[j][0]);
    if( i > -1 )
    {
      pIn->variable[i].addAuxCount();
      var.push_aux( pIn->variable[i].name );
    }
  }

  // relation between attributes time:climatology and var:cell_methods

  // a) cell_methods without enough time: instances
  if( t_ix.size() < 2 )
  {
    if( j > -1 )
    {
      if( notes->inq(bKey + "74a", var.name) )
      {
        std::string capt(captAtt(var.name, n_cell_methods, no_colon)) ;
        capt += "is ill-formatted for a " + n_climatology ;

        std::string text("Found: " + var.name + ":");
        text += hdhC::sAssign(n_cell_methods, var.getAttValue(n_cell_methods));

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    return false;
  }

  // The [within|over days|years] option of cell_methods.
  // Corresponds to climatologies (chapter 7.4).
  // time: is the only dimension; this implies that there is no 'where' clause.
  // Note that str0 is a single blank-separated worid-list.

  std::string nextArg;  // within | over
  std::string within("within");
  std::string over("over");

  std::vector<std::string> period;
  period.push_back("years");
  period.push_back("days");

  if( t_ix.size() == 2 )
  {
     // two valid formats
     // 1) time: method1 within years time: method2 over years
     // 2) time: method1 within days time: method2 over days

     std::string& str0 = method[t_ix[0]];
     std::string& str1 = method[t_ix[1]];

     size_t j;
     std::string what;

     for( j=0 ; j < 2 ; ++j )
     {
       what = "use of key-word=within in";
       size_t pos;
       if( (pos=str0.find(within)) < std::string::npos)
       {
         what="period after key-word=within,";
         if( str0.find(period[j], pos) < std::string::npos )
         {
            if( (pos=str1.find(over)) < std::string::npos )
            {
              what= "use of key-word=over in";
              if( str1.find(period[j], pos)< std::string::npos )
              {
                what.clear();
                break;
              }
              else
              {
                what="period after key-word=over in";
                break;
              }
            }
         }
       }
     }

     if( what.size() )
     {
        if( notes->inq(bKey + "74d", var.name) )
        {
          std::string capt( captAtt(var.name, n_cell_methods + " for a climatology"));
          capt += "Invalid " + what  ;
          capt += captVal(var.getAttValue(n_cell_methods));

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        return false;
     }
  }

  else if( t_ix.size() == 3 )
  {
     // valid format
     // time: method1 within days
     //    time: method2 over days
     //       time: method2 over years
     std::string& str0 = method[t_ix[0]] ;
     std::string& str1 = method[t_ix[1]] ;
     std::string& str2 = method[t_ix[2]] ;

     int state;
     size_t pos;

     state=0;
     if( (pos=str0.find(within)) < std::string::npos )
     {
       if( str0.find(period[1], pos) < std::string::npos )
       {
         state=1;
         if( (pos=str1.find(over)) < std::string::npos )
         {
           if( str1.find(period[1], pos) < std::string::npos )
           {
             state=2;
             if( (pos=str2.find(over)) < std::string::npos )
             {
               if( str2.find(period[0], pos) < std::string::npos )
                 state = -1;  // aboslutely valid
             }
           }
         }
       }
     }

     if( state > -1 )
     {
        if( notes->inq(bKey + "74d", var.name) )
        {
          std::string capt( captAtt(var.name, n_cell_methods)) ;
          capt += "Invalid condition" + captVal(method[t_ix[state]],false) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        return false;
     }
  }
  else
    return false;  // don't know what so many time: occurrences could be

  // found valid cell_methods for a climatology,
  // but a corresponding attribute was not defined in the time variable
  if( ! pIn->variable[timeIx].isValidAtt(n_climatology) )
  {
    if( notes->inq(bKey + "74e", var.name) )
    {
      std::string capt
        (captAtt(var.name, n_cell_methods + " for climatologies")) ;
      capt += captAtt(timeName, n_climatology, no_colon, s_upper) + "is missing" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return true ;
}

void
CF::chap8(void)
{
  // packed data
  if( ! notes->inq(bKey + "8", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    chap8_1(var) ; // packed data (scale_factor, add_offset)
    chap8_2(var) ; // compression by gathering
  }

  return ;
}

void
CF::chap8_1(Variable& var)
{
  if( !isCheck )
    return ;

  // packed data (scale_factor, add_offset)
  // CF doesn't state that both must be available.
  // Default values scale_factor=1 and add_offset=0 are reasonable.

  std::vector<nc_type> attType;
  std::vector<std::string> attStr;
  bool attBool[] = { false, false };
  int att_ix = -1;  // if an att is omitted, then the index to the other one

  attStr.push_back("scale_factor");
  attStr.push_back("add_offset");

  for( size_t i=0 ; i < attStr.size() ; ++i )
    attType.push_back( NC_NAT );

  for( size_t i=0 ; i < attStr.size() ; ++i )
  {
    attBool[i]=false;

    int j;
    if( (j=var.getAttIndex(attStr[i])) > -1 )
    {
      attBool[i]=true;
      att_ix=i;
      attType[i] = var.attType[j] ;
    }
  }

  // not packed
  if( ! (attBool[0] || attBool[1] ) )
    return;

  if( attBool[0] && attBool[1] )
  {
    if( attType[0] != attType[1] )
    {
      if( notes->inq(bKey + "81a", var.name) )
      {
        std::string capt(captVar(var.name)) ;
        capt += captAtt(s_empty,
                attStr[0] + "<" + pIn->nc.getTypeStr(attType[0]) + ">", s_upper, no_colon);
        capt += "and " + attStr[1]+ "<" + pIn->nc.getTypeStr(attType[1]) + ">";
        capt += " have to be the same type";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      return;
    }
  }

  // var and att of different type
  if( (attBool[att_ix] && (attType[att_ix] != var.type) ) )
  {
    // the variable must be a particular type
    if( ! (var.type == NC_INT || var.type == NC_BYTE || var.type == NC_SHORT) )
    {
      if( notes->inq(bKey + "81b", var.name) )
      {
        std::string capt("type of " + captVar(var.name, false));
        capt += "should be int, short or byte, because" ;

        capt += " it is different from the type of scale_factor|add_offset," ;

        std::string text(var.name + ":");
        if( attBool[0] )
        {
          text += attStr[0] + " type=";
          text += pIn->nc.getTypeStr(attType[0]);
        }
        if( attBool[0] && attBool[1] )
        {
          text += ", " ;
          text += var.name + ":";
        }
        if( attBool[1] )
        {
          text += attStr[1] +" type=";
          text += pIn->nc.getTypeStr(attType[1]);
        }
        text += "\n";
        text += var.name;
        text += " type=";
        text += pIn->nc.getTypeStr(var.type);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // var and att still of different type
    if( !( (attType[att_ix] == NC_FLOAT || attType[att_ix] == NC_DOUBLE ) ) )
    {
       if( notes->inq(bKey + "81c", var.name) )
       {
         std::string capt("types are different between " + captVar(var.name, no_colon));
         capt += "and " + n_attribute + "s" ;
         capt += captVal(attStr[0]) + "and" + captVal(attStr[1], false) ;
         capt += ", which have to be float or double";

         std::string text("Found: " + var.name + ":");
         if( attBool[0] )
         {
           text += attStr[0] + " type=";
           text += pIn->nc.getTypeStr(attType[0]);
         }
         if( attBool[0] && attBool[1] )
         {
           text += ", " ;
           text += var.name + ":";
         }
         if( attBool[1] )
         {
           text += attStr[1] +" type=";
           text += pIn->nc.getTypeStr(attType[1]);
         }
         text += "\n";
         text += var.name;
         text += " type=";
         text += pIn->nc.getTypeStr(var.type);

         (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }

  // recommendation
  // if scale_factor and add_offset are type float, the variable should not be int
  if( attType[0] == NC_FLOAT && attType[1] == NC_FLOAT
         && var.type == NC_INT )
  {
     if( notes->inq(bKey + "81d", var.name) )
     {
       std::string capt(captVar(var.name));
       capt += "If scale_factor and add_offset are" ;
       capt += captVal("float", false) + ", then the ";
       capt += n_variable + " should not be" + captVal("int") ;

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
  }

  // variable, valid_ min, max and range must be the same type
  if( var.isValidAtt(n_valid_range) || var.isValidAtt(n_valid_min) ||
         var.isValidAtt(n_valid_max) )
  {
    int j;
    bool isNotType[]={ false, false, false };

    if( (j=var.getAttIndex(n_valid_range)) > -1
           && var.attType[j] != var.type )
      isNotType[0] = true;
    if( (j=var.getAttIndex(n_valid_min)) > -1
           && var.attType[j] != var.type )
      isNotType[1] = true;
    if( (j=var.getAttIndex(n_valid_max)) > -1
           && var.attType[j] != var.type )
      isNotType[2] = true;

    if( isNotType[0] || isNotType[1] || isNotType[2] )
    {
      if( notes->inq(bKey + "81e", var.name) )
      {
        std::string capt("types of " + captVar(var.name, no_colon) );
        capt += "and " + n_attribute ;

        if( (isNotType[0] && isNotType[1])
              || (isNotType[0] && isNotType[2])
                 || (isNotType[1] && isNotType[2]) )
           capt += "s";
        capt += blank;

        if( isNotType[0] )
           capt += n_valid_range ;

        if( isNotType[1] )
        {
           if( isNotType[0] )
           {
             if( isNotType[2] )
               capt += ", " ;
             else
               capt += " and " ;
           }
           capt += n_valid_min ;
        }

        if( isNotType[2] )
        {
           if( isNotType[0] && isNotType[1] )
             capt += "," ;
           if( isNotType[0] || isNotType[1] )
             capt += " and " ;
           capt += n_valid_max ;
        }

        capt += " have to be the same" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  return;
}

void
CF::chap8_2(Variable& var)
{
  // compression by gathering
  size_t sz = pIn->varSz;

  std::string compressVal;
  Split x_cmp;

  // any evidence for a compressed-index variable? Tests also for int-type
  bool isIntType=false;
  bool isEvidence=isCompressEvidence(var, &isIntType);
  compressVal = var.getAttValue(n_compress, lowerCase);
  x_cmp = compressVal;

  bool isFailed=false;

  if( var.isCompress )
  {
    var.addAuxCount() ;

    if(isCheck && !isIntType)
    {
      if( notes->inq(bKey + "82a", var.name) )
      {
        std::string capt(captAtt(var.name, n_compress, no_colon)) ;
        capt += "is specified, but the " + n_variable;
        capt += " is not index-type, found";
        capt += captVal(pIn->nc.getTypeStr(var.type), false) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      isFailed=true;
    }

    for( size_t j=0 ; j < x_cmp.size() ; ++j )
    {
      for( size_t jx=0 ; jx < sz ; ++jx )
        if( pIn->variable[jx].name == x_cmp[j] )
          pIn->variable[jx].addAuxCount() ;

      // compress must only declare dims
      if( !hdhC::isAmong(x_cmp[j], dimensions) )
      {
        if( notes->inq(bKey + "82b", var.name) )
        {
          std::string capt(captAtt(var.name, n_compress, no_colon)) ;
          capt += "names a non-existing " + n_dimension ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        isFailed=true;
      }
    }
  }

  else if( isCheck && isEvidence )
  {
     if( ! ( cFVal > 15
         && pIn->variable[pIn->varSz].isValidAtt(n_featureType) ) )
     {
       // Note: CF-1.6 allows scalar descriptive variables not in compress context
       if( notes->inq(bKey + "82a", var.name) )
       {
         std::string capt(cFVersion + ": " + captVal(var.name)) ;
         capt += "is estimated as compressed-index " + n_variable ;
         capt += ", but a " + captAtt(n_compress) + "is missing";

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }

       isFailed=true;
     }
  }

  if( isFailed )
    return;


  // test for index-range under-|overflow;
  // this is only reasonable, when the checks above didn't fail
  if( compressVal.size() )
  {
    // get dimensions
    std::vector<int> vDims;
    for( size_t j=0 ; j < x_cmp.size() ; ++j )
       vDims.push_back( pIn->nc.getDimSize(x_cmp[j]) );

    int d_max=1;
    for(size_t i=0 ; i < vDims.size() ; ++i )
       d_max *= vDims[i] ;

    //get compressed indices
    MtrxArr<int> ma;
    pIn->nc.getData(ma, var.name );
    int ma_min=d_max+1;
    int ma_max=-1;
    for( size_t i=0 ; i < ma.size() ; ++i )
    {
       if( ma_max < ma[i] )
          ma_max = ma[i] ;
       if( ma_min > ma[i] )
          ma_min = ma[i] ;
    }

    bool is=false;

    if( ma_min <= 0 )
    {
       if( ma_min == 0 )
       {
         if( notes->inq(bKey + "82c", var.name) )
         {
           std::string capt("warning for FORTAN: Compressed indices values of ") ;
           capt += n_variable + captVal(var.name) + "apply C index convention";

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
       else
         is=true;
    }

    if( ma_max >= d_max )
    {
       if( ma_max == d_max )
       {
         if( notes->inq(bKey + "82d", var.name) )
         {
           std::string capt("warning for C/C++: Compressed indices of ");
           capt += captVar(var.name, false) ;
           capt += "apply FORTRAN index convention";

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
       else
         is=true;
    }

    if(is)
    {
      if( notes->inq(bKey + "82e", var.name) )
      {
        std::string capt("compress-" + captVar(var.name)) ;
        capt += "A data index exceeds maximum number of data points, found " ;
        capt += hdhC::sAssign("index", hdhC::itoa(ma_max));
/*
        for(size_t i=0 ; i < vDims.size() ; ++i )
        {
           if( i )
             capt += " x " ;
           capt += hdhC::itoa(vDims[i]);
        }
*/
        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  return ;
}

void
CF::chap9(void)
{
  if( ! notes->inq(bKey + "9", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  // discrete sampling geometries
  std::vector<std::string*> dsg_att;

  dsg_att.push_back(&n_featureType);
  dsg_att.push_back(&n_cf_role);
  dsg_att.push_back(&n_instance_dimension);
  dsg_att.push_back(&n_sample_dimension);

  if( cFVal < 16 )
  {
    std::vector<std::string> coll;

    for( size_t i=0 ; i < pIn->variable.size() ; ++i )
    {
      Variable& var = pIn->variable[i] ;
      for( size_t j=0 ; j < dsg_att.size() ; ++j )
      {
         if( var.isValidAtt(*(dsg_att[j])) )
         {
            if( !hdhC::isAmong(*(dsg_att[j]), coll) )
               coll.push_back(*(dsg_att[j])) ;
         }
      }
    }

    if( coll.size() )
    {
       if( notes->inq(bKey + "0a") )
       {
         std::string capt("warning: Usage of a CF-1.6 specific attributes in ");
         capt += cFVersion ;
         capt += ", found" ;
         for( size_t i=0 ; i < coll.size() ; ++i )
         {
           if(i)
             capt += ",";
           capt += captVal(coll[i],false) ;
         }

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }

    return;
  }

  // checks are performed for case insensitivity.
  std::vector<std::string> validFT_vs;
  validFT_vs.push_back("point");
  validFT_vs.push_back("timeseries");
  validFT_vs.push_back("profile");
  validFT_vs.push_back("trajectory");
  validFT_vs.push_back(validFT_vs[1]+validFT_vs[2]);
  validFT_vs.push_back(validFT_vs[3]+validFT_vs[2]);

  // collection of strings of found attributes

  // >CF-1.5: global attribute
  std::vector<std::string> vs_featureType;

  Variable& glob = pIn->variable[pIn->varSz] ;

  // Get the featureType(s). Note that only a single one is expected by CF-1.6
  chap9_featureType(validFT_vs, vs_featureType) ;

  // warnings for attributes cf_role|instance_dimension|sample_dimension
  // which are confusing when global
  for( size_t i=1 ; i < dsg_att.size() ; ++i )
  {
     if( glob.isValidAtt(*(dsg_att[i])) )
     {
        if( notes->inq(bKey + "9c", n_global) )
        {
          std::string capt("warning: " + captVal(*(dsg_att[i]))) ;
          capt += "should not be a global " + n_attribute;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
     }
  }

  // to be used below
  std::vector<int> xyzt_ix;
  std::vector<size_t> dv_ix;
  chap9_getSepVars(xyzt_ix, dv_ix);

  // check n_instance_dimension properties
  chap9_sample_dimension(dv_ix);

  // scan variables for occurrences of attribute cf_role attribute,
  // sample_dimension

  std::vector<std::string> vs_cr;
  vs_cr.push_back("timeseries_id") ;
  vs_cr.push_back("profile_id") ;
  vs_cr.push_back("trajectory_id") ;

  std::vector<std::pair<int, int> > vp_cf_role_ix;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    // look for an unknown cf_role value
    Variable& var = pIn->variable[i] ;

    int j=-1;
    if( (j=var.getAttIndex(n_cf_role)) > -1 )
    {
      vp_cf_role_ix.push_back( std::pair<int, int>(static_cast<int>(i),j ) );

      if( !hdhC::isAmong(var.attValue[j][0], vs_cr) )
      {
        if( notes->inq(bKey + "9e", var.name) )
        {
          std::string capt(captAtt(var.name, n_cf_role)) ;
          capt += "Invalid value";
          capt += captVal(var.attValue[j][0], false);

          // amend an omitted trailing '_id'
          for( size_t l=0 ; l < vs_cr.size() ; ++l )
          {
            if( (var.attValue[j][0] + "_id") == vs_cr[l] )
            {
               var.attValue[j][0] += "_id" ;
               capt += ", missing trailing _id " ;
               break;
            }
          }

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

    }
  }

  // infer the featureType by the layout
  std::vector<std::string> guessedFT(
      chap9_guessFeatureType(validFT_vs, xyzt_ix, dv_ix) );

  if( vs_featureType.size() )
  {
    // do cf_role and featureType comply?
    for( size_t i=0 ; i < vp_cf_role_ix.size() ; ++i )
    {
      std::string str =
          pIn->variable[vp_cf_role_ix[i].first].attValue[vp_cf_role_ix[i].second][0] ;

      str = str.substr(0, str.size() - 3);
      std::string s0( hdhC::Lower()(str));

      size_t j;
      for(j=0 ; j < vs_featureType.size() ; ++j)
        if( vs_featureType[j].find(s0) < std::string::npos )
          break;

      if( j == vs_featureType.size() )
      {
        if( notes->inq(bKey + "9f", n_global) )
        {
          std::string capt(n_global + blank +
                  captAtt(hdhC::sAssign(n_featureType, vs_featureType[0])));
          capt += "is not compatible with " ;
          capt += captAtt(hdhC::sAssign(n_cf_role, str +"_id")) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }

    // any label without cf_role?
    bool isPoint=false;
    if( vs_featureType.size()  && hdhC::Lower()(vs_featureType[0]) == "point" )
      isPoint=true;
    else if( guessedFT.size()  && hdhC::Lower()(guessedFT[0]) == "point" )
      isPoint=true;

    if( !isPoint )
    {
      for( size_t i=0 ; i < pIn->varSz ; ++i )
      {
        Variable var_l = pIn->variable[i];

        if( var_l.isLabel && !var_l.isValidAtt(n_cf_role) )
        {
          if( notes->inq(bKey + "9h", n_global) )
          {
            std::string capt("Reco: Variable" + captVal(var_l.name));
            capt += "should have " + captAtt(n_cf_role);

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }

    // compare to a guessed FT
    if( isFeatureType && guessedFT.size() )
    {
      size_t i;
      bool is=true;
      for( i=0 ; i < vs_featureType.size() ; ++i )
      {
        size_t j;
        size_t sz = guessedFT.size() ;

        for( j=0 ; j< sz ; ++j )
        {
          if( guessedFT[j].find(vs_featureType[i]) < std::string::npos )
          {
            is=false;
            break;
          }
        }

        if( j == sz )
        {
          for( j=0 ; j < sz ; ++j )
          {
            if( vs_featureType[i].find(guessedFT[j]) < std::string::npos )
            {
              is=false;
              break;
            }
          }
        }

        if(is)
          break;
      }

      if( is )
      {
        if( notes->inq(bKey + "9g", n_global) )
        {
          std::string capt("mismatch between provided ");
          if( vs_featureType.size() == 1 )
          {
            capt += captAtt(hdhC::sAssign(n_featureType, vs_featureType[0])) ;
          }
          else
          {
            capt += captAtt(n_featureType) ;
            for( size_t i=0 ; i < vs_featureType.size() ; ++i )
            {
              if(i)
                capt += ",";
              else
                capt += "<";

              capt += vs_featureType[i];
            }
            capt += ">";
          }
          capt += " and " ;
          if( guessedFT.size() > 1 )
            capt += "ambiguous " ;
          capt += "guess: ";
          for( size_t i=0 ; i < guessedFT.size() ; ++i )
          {
            if(i)
              capt += ",";
            capt += guessedFT[i];
          }

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }
  else if( guessedFT.size() )
  {
    // reco for orthogonal multidimensions array representation
    for( size_t i=0 ; i < dv_ix.size() ; ++i)
    {
      Variable& var_dv = pIn->variable[ dv_ix[i] ] ;

      if( chap9_orthoMultDimArray(var_dv, xyzt_ix) )
      {
        if( notes->inq(bKey + "94a", n_global, NO_MT) )
        {
          std::string capt("Reco: File with orthogonal multi-dimensional array ");
          capt += captVar(var_dv.name, no_colon) + "should have global " ;
          capt += captAtt(n_featureType) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
      else
      {
        if( notes->inq(bKey + "9i", n_global, NO_MT) )
        {
          std::string capt("suspecting missing " );
          capt += captAtt(n_featureType, no_colon, no_blank) ;
          capt += ", " ;
          if( guessedFT.size() > 1 )
            capt += "ambiguous " ;
          capt += "guess: ";
          for( size_t i=0 ; i < guessedFT.size() ; ++i )
          {
            if(i)
              capt += ",";
            capt += guessedFT[i];
          }

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        isFeatureType = true;
      }
    }
  }


  // missing value
  chap9_MV(dv_ix) ;

  return;
}

void
CF::chap9_featureType(
  std::vector<std::string> &validFT_vs,
  std::vector<std::string> &vs_featureType)
{
  Variable& glob = pIn->variable[pIn->varSz] ;

  // for checking misspelling by blanks or multiple occurrences
  int att_ix;
  if( (att_ix=glob.getAttIndex(n_featureType)) > -1 )
  {
     isFeatureType=true;

     // check for these invalid cases:
     // 1) misspelling by separation
     // 2) CF-v1.6: multiple featureTypes
     // 3) misspelled
     Split x_str;
     x_str.setSeparator(" ,");

     for(size_t i=0 ; i < glob.attValue[att_ix].size() ; ++i )
     {
        x_str = hdhC::Lower()(glob.attValue[att_ix][i]) ;

        for( size_t x=0 ; x < x_str.size() ; ++x)
        {
          double editDistance, eDMin;
          size_t eDMin_ix;
          size_t k;
          eDMin=100.;
          for( k=0 ; k < validFT_vs.size() ; ++k)
          {
            std::string s=validFT_vs[k];
            editDistance = wagnerFischerAlgo(x_str[x], validFT_vs[k]);

            if( editDistance < eDMin )
            {
              eDMin = editDistance ;
              eDMin_ix = k;
            }

            if( editDistance == 0 )
              break;
          }

          if( eDMin < 0.5 && x_str[x] != validFT_vs[eDMin_ix] )
          {
            if( notes->inq(bKey + "0h", n_global) )
            {
              std::string capt("is " + captAtt(s_empty,
                      hdhC::sAssign(n_featureType, x_str[x]), no_colon, no_blank) ) ;
              capt += " misspelled? Did you mean " ;
              capt += validFT_vs[eDMin_ix] + "?";

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }

            // correction for internal use
            vs_featureType.push_back(validFT_vs[eDMin_ix]);
            continue;
          }

          vs_featureType.push_back(x_str[x]);
        }
     }
  }

  if( cFVal == 16 && vs_featureType.size() > 1 )
  {
    if( notes->inq(bKey + "9a", n_global) )
    {
      std::string capt("CF-1.6 expects only a single value for global ");
      capt += captAtt(n_featureType, no_colon, no_blank) + ", found" ;
      for( size_t i=0 ; i < vs_featureType.size() ; ++i)
      {
        if(i)
          capt += ",";
        capt += captVal(vs_featureType[i], false) ;
      }

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  for( size_t j=0 ; j < vs_featureType.size() ; ++j)
  {
    bool is=true;
    for( size_t i=0 ; i < validFT_vs.size() ; ++i)
    {
      if( validFT_vs[i] == vs_featureType[j] )
      {
         is=false;
         break;
      }
    }

    if(is)
    {
      if( notes->inq(bKey + "9d", n_global) )
      {
        std::string capt("invalid " + n_global + blank ) ;
        capt += captAtt(n_featureType, no_colon, no_blank) ;
        capt += captVal(vs_featureType[j]) ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  std::vector<std::pair<size_t, int> > cf_role_ix;
  std::vector<std::pair<size_t, int> > instance_dim_ix;
  std::vector<std::pair<size_t, int> > sample_dim_ix;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    // featureType should only be a global attribute
    std::string str(var.getAttValue(n_featureType, lowerCase)) ;
    if( str.size() )
    {
       if( notes->inq(bKey + "9b", var.name) )
       {
         std::string capt("warning: ");
         capt += captAtt(var.name, n_featureType, no_colon, s_upper) ;
         capt += "is confusing as " + n_attribute + " of ";
         capt += captVar(var.name, false);

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }

    int j;
    if( (j=var.getAttIndex(n_cf_role)) > -1 )
      cf_role_ix.push_back( std::pair<size_t, int>(i, j) ) ;

    if( (j=var.getAttIndex(n_instance_dimension)) > -1 )
      instance_dim_ix.push_back( std::pair<size_t, int>(i, j) ) ;

    if( (j=var.getAttIndex(n_sample_dimension)) > -1 )
      sample_dim_ix.push_back( std::pair<size_t, int>(i, j) ) ;
  }

  return ;
}

void
CF::chap9_getSepVars(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
   for( size_t j=0 ; j < 4 ; ++j )
      xyzt_ix.push_back(-1) ;

   for(size_t i=0 ; i < pIn->varSz ; ++i )
   {
      bool isXYZT[4];
      isXYZT[0] = pIn->variable[i].coord.isX ;
      isXYZT[1] = pIn->variable[i].coord.isY ;
      isXYZT[2] = pIn->variable[i].coord.isZ ;
      isXYZT[3] = pIn->variable[i].coord.isT ;

      size_t m;
      for(m=0 ; m < 4 ; ++m)
      {
        // for multiple coordinates for a given type,
        // select the one with the lowest number of dimensions
        if( isXYZT[m] )
        {
          if(xyzt_ix[m] > -1 )
          {
            Variable& var_0 = pIn->variable[ xyzt_ix[m] ] ;
            Variable& var_1 = pIn->variable[i] ;
            if( var_0.dimName.size() < var_1.dimName.size() )
              continue;
          }

          xyzt_ix[m] = i ;
          break;
        }
      }

      if( m == 4 && pIn->variable[i].isDataVar() && !pIn->variable[i].isScalar )
        dv_ix.push_back( i );
   }

   return;
}

std::vector<std::string>
CF::chap9_guessFeatureType(std::vector<std::string> &validFT_vs,
  std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix )
{
   std::vector<std::string> guessedFT;

   if( dv_ix.size() && chap9_horizontal(xyzt_ix) )
   {
      // for each variant of all featureTypes: when X and Y are given, they
      // share the same dimensions. Only the X-index will be used below.
      // When y s given only, the index was mapped to xyzt[0].

      bool isPoint;
      if( (isPoint=chap9_point(xyzt_ix, dv_ix)) )
        guessedFT.push_back( validFT_vs[0] ) ;

      if( chap9_timeSeries(xyzt_ix, dv_ix) )
        guessedFT.push_back( validFT_vs[1] ) ;

      if( chap9_profile(xyzt_ix, dv_ix) )
        guessedFT.push_back( validFT_vs[2] ) ;

      // note that there could be a case which is point and trajectory
      if( chap9_trajectory(xyzt_ix, dv_ix) )
        guessedFT.push_back( validFT_vs[3] ) ;

      if( chap9_timeSeriesProfile(xyzt_ix, dv_ix) )
        guessedFT.push_back( validFT_vs[4] ) ;

      if( chap9_trajectoryProfile(xyzt_ix, dv_ix) )
        guessedFT.push_back( validFT_vs[5] ) ;
   }

   return guessedFT ;
}

bool
CF::chap9_horizontal(std::vector<int>& xyzt_ix)
{
  // when both horizontal dimensions X and Y are given, then
  // they are identical for all featureTypes.
  // If X is not in use, then Y is mapped to X.

  if( xyzt_ix[0] == -1 && xyzt_ix[1] == -1 )
     return false;  // not a featureType

  else if( xyzt_ix[0] == -1)
    xyzt_ix[0] = xyzt_ix[1] ; // and true

  //else if(xyzt_ix[1] == -1) // X is in use
  //  is=true;

  else if( xyzt_ix[0] > -1)
  {
    Variable& var_x = pIn->variable[ xyzt_ix[0] ] ;
    Variable& var_y = pIn->variable[ xyzt_ix[1] ] ;

    for(size_t i=0 ; i < var_x.dimName.size() ; ++i )
      if( var_x.dimName[i] != var_y.dimName[i] )
         return false;
  }

  return true;
}

void
CF::chap9_MV(std::vector<size_t>& dv_ix)
{
  /* the chapter of CF-1.6 about Missing Data is impractical as a guide
     for a check. Thus, a hierachical procedure is used here to annotate
     any combination between auxiliaries and data variables as to MV.
     0) MV of a scalar aux that is named by a coordinate attribute
        ( this has been checked before).
     1) index considerations between 1-D auxiliaries and corresponding data vars.
     2) multi-dim indexes of auxiliaries and corresponding data variables.
  */
/*
  if( ! notes->inq(bKey + "9.6", s_empty, "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

  }
*/

  return ;
}

bool
CF::chap9_orthoMultDimArray(Variable& var, std::vector<int>& xyzt_ix)
{
  // characteristics of OMDA:
  // a) each coordinate variable depends on only a single dimension,
  // b) the data variable has to depend on all the dimensions of a),
  // c) the data variable has to have more than a single dimension.
  std::vector<std::string> vsd;

  for(size_t j=0 ; j < xyzt_ix.size() ; ++j)
  {
    int c = xyzt_ix[j];
    if( c > -1 )
    {
      Variable& var_c = pIn->variable[c];

      if( var_c.dimName.size() != 1 )
        return false;

      vsd.push_back( var_c.dimName[0] ) ;
    }
  }

  if( var.dimName.size() > 1
        && hdhC::isAmong(vsd, var.dimName, true) )
    return true;

  return false;
}

bool
CF::chap9_point(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // dv_ix.size() > 0 was checked before

  // all with a single, identical dimension.

  // Two exceptions with a second special dimension:
  // a) var of char Type,
  // b) bounds with vertices.
  // Note that this is not covered by CF-1.6.

  if( effDims_ix.size() != 1 )
    return false;

  std::string& dName = dimensions[ effDims_ix[0] ] ;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( !var.dimName.size() || var.dimName[0] != dName )
      return false;
  }

  return true;
}

bool
CF::chap9_profile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // def.: an ordered set of datapoints along a vertical line at a fixed
  // horizontal position and fixed time.


  // dv_ix.size() > 0 was checked before

  // dv_ix.size() > 0 was checked before
  int x_ix = xyzt_ix[0] ;
//  int y_ix = xyzt_ix[1] ;
  int z_ix = xyzt_ix[2] ;
  int t_ix = xyzt_ix[3] ;

  if( z_ix == -1 )
    return false;

  Variable& var_z = pIn->variable[z_ix] ;

  // a profile should have more than a single Z value
  if( var_z.isScalar )
    return false;
  if(var_z.dimName.size() == 1 && pIn->nc.getDimSize(var_z.dimName[0]) == 1)
    return false;

  // dims of time must be identical to those of the geo-coords
  if( x_ix > -1 )
  {
    if( t_ix == -1 )
      return false;

    Variable& var_x = pIn->variable[x_ix] ;
    Variable& var_t = pIn->variable[t_ix] ;

    if( var_t.dimName.size() > 1 )
      return false;
    else if( var_t.dimName.size() != var_x.dimName.size() )
      return false;
    else
    {
      for(size_t i=0 ; i < var_t.dimName.size() ; ++i)
        if( var_x.dimName[i] != var_t.dimName[i] )
          return false;
    }
  }

  // dims of Z are not shared by X
  if( x_ix > -1 )
  {
    Variable& var_x = pIn->variable[x_ix] ;

    if( hdhC::isAmong(var_z.dimName, var_x.dimName) )
      return false;
  }

  // dims of Z are not shared by T
  if( t_ix > -1 )
  {
    Variable& var_t = pIn->variable[t_ix] ;

    if( hdhC::isAmong(var_z.dimName, var_t.dimName) )
      return false;
  }

  // dims of Z must share those of the data variable
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

    // false: all items of the first vector must be in the second one
    if( ! hdhC::isAmong(var_z.dimName, var.dimName, false) )
      return false;
  }

  return true;
}

void
CF::chap9_sample_dimension(std::vector<size_t>& dv_ix)
{
  std::vector<size_t> vs_sd_ix;

  // scan for attribute sample_dimension
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    int j=-1;
    if( (j=var.getAttIndex(n_sample_dimension)) > -1 )
    {
      var.addAuxCount();

      if( !pIn->nc.isIndexType(var.name) )
      {
        if( notes->inq(bKey + "932d", var.name) )
        {
          std::string capt(captVar(var.name));
          capt += captAtt(n_sample_dimension) ;
          capt += "is only allowed for index variables";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue ;
      }

/*
      if( var.dimName.size() != 1 )
      {
        if( notes->inq(bKey + "932ax", var.name) )
        {
          std::string capt(captVar(var.name, no_colon));
          capt += "with " + captAtt(n_sample_dimension, s_lower) ;
          capt += " should not have multiple dimensions, found";
          capt += captVal(var.getDimNameStr()) ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue ;
      }
*/

      vs_sd_ix.push_back(i);

      // does the dimension named by sample_dimension exist?
      int dSz = pIn->nc.getDimSize( var.attValue[j][0]) ;
/*
      if( dSz < 0 )
      {
        if( notes->inq(bKey + "932bx", var.name) )
        {
          std::string capt(captVar(var.name));
          capt += "with " + captAtt(n_sample_dimension) ;
          capt += hdhC::sAssign(var.attValue[j][0]) ;
          capt += "does not match any dimension";

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue;
      }
*/

      // does the sum of values of the sample_dim variable match
      // the dimenision indicated by the attribute
      MtrxArr<int> ma;
      var.getData(ma, 0, pIn->nc.getDimSize(var.dimName[0]));
      int sum=0;
      if( ma.validRangeBegin.size() == 1 )
      {
        for(size_t i=ma.validRangeBegin[0] ; i < ma.validRangeEnd[0] ; ++i )
          sum += ma[i] ;

        if( sum != dSz )
        {
          if( notes->inq(bKey + "932a", var.name) )
          {
            std::string capt(captVar(var.name + " with " + n_sample_dimension)) ;
            capt += "The sum of data values=" + hdhC::itoa(sum);

            if( sum > dSz )
              capt += " over";
            else
              capt += " under";

            capt += "-determines " + n_dimension ;
            capt += captVal(var.attValue[j][0] + "=" + hdhC::itoa(dSz)) ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
      else if( notes->inq(bKey + "932b", var.name) )
      {
        std::string capt(captVar(var.name, no_colon));
        capt += "with " +  captAtt(n_sample_dimension, no_colon);
        if(ma.validRangeBegin.size() == 0 )
//          capt += "must not have missing values";
//        else
          capt += "must have data" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  // Any missing sample_dimension attribute?
  // All data variables have to depend on only a single dim, which
  // would be the one stated by a missed sample_dimension attribute.
  std::string name;
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;
    if( var.dimName.size() != 1 )
      return;

    if(i==0)
      name = var.dimName[0];
    else if( var.dimName[0] != name && !pIn->nc.isIndexType(var.name) )
      return;
  }

  std::vector<int> vs_dSz;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    // trivial: the variable must not have a sampe_dimension att
    if( hdhC::isAmong(i, vs_sd_ix) )
      continue ;

    Variable& var = pIn->variable[i] ;

    // the variable of a missed att must be index type.
    if( var.dimName.size() == 1 && pIn->nc.isIndexType(var.name) )
    {
      // the sum of values must match the dimension of the data variables

      // the size of the dimension whose name would be assign to sample_dimension
      int dSz = pIn->nc.getDimSize(name) ;

      MtrxArr<int> ma;

      var.getData(ma, 0, pIn->nc.getDimSize(var.dimName[0]));
      int sum=0;
      if( ma.validRangeBegin.size() )
      {
        for(size_t j=ma.validRangeBegin[0] ; j < ma.validRangeEnd[0] ; ++j )
          sum += ma[j] ;

        if( sum == dSz )
        {
          var.addAuxCount();

          if( notes->inq(bKey + "932c", var.name) )
          {
            std::string capt(captVar(var.name));
            capt += "The sum of values of an indexing variable suggests a missing " ;
            capt += captAtt(n_sample_dimension, no_colon);

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  return ;
}

bool
CF::chap9_timeSeries(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // dv_ix.size() > 0 was checked before
  int x_ix = xyzt_ix[0] ;
  int z_ix = xyzt_ix[2] ;
  int t_ix = xyzt_ix[3] ;

  if( t_ix == -1 )
    return false;

  Variable& var_t = pIn->variable[t_ix] ;

  // a TS should have more than a single time value
  if( var_t.isScalar )
    return false;
  if(var_t.dimName.size() == 1 && pIn->nc.getDimSize(var_t.dimName[0]) == 1)
    return false;

  // Z should be only scalar (per X or at all)
  if( z_ix > -1 )
  {
    Variable& var_z = pIn->variable[z_ix] ;
    if( !var_z.isScalar )
    {
      if( x_ix > -1 )
      {
        Variable& var_x = pIn->variable[x_ix] ;
        if( var_z.dimName.size() == var_x.dimName.size() )
        {
          for(size_t i=0 ; i < var_x.dimName.size() ; ++i )
            if( var_x.dimName[i] != var_z.dimName[i] )
              return false;
        }
        else
          return false;
      }
      else
        return false;
    }
  }

  // dims of time must be different from those of the geo-coords
  if( x_ix > -1 )
  {
    Variable& var_x = pIn->variable[x_ix] ;

    if( var_t.dimName.size() == var_x.dimName.size() )
    {
      for(size_t i=0 ; i < var_t.dimName.size() ; ++i)
        if( var_x.dimName[i] == var_t.dimName[i] )
          return false; // not a timeSeries
    }
  }

  // data variable vs. time variable
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

    //  i) dims are the same
    if( var.dimName.size() == var_t.dimName.size() )
    {
      for( size_t j=0 ; j < var.dimName.size() ; ++j )
        if( var.dimName[j] != var_t.dimName[j] )
            return false;
    }

    // ii) the only dimensions of time is shared by the data variables
    else
    {
      if( var_t.dimName.size() != 1 )
        return false;
      else if( ! hdhC::isAmong(var_t.dimName[0], var.dimName) )
        return false;
    }
  }

  return true;
}

bool
CF::chap9_trajectory(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // dv_ix.size() > 0 was checked before
  int x_ix = xyzt_ix[0] ;
  int z_ix = xyzt_ix[2] ;
  int t_ix = xyzt_ix[3] ;

  // a trajectory must have horizontal coordinates, otherwise it
  // would be called profile or single point.
  if( x_ix == -1 )
    return false;

  Variable& var_x = pIn->variable[x_ix] ;

  // dims of T and X must be identical
  if( t_ix > -1 )
  {
    Variable& var_t = pIn->variable[t_ix] ;

    if( var_t.dimName.size() != var_x.dimName.size() )
      return false;

    for(size_t i=0 ; i < var_t.dimName.size() ; ++i)
      if( var_x.dimName[i] != var_t.dimName[i] )
        return false;
  }

  // a set of trajectories may have z() or z(t) per each, but
  // not a profile
  if( z_ix > -1 )
  {
    Variable& var_z = pIn->variable[z_ix] ;

    if( ! (var_z.isScalar
          || (var_z.dimName.size() == 1
                && pIn->nc.getDimSize(var_z.dimName[0])) ) )
    {
      // if Z, then only once per each X, i.e. same dims
      size_t i;
      for(i=0 ; i < var_z.dimName.size() ; ++i)
        if( ! hdhC::isAmong(var_z.dimName[i], var_x.dimName ) )
          break;

      if( i < var_z.dimName.size() )
        return false;
    }
  }

  // data variable vs. X must have identical dims sets
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

    if( var.dimName.size() != var_x.dimName.size() )
      return false;

    for(size_t i=0 ; i < var.dimName.size() ; ++i)
      if( var_x.dimName[i] != var.dimName[i] )
        return false;
  }

  return true;
}

bool
CF::chap9_timeSeriesProfile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // dv_ix.size() > 0 was checked before
  int x_ix = xyzt_ix[0] ;
  int z_ix = xyzt_ix[2] ;
  int t_ix = xyzt_ix[3] ;

  if( t_ix == -1 || z_ix == -1 )
    return false;

  Variable& var_t = pIn->variable[t_ix] ;
  Variable& var_z = pIn->variable[z_ix] ;

  // Z should not be scalar or 1-D of size==1
  if( var_z.isScalar )
    return false;
  if( var_z.dimName.size() == 1 && pIn->nc.getDimSize(var_z.dimName[0]) == 1)
    return false;

  // T should not be scalar or 1-D of size==1
  if( var_t.isScalar )
    return false;
  if( var_t.dimName.size() == 1 && pIn->nc.getDimSize(var_t.dimName[0]) == 1)
    return false;

  if( x_ix > -1 )
  {
    Variable& var_x = pIn->variable[x_ix] ;

    // Z should have dim not in X + T
    size_t i;
    for(i=0 ; i < var_z.dimName.size() ; ++i)
    {
      if( ! hdhC::isAmong(var_z.dimName[i], var_x.dimName ) )
        if( ! hdhC::isAmong(var_z.dimName[i], var_t.dimName ) )
          break;
    }
    if( i==var_z.dimName.size() )
      return false;

    // all dims of X and T should be different
    if( hdhC::isAmong(var_t.dimName, var_x.dimName, true ) )
      return false;
  }

  // data variable vs. Z
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

    // if num of var.dims > 1, then these should match the effDims
    if( var.dimName.size() > 1 && var.dimName.size() == effDims_ix.size() )
    {
      for(size_t d=0 ; d < effDims_ix.size() ; ++d )
        if( ! hdhC::isAmong( dimensions[ effDims_ix[d] ], var.dimName) )
          return false;
    }

    if( var.dimName.size() == 1 && var_z.dimName.size() == 1 )
    {
      // dim of var and Z must be the same
      if( var.dimName[0] != var_z.dimName[0] )
        return false;
    }
  }

  return true;
}

bool
CF::chap9_trajectoryProfile(std::vector<int>& xyzt_ix, std::vector<size_t>& dv_ix)
{
  // dv_ix.size() > 0 was checked before
  int x_ix = xyzt_ix[0] ;
  int z_ix = xyzt_ix[2] ;
  int t_ix = xyzt_ix[3] ;

  if( x_ix == -1 || z_ix == -1 )
    return false;

  Variable& var_x = pIn->variable[x_ix] ;

  if( var_x.isScalar )
    return false;

  Variable& var_z = pIn->variable[z_ix] ;

  if( var_z.isScalar )
    return false;

/*
  if( var_z.isScalar
          || (var_z.dimName.size() == 1 && pIn->nc.getDimSize(var_z.dimName[0]) ) )
    return false;
*/

  // dims of time, if given, and horizontal coord must be identical
  if( t_ix > -1 )
  {
    Variable& var_t = pIn->variable[t_ix] ;

    if( var_t.dimName.size() != var_x.dimName.size() )
      return false;

    for(size_t i=0 ; i < var_t.dimName.size() ; ++i)
      if( var_x.dimName[i] != var_t.dimName[i] )
        return false;

    // Z should have a dim not T
    size_t i;
    for(i=0 ; i < var_z.dimName.size() ; ++i)
      if( ! hdhC::isAmong(var_z.dimName[i], var_t.dimName ) )
        break;

    if( i==var_z.dimName.size() )
      return false;

  }

  // Z should have a dim not in X
  size_t i;
  for(i=0 ; i < var_z.dimName.size() ; ++i)
    if( ! hdhC::isAmong(var_z.dimName[i], var_x.dimName ) )
      break;

  if( i==var_z.dimName.size() )
    return false;

  // data variable vs. Z
  for( size_t i=0 ; i < dv_ix.size() ; ++i )
  {
    Variable& var = pIn->variable[dv_ix[i]] ;

    // Z) dims of dv and Z are the same
    if( var.dimName.size() != var_z.dimName.size() )
      return false;

    for( size_t j=0 ; j < var.dimName.size() ; ++j )
      if( var.dimName[j] != var_z.dimName[j] )
        return false;

    //
  }

  return true;
}

const char* CF_AttName[] = {
  "standard_error_multiplier"          ,
  "ancillary_variables", "axis"        , "calendar"     , "bounds"       , "cell_measures"
, "cell_methods"       , "climatology" , "comment"      , "compress"     , "Conventions"
, "coordinates"        , "_FillValue"  , "flag_masks"   , "flag_meanings", "flag_values"
, "formula_terms"      , "grid_mapping", "history"      , "institution"  , "leap_month"
, "leap_year"          , "long_name"   , "missing_value", "month_lengths", "positive"
, "references"         , "scale_factor", "source"       , "add_offset"   , "valid_range"
, "standard_name"      , "title"       , "units"        , "valid_max"    , "valid_min"
, "grid_mapping_name"

, "earth_radius"                    , "false_easting"                       , "false_northing"
, "grid_north_pole_latitude"        , "grid_north_pole_longitude"           , "inverse_flattening"
, "latitude_of_projection_origin"   , "longitude_of_central_meridian"       , "longitude_of_prime_meridian"
, "longitude_of_projection_origin"  , "north_pole_grid_longitude"           , "perspective_point_height"
, "scale_factor_at_central_meridian", "scale_factor_at_projection_origin"   , "semi_major_axis"
, "semi_minor_axis"                 , "standard_parallel"   , "straight_vertical_longitude_from_pole"

};

const char CF_AttType[] = {
  'N'
, 'S'                  , 'S'           , 'S'            , 'S'            , 'S'
, 'S'                  , 'S'           , 'S'            , 'S'            , 'S'
, 'S'                  , 'D'           , 'D'            , 'S'            , 'D'
, 'S'                  , 'S'           , 'S'            , 'S'            , 'N'
, 'N'                  , 'S'           , 'D'            , 'N'            , 'S'
, 'S'                  , 'N'           , 'S'            , 'N'            , 'N'
, 'S'                  , 'S'           , 'S'            , 'N'            , 'N'
, 'S'

,'N', 'N', 'N'
,'N', 'N', 'N'
,'N', 'N', 'N'
,'N', 'N', 'N'
,'N', 'N', 'N'
,'N', 'N', 'N'
};

std::vector<std::string> CF::attName(CF_AttName, CF_AttName + 55);
std::vector<char>        CF::attType(CF_AttType, CF_AttType + 55);
