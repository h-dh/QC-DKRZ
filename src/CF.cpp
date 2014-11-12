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

     if( split[0] == "rT"
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
CF::cellMethods_Climatology(
  std::vector<std::string> &name,
  std::vector<std::string> &method,
  Variable& var)
{
  // a climatology must have at least two time: dimensions; find the indexes
  std::vector<size_t> t_ix;
  for(size_t i=0 ; i < name.size() ; ++i )
    if( name[i] == (timeName + ":") )
       t_ix.push_back(i);

  // relation between attributes time:climatology and var:cell_methods

  // a) cell_methods without enough time: instances
  if( t_ix.size() < 2 )
  {
    if( pIn->variable[timeIx].isClimatology )
    {
      if( notes->inq(bKey + "74j", var.name) )
      {
        std::string capt("!" + var.name + ":" + n_cell_methods) ;
        capt += " is ill-formatted for " + pIn->variable[timeIx].name;
        capt += ":" + n_climatology ;

        std::string text("Found: " + var.name);
        text += ":" + n_cell_methods + "=" + var.getAttValue(n_cell_methods);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    return ;
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
     bool is=true;
     for( j=0 ; j < 2 ; ++j )
     {
       size_t pos;
       if( (pos=str0.find(within)) < std::string::npos)
       {
         if( str0.find(period[j], pos) < std::string::npos )
         {
            if( (pos=str1.find(over)) < std::string::npos )
            {
              if( str1.find(period[j], pos)< std::string::npos )
              {
                is = false;
                break;
              }
            }
         }
       }
     }

     if( is )
     {
        if( notes->inq(bKey + "74l", var.name) )
        {
          std::string capt("invalid " + var.name) ;
          capt += ":" + n_cell_methods + "=" + timeName + ": " + str0 ;
          capt += " " + timeName + ": " + str1 ;

          std::string text("Required: " + var.name);
          text += ":" + n_cell_methods + "=" ;
          text += timeName + ": method_1 within years|days ";
          text += timeName + ": method2 over years|days" ;

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }

        return ;
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

     bool is=true;
     size_t pos;

     if( (pos=str0.find(within)) < std::string::npos )
     {
       if( str0.find(period[1], pos) < std::string::npos )
       {
         if( (pos=str1.find(over)) < std::string::npos )
         {
           if( str1.find(period[1], pos) < std::string::npos )
           {
             if( (pos=str2.find(over)) < std::string::npos )
             {
               if( str2.find(period[0], pos) < std::string::npos )
                 is = false;
             }
           }
         }
       }
     }

     if( is )
     {
        if( notes->inq(bKey + "733e", var.name) )
        {
          std::string capt("!" + var.name) ;
          capt += ":" + n_cell_methods + "=time: method1 within days" ;
          capt += " time: method2 over days" ;
          capt += " time: method3 over years" ;

          std::string text("Found: " + var.name);
          text += ":" + n_cell_methods + "=" + str0 + " " +  str1 + " " + str2 ;

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }

        return ;
     }
  }
  else
    return;  // don't know what so many time: occurrences could be

  // found  valid cell_methods for a climatology,
  // but a corresponding attribute was not defined in the time variable
  if( ! pIn->variable[timeIx].isValidAtt(n_climatology) )
  {
    if( notes->inq(bKey + "74k", var.name) )
    {
      std::string capt("!" + var.name) ;
      capt += ":" + n_cell_methods + " is formatted for a " + n_climatology + ", but " ;
      capt += timeName + ":" + n_climatology + " is missing" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return ;
}

void
CF::cellMethods_Comment(std::string &par, Variable& var)
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
           std::string capt("!" + var.name);
           capt += ":" + n_cell_methods + " key-word=comment: should be omitted ";
           capt += "if there is no standardized information" ;

           std::string text("found=" + par);

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
        }
      }
      else
      {
        if( notes->inq(bKey + "732d", var.name) )
        {
           std::string capt("!" + var.name);
           capt += ":" + n_cell_methods + " with swapped order of information, ";
           capt += "required is (interval: val units comment: text)" ;

           std::string text("found=" + par);

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
        }
      }

      return ;
    }

    if( x_par[x_par.size()-1] == "comment:" )
    {
      if( notes->inq(bKey + "732b", var.name) )
      {
         std::string capt("!" + var.name);
         capt += ":" + n_cell_methods;
         capt += " provides incomplete non-standardized information" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }

      return ;
    }

    if( x_par[0] != "interval:" )
      return ;  // comment by non-standardised information
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
        std::string capt("!" + var.name);
        capt += ":" + n_cell_methods ;
        capt += " interval information requires (interval: value units), found ";
        capt += par;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
  }

  return ;
}

void
CF::cellMethods_Method(std::string &str0, Variable& var)
{
  // str0 contains only the method item
  std::vector<std::string> term;
  term.push_back("point");
  term.push_back("sum");
  term.push_back("maximum");
  term.push_back("median");
  term.push_back("mid_range");
  term.push_back("minimum");
  term.push_back("mean");
  term.push_back("mode");
  term.push_back("standard_deviation");
  term.push_back("variance");

  // str0 must contain one and only one of the terms
  int count=0;
  size_t pos, p;
  for( size_t i=0 ; i < term.size() ; ++i )
  {
    if( (p=str0.find(term[i])) < std::string::npos )
    {
      pos=p;
      ++count;
    }
  }

  if( pos && count == 1 )  // the one and only must begin at pos==0 item in str0
     count = 0;

  if( count != 1 )
  {
     if( notes->inq(bKey + "73d", var.name) )
     {
        std::string capt("!" + var.name);
        capt += ":" + n_cell_methods;

        if( count == 0 )
          capt += " does not contain a valid method" ;
        else
          capt += " contains more than one valid method, found " + str0;

        std::string text("valid methods: ");
        text += term[0];
        for( size_t i=1 ; i < term.size() ; ++i )
          text += ", " + term[i] ;

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
     }
  }

  return ;
}

void
CF::cellMethods_Name(std::string &name, Variable& var)
{
  // valid names:  dimension of the variable, a scalar coordinate variable,
  //               , a standard name indicating the axis, or area.

  Split x_name(name, " :");

  for( size_t x=0 ; x < x_name.size() ; ++x )
  {

    if( chap7_3_4(var, x_name[x]) )  //standard_name or area
      continue;

    bool isCont=false;
    size_t ix;

    // dim of a var
    for( size_t i=0 ; i < var.dimName.size() ; ++i )
    {
      if( var.dimName[i] == x_name[x] )
      {
        isCont=true;
        break;
      }
    }

    if( isCont )
      continue;

    bool is=true;

    for( ix=0 ; ix < pIn->varSz ; ++ix )
    {
      // a scalar coord
      if( pIn->variable[ix].name == x_name[x] && pIn->variable[ix].isScalar )
      {
        is=false;
        break;
      }

      // a standard name
      if( pIn->variable[ix].getAttValue(n_standard_name) == x_name[x] )
      {
        is=false;
        break;
      }
    }

    if( is )
    {
      if( notes->inq(bKey + "73c", var.name) )
      {
        std::string capt("!" + var.name + ":" + n_cell_methods);
        capt += " with invalid name:=" + x_name[x];

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  return ;
}

void
CF::cellMethods_WhereOver(
  std::string &str0,
  Variable& var,
  std::string mode)
{
  //return true for failure
  // mode: where or over
  std::string where("where");
  std::string over("over");

  // find the word after the mode string, i.e. 'where' or 'over'
  Split x_str0(str0);
  std::string type;

  //prevent to check a climatology case
  if( mode == where && str0.find(where) == std::string::npos )
     return;

  size_t iw;
  for( iw=0 ; iw < x_str0.size() ; ++iw )
  {
     if( x_str0[iw] == mode )
     {
       if( (iw+1) < x_str0.size() )
         type=x_str0[++iw] ;

       break;
     }
  }

  if( type.size() == 0 )
  {
    if( notes->inq(bKey + "733h", var.name) )
    {
      std::string capt("!" + var.name);
      capt += ":" + n_cell_methods ;
      if( iw == x_str0.size() )
        capt += " with missing key-word=" + mode ;
      else
        capt += " declares key-word=" + mode + " without a type";

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return ;  // no where |over clause specified
  }

  std::vector<std::string> areaType;

  areaType.push_back("all_area_types");
  areaType.push_back("bare_ground");
  areaType.push_back("burnt_vegetation");
  areaType.push_back("c3_plant_functional_types");
  areaType.push_back("c4_plant_functional_types");
  areaType.push_back("clear_sky");
  areaType.push_back("cloud");
  areaType.push_back("crops");
  areaType.push_back("floating_ice");
  areaType.push_back("ice_free_land");
  areaType.push_back("ice_free_sea");
  areaType.push_back("lake_ice_or_sea_ice");
  areaType.push_back("land");
  areaType.push_back("land_ice");
  areaType.push_back("natural_grasses");
  areaType.push_back("pastures");
  areaType.push_back("primary_deciduous_trees");
  areaType.push_back("primary_evergreen_trees");
  areaType.push_back("sea");
  areaType.push_back("sea_ice");
  areaType.push_back("secondary_deciduous_trees");
  areaType.push_back("secondary_evergreen_trees");
  areaType.push_back("shrubs");
  areaType.push_back("snow");
  areaType.push_back("trees");
  areaType.push_back("vegetation");

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
        if( notes->inq(bKey + "733a", var_tv.name) )
        {
          std::string capt("area-type-variable=" + var_tv.name);
          capt += " declared by " + var.name ;
          capt += ":" + n_cell_methods + ":" + mode  ;
          capt += " " + txt[i] ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
     }
  }
  else
  {
     // check for the first convention
     size_t i;
     for( i=0 ; i < areaType.size() ; ++i )
       if( areaType[i] == type )
         break;

     if( i == areaType.size() )
     {
        if( notes->inq(bKey + "733b", var.name) )
        {
          std::string capt("!" + var.name);
          capt += ":" + n_cell_methods + ":";
          capt += mode + " declares invalid area_type=" + type ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
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
          if( notes->inq(bKey + "733d", var.name) )
          {
            std::string capt("!" + var.name + ":" + n_cell_methods);
            capt += "=(where type1 over type2=" + tVar.name;
            capt += ") where " + tVar.name ;
            capt += " must not have more than a single area-type string" ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }

          return;
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
      size_t j;
      for( j=0 ; j < areaType.size() ; ++j )
        if( areaType[j] == vs[l] )
          break;

      if( j == areaType.size() )
        tvFail.push_back(vs[l]);
    }

    if( tvFail.size() )
    {
      if( notes->inq(bKey + "733c", tVar.name) )
      {
         std::string capt("type-area variable=" + tVar.name);
         capt += " contains " ;
         if( tvFail.size() > 5 )
            capt += "more than five invalid area_type strings";
         else
         {
           capt += "invalid area_type=" ;
           capt += tvFail[0];
           for( size_t l=1 ; l < tvFail.size() ; ++l)
             capt += ", " + tvFail[l] ;
         }

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }
    }

    // the type-var is an auxiliary coordinate variable
    tVar.coord.isAny=true;

    Split x_c( var.getAttValue(n_coordinates) );
    size_t i;
    for(i=0 ; i < x_c.size() ; ++i)
       if( x_c[i] == tVar.name )
          break;

    if( i == x_c.size() )
    {
      if( notes->inq(bKey + "733k", tVar.name) )
      {
         std::string capt(var.name + " requires a coordinate attribute with");
         capt += " auxiliary coordinate=" + tVar.name + " included" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }
    }
  }

  if( mode == where && str0.find(over) < std::string::npos )
    cellMethods_WhereOver(str0, var, over) ;

  return ;
}

void
CF::checkAuxCoordData(Variable& var)
{
  struct hdhC::FieldDataMeta fDM;

  int indexFV = -1;
  int indexInf = -1;

  // all coordinate variables must be strictly monotonic
  size_t sz ;

  if( var.isUnlimited() )
  {
    if( (sz = pIn->nc.getNumOfRecords()) == 0 )
       return;
  }
  else
    sz = pIn->nc.getRecordSize(var.name);

  if( sz )
     --sz;

  for( size_t rec=0 ; rec <= sz ; ++rec )
  {
    var.getData(pIn->nc, rec);

    var.pDS->clear();
    var.pDS->add( (void*) var.pMA );
    fDM = var.pDS->getMeta() ;

    if( fDM.isValid )
    {
      if( fDM.fillingValueNum )
      {
         indexFV=rec;
         break;
      }
      else if( fDM.infCount || fDM.nanCount )
      {
         indexInf=rec;
         break;
      }
      else
      {
         if( fDM.max > var.range[1] )
            var.range[1] = fDM.max;
         if( fDM.min < var.range[0] )
            var.range[0] = fDM.min;
      }
    }
  }

  if( indexFV > -1 || indexInf > -1 )
  {
    var.range[0] = MAXDOUBLE;
    var.range[1] = -MAXDOUBLE;

    if( indexFV > -1 && notes->inq(bKey + "12a", var.name) )
    {
      std::string capt("auxiliary coordinate variable=" + var.name);
      capt += " must not have _FillValue data";
      if( sz )
      {
        capt += ", found in record num=" ;
        capt += hdhC::itoa(indexFV+1) + "." ;
      }

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    if( indexInf > -1 && notes->inq(bKey + "12a", var.name) )
    {
      std::string capt("auxiliary coordinate variable=" + var.name);
      capt += " contains Inf/NaN";
      if( sz )
        capt += " in record num=" + hdhC::itoa(indexInf+1) ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
}

void
CF::checkCoordVarFillValueAtt(Variable& var)
{
   std::string sF(n_FillValue);
   std::string sM("missing_value");

   bool isF = var.isValidAtt(sF) ? true : false ;
   bool isM = var.isValidAtt(sM) ? true : false ;

   if( isF || isM )
   {
      if( notes->inq(bKey + "12c", var.name) )
      {
        std::string capt("coordinate variable=");
        capt += var.name + " must not have attribute " ;
        if( isF )
        {
          capt +=  sF ;
          if( isM )
            capt += " and " ;
        }

        if( isM )
           capt += sM ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
   }

   return;
}

void
CF::checkCoordVarValues(Variable& var)
{
  // checks only a meta-data declaration; a _FV present in the data section
  // will be detected below.
  checkCoordVarFillValueAtt(var) ;

  // no check of an unlimited coord var for efficiency
//  if( var.isUnlimited() )
//     return;

  // all coordinate variables must be strictly monotonic
  size_t num ;
  MtrxArr<double> mv;

  if( var.isUnlimited() )
  {
    if( (num = pIn->nc.getNumOfRecords()) == 0 )
       return;

    pIn->nc.getData(mv, var.name, 0, num );
  }
  else
  {
    num = pIn->nc.getRecordSize(var.name);
    pIn->nc.getData(mv, var.name );
  }

  bool isMonotonic=true;
  bool isDifferent=true;
  bool isFillValue=false;

  size_t i=0;

  if( mv.size() == 0 )
  {
    if( notes->inq(bKey + "12d", var.name) )
    {
      std::string capt("the data section of coordinate variable=");
      capt += var.name + " is empty" ;
    }

    return;
  }

  if( mv.size() )
  {
    if( is_FillValue(var, mv[0]) )
    {
       i=0;
       isFillValue=true ;
    }
    else if( mv.size() == 1 )
      return;
  }

  // more than a single value
  if( !isFillValue )
  {
    if( is_FillValue(var, mv[1]) )
    {
       i=1;
       isFillValue=true;
    }

    else if( mv[1] == mv[0] )
    {
       isDifferent = false;
       i=0;
    }

    else
    {
      // the case that a _FillValue is within the data range
      // is treated elsewhere
      double sig=(mv[1]-mv[0]) / fabs(mv[1]-mv[0]);

      for( i=1 ; i < mv.size() ; ++i )
      {
        double v0=sig*mv[i-1];
        double v1=sig*mv[i];

        if( v1 > v0 )
          continue;

        if( v1 < v0 )
           isMonotonic=false;
        else if( v1 == v0 )
          isDifferent=false;

        if( is_FillValue(var, mv[i]) )
          isFillValue=true ;
        else if( is_FillValue(var, mv[i-1]) )
          isFillValue=true ;

        break;
      }
    }
  }

  if( isFillValue && notes->inq(bKey + "12a", var.name) )
  {
    std::string capt("all values of coordinate variable=");
    capt += var.name + " have to be set, found " + n_FillValue + " at index=" ;
    capt += hdhC::itoa(i) + "." ;

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  else if( ! isMonotonic )
  {
    // exceeding of a valid_range would get higher precedence
    int jxVRange ;
    if( (jxVRange=var.getAttIndex(n_valid_range)) > -1 )
    {
      double range[2];
      range[0] = hdhC::string2Double(var.attValue[jxVRange][0]) ;
      range[1] = hdhC::string2Double(var.attValue[jxVRange][1]) ;

      // monotonicity could be broken by a value within three consecutive values
      bool isA;
      bool isB = range[0] > mv[i-1] || range[1] < mv[i-1] ;
      bool isC = range[0] > mv[i]   || range[1] < mv[i] ;
      if( i > 1 )
         isA   = range[0] > mv[i-2] || range[1] < mv[i-2] ;
      else
         isA=false;

      if( isA || isB || isC )
      {
        if( notes->inq(bKey + "251d", var.name) )
        {
          int j;
          if(isA)
            j=i-2;
          else if(isB)
            j=i-1;
          else
            j=i;

          std::string capt("!" + var.name + ":" + n_valid_range + "=[");
          capt += var.attValue[jxVRange][0] + " ," ;
          capt += var.attValue[jxVRange][1] + "] is exceeded by v=" ;
          capt += hdhC::double2String(mv[j]);

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
    else if( notes->inq(bKey + "12b", var.name) )
    {
      std::string capt("coordinate variable=");
      capt += var.name + " has to be strictly monotonic. Broken around index=" ;
      capt += hdhC::itoa(i-1) + "." ;

      std::string text(" Found: ");
      if( i > 1 )
      {
        text += "..., ";
        text += "value[";
        text += hdhC::itoa(i-2) + "]=" ;
        text += hdhC::double2String(mv[i-2]);
      }

      if( i )
      {
        if(i>1)
          text += ", ";
        text += "value[";
        text += hdhC::itoa(i-1) + "]=" ;
        text += hdhC::double2String(mv[i-1]);
      }

      text += ", value[";
      text += hdhC::itoa(i) + "]=" ;
      text += hdhC::double2String(mv[i]);
      text += ", ...";

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  else if( ! isDifferent && notes->inq(bKey + "12a", var.name) )
  {
    // index starting with 1 is more convenient for non-C users
    ++i;

    std::string capt("all values of coordinate variable=");
    capt += var.name + " have to be different, found equal values at indexes=" ;
    capt += hdhC::itoa(i-1) + "," + hdhC::itoa(i) ;

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  return;
}

void
CF::checkGroupRelation(void)
{
  // a group for each dimension of a variable. Each contains
  // the variable names referring the dimension.

  std::vector<std::vector<std::string> > group;
  std::vector<std::string>               groupName;
  std::vector<bool>                      groupIsRelated;
  std::vector<std::vector<std::string> > containedGroup;

  std::vector<std::string> scalarVar;

  // an unreferred dimension? The vector will be reused later in a different context as:
  // dimName + coordinates-att ones
  std::vector<std::string> dims( pIn->nc.getDimNames() );

  Split x_compress;  // also switch

  for( size_t d=0 ; d < dims.size() ; ++d )
  {
    size_t v;
    for( v=0 ; v < pIn->varSz ; ++v )
    {
      Variable& var = pIn->variable[v];

      if( var.name == dims[d] )
         break;

      size_t j;
      for( j=0 ; j < var.dimName.size() ; ++j )
         if( var.dimName[j] == dims[d] )
            break;

      if( j < var.dimName.size() )
         break;
    }

    if( v == pIn->varSz )
    {
       // there is an exception: dims specified by a compress attribute are never referenced
       if( x_compress.size() == 0 )
       {
          for(size_t i=0 ; i < pIn->varSz ; ++i )
          {
             Variable& var = pIn->variable[i] ;

             int j;
             if( (j=var.getAttIndex("compress")) > -1 )
             {
                x_compress = var.attValue[j][0] ;
                break;
             }
          }
       }

       size_t i;
       for(i=0 ; i < x_compress.size() ; ++i )
         if( x_compress[i] == dims[d] )
            break;

       // dimension not used
       if( i == x_compress.size() && notes->inq(bKey + "0b") )
       {
         std::string capt("warning: dimension=" + dims[d] + " is never used") ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }
  }

  std::vector<size_t> scalar_ix;

  std::string n_sample_dimension("sample_dimension");
  std::string n_instance_dimension("instance_dimension");

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    if( var.boundsOf.size() )
       continue;  // skip bounds; the corresponding var relation is checked elsewhere

    if( var.dimName.size() == 0
           || (var.type == NC_CHAR && var.dimName.size() == 1 ) )
    {
      if( var.isValidAtt(n_cf_role) )
        continue;
      else
      {
        scalarVar.push_back( var.name ) ;
        scalar_ix.push_back(i);
      }
    }

    dims.clear();

    for( size_t j=0 ; j < var.dimName.size() ; ++j )
       dims.push_back( var.dimName[j] );

    size_t dimSz=dims.size();

    for( size_t j=0 ; j < dimSz ; ++j )
    {
      if( var.type == NC_CHAR && j == (dimSz-1) )
        break ; // exclude the dim providing max string length

      std::vector<std::string> vs;

      size_t g;
      for( g=0 ; g < groupName.size() ; ++g )
        if( groupName[g] == dims[j] )
           break ;

      if( g == groupName.size() )
      {
         groupName.push_back( dims[j] );
         groupIsRelated.push_back(false);
         group.push_back( std::vector<std::string>() );
         containedGroup.push_back(vs);
      }

      group[g].push_back(var.name);

      // a contained group name (in fact a dimension name)
      std::string str( var.getAttValue(n_sample_dimension,lowerCase) );
      if( str.size() )
        containedGroup[g].push_back( str );
      str = var.getAttValue(n_instance_dimension,lowerCase) ;
      if( str.size() )
        containedGroup[g].push_back( str );

      // include vars specified in the attributes: coordinates,
      // cell_measures. Add the corresponding dimensions as containedGroup
      std::vector<std::string> specialAtt;
      specialAtt.push_back(n_coordinates);
      specialAtt.push_back(n_cell_measures);
      specialAtt.push_back("compress");
      specialAtt.push_back(n_formula_terms);

      // scan all other vars for specific attributes which link to
      // the current variable
      for( size_t m=0 ; m < pIn->varSz ; ++m )
      {
         if( i == m )
            continue;

         for( size_t s=0 ; s < specialAtt.size() ; ++s )
         {
           int kx = pIn->variable[m].getAttIndex(specialAtt[s]) ;

           if( kx > -1 )
           {
              // get a list of strings cleared of :-tailed key-words
              vs = getKeyWordArgs(
                     pIn->variable[m].attValue[kx][0], "arg", ':') ;

              for( size_t n=0 ; n < vs.size() ; ++n )
              {
                // get corresponding dimensions, i.e. groupNames
                int ky ;

                if( (ky=pIn->getVarIndex(vs[n])) > -1 )
                {
                  for(size_t ly=0 ; ly < pIn->variable[ky].dimName.size() ; ++ly)
                    containedGroup[g].push_back( pIn->variable[ky].dimName[ly] );
                }
              }
           }
         }
      }
    }
  }

  if( pIn->varSz == 0 )  // a file only with global attributes
     return;

  if( groupName.size() == 0 )
     return;

  // identical groups (unsorted)
  bool is=true;
  for( size_t ni=0 ; ni < groupName.size() ; ++ni )
  {
    for( size_t nj=0 ; nj < groupName.size() ; ++nj )
    {
      if( ni == nj )
         continue;

      for( size_t mi=0 ; mi < group[ni].size() ; ++mi )
      {

        size_t mj;
        for( mj=0 ; mj < group[nj].size() ; ++mj )
           if( group[ni][mi] == group[nj][mj] )
              break;

        if( mj == group[nj].size() )
          is=false;
      }
    }
  }

  if( is )
     return;

  // variables linking groups
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i];

    std::vector<size_t> gIndex;

    for( size_t j=0 ; j < var.dimName.size() ; ++j )
      for( size_t k=0 ; k < groupName.size() ; ++k )
         if( groupName[k] == var.dimName[j] )
            gIndex.push_back(k) ;

    if( gIndex.size() > 1 )
      for( size_t t=0 ; t < gIndex.size() ; ++t )
         groupIsRelated[gIndex[t]] = true;
  }

  // a group containing a group name links the two
  for( size_t i0=0 ; i0 < containedGroup.size() ; ++i0 )
  {
    for( size_t i1=0 ; i1 < containedGroup[i0].size() ; ++i1 )
    {
      for( size_t j=0 ; j < groupName.size() ; ++j )
      {
        if( groupName[j] == containedGroup[i0][i1] )
        {
           groupIsRelated[i0] = true;
           groupIsRelated[j] = true;
        }
      }
    }
  }

 for( size_t i= 0 ; i < groupIsRelated.size() ; ++i )
 {
    if( ! groupIsRelated[i] && group[i].size() == 1 )
    {
       // found a variable unrelated to any other
       if( notes->inq(bKey + "0c") )
       {
         std::string capt("warning: variable=" + group[i][0]);
         capt += " is not related to any other variable" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }
  }

  return;
}

void
CF::checkSN_Modifier(std::string &vName, std::string &sn, std::string &mod)
{
  std::vector<std::string> valid_mod;
  valid_mod.push_back("detection_minimum");
  valid_mod.push_back("number_of_observations");
  valid_mod.push_back("standard_error");
  valid_mod.push_back("status_flag");

  bool is=true;
  for(size_t i=0 ; i < valid_mod.size() ; ++i )
  {
    if( valid_mod[i] == mod )
    {
       is=false;
       break;
    }
  }

  if( is )
  {
    if( notes->inq(bKey + "33f", vName) )
    {
      std::string capt("!" + vName + ":standard_name with undefined modifier=") ;
      capt += mod ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
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

  // coordinates, axis, positive
  finalAtt() ;

  // find groups of related variables
  checkGroupRelation();

  findAmbiguousCoords();

  if( isCheck )
  {
    if(followRecommendations )
      rc_chap();
  }

  return true;
}

void
CF::finalAtt(void)
{
   // check requirements of axes
   finalAtt_axis();
   finalAtt_coordinates();
   finalAtt_positive();

   return;
}

void
CF::finalAtt_axis(void)
{
  // A variable may have both coordinate and auxiliary coordinate variables,
  // but only one of them may have an axis attribute.
  // However, it is recommended that one of them has an axis attribute.

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
          std::string capt("!" + var.name);
          capt += ":axis is not X, Y, Z, or T (case insensitive)" ;

          std::string text("Found ");
          text += var.name + ":axis=";
          text += var.getAttValue(n_axis);

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }
      }

      if( var.coord.isCoordVar || cFVal > 15 )
      {
        if( axis.size() )
        {
          if( axis == "t" )
            ++var.coord.indication_T;
          else if( axis == "x" )
             ++var.coord.indication_X;
          else if( axis == "y" )
            ++var.coord.indication_Y;
          else if( axis == "z" )
            ++var.coord.indication_Z;

          // axis must be consistent with the coordinate variable
          std::string capt("found ");

          if( axis != "x" && var.coord.isX )
            capt += var.name +":" + n_axis + "="
                     + var.getAttValue(n_axis) + ", expected axis=X";
          else if( axis != "y" && var.coord.isY )
            capt += var.name +":" + n_axis + "="
                     + var.getAttValue(n_axis) + ", expected axis=Y";
          else if( axis != "z" && var.coord.isZ )
            capt += var.name +":" + n_axis + "="
                     + var.getAttValue(n_axis) + ", expected axis=Z";
          else if( axis != "t" && var.coord.isT )
            capt += var.name +":" + n_axis + "="
                     + var.getAttValue(n_axis) + ", expected axis=T";

          if( capt.size() > 6 )
          {
            if( notes->inq(bKey + "4b", var.name) )
            {
              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }
      }
      else
      {
        if( cFVal < 16 && notes->inq(bKey + "5a", var.name) )
        {
          std::string capt("variable=" + var.name);
          capt += " was estimated as auxiliary coordinate, ";
          capt += "i.e. attribute:axis is not allowed when " ;
          capt += cFVersion ;

          std::string text("Found ");
          text += var.name + ":axis=";
          text += var.getAttValue(n_axis);

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  char xs[]={'X', 'Y', 'Z', 'T'};

  // relations between data variables and corresponding auxiliary and pure
  // coordinate variables.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( !var.isDataVar || var.coord.isCoordVar || var.isCoordinate() )
       continue;

    // collection of indexes of all var-reps of the current variable's dims
    // and of existing variables declared in the coordintaes attribute.
    std::vector<int> coll_ix;
    std::vector<int> coll_dim_ix;
    std::vector<int> coll_ix_all;

    // dim-list
    int vIx;
    for( size_t l=0 ; l < var.dimName.size() ; ++l )
    {
       if( (vIx=pIn->getVarIndex(var.dimName[l])) > -1 )
       {
          coll_dim_ix.push_back(vIx);
          coll_ix_all.push_back(vIx);
       }
    }

    // coordinates attribute list
    int jx;
    if( (jx=var.getAttIndex(n_coordinates)) > -1 )
    {
       Split x_coord(var.attValue[jx][0]) ;
       for(size_t l=0 ; l < x_coord.size() ; ++l )
         if( (vIx=pIn->getVarIndex(x_coord[l])) > -1 )
           coll_ix_all.push_back( vIx );
    }

    // unique
    for(size_t c=0 ; c < coll_ix_all.size() ; ++c )
    {
      size_t d;
      for(d=0 ; d < coll_ix.size() ; ++d )
        if( coll_ix[d] == coll_ix_all[c] )
          break;

      if( d == coll_ix.size() )
        coll_ix.push_back( coll_ix_all[c] );
    }

    // take into account the four axis
    size_t countAuxAxis[] = { 0, 0, 0, 0 };
    size_t countCoordAxis[] = { 0, 0, 0, 0 };

    for( size_t l=0 ; l < 4 ; ++l )
    {
      for( size_t c=0 ; c < coll_ix.size() ; ++c )
      {
         size_t ix=coll_ix[c];
         bool is=false;

         if( l == 0 && pIn->variable[ix].coord.isX )
           is=true;
         else if( l == 1 && pIn->variable[ix].coord.isY )
           is=true;
         else if( l == 2 && pIn-> variable[ix].coord.isZ )
           is=true;
         else if( l == 3 && pIn->variable[ix].coord.isT )
           is=true;

         if( is && pIn->variable[ix].isValidAtt(n_axis) )
         {
           if( pIn->variable[ix].coord.isCoordVar )
             ++countCoordAxis[l];
           else
             ++countAuxAxis[l];
         }
      }

      if( followRecommendations )
      {
        // recommendation: horizontal coordinate variable
        // should have an axis attribute
        if( l < 2 )
        {
          if( countAuxAxis[l] == 0 )
          {
            bool isA=false;

            size_t ix;
            for( size_t c=0 ; c < coll_ix.size() ; ++c )
            {
               ix=coll_ix[c];

               if( l == 0 && pIn->variable[ix].coord.isX
                     && ! pIn->variable[ix].isValidAtt(n_axis) )
               {
                 isA=true;
                 break;
               }
               else if( l == 1 && pIn->variable[ix].coord.isY
                     && ! pIn->variable[ix].isValidAtt(n_axis) )
               {
                 isA=true;
                 break;
               }
            }

            if( isA && ( cFVal > 15 || pIn->variable[ix].coord.isCoordVar) )
            {
              std::string tag(bKey + "5g");
              if( !notes->findMapItem(tag, pIn->variable[ix].name)
                     && notes->inq(tag, pIn->variable[ix].name) )
              {
                std::string capt("recommendation: horizontal coordinate variable=") ;
                capt += pIn->variable[ix].name + " should have attribute axis=" ;
                capt += xs[l];

                (void) notes->operate(capt) ;
                notes->setCheckCF_Str( fail );
              }
            }
          }
        }
      }

      int isC=0 ;

      // check for more than a single X, Y, Z, or T associated to the dim-list
      size_t sz = coll_dim_ix.size() ;
      if( sz )
         --sz;

      for( size_t c0=0 ; c0 < sz ; ++c0 )
      {
        size_t i0=coll_dim_ix[c0];

        for( size_t c1=c0+1 ; c1 < sz+1 ; ++c1 )
        {
          size_t i1=coll_dim_ix[c1];

          if( l== 0 && pIn->variable[i0].coord.isX && pIn->variable[i1].coord.isX )
            ++isC ;
          if( l== 1 && pIn->variable[i0].coord.isY && pIn->variable[i1].coord.isY )
            ++isC ;
          if( l== 2 && pIn->variable[i0].coord.isZ && pIn->variable[i1].coord.isZ )
            ++isC ;
          if( l== 3 && pIn->variable[i0].coord.isT && pIn->variable[i1].coord.isT )
            ++isC ;
        }
      }

      if( isC > 0)  // get precedence over isB
      {
        if( notes->inq(bKey + "5e", var.name) )
        {
          std::string capt("!" + var.getDimNameStr(true));
          capt += " has more than one dimension for the " ;
          capt += xs[l] ;
          capt += "-axis" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
      else if( countAuxAxis[l] && countCoordAxis[l] && cFVal > 15 )
      {
        // note that both auxiliary and pure coordinate variable are taken into account
        if( notes->inq(bKey + "5d", var.name) )
        {
          std::string capt(cFVersion + ": " + var.name + ":");
          capt += n_axis + "=";
          capt += xs[l] ;
          capt += " for both auxiliary coordinate variable and coordinate variable" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
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
  // All of a variable's spatiotemporal dimensions that are not latitude, longitude,
  // vertical, or time dimensions are required to be associated with the relevant
  // latitude, longitude, vertical, or time coordinates via the new coordinates
  // attribute of the variable.

  // A) find missing auxiliary coordinate names in the coordinates attribute
  // B) non-var-representive dimensions require a coordiantes attribute

  // C) coordinate var must not have coordinates attribute
  // D) coordinates attribute contains non-existing variable

  std::vector<size_t> dv_ix ;  // indexes to the data variables
  std::vector<size_t> cv_ix ;  // indexes to the coordinate variables
  std::vector<size_t> acv_ix ;  // indexes to the auxiliary coordinate variables
  Split x_ca;

  // Prelude
  std::vector<size_t> tmp_dv_ix ;

  for( size_t ix = 0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix] ;

     if( var.coord.isCoordVar )
       cv_ix.push_back(ix);
     else if( var.isDataVar && var.boundsOf.size() == 0 )
       tmp_dv_ix.push_back(ix);
     else if( var.isCoordinate() || var.isLabel )
       acv_ix.push_back(ix);
  }

  // False alarm by data variables which are are not. These may be
  // prevented ny the assumption that data variables have a higher number
  // of dimensions compared to auxiliary coordiante variables. If a file
  // contains a mix of totally different data variable (e.g. 1-d, 2D, 3D)
  // this assumption fails.

  size_t mix=0;
  for( size_t i = 1 ; i < tmp_dv_ix.size() ; ++i )
    if( pIn->variable[tmp_dv_ix[mix]].dimName.size()
            < pIn->variable[tmp_dv_ix[i]].dimName.size() )
       mix = i;

  for( size_t i = 0 ; i < tmp_dv_ix.size() ; ++i )
    if( pIn->variable[tmp_dv_ix[i]].dimName.size()
           == pIn->variable[tmp_dv_ix[mix]].dimName.size() )
       dv_ix.push_back( tmp_dv_ix[i] );

  // A) Find missing auxiliary coordinate names in the coordinates attribute.
  //    For each aux-var do:
  //    a) get the dim-list
  //    b) for each data var depending on the aux-var-dim-list
  //       (there could be other dims)
  //    c) there is a coordinates attribute?
  //    d) is aux-var contained in the coordintas att list

  for( size_t av = 0 ; av < acv_ix.size() ; ++av )
  {
     Variable& var_av = pIn->variable[acv_ix[av]] ;

     // loop over the data var list
     for( size_t dv=0 ; dv < dv_ix.size() ; ++dv )
     {
        Variable& var_dv = pIn->variable[dv_ix[dv]] ;

        // is the auxiliary listed in the coordinates attribute?
        int ca_jx = var_dv.getAttIndex(n_coordinates);

        if( ca_jx > -1 )
        {
          x_ca = var_dv.getAttValue(n_coordinates);

          bool is=false;
          for( size_t i=0 ; i < x_ca.size() ; ++i )
          {
             if( x_ca[i] == var_av.name )
             {
               is=true;
               break;
             }
          }

          if( is )
            continue;  // try the next data variable
        }

        // count number of aux-dims also being data var dims
        size_t count = 0;

        for( size_t i = 0 ; i < var_av.dimName.size() ; ++i )
          for( size_t j = 0 ; j < var_dv.dimName.size() ; ++j )
             if( var_av.dimName[i] == var_dv.dimName[j] )
                ++count;

        if( count == var_av.dimName.size() )
        {
           // b) yes
           // c) is there any coordinates att?
           if(  ca_jx > -1 )
           {
              size_t k;
              for( k=0 ; k < x_ca.size() ; ++k )
                if( var_av.name == x_ca[k] )
                  break;

              if( k == x_ca.size() )   // aux-var is missing in coordinates att
              {
                 bool isNoException=true;
                 int j;
                 if( (j=var_dv.getAttIndex("grid_mapping")) > -1 )
                   if( var_dv.attValue[j][0] == var_av.name )
                      isNoException = false;

                if( isNoException && notes->inq(bKey + "5b", var_dv.name) )
                {
                  std::string capt("auxiliary coordinate variable=");
                  capt += var_av.name + " is missing in ";
                  capt += var_dv.name + ":" + n_coordinates ;

                  (void) notes->operate(capt) ;
                  notes->setCheckCF_Str( fail );
                }
              }
           }
           else
           {
             // aux-var requires a coordinates att which is missing

             //exception: compressed data
             bool isNotCompressed=true;

             if( compressIx > -1 )
             {
               Variable& var_cmpr = pIn->variable[compressIx] ;

               if( var_dv.dimName.size() == 1 &&
                  var_dv.dimName[0] == var_cmpr.name )
                   isNotCompressed=false;
             }

             if( isNotCompressed )
             {
               std::string tag(bKey + "5c");
               if( ! ( notes->findMapItem(tag, var_dv.name)
                       || notes->findMapItem(bKey + "56a", var_dv.name) )
                      && notes->inq(tag, var_dv.name) )
               {
                 std::string capt("data variable=" + var_dv.name );
                 capt += " depends on auxiliary coordinate=" + var_av.name;
                 capt += " requiring attribute=" + n_coordinates;

                 (void) notes->operate(capt) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }
        }
     }
  }

  // this case is not covered by A)
  // B) non-var-rep dim requires a coordinates attribute
  //   a) check data var dim-list for non-vars
  //   b) find aux-var depending on it
  //   c) is aux-var contained in the coordinates att list of data var

  for( size_t dv=0 ; dv < dv_ix.size() ; ++dv )
  {
     Variable& var_dv = pIn->variable[dv_ix[dv]] ;

     for( size_t j=0 ; j < var_dv.dimName.size() ; ++j )
     {
        size_t i;
        for( i=0 ; i < pIn->varSz ; ++i )
          if( pIn->variable[i].name == var_dv.dimName[j] )
            break;

        if( i == pIn->varSz )
        {  // a) is given
           if( acv_ix.size() )
           {
             size_t av;
             for( av=0 ; av < acv_ix.size() ; ++av )
             {
               Variable& var_av = pIn->variable[acv_ix[av]];

               for( size_t k=0 ; k < var_av.dimName.size() ; ++k )
               {
                  // auxiliary coord depending on the non-var dim
                  if( var_av.dimName[k] == var_dv.dimName[j] )
                  {
                     int u = var_dv.getAttIndex( n_coordinates );
                     if( u == -1 )
                     {
                       std::string tag(bKey + "5c");
                       if( ! ( notes->findMapItem(tag, var_dv.name)
                             || notes->findMapItem(bKey + "56a", var_dv.name) )
                                 && notes->inq(tag, var_dv.name) )
                       {
                         std::string capt("data variable=" + var_dv.name );
                         capt += " requires attribute=" ;
                         capt += var_dv.name + ":" + n_coordinates;
                         capt += ", depends on auxiliary coordinate=" + var_av.name;

                         (void) notes->operate(capt) ;
                         notes->setCheckCF_Str( fail );
                       }
                     }
                     else
                     {
                       Split x_ca(var_dv.attValue[u][0]);

                       size_t w;
                       for(w=0 ; w < x_ca.size() ; ++w )
                         if( x_ca[w] == var_av.name )
                           break;

                       if( w == x_ca.size() )
                       {
                         std::string tag(bKey + "5b");
                         if( ! notes->findMapItem(tag, var_dv.name)
                              && notes->inq(tag, var_dv.name ) )
                         {
                           std::string capt("auxiliary coordinate variable=");
                           capt += var_av.name + " is missing in ";
                           capt += var_dv.name + ":" + n_coordinates ;

                           (void) notes->operate(capt) ;
                            notes->setCheckCF_Str( fail );
                         }
                       }
                     }
                  }
               }
             }
           }
/*
           else
           {
             if( notes->inq(bKey + "5d", var_dv.name) )
             {
                std::string capt("dimension=" + var_dv.dimName[j] );
                capt += " of data variable=" + var_dv.name ;
                capt += " is not associated to any coordinate variable";

                (void) notes->operate(capt) ;
                notes->setCheckCF_Str( fail );
             }
           }
*/
        }
     }
  }

  // C) coordinate var must not have coordinates attribute
  // D) coordinates att contains non-existing variable
  std::string ca_val;

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix] ;

    int ca_ix = var.getAttIndex(n_coordinates);

    bool isAnyCoord = var.isCoordinate() ;

    if( ca_ix > -1 )
    {
      if( var.coord.isCoordVar || isAnyCoord )
      {
        if( notes->inq(bKey + "13b") )
        {
          std::string capt;
          if( !var.coord.isCoordVar )
             capt = "auxiliary ";

          capt += "coordinate variable=" + var.name;
          capt += " must not have a coordinates attribute" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      Split x_ca( var.attValue[ca_ix][0] );
      for( size_t j=0 ; j < x_ca.size() ; ++j )
      {
        int vCa ;
        if( (vCa=pIn->getVarIndex(x_ca[j])) == -1 )
        {
          if( notes->inq(bKey + "13a") )
          {
            std::string capt("!" + var.name);
            capt += ":coordinates contains name of non-existing variable=" ;
            capt += x_ca[j] ;

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
          std::string capt("!" + var.name + ":" + n_positive );
          capt += ": this attribute is only allowed for vertical coordinates" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
       }
     }

     if( var.coord.isZ && ! var.coord.isZ_DL && ! var.isValidAtt(n_positive) )
     {
       if( !cmpUnits(var.units, "Pa") && notes->inq(bKey + "43c", var.name) )
       {
          std::string capt("!" + var.name + ":" + n_positive );
          capt += " is required for dimensional vertical coordinates with non-pressure units" ;

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
  // ambigous (weired) setting of attributes may result in a state
  // without any data variable

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( !( var.isDataVar && var.coord.isCoordVar) )
    {
       if( var.dimName.size() > 2 )
          var.indication_DV += 10 ;

       // look for attributes which should be asssociated only
       // to data varriables, although we know real files better
       if( var.isValidAtt(n_axis) )
          --var.indication_DV ;
       if( var.isValidAtt(n_bounds) )
          --var.indication_DV ;
       if( var.isValidAtt(n_cell_measures) )
          ++var.indication_DV ;
       if( var.isValidAtt(n_cell_methods) )
          ++var.indication_DV ;
       if( var.isValidAtt(n_cf_role) )
          --var.indication_DV ;
       if( var.isValidAtt(n_climatology) )
          --var.indication_DV ;
       if( var.isValidAtt(n_coordinates) )
          --var.indication_DV ;
       if( var.isValidAtt(n_FillValue) )
          ++var.indication_DV ;
       if( var.isValidAtt(n_missing_value) )
          ++var.indication_DV ;
       if( var.isValidAtt(n_positive) )
          --var.indication_DV ;
       if( var.isValidAtt("grid_mapping") )
          ++var.indication_DV ;
       if( var.isValidAtt("ancilliary_variables") )
          ++var.indication_DV ;
       if( var.isValidAtt("flag_values") )
          ++var.indication_DV ;
       if( var.isValidAtt("flag_masks") )
          ++var.indication_DV ;

       if( var.indication_DV > 2 )
       {
          var.isDataVar = true ;
          var.coord.isX=false;
          var.coord.isY=false;
          var.coord.isZ=false;
          var.coord.isT=false;
       }
    }
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
   // test for variables with ambigous coordinate info .
   int count[4];
   char type[4];

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      if( var.coord.isCoordVar )
      {
         var.isDataVar = false;
         continue;
      }
      else if( var.isCoordinate() || var.isMapVar )
      {
         var.isDataVar = false;
         continue;
      }
      else if( var.boundsOf.size() )
      {
         var.isDataVar = false;
         continue;
      }

      count[0] = var.coord.indication_X;
      type[0] ='X';
      count[1] = var.coord.indication_Y;
      type[1] ='Y';
      count[2] = var.coord.indication_T;
      type[2] ='T';
      count[3] = var.coord.indication_Z;
      type[3] ='Z';

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
      if( (count[0] - var.indication_DV) > 1 )
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

            var.isDataVar = false;
         }
         else if( notes->inq(bKey + "0d", var.name) )
         {
           std::string capt("bail out for variable=" + var.name ) ;
           capt += " has ambiguous coordinate status" ;

           std::string text("Found number of indications:" );
           text += "coordinate variable: 0 ";
           text += ", X: " + hdhC::itoa(var.coord.indication_X) ;
           text += ", Y: " + hdhC::itoa(var.coord.indication_Y) ;
           text += ", Z: " + hdhC::itoa(var.coord.indication_Z) ;
           text += ", T: " + hdhC::itoa(var.coord.indication_T) ;

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
         }
      }
   }

   return;
}

bool
CF::findLabel(Variable& var)
{
   // evidences for a label:
   // a) variable is char type
   // b) the last dimension is used only once for all variables

   if( var.type != NC_CHAR || var.dimName.size() == 0 )
      return var.isLabel;

   std::string& lastDimName = var.dimName[var.dimName.size()-1];

   for( size_t i=0 ; i < pIn->variable.size() ; ++i )
   {
     if( pIn->variable[i].name == var.name )
        continue;

     for( size_t j=0 ; j < pIn->variable[i].dimName.size() ; ++j )
        if( pIn->variable[i].dimName[j] == lastDimName )
           return var.isLabel ;
   }

   var.isLabel=true;

   return var.isLabel ;
}

bool
CF::findCellMeasures(Variable& var)
{
   // don't know what happens if a user will put cell_measures
   // to an auxiliary coordinate variable
   if( var.coord.isCoordVar )
      return false;

   // find valid variable declared by a cell_measures attribute
   int j=var.getAttIndex(n_cell_measures) ;
   if( j == -1 )
      j=var.getAttIndex("cell_measure") ;

   if( j > -1 )
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
            vari.isDataVar = false;
            return true; // ok, there is one
         }
       }
     }
   }

   return false ;
}

void
CF::findTimeVariable(void)
{
   timeIx = -1 ;

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     if( pIn->variable[i].name == "time" )
     {
       timeName = pIn->variable[i].name;
       timeIx = static_cast<int>(i);
       return;
     }
   }

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      if( pIn->variable[i].coord.isT )
      {
        timeName = pIn->variable[i].name;
        timeIx = static_cast<int>(i);
        return;
      }
   }

   // try the unlimited dim, could go wrong
   timeName = pIn->nc.getUnlimitedDimName();
   timeIx = pIn->getVarIndex(timeName) ;

   return;
}

std::vector<std::string>
CF::getKeyWordArgs(std::string& str, std::string mode, char tail)
{
   // mode:s
   //      "char"           all key-words tailed by "char"
   //      "arg" [default]  all word which are not key:
   //      "1st"            first of each pair
   //      "1st"            second of each pair

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

bool
CF::getPureDimension(void)
{
  if( isPureDimInit )
     return pureDimension.size() ;

  isPureDimInit=true;

  std::vector<std::string> dn( pIn->nc.getDimNames() ); // all dimension names

  for( size_t k=0 ; k < dn.size() ; ++k )
  {
    size_t ix;
    for( ix=0 ; ix < pIn->varSz ; ++ix )
      if( pIn->variable[ix].name == dn[k] )
        break;

    if( ix == pIn->varSz )
      pureDimension.push_back( dn[k] );
  }

  return pureDimension.size() ;
}

void
CF::hasBounds(Variable& var)
{
  // this checks whether a variable declared by a bounds or climatology attribute
  // is supplied.

  int j;
  std::string type[2];
  type[0] = n_bounds;
  type[1] = n_climatology;

  for( size_t i=0 ; i < 2 ; ++i )
  {
    if( (j=var.getAttIndex( type[i]) ) > -1 )
    {
       if( pIn->getVarIndex( var.attValue[j][0] ) == -1 )
       {
         std::string key;
         if( i == 0)
           key = "71g" ;
         else
           key = "74m";

         if( notes->inq(bKey + key) )
         {
           std::string capt("!" + var.name + ":" + type[i] ) ;
           capt += "=" + var.attValue[j][0] ;
           capt += " declares a non-existing " ;
           if( i == 0)
              capt += "boundary" ;
           else
              capt += n_climatology ;
           capt+= " variable";

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
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

  bool isCV_X=false;
  bool isCV_Y=false;
  bool isCV_Z=false;
  bool isCV_T=false;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( var.coord.isCoordVar )
    {
      if( var.coord.isX )
        isCV_X = true;
      else if( var.coord.isY )
        isCV_Y = true;
      else if( var.coord.isZ )
        isCV_Z = true;
      else if( var.coord.isT )
        isCV_T = true;
    }
    else
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
      if( aux.coord.isX && !isCV_X )
         aux.coord.isCoordVar = true;
      if( aux.coord.isY && !isCV_Y )
         aux.coord.isCoordVar = true;
      if( aux.coord.isZ && !isCV_Z )
         aux.coord.isCoordVar = true;
      if( aux.coord.isT && !isCV_T )
         aux.coord.isCoordVar = true;
    }
    else
    {
      for( size_t j=0 ; j < aux.dimName.size() ; ++j )
      {
        int v ;
        if( (v=pIn->getVarIndex(aux.dimName[j])) > -1 )
        {
          if( vs_aux_ix[i] == v )
             continue;

          Variable& cVar = pIn->variable[v] ;

          if( cVar.coord.isCoordVar )
          {
            bool isX = aux.coord.isX && aux.coord.isX != cVar.coord.isX ;
            bool isY = aux.coord.isY && aux.coord.isY != cVar.coord.isY ;
            bool isZ = aux.coord.isZ && aux.coord.isZ != cVar.coord.isZ ;
            bool isT = aux.coord.isT && aux.coord.isT != cVar.coord.isT ;

            if( isX || isY || isZ || isT )
            {
              cVar.coord.isX = false ;
              cVar.coord.isY = false ;
              cVar.coord.isZ = false ;
              cVar.coord.isT = false ;
            }
          }
        }
      }
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

  std::string str[2];
  str[0] = n_bounds;
  str[1] = n_climatology;
  std::string t;

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var2 = pIn->variable[ix] ;

    if( var2.name == var.name )
       continue;

    for( size_t n=0 ; n < 2 ; ++n )
    {
      // the value of the attribute
      t = var2.getAttValue(str[n], lowerCase) ;

      if( t == var.name )
      {
        if( n == 1 )
           var2.isClimatology=true;

        var.isDataVar = false ;

        var.boundsOf = var2.name ;
        var2.bounds = var.name ;

        return true;
      }
    }
  }

  return false ;
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
  qC=0;
  tC=0;

  cFVal=14;  // the default for checking
  followRecommendations=false;
  isCF14_timeSeries=false;
  isCheck=false;
  isPureDimInit=false;

  timeIx=-1;
  compressIx=-1;

  bKey="CF_";
  fail="FAIL";

  n_axis="axis";
  n_bounds="bounds";
  n_cell_measures="cell_measures";
  n_cell_methods="cell_methods";
  n_cf_role="cf_role";
  n_climatology="climatology";
  n_coordinates="coordinates";
  n_FillValue="_FillValue";
  n_formula_terms="formula_terms";
  n_long_name="long_name";
  n_latitude="latitude";
  n_longitude="longitude";
  n_positive="positive";
  n_standard_name="standard_name";
  n_units="units";
  n_valid_min="valid_min";
  n_valid_max="valid_max";
  n_valid_range="valid_range";

  ut_set_error_message_handler( ut_ignore );
  unitSystem = ut_read_xml(NULL);
}

bool
CF::isCompressEvidence(Variable& var, bool* isIntType)
{
   // evidence of a compress index variable is given when
   // 1) the type of the variable is int-alike
   // 2) coordinate vars lon and lat are not referenced any further
   // 3) no attributes: axis, positive, units, standard_name,
   //    flag_values, flag_masks, _FillValue
   // 4) lon and lat must be coordiante variables

   // 1)
   if( (*isIntType=pIn->nc.isIndexType(var.name)) )
   {
     std::vector<std::string> vs_notAnAtt;
     vs_notAnAtt.push_back(n_units);
     vs_notAnAtt.push_back(n_standard_name);
     vs_notAnAtt.push_back(n_FillValue);
     vs_notAnAtt.push_back(n_axis);
     vs_notAnAtt.push_back(n_positive);
     vs_notAnAtt.push_back("flag_values");
     vs_notAnAtt.push_back("flag_masks");

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

          for( size_t k=0 ; k < rVar.dimName.size() ; ++k )
          {
             if( coordVar.name == rVar.dimName[k])
               return false;
          }
       }
     }
   }

   // 4
   if( count != 2 )
      return false;

   return true; // looks like a compress index variable
}

template <typename T>
bool
CF::is_FillValue(Variable& var, T val)
{
   if( var.fillValue )
   {
      if( val == var.doubleFillValue )
        return true;
   }

   if( var.missingValue )
   {
      if( val == var.doubleMissingValue )
        return true;
   }

   if( var.type == NC_BYTE )
   {
      if( val == static_cast<T>(NC_FILL_BYTE) )
        return true;
   }
   if( var.type == NC_CHAR )
   {
      if( val == static_cast<T>(NC_FILL_CHAR) )
        return true;
   }
   if( var.type == NC_SHORT )
   {
      if( val == static_cast<T>(NC_FILL_SHORT) )
        return true;
   }
   if( var.type == NC_INT )
   {
      if( val == static_cast<T>(NC_FILL_INT) )
        return true;
   }
   if( var.type == NC_LONG )
   {
      if( val == static_cast<T>(NC_FILL_INT) )  // correct
        return true;
   }
   if( var.type == NC_FLOAT )
   {
      if( val == static_cast<T>(NC_FILL_FLOAT) )
        return true;
   }
   if( var.type == NC_DOUBLE )
   {
      if( val == static_cast<T>(NC_FILL_DOUBLE) )
        return true;
   }
   if( var.type == NC_UBYTE )
   {
      if( val == static_cast<T>(NC_FILL_UBYTE) )
        return true;
   }
   if( var.type == NC_USHORT )
   {
      if( val == static_cast<T>(NC_FILL_USHORT) )
        return true;
   }
   if( var.type == NC_UINT )
   {
      if( val == static_cast<T>(NC_FILL_UINT) )
        return true;
   }
   if( var.type == NC_INT64 )
   {
      if( val == static_cast<T>(NC_FILL_INT64) )
        return true;
   }
   if( var.type == NC_UINT64 )
   {
      if( val == static_cast<T>(NC_FILL_UINT64) )
        return true;
   }
/*
   if( vartype == NC_STRING
   {
      MtrxArr<> mv;
      char val = pIn->nc.getData(mv, vName, rec );
      if( val == NC_FILL_STRING )
        return true;
   }
*/

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
  else if( className == "QC" )
    qC = dynamic_cast<QC*>(p) ;
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

bool
CF::run(void)
{
   init();

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

     vIx[ var.name ] = i ;

     // some checks brought forward for convenience

     chap5_1(var);  // find coordinate variables

     if( pIn->variable[i].dimName.size() == 0 )
        var.isScalar=true;

     if( i == 0 && pIn->variable[i].dimName.size() == 1 )
       firstDim=pIn->variable[i].dimName[0] ;

     if( firstDim.size() )
     {
       if( pIn->variable[i].dimName.size() == 0 )
         firstDim.clear();
       else if( pIn->variable[i].dimName.size() > 1 )
         firstDim.clear();
       else if( pIn->variable[i].dimName[0] != firstDim )
         firstDim.clear();
     }
   }

   if( firstDim.size() )
     isCF14_timeSeries = true;

   return entry();
}

void
CF::chap(void)
{
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
      Variable& var = pIn->variable[i] ;

      if( var.isValidAtt(n_cf_role) )
         var.isDataVar = false;
      else if( var.isValidAtt("sampling_dimension") )
         var.isDataVar = false;
      else if( var.isValidAtt("instance_dimension") )
         var.isDataVar = false;

      else if( findLabel(var) )
         var.isDataVar = false;
      else if( findCellMeasures(var) )
         var.isDataVar = false;
   }

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

   final_dataVar();

   inqAuxVersusPureCoord();

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

         // is varj an auxiliary depending on vari and its dimensions, respectively?
         // it is possible that still an ambiguity remains
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


   for( size_t i=0 ; i < pIn->varSz ; ++i )
     pIn->variable[i].disableAmbiguities();


   return;
}

void
CF::chap2(void)
{
  if( ! notes->inq(bKey + "2", "", "INQ_ONLY" ) )
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
        std::string capt("variable=") ;
        capt +=  var.name + " must not be type NC_STRING";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    for( size_t j=0 ; j < var.attName.size() ; ++j )
    {
      if( pIn->nc.getAttType(var.attName[j], var.name) == NC_STRING )
      {
        if( notes->inq(bKey + "22b", var.name) )
        {
          std::string capt("attribute=") ;
          capt += var.name + ":" ;
          capt += var.attName[j] + " must not be type NC_STRING";

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
   std::vector<std::string> dims( pIn->nc.getDimNames() );
   for( size_t i=0 ; i < dims.size() ; ++i )
   {
     if( ! hdhC::isAlpha(dims[i][0]) )
     {
       fault.push_back("dimension=") ;
       fault.back() += dims[i] ;
       fault.back() += " should begin with a letter" ;
     }

     for( size_t j=1 ; j < dims[i].size() ; ++j )
     {
       if( ! ( hdhC::isAlpha(dims[i][j])
                 || hdhC::isDigit(dims[i][j])
                     || dims[i][j] == '_') )
       {
         fault.push_back("dimension=") ;
         fault.back() += dims[i] ;
         fault.back() += " should be composed of letters, digits, and underscores" ;
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
       fault.push_back("variable=") ;
       fault.back() += var.name ;
       fault.back() += " should begin with a letter" ;
     }

     for( size_t j=1 ; j < var.name.size() ; ++j )
     {
       if( ! ( hdhC::isAlpha(var.name[j])
                   || hdhC::isDigit(var.name[j])
                        || var.name[j] == '_') )
       {
         fault.push_back("variable=") ;
         fault.back() += var.name ;
         fault.back() += " should be composed of letters, digits, and underscores" ;
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
         fault.push_back("attribute " + var.name + ":") ;
         fault.back() += var.attName[j] ;
         fault.back() += " should begin with a letter" ;
       }

       for( size_t k=1 ; k < var.attName[j].size() ; ++k )
       {
         if( ! ( hdhC::isAlphaNum(var.attName[j][k])
                   || var.attName[j][k] == '_') )
         {
           fault.push_back(var.name + ":") ;
           fault.back() += var.attName[j] ;
           fault.back() += " should not contain character=" ;
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
         std::string capt("dimensions of ");
         capt += var.getDimNameStr(true) ;
         capt += " should have different names" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
   }

   return;
}

void
CF::chap2_5_1(void)
{
  std::string n_missing_value("missing_value");

  std::string bbKey(bKey + "251");

  double minVal, maxVal, range[2];

   // missing data
   for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
   {
      Variable& var = pIn->variable[ix];

      int jxMV     = var.getAttIndex(n_missing_value) ;
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
           mfName[0] = "" ;
        else if( followRecommendations )
        {
          if( notes->inq(bbKey + "a", var.name) )
          {
            std::string capt
            ("recommendation: " + var.name + ":" + n_missing_value + " and ");
            capt += n_FillValue + " should have the same value" ;

            std::string text("found " + n_missing_value + "=");
            text += mfvStr[0];
            text += ", _FillValue=";
            text += mfvStr[1];

            (void) notes->operate(capt, text) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
      else if( jxMV > -1 )
      {
         mfName.push_back(n_missing_value);
         mfName.push_back("");
         mfvStr.push_back(var.attValue[jxMV][0]);
         mfvStr.push_back("");
         mfv.push_back(hdhC::string2Double(mfvStr[0]) );
      }
      else if( jxFV > -1 )
      {
         mfName.push_back("");
         mfName.push_back(n_FillValue);
         mfvStr.push_back("");
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
             if( notes->inq(bbKey + "b", var.name) )
             {
               std::string capt("recommendation: " + var.name + ":" );
               capt += mfName[k] + " should not be within " + n_valid_range ;

               std::string text("Found: " +mfName[k] + "=" + mfvStr[k]);
               text += ", " + n_valid_range + "=[" + var.attValue[jxVRange][0] ;
               text += ", " + var.attValue[jxVRange][1] + "]";

               (void) notes->operate(capt, text) ;
               notes->setCheckCF_Str( fail );
             }
           }

           else if( jxVMin > -1 && jxVMax > -1 )
           {
             if( minVal < mfv[k] && maxVal > mfv[k] )
             {
               if( notes->inq(bbKey + "b", var.name) )
               {
                 std::string capt("recommendation: " + var.name + ":") ;
                 capt += mfName[k] ;
                 capt += " should not be within the range specified by ";
                 capt += n_valid_min + " and " + n_valid_max  ;

                 std::string text("Found: " + mfName[k] + "=" + mfvStr[k]);
                 text += ", [" + n_valid_min + "=" + var.attValue[jxVMin][0] ;
                 text += ", " + n_valid_max + "=" + var.attValue[jxVMax][0] + "]" ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }

           else if( jxVMax > -1 )
           {
             if( maxVal > mfv[k] )
             {
               if( notes->inq(bbKey + "b", var.name) )
               {
                 std::string capt("recommendation: " + var.name + ":" );
                 capt += mfName[k] ;
                 capt += " should not be smaller than the " ;
                 capt += n_valid_max +" value" ;

                 std::string text("Found: " + mfName[k] + "=" + mfvStr[k]);
                 text += ", " + n_valid_max + "=" + var.attValue[jxVMax][0] ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }
           else if( jxVMin > -1 )
           {
             if( minVal < mfv[k] )
             {
               if( notes->inq(bbKey + "b", var.name) )
               {
                 std::string capt("recommendation: " + var.name + ":") ;
                 capt += mfName[k] ;
                 capt += " should not be larger than the ";
                 capt += n_valid_min + " value" ;

                 std::string text("Found: " + mfName[k] + "=" + mfvStr[k]);
                 text += ", " + n_valid_min + "=" + var.attValue[jxVMin][0] ;

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }
             }
           }
         }
      }

      if( jxVRange > -1 && ( jxVMin > -1 || jxVMax > -1) )
      {
         if( jxVMin > -1 && range[0] != minVal)
         {
           if( notes->inq(bbKey + "c", var.name) )
           {
             std::string capt("warning: " + var.name + ":");
             capt +=n_valid_range + "(min) and value of " + n_valid_min ;
             capt += " are different" ;

             std::string text("Found: " + n_valid_range + "(min)=" );
             text += hdhC::string2Double(var.attValue[jxVRange][0]) ;
             text += " vs. " + n_valid_min + "=" + var.attValue[jxVMin][0] ;

             (void) notes->operate(capt, text) ;
             notes->setCheckCF_Str( fail );
           }
         }

         if( jxVMax > -1 && range[1] !=  maxVal)
         {
           if( notes->inq(bbKey + "c", var.name) )
           {
             std::string capt("warning: " + var.name + ":");
             capt +=n_valid_range + "(max) and value of " + n_valid_max ;
             capt += " are different" ;

             std::string text("Found: " + n_valid_range + "(max)=" );
             text += hdhC::string2Double(var.attValue[jxVRange][1]) ;
             text += " vs. " + n_valid_max + "=" + var.attValue[jxVMax][0] ;

             (void) notes->operate(capt, text) ;
             notes->setCheckCF_Str( fail );
           }
         }
      }

/*
        if( notes->inq(bKey + "22c", var.name) )
        {
          std::string capt("!" + var.name);
          capt +=":valid_range must not be present" ;
          capt += " if valid_min and/or valid_max are available" ;

          std::string text(var.name + ":" + attStr[3]) ;
          text += "=" + var.getAttValue(attStr[3]) ;

          std::string t0(var.getAttValue(attStr[1]));
          if( t0.size() )
          {
            text += "\n";
            text += var.name +":";
            text += attStr[1] + "=" + t0 ;
          }

          t0 = var.getAttValue(attStr[2]);
          if( t0.size() )
          {
            text += "\n";
            text += var.name +":" ;
            text += attStr[2] + "=" + t0 ;
          }

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }
*/
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

   // indexes of variables and attributes
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     nc_type var_type = pIn->nc.getVarType(var.name) ;

     for( size_t j=0 ; j < pIn->variable[i].attName.size() ; ++j)
     {
       std::string &aName = pIn->variable[i].attName[j];

       for( size_t k=0 ; k < CF::attName.size() ; ++k )
       {
         if( aName == CF::attName[k] )
         {
            nc_type att_type = pIn->nc.getAttType(aName, var.name) ;

            if( CF::attType[k] == 'D' )
            {
               if( att_type != var_type )
               {
                 if( notes->inq(bKey + "26a", var.name) )
                 {
                   std::string capt("variable=" + var.name);
                   capt += " and " + var.name + ":";
                   capt += aName ;
                   capt += " have to be the same type";

                   std::string text("type of ");
                   text += var.name + ":";
                   text += aName + "=" ;
                   text += pIn->nc.getAttTypeStr(aName, var.name) ;
                   text += "\nType of variable=";
                   text += pIn->nc.getVarTypeStr(var.name) ;

                   (void) notes->operate(capt, text) ;
                   notes->setCheckCF_Str( fail );
                 }
               }
            }

            else if( CF::attType[k] == 'S' )
            {
               if( att_type != NC_CHAR )
               {
                 if( notes->inq(bKey + "26b", var.name) )
                 {
                   std::string capt("the type of ");
                   capt += var.name + ":" ;
                   capt += aName ;
                   capt += " has to be STRING";

                   std::string text("type of ");
                   text += var.name + ":";
                   text += aName + "=" ;
                   text += pIn->nc.getAttTypeStr(aName, var.name) ;

                   (void) notes->operate(capt, text) ;
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
                   std::string capt("the type of ");
                   capt += var.name + ":" ;
                   capt += aName ;
                   capt += " has to be NUMERIC";

                   std::string text("type of ");
                   text += var.name + ":";
                   text += aName + "=" ;
                   text += pIn->nc.getAttTypeStr(aName, var.name) ;

                   (void) notes->operate(capt, text) ;
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
  if( (j=pIn->getVarIndex("NC_GLOBAL")) > -1 )
  {
    std::string n_att("Conventions");
    glob_cv = pIn->variable[j].getAttValue(n_att, lowerCase) ;

    // try for a misspelled att
    bool isMisspelled=false;

    if( glob_cv.size() == 0 )
    {
      for( size_t k=0 ; k < pIn->variable[j].attName.size() ; ++k )
      {
         aName = hdhC::Lower()(pIn->variable[j].attName[k]) ;
         if( aName.substr(0,10) == "convention" )
         {
           glob_cv=pIn->variable[j].getAttValue(aName, lowerCase) ;
           isMisspelled=true;
           break;
         }
      }
    }

    if( isMisspelled )
    {
      if( notes->inq(bKey + "261f") )
      {
         std::string capt("misspelled attribute name=Conventions,");
         capt += " found: name=" + aName ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }
    }
  }

  if( glob_cv.size() == 0 )
  {
    if( notes->inq(bKey + "261a") )
    {
      std::string capt("missing global attribute=Conventions") ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }
  else if( glob_cv.substr(0,3) != "cf-" )
  {
    std::string tag(bKey+"261b");
    if( ! notes->findMapItem(tag) && notes->inq(tag) )
    {
      std::string capt("unknown Conventions=" + glob_cv);

      // test whether "CF-" is missing
      if( glob_cv.size() > 2 )
      {
        if( hdhC::isDigit(glob_cv[0])
            && glob_cv[1] == '-' && hdhC::isDigit(glob_cv[2]) )
        {
           capt += ", prefix=CF- is missing" ;
           glob_cv = "CF-" + glob_cv ;
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
    isCheck=false;
    return false;
  }

  return true;
}

void
CF::chap3(void)
{
  if( ! notes->inq(bKey + "3", "", "INQ_ONLY" ) )
     return ;  // checks are selected to be discarded by 'D'

  chap3_3() ; // standard_name; include look-up to xml table
  chap3_4();  // ancillary_variables
  chap3_5();  // flags

  return;
}

void
CF::chap3_3(void)
{
   // standard_name and some more, comparable to
   // http://cf-pcmdi.llnl.gov/documents/cf-standard-names/cf-standard-name-table.xml

   // note that units, amip and/or grib are only checkable for a valid standard_name.

   // collection of syntactically faulty names
   std::vector<std::string> fault;

   // collection of correct names; to be used below
   std::vector<struct SN_Entry> sne;

   // index to standard_names
   std::vector<size_t> v_ix;
   std::vector<size_t> sn_ix;
   std::vector<int>    units_ix;
   std::vector<int>    grib_ix;
   std::vector<int>    amip_ix;

   std::string str;
   std::string n_grib("grib");
   std::string n_amip("amip");

   // get indexes of variables with a standard_name
   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     int j;
     if( (j=var.getAttIndex(n_standard_name)) > -1 )
     {
        v_ix.push_back( i );
        sn_ix.push_back( j );

        units_ix.push_back( var.getAttIndex(n_units) ) ;
        amip_ix.push_back( var.getAttIndex(n_amip) );
        grib_ix.push_back( var.getAttIndex(n_grib) );
     }
   }

   // search the cf-standard-name-table.xml;
   // for efficiency: sort standard_names: if these are all CF conform, then
   // the table will be read only once.
   size_t v_swap;
   std::vector<int> zx;
   int sz=static_cast<int>(v_ix.size());
   for( int i=0 ; i < sz ; ++i )
     zx.push_back(i);

   sz=static_cast<int>(zx.size());
   for( int i=0 ; i < sz ; ++i )
   {
     for( int j=i+1 ; j < sz ; ++j )
     {
        std::string &var_i = pIn->variable[ v_ix[zx[i]] ].attValue[ sn_ix[zx[i]] ][0];
        std::string &var_j = pIn->variable[ v_ix[zx[j]] ].attValue[ sn_ix[zx[j]] ][0];

        if( var_j <  var_i )
        {
          v_swap = zx[i] ;
          zx[i] = zx[j];
          zx[j] = v_swap;
        }
     }
   }

   for( size_t i=0 ; i < zx.size() ; ++i )
   {
      std::string &t = pIn->variable[ v_ix[zx[i]] ].attValue[ sn_ix[zx[i]] ][0] ;
      sne.push_back( SN_Entry(t) );
   }

   // note in the structure whether a name was found and the standard name,
   // moving any modifier into the struct member remainder.
   // Also works for blank-separated (instead of '_') standard names.
   if( ! sne.size() || ! scanStdNameTable(sne) )
      // could not open/read the table
     return;

   // any annotation for a standard_name
   for( size_t i=0 ; i < zx.size() ; ++i )
   {
      int ii = zx[i] ;
      Variable& var = pIn->variable[v_ix[ii]] ;

      // take into account that a standard name may contain blanks (error)
      // or a modifier may be appended
      if( ! sne[i].found )
      {
        if( notes->inq(  bKey + "33b", var.name) )
        {
          std::string capt("!" + var.name + ":");
          capt += n_standard_name + "=" ;
          capt += sne[i].name + " is not CF compatible" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        zx[i]=-1; // disable the following units check
        continue;
      }

      std::string t = sne[i].name.substr(0, sne[i].std_name.size()) ;
      if( t.find(' ') < std::string::npos )
      {
        if( notes->inq(bKey + "33a", var.name ) )
        {
          std::string capt("!" + var.name + ":" ) ;
          capt += n_standard_name + " contains blanks" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      // any remainder could be a modifier
      if( sne[i].remainder.size() )
      {
        Split x_rem( sne[i].remainder );

        if( x_rem.size() == 1 )
          // is it a valid modifier?
          checkSN_Modifier( var.name, sne[i].std_name, sne[i].remainder );
        else if( x_rem.size() > 1 )
        {
          if( notes->inq(bKey + "33g", var.name) )
          {
            std::string capt("!");
            capt += var.name + ":standard_name has too many modifiers" ;

            std::string text(var.name + ":");
            text += n_standard_name + "=" ;
            text += sne[i].name ;

            (void) notes->operate(capt, text) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }

      // units convertable to the canonical one. Note that udunits can not
      // discern degrees_north and degrees_east. This will be done in chap4_1.
      var.canonical_units = sne[i].canonical_units ;

      // special: standard_name with modifier=number_of_observations requires units=1
      if( units_ix[ii] > -1 )
      {
        std::string& units = var.attValue[ units_ix[ii]][0];

        bool ordCase = ! cmpUnits( units, sne[i].canonical_units) ;
        bool spcCase = sne[i].remainder == "number_of_observations" ;

        if( ordCase && ! spcCase )
        {
          if( ! chap3_1_inqDeprecatedUnits(var, units) )
          {
            if( notes->inq(bKey + "33c", var.name) )
            {
              std::string capt("!" + var.name + ":" + n_units );
              capt += " are not CF compatible for" + n_standard_name + "=";
              capt += sne[i].std_name ;

              std::string text(var.name + ":" );
              text += n_units + " (file)=";
              text += var.attValue[ units_ix[ii]][0] ;
              text += "\n";
              text += var.name + ":" ;
              text += n_units + " (table)=" ;
              text += sne[i].canonical_units ;

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }

        if(spcCase && units != "1" )
        {
          if( notes->inq(bKey + "33h", var.name) )
          {
            std::string capt("!" + var.name );
            capt += ":standard_name modifier=number_of_observations requires ";
            capt += var.name + ":units=1";

            std::string text("Found: " + var.name +":" );
            text += n_units + "=" + units;

            (void) notes->operate(capt, text) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }

      // amip
      bool is=false;
      if( amip_ix[ii] > -1 )
      {
         is=true;
         Split x_amip(sne[i].amip );

         for( size_t j=0 ; j < x_amip.size() ; ++j )
         {
           if( x_amip[j] == var.attValue[ amip_ix[ii]][0] )
           {
             is=false;
             break;
           }
         }
      }

      if(is)
      {
        if( notes->inq(bKey + "33d", var.name) )
        {
          std::string capt("AMIP code=" + var.attValue[ amip_ix[ii]][0] );
          capt += " is not CF compatible, expected " + sne[i].amip ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }

      // grib
      is=false;
      if( grib_ix[ii] > -1 )
      {
         is=true;
         Split x_grib(sne[i].grib );

         for( size_t j=0 ; j < x_grib.size() ; ++j )
         {
           if( x_grib[j] == var.attValue[ grib_ix[ii]][0] )
           {
             is=false;
             break;
           }
         }
      }

      if(is)
      {
        if( notes->inq(bKey + "33e", var.name) )
        {
          std::string capt("GRIP code=" + var.attValue[ grib_ix[ii]][0] );
          capt += " is not CF compatible, expected " + sne[i].grib ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
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
   std::string av("ancillary_variables");

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     int j;
     if( (j=var.getAttIndex(av)) > -1 )
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
             std::string capt("!" + var.name + ":");
             capt += av;
             capt += " declares undefined variable=";
             capt += x_av[jj] ;

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
   f_name.push_back("flag_values");
   f_name.push_back("flag_masks");
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
        nc_type var_type = pIn->nc.getVarType(var.name) ;

        std::string key(bKey + "35");

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
            if( notes->inq(key+"d", var.name) )
            {
              std::string capt("the values of ");
              capt += var.name  ;
              capt += ":flag_values have to be mutually exclusive";

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

          if( ! (var_type == pIn->nc.isIndexType(var.name)
                     || var_type == NC_CHAR ) )
          {
            if( notes->inq(key+"e", var.name) )
            {
              std::string capt("type of ");
              capt += var.name  ;
              capt += " has to be bit-field-expression compatible";

              std::string text("type of ");
              text += var.name + "is" ;
              text += pIn->nc.getVarTypeStr(var.name) ;

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
            }
          }

          // the flag_masks att values must be non-zero
          for( size_t l=0 ; l < pf[1]->size() ; ++l )
          {
             if( (*pf[1])[l] == "0" )
             {
               if( notes->inq(key+"f", var.name) )
               {
                 std::string capt("the values of ");
                 capt += var.name + ":flag_masks have to be non-zero" ;

                 std::string text("found ");
                 for( size_t k=0 ; k < pf[1]->size() ; ++k )
                 {
                    if( k )
                      text += ", ";
                    text += (*pf[1])[k] ;
                 }

                 (void) notes->operate(capt, text) ;
                 notes->setCheckCF_Str( fail );
               }

               break;
             }
          }
        }

        if( x_f_meanings.size() == 0 && pf[0]->size() )
        {
          // if flag_values is present, then the flag_meanings att must be specified
          if( notes->inq(key+"c", var.name) )
          {
            std::string capt("if flag_values present, then flag_meanings have to be specified");

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }

          // remap for comparison of flag_values and flag_masks, if both are available
          x_f_meanings = (*pf[0])[0] ;  // split of attValue[...][0]
          f_name[2] = f_name[0] ;
          pf[2] = pf[0];
        }

        // number of items for flag_values and flag_mask, resp., must equal
        // the number of flag_meanings
        char c[] = { 'a', 'b' };
        for( size_t k=0 ; k < 2 ; ++k )
        {
          if(pf[k] == 0)
            continue;

          if( x_f_meanings.size() != pf[k]->size() )
          {
            if( notes->inq(key+c[k], var.name) )
            {
              std::string capt("different numbers of ");
              capt += var.name + ":" ;
              capt += f_name[k] + " and ";
              capt += var.name + ":" ;
              capt += f_name[2] + " items";

              std::string text(var.name + ":");
              text += f_name[k] + "=" ;
              for( size_t l=0 ; l < pf[k]->size() ; ++l )
              {
                 if( l )
                   text += ", ";
                 text += (*pf[k])[l] ;
              }
              text += "\n" ;
              text += var.name + ":";
              text += f_name[2] + "=" ;
              text += (*pf[k])[0] ;

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
            }
          }
        }

        if( cFVal > 15 )
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

           if( is && notes->inq(key+"g", var.name) )
           {
              std::string t;
              t = c;
              std::string capt(cFVersion + ": " + var.name +":");
              capt += "flag_meanings have to consist of alpha-numerics,";
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
CF::chap4(void)
{
  // Coordinate Types
  findTimeVariable();

  if( ! notes->inq(bKey + "4", "", "INQ_ONLY" ) )
     return ;  // checks are selected to be discarded by 'D'

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
     Variable& var = pIn->variable[ix];

     // pass only unresolved
     if( !var.isChecked )
       chap4(var);
  }

  return;
}

void
CF::chap4(Variable& var)
{
  if( var.isCompress )
     return;

  hasBounds(var);

  // identifying bounds before checking coordinate types
  // simplifies things.
  if( isBounds(var) )
    return;

// Coordinate Types: functions return true if identified

  if( chap4_1(var) )  // lat/lon or rotated coord
    return;

  if( chap4_3(var) )  // vertical coord, also dimensionless
    return;

  (void) chap4_4(var) ;  // time

  return ;
}

bool
CF::chap4_1(Variable& var)
{
  std::string units = var.getAttValue(n_units) ;

  // possible return values: lon, lat, rlon, rlat, size==0
  std::string coordType( units_lon_lat(var, units) );

  bool isGridUnits = coordType.size() ? true : false;

  std::string sn( var.getAttValue(n_standard_name, lowerCase) );

  bool isX=false;  // optional, but if, then axis=X
  bool isY=false;  // optional, but if, then axis=Y

  std::string axis( var.getAttValue(n_axis, lowerCase) );

  // axis is optional, but if provided, then X/Y/Z
  if( axis.size() )
  {
    if( axis == "y" )
      isY = true;
    else if( axis == "x" )
      isX = true;
    else if( isCheck && isGridUnits )
    {
      std::string key(bKey);
      if( coordType == "lat" )
        key += "41c" ;
      else if( coordType == "lon" )
        key += "41b" ;
      else if( coordType[0] == 'r' )
        key += "41d" ;

      if( notes->inq( key, var.name) )
      {
        std::string capt( "!" + var.name );
        capt += ":axis is optional, but has to be " ;
        if( coordType == "lat" || coordType == "rlat" )
          capt += "Y" ;
        else if( coordType == "lon" || coordType == "rlon" )
          capt += "X" ;

        std::string text(var.name);
        text += ":axis (file)=";
        text += var.getAttValue(n_axis) ;

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( !isGridUnits && var.units.size() == 0 && (isX || isY) )
  {
     if( ! notes->findMapItem(bKey + "5c", var.name)
            && notes->inq(bKey + "41e", var.name) )
     {
       std::string capt("!" + var.name + ":axis=" );
       capt +=  var.getAttValue(n_axis);
       capt += ", but the units attribute is missing";

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
  }

  bool isStndName=false;

  if( sn.size() == 0 )
    sn = var.getAttValue(n_long_name) ;

  if( sn == n_longitude )
  {
    isStndName=true;
    isX=true;
    axis="X";
    ++var.coord.indication_X ;
  }
  else if( sn == n_latitude )
  {
    isStndName=true;
    isY=true;
    axis="Y";
    ++var.coord.indication_Y ;
  }

  if( isCheck && !isGridUnits && isStndName  )
  {
     if( ! notes->findMapItem(bKey + "5c", var.name)
           && ! notes->findMapItem(bKey + "41e", var.name)
              && notes->inq(bKey + "41e", var.name) )
     {
       std::string capt("!" + var.name );
       capt +=  ":standard_name=" + sn ;
       capt += " indicates " + axis;
       capt += ", but the units attribute is missing";

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
  }

  if( isCheck && isGridUnits && var.canonical_units.size()  )
  {
     // var.canonical_units ensures that the standard_name is CF conform
//     if( sn.substr(0, coordType.size()) != coordType )
     if( ! cmpUnits(units, var.canonical_units) )
     {
       if( notes->inq(bKey + "41a", var.name) )
       {
         std::string capt("!" + var.name );
         capt += ":units=" + var.units;
         capt +=  " whereas the standard_name=" + sn ;
         capt += " is associated with canonical_units=" + var.canonical_units;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }

  if( isX || isY )
  {
    var.isDataVar=false;

    if( isY )
      var.coord.isY=true;
    else if( isX )
      var.coord.isX=true;

    var.isChecked=true;

    return true;
  }

  return false;
}

bool
CF::chap4_3(Variable& var)
{
  // vertical coordinate

  // the case of adimensionless vertical coordinate was checked elsewhere
  if( chap4_3_1(var) )  // 2-dimensional or scalar?
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

  bool isP=false;
  bool isM=false;
  bool isZ=false;

  if( units.size() )
  {
    if( (isP=cmpUnits( units, "Pa")) || (isM=cmpUnits( units, "m")) )
       ++var.coord.indication_Z ;

    if( var.coord.isCoordVar ) //|| isCF14_timeSeries )
       ++var.coord.indication_Z;
  }

  bool isPositiveValue;
  if( (isPositiveValue=positive == "up" || positive == "down") )
  {
    if( var.coord.isCoordVar )
      isZ=true;
    else
      ++var.coord.indication_Z ;
  }

  if( axis == "z" )
  {
    if( var.coord.isCoordVar )
      isZ=true;
     else
      ++var.coord.indication_Z ;
  }

  std::string stnd_name( var.getAttValue(n_standard_name, lowerCase) ) ;

  if( !isZ )
  {
    if( stnd_name.size() )
    {
      // any indication for a Z axis in the standard name?
      if( stnd_name.substr(0,6) == "height" )
      {
        if( var.coord.isCoordVar )
          isZ=true;
        else
          ++var.coord.indication_Z ;
      }
      else if( stnd_name.substr(0,8) == "altitude" )
      {
        if( var.coord.isCoordVar )
          isZ=true;
        else
          ++var.coord.indication_Z ;
      }
      else if( stnd_name.substr(0,5) == "depth" )
      {
        if( var.coord.isCoordVar )
          isZ=true;
        else
          ++var.coord.indication_Z ;
      }
    }
  }

  // a little bit of fuzzy logic, but could be reset later
  if( !isZ )
  {
     if( (isP || isM) && var.coord.indication_Z > 1 )
       isZ = true;
     else  if( var.coord.indication_Z > 3 )
       isZ = true;
  }

  if( isCheck && units.size() == 0 )
  {
    bool is=false;
    if( var.coord.isCoordVar && cmpUnits(var.canonical_units, "m") )
      is=true;
    else if( isZ || var.coord.indication_Z > 1 )
      is=true;

    if(is)
    {
      if( notes->inq(bKey + "431a", var.name) )
      {
        notes->eraseMapItem(bKey + "44a", var.name); // safe

        std::string capt("parameters of variable=" + var.name);
        capt += " indicate a vertical coordinate variable, but attribute=units is missing";

        std::string text;
        if( var.coord.isCoordVar )
        {
           if( text.size() )
              text += ", coordinate variable: yes";
        }

        if( stnd_name.size() )
        {
           if( text.size() )
              text += ", ";
           text += n_standard_name + "=" + stnd_name;
        }

        if( positive.size() )
        {
           if( text.size() )
              text += ", ";
           text += n_positive + "=" + var.getAttValue(n_positive);
        }

        if( axis.size() )
        {
           if( text.size() )
              text += ", ";
           text += n_axis + "=" + var.getAttValue(n_axis);
        }

        (void) notes->operate(capt, "Indicators: " + text) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  // conclusion
  if( isZ )
    var.coord.isZ = true;

  // check properties and validity
  if(positive.size() )
  {
    if(var.coord.isZ )
    {
      if( var.coord.isCoordVar )
      {
        if( ! isPositiveValue )
        {
          if( notes->inq(bKey + "43a", var.name) )
          {
             std::string capt("!" + var.name);
             capt += ":positive has to be either Up oder Down" ;

             std::string text("Found ");
             text += var.name + ":" + n_positive ;
             text += "=" + var.getAttValue(n_positive) ;

             (void) notes->operate(capt, text) ;
             notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  // conclusion
  if( var.coord.isZ )
  {
     var.isChecked=true;
     var.isDataVar=false;
     return true;
  }

  return false;
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

  int ix =-1 ;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
     Variable& var = pIn->variable[i] ;

     if( var.isValidAtt(n_formula_terms) )
     {
        ix=i;
        break;
     }

     int j;
     if( (j=var.getAttIndex(n_standard_name)) > -1 )
     {
        for(size_t k=0 ; k < valid_sn.size() ; ++k )
        {
           if( valid_sn[k] == var.attValue[j][0] )
           {
              ix=i;
              break;
           }
        }

        if( ix > -1 )
           break;
     }
  }

  if( ix > -1 && chap4_3_2(pIn->variable[ix], valid_sn, valid_ft) )
  {
    Variable& var = pIn->variable[ix];
    var.coord.isZ    = true;
    var.coord.isZ_DL = true;
    var.isDataVar=false;
    var.isChecked=true;
  }

  return;
}

bool
CF::chap4_3_2(Variable& var,
   std::vector<std::string>& valid_sn,
   std::vector<std::string>& valid_ft )
{
  // dimensionless vertical coordinate
  int found_sn_ix=-1;
  int found_ft_ix=-1;
  std::vector<std::pair<std::string, std::string> > p_found_ft ;

  // cross-check of standard_name vs. formula_terms
  if( xCheckSNvsFT(var, valid_sn, valid_ft, found_sn_ix, found_ft_ix,
                   p_found_ft) )
     return false;

  bool isDimless=true;

  // tolerated units for COARDS compatibility: level, layer, sigma_level
  int units_ix=-1;

  std::string units( var.getAttValue(n_units) );

  if( units.size() )
  {
    if( chap3_1_inqDeprecatedUnits(var, units) )
      ++var.coord.indication_Z;
    else
      units_ix=var.getAttIndex(n_units);
  }

  if( isCheck && units_ix > -1 )
  {
    std::string key;
    std::string capt;
    std::string text;

    if( units != "1" )
    {
      key = "CF_432b";
      capt = "Dimensionless vertical coordinates with units of a physical value";
      text = "units (file)=" ;
      text += units ;

      if( notes->inq( key, var.name) )
      {
        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  std::vector<std::string> fTerms;
  Split x_fTerms( valid_ft[found_ft_ix] );
  for( size_t i=0 ; i < x_fTerms.size() ; ++i )
     fTerms.push_back( x_fTerms[i] );

  // after return, fTerms contains associated variables.
  verify_formula_terms(var,found_ft_ix,
          valid_ft[ found_ft_ix ], fTerms, p_found_ft);

  if( isDimless )
  {
     ++var.coord.indication_Z;
     var.coord.isCoordVar=true;
     var.coord.isZ_DL=true;
     var.coord.isZ=true;

     int i;
     for( size_t j=0 ; j < fTerms.size() ; ++j )
     {
       if( (i=pIn->getVarIndex(fTerms[j])) > -1 )
       {
         pIn->variable[i].isChecked=true;
         pIn->variable[i].isDataVar = false;
       }
     }
  }

  return isDimless;
}

bool
CF::chap4_4(Variable& var)
{
  // test for time units and verify a correct format
  if( timeUnitsFormat(var) )
  {
    ++var.coord.indication_T;

    if( var.coord.isCoordVar
            || var.getAttValue(n_standard_name, lowerCase) == timeName )
    {
       var.coord.isT=true;
       var.isChecked=true;
       var.isDataVar = false;
    }

    // calendar and related stuff
    if( chap4_4_1(var) )
      ++var.coord.indication_T;
  }
  else
  {
     std::vector<std::string> timeAtt;
     timeAtt.push_back("calendar");
     timeAtt.push_back("month_lengths");
     timeAtt.push_back("leap_year");
     timeAtt.push_back("leap_month");

     // particular for some attributes
     if( isCheck )
     {
       for( size_t j=0 ; j < timeAtt.size() ; ++j )
       {
         if( var.isValidAtt( timeAtt[j]) )
         {
           if( notes->inq(bKey + "44l", var.name) )
           {
             std::string capt("!" + var.name + ":");
             capt += timeAtt[j] + " may only be attached to the time coordinate";

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }
       }
     }
  }

  return false;
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

  std::string n_calendar("calendar");
  std::string v_cal( var.getAttValue(n_calendar, lowerCase) );

  std::string str;
  std::string month_lengths("month_lengths") ;

  if( v_cal.size() == 0 )
  {
     if( ! var.isValidAtt(month_lengths) )
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
      std::string capt("!" + var.name + ":calendar=");
      capt += str;
      capt += " is misspelled, expected: " + v_cal ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  bool isTime = v_cal.size();

  if( !isCheck )
    return isTime;

  if( v_cal.size() )
  {
     for( size_t i=0 ; i < def_cal.size() ; ++i )
       if( def_cal[i] == v_cal )
          return isTime;
  }

  // calendar is non-standard or not defined.

  // --> month_lengths is required with 12 values
  int ml;
  if( (ml=var.getAttIndex(month_lengths)) > -1 )
  {
    if( var.attValue[ml].size() != 12 )
    {
      if( notes->inq(bKey + "441b", var.name) )
      {
        std::string capt("!" + var.name + ":month_lengths requires 12 values") ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }
  else if( v_cal.size() && notes->inq(bKey + "441h", var.name) )
  {
    std::string capt("non-standard " + var.name ) ;
    capt += ":calendar=" + v_cal + " requires attribute=month_lengths" ;

    (void) notes->operate(capt) ;
    notes->setCheckCF_Str( fail );
  }

  // if att=leap_month is defined, then leap_year is required
  bool isLY=false;
  bool isLMV=false;

  int i;
  if( (i=var.getAttIndex("leap_month")) > -1 )
  {
    isLMV=true;

    std::string capt1;
    std::string capt2;

    // value must be in the range 1-12
    if( var.attValue[i][0].size() )
    {
      double dv=hdhC::string2Double( var.attValue[i][0] ) ;
      int iv = static_cast<int>(dv);

      if( (dv - static_cast<double>(iv)) != 0. )
      {
        capt1 = "!" + var.name + ":leap_month has to be integer scalar" ;
        capt1 += ", found " + var.attValue[i][0] ;
      }

      if(! (iv > 0 && iv < 13) )
      {
        capt2 = "!" + var.name ;
        capt2 += ":leap_month requires a value in the range 1-12";
        capt2 += ", found " + var.attValue[i][0];
      }
    }
    else  // empty attribute
      capt2 = "!" + var.name + ":leap_month requires a value in the range 1-12";

    if( capt1.size() && notes->inq(bKey + "441e", var.name) )
    {
      (void) notes->operate(capt1) ;
       notes->setCheckCF_Str( fail );
    }

    if( capt2.size() && notes->inq(bKey + "441d", var.name) )
    {
      (void) notes->operate(capt2) ;
      notes->setCheckCF_Str( fail );
    }
  }

  if( (i=var.getAttIndex("leap_year")) > -1 )
  {
    isLY=true;

    // value must be in integer
    double dv=hdhC::string2Double( var.attValue[i][0] ) ;
    int iv = static_cast<int>(dv);

    if( (dv - static_cast<double>(iv)) != 0. )
    {
      if( notes->inq(bKey + "441e", var.name) )
      {
        std::string capt("!" + var.name + ":leap_year has to be integer scalar") ;
        std::string text("Found: leap_year=" + var.attValue[i][0]);

        (void) notes->operate(capt,text) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  // rc_chap4_4_1()
  if( followRecommendations && isLMV && !isLY )
  {
    if( notes->inq(bKey + "441c", var.name) )
    {
      std::string capt("Recommendation: ");
      capt += var.name + ":leap_month should not appear unless leap_year is present" ;

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
  if( ! notes->inq(bKey + "5", "", "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  // find coordinate variables
  // chap5_1(Variable&) ; // applied in run() for convenience

  // find 'coordinates' attribute(s)
  chap5_2() ;

  // reduced horizontal grid
  // chap5_3() ;  see chap8_2()

  // time series of station data
  if( cFVal < 16 )
    chap5_4() ;

  // chap5_5 (trajectories): checked implicitely

  // grid mapping
  chap5_6();

  // scalar coordinate was checked in run()

  return ;
}

void
CF::chap5_1(Variable& var)
{
  // Coordinate variables: a single dimension and identical name for
  // dimension and variable, e.g. time(time).

  // Note that a dimless vertical coordinate was considered before.
  if( var.isChecked )
    return ;

  if( var.dimName.size() == 1 && var.dimName[0] == var.name )
  {
     var.coord.isCoordVar=true;
     var.isDataVar=false;

     if( isCheck )
       checkCoordVarValues(var) ;

     if( var.isValidAtt("compress") )
     {
        compressIx=pIn->getVarIndex(var.name);
        var.isCompress=true;
     }
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

    Split x_ca( var.getAttValue(n_coordinates) );

    int ii;
    for(size_t k=0 ; k < x_ca.size() ; ++k )
    {
      // It is permissible to have coordinate variables included.
      if( (ii=pIn->getVarIndex(x_ca[k])) > -1 )
      {
        if( ! pIn->variable[ii].coord.isCoordVar )
        {
          std::string su( units_lon_lat(var) );

          if( su == "lat" )
            ++pIn->variable[ii].coord.indication_Y;
          else if( su == "lon" )
            ++pIn->variable[ii].coord.indication_X;
          else if( su == "rlat" )
            ++pIn->variable[ii].coord.indication_Y;
          else if( su == "rlon" )
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
         var2.isDataVar = false ;

         std::string su( units_lon_lat(var2) );
         if( su == "lat" || su == "rlat" )
           ++var2.coord.indication_Y;
         else if( su == "lon" || su == "rlon" )
           ++var2.coord.indication_X;

         continue ;
      }
    }
  }

  return;
}

void
CF::chap5_4(void)
{
  // time series of station data:
  // there is a dimension whithout variable representation
  // and lon and lat auxiliaries depend only on this dimension.

  if( getPureDimension() )
  {
    for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
    {
      Variable& var = pIn->variable[ix];

      if( var.isChecked || var.coord.isCoordVar )
        continue;

      for( size_t k=0 ; k < pureDimension.size() ; ++k )
      {
         if( var.dimName.size() == 1 && var.dimName[0] == pureDimension[k] )
         {
           std::string su( units_lon_lat(var) );

           if( su == "lat" || su == "rlat" )
              ++var.coord.indication_Y ;
           else if( su == "lon" || su == "rlon" )
              ++var.coord.indication_X ;
           else
             continue;  // skip next line

           var.isDataVar = false;
         }
      }
    }
  }

  return ;
}

void
CF::chap5_6(void)
{
  std::string gm("grid_mapping");

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
    std::string str;

    if( var.isValidAtt(gm) )
    {
       str = var.getAttValue(gm) ;

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
         mapCoord[0] = "grid_latitude" ;
         mapCoord[1] = "grid_longitude" ;
       }
       else if( ! (retVal=chap5_6_gridMappingVar(var, str, "")) )
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
             std::string capt("!" + var.name + ":" + gm);
             capt += " declares non-existing grid_mapping variable="  ;
             capt += var.getAttValue(gm);

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
           else if( notes->inq(bKey + "56h") )
           {
             std::string capt("!" + var.name);
             capt += ":grid_mapping is empty" ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }

         else if( retVal == 2 && notes->inq(bKey + "56f") )
         {
           std::string capt("!" + str + ":" + gm +"_name is not provided");

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }

         else if( retVal > 2 && notes->inq(bKey + "56g") )
         {
           // index==retVal-3 takes back an action done in chap5_6_gridMappingVar()
           std::string capt("grid mapping variable=" + str );
           capt += " with undefined grid_mapping_name=" ;
           capt += pIn->variable[retVal-3].getAttValue("grid_mapping_name");

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
      if( var.isValidAtt(gm + "_name") )
      {
        if( notes->inq(bKey + "56e", var.name) )
        {
          std::string capt( "missing grid_mapping attribute,");
          capt += " found detached grid-mapping variable=" + var.name;

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
   //         2: found grid_mapping variable with no grid_mapping_name
   //    3 + ix: found grid_mapping variable with invalid grid_mapping_name

   // find grid_mapping variable gmn
   // and check existence of grid_mapping_name att
   int ix = pIn->getVarIndex(s);  // index of the grid mapping variable

   int retVal=1;

   if( ix > -1 )
   {
     Variable& var_gmv = pIn->variable[ix] ;
     var_gmv.isDataVar=false;
     var_gmv.isMapVar=true;

     retVal=2;

     // the grid mapping variabel must have attribute grid_mapping_name
     std::string str( var_gmv.getAttValue("grid_mapping_name") );

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

           for( size_t i=0 ; i < vs_gmn.size() ; ++i )
           {
             if( str == vs_gmn[i] )
             {
                gmn = str ;
                break;
             }
           }
        }

        retVal = (str == gmn) ? 0 : (3+ix);
     }

     // recommendation: grid mapping variable should have no dimensions
     if( followRecommendations && var_gmv.dimName.size() )
     {
        if( notes->inq(bKey + "56d", var_gmv.name) )
        {
          std::string capt("recommendation: grid mapping variable=");
          capt += var_gmv.name + " should have no dimensions" ;

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
     if( ! notes->findMapItem(bKey + "5c", dataVar.name)
             && notes->inq(bKey + "56a", dataVar.name) )
     {
        std::string capt("grid mapping requires attribute ");
        capt += dataVar.name + ":";
        capt += n_coordinates  ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
   }

   // each name in mCV should have a corresponding standard_name
   // in one of the variables indicating an auxiliary coordinate.
   bool is_sn[] = {true, true};

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     // coordinate variables associated with the mapping
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
         std::string capt("grid mapping: coordinate variable with standard_name=");
         capt += mCV[l] ;
         capt += " is required";

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
  if( ! notes->inq(bKey + "6", "", "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  std::vector<size_t> label_ix;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
     Variable& var_i = pIn->variable[i];

     if( var_i.type == NC_CHAR )
     {
       // look for a unique dimension
       size_t id;
       size_t sz = var_i.dimName.size();
       size_t count=0;

       for( id=0 ; id < sz ; ++id )
       {
         count=0;

         for( size_t k=0 ; k < pIn->varSz ; ++k )
         {
           if( i == k )
              continue;

           Variable& var_k = pIn->variable[k];

           for( size_t kd=0 ; kd < var_k.dimName.size() ; ++kd )
             if( var_i.dimName[id] == var_k.dimName[kd] )
               ++count;
         }

         if( count == 0 )
           break;
       }

       if( (sz == 1 && count) || count == 0 )
       {
         // 1st condition: a false non-detection, the char-length dim is missing
         // 2nd condition: regular case
         var_i.isLabel=true;
         var_i.isDataVar=false;
         label_ix.push_back(i);
       }
     }
  }

  if( label_ix.size() == 0 )
     return;

  // declared here because of the scope
  std::vector<std::string> vs_region_table;

  // any label not named in a coordinates attribute?
  for( size_t l=0 ; l < label_ix.size() ; ++l )
  {
    Variable& label = pIn->variable[label_ix[l]];

    // first: var_ix, second: index of coordinates attribute containing a label
    std::vector<std::pair<size_t, int> > ca_ix;
    bool isNoException=true;

    for( size_t i=0 ; i < pIn->varSz ; ++i )
    {
      if( i == label_ix[l] )
         continue;

      Variable& var = pIn->variable[i];

      int j;

      if( (j=var.getAttIndex("grid_mapping")) > -1 )
        if( var.attValue[j][0] == label.name )
           isNoException = false;

      if( (j=var.getAttIndex(n_coordinates)) > -1 )
      {
         Split x_ca(var.attValue[j][0]);
         for( size_t x=0 ; x < x_ca.size() ; ++x )
         {
           if( x_ca[x] == label.name )
           {
              ca_ix.push_back( std::pair<size_t, int>(i, j) ) ;
              break;
           }
         }
      }
    }

    if( isNoException && ca_ix.size() == 0 )
    {
      if( notes->inq(bKey + "6a", label.name) )
      {
         std::string capt("label ");
         capt += label.name + " is not listed in any coordinates attribute" ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
      }
    }

    for( size_t c=0 ; c < ca_ix.size() ; ++c)
    {
      // A label must have one or two dimensions.
      // The maximum string-length cannot be checked

      // If the label is not a scalar auxiliary type one , then the first dimension
      // must match one of the dims of the data variable
      if( label.dimName.size() == 2 )
      {
        Variable& dv = pIn->variable[ca_ix[c].first] ;

         size_t i;
         for( i=0 ; i < dv.dimName.size() ; ++i )
            if( label.dimName[0] == dv.dimName[i] )
              break;

         if( i == dv.dimName.size() )
         {
           if( notes->inq(bKey + "6b", dv.name) )
           {
              std::string capt("label ");
              capt += label.name + " and variable=" ;
              capt += dv.name;
              capt += " have to share a dimension";

              std::string text("label: ");
              text += label.getDimNameStr(true);
              text += "\n variable: ";
              text += dv.getDimNameStr(true);

              (void) notes->operate(capt, text) ;
              notes->setCheckCF_Str( fail );
           }
         }
       }
    }

    // check standardized-region-names. Note that it is possible that
    // neither a standard_name nor any data is provided
    int j = label.getAttIndex(n_standard_name) ;
    if( j > -1 && label.attValue[j][0] == "region" )
    {
      if( ! pIn->nc.isNoRecords(label.name) )
      {
        std::vector<std::string> vs_region_file;
        pIn->nc.getData(vs_region_file, label.name);

        bool isEmpty=true; //for limited variables
        for( size_t f=0 ; f < vs_region_file.size() ; ++f)
        {
          if( vs_region_file[f].size() )
          {
            isEmpty=false;
            vs_region_file[f] = hdhC::Lower()(vs_region_file[f]);
          }
        }

        if( isEmpty )
           continue;

        if( vs_region_table.size() == 0 )
        {
          if( region_table.size() == 0 )
             return;

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
        bool is=false;
        for( size_t f=0 ; f < vs_region_file.size() ; ++f)
        {
          for( size_t t=0 ; t < vs_region_table.size() ; ++t)
          {
             if( vs_region_file[f] == vs_region_table[t] )
             {
                is=true;
                break;
             }
          }

          if(is)
            break;
        }

        if( !is && label.attValue[j][0] == "region" )
        {
          if( notes->inq(bKey + "6c", label.name) )
          {
             std::string capt("label ");
             capt += label.name + ":" + n_standard_name + "=region" ;
             capt += " requires supply of names from the cf-standardized-region-names table";

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  return ;
}

void
CF::chap7(void)
{
  // cells
  if( ! notes->inq(bKey + "7", "", "INQ_ONLY" ) )
    return ;  // checks are selected to be discarded by 'D'

  // cell boundaries
  chap7_1() ;

  // cell measure
  chap7_2() ;

  // cell methods
  chap7_3() ;

  // climatological statistics
  chap7_4() ;

  return ;
}

void
CF::chap7_1(void)
{
  if( !isCheck )
    return ;

  // cell boundaries
  std::string units;

  // if method isBounds() detects bounds, then it sets:
  // var.boundsOf = pIn->variable[...].name ;
  // pIn->variable[...].bounds = var.name ;

  // look for some indexes
  // first: variable.boundsOf, i.e. is bound
  // second: variable.bounds, i.e. has bounds
  std::vector<std::pair<int, int> > ix;

  // Is a variable declared as bounds of another variable? Yes, then
  // corresponding variable members are 'boundsOf' and 'bounds', repectively.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    // find coordinate variables with a bound or climatology declaration.
    if( isBounds(pIn->variable[i]) )
       ix.push_back( std::pair<int, int>
            (static_cast<int>(i),
               static_cast<int>(pIn->getVarIndex(pIn->variable[i].boundsOf) ) ) );
  }

  // Identify bounds which have not been declared.
  if( followRecommendations )
  {
    // find matching pairs of bounds and its associated variable
    for( size_t k=0 ; k < pIn->varSz ; ++k )
    {
      Variable& var_has = pIn->variable[k] ;

      if( ! var_has.coord.isCoordVar )
        continue;

      if( var_has.isScalar )
        continue;

      // exclude vars known to have bounds
      bool isCont = false;
      for( size_t u=0 ; u < ix.size() ; ++u )
      {
        if( ix[u].second == static_cast<int>(k) )
        {
          isCont=true;
          break;
        }
      }

      if( isCont )
         continue;

      for( size_t i=0 ; i < pIn->varSz ; ++i )
      {
        if( i == k )
          continue;

        Variable& var_is = pIn->variable[i] ;

        // must be coordinate variable
        if( ! var_is.coord.isCoordVar )
          continue;
        if( var_is.isScalar )
          continue;

        // exclude vars known to be bounds
        isCont = false;
        for( size_t u=0 ; u < ix.size() ; ++u )
        {
          if( ix[i].first == static_cast<int>(k) )
          {
            isCont=true;
            break;
          }
        }

        if( isCont )
           continue;
      }
    }
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

/*
    if( sz_has == 0 )
    {
      // a scalar variable has no dims;
      // this is equivalent to a single dim of size=1
      if( sz_is == 0 )
        isMissingVerticesDim=true;
      else if( sz_is > 1 )
        hasTooManyVerticesDim=true;
      else
        ++sz_is; // set size==1
    }
    else
    {
*/
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
//    }

    if( isNotIdenticalSubSet )
    {
      if( ! notes->findMapItem(bKey + "74f", var_is.name)
              && notes->inq(bKey + "71a", var_is.name) )
      {
        std::string capt("boundary variable=" + var_is.getDimNameStr(true));
        capt += " must have the dimensions" ;
        capt += " of the associated variable=";
        capt += var_has.getDimNameStr(true) + " as sub-set" ;

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
        std::string capt("boundary variable=" + var_is.name);
        capt += " is missing the additional dimension for the vertices" ;

        std::string text("Found: ");
        text += var_is.getDimNameStr(true);
        text += " vs. " + var_has.getDimNameStr(true);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    else if( hasTooManyVerticesDim )
    {
      if( notes->inq(bKey + "71f", var_is.name) )
      {
        std::string capt("boundary variable=" + var_is.name);
        capt += " has too many additional dimensions for the vertices" ;

        std::string text("Found: ");
        text += var_is.getDimNameStr(true);
        text += " vs. " + var_has.getDimNameStr(true);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // a boundary variable must have a numeric type
    if( var_is.type == NC_CHAR )
    {
      if( notes->inq(bKey + "71c", var_is.name) )
      {
        std::string capt("boundary=" + var_is.name );
        capt += " has to be a numeric type" ;

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
         if( f == 0 )
           tag = "71d";
         else
           tag = "71e";

         if( notes->inq(bKey + tag , var_is.name) )
         {
           std::string capt;
           if( var_has.isClimatology )
              capt = n_climatology;
           else
              capt = "boundary";

           capt += " variable=" + var_is.name + ":" ;
           if( f == 0 )
           {
             capt += n_units + "=" + var_is.getAttValue(n_units) ;
             capt += " and " + var_has.name + ":" + n_units + "=" ;
             capt += var_has.getAttValue(n_units) ;
           }
           else
           {
             capt += n_standard_name + "=" + var_is.getAttValue(n_standard_name);
             capt += " and " + var_has.name + ":"  + n_standard_name + "=" ;
             capt += var_has.getAttValue(n_standard_name) ;
           }

           capt += " are different" ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
    }

    rc_chap7_1(var_is);
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
  std::string n_cm_alt=n_cell_measures.substr(0, n_cell_measures.size()-1);
  int meas_ix; // index of the measure variable

  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    int jx = var.getAttIndex(n_cell_measures) ;

    // a special inquiry
    if( jx == -1 )
    {
      if( (jx=var.getAttIndex(n_cm_alt)) > -1 )
      {
        if( notes->inq(bKey + "72a", var.name) )
        {
          std::string capt("misspelled attribute " ) ;
          capt += var.name ;
          capt += ":" + n_cell_measures + ", found ";
          capt += n_cm_alt ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        var.attName[jx] = n_cell_measures ;
      }
    }

    if( jx == -1 )  // cell_measures not available
      continue;

    cm = var.getAttValue(n_cell_measures);

    // syntax: 'key: variable'
    // only two key-words (simultaneously the name of variable)
    // are defined: area and volume.
    Split x_cm(cm);

    // check key-words
    std::vector<std::string> cm_key( getKeyWordArgs(cm, "key", ':') );

    if( cm_key.size() == 0 )
    {
       // missing key-word
       if( notes->inq(bKey + "72b", var.name) )
       {
         std::string capt("!" + var.name) ;
         capt += ":" + n_cell_measures  ;
         capt += " misses key-word area: or volume:" ;

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
           std::string capt("!" + var.name) ;
           capt += ":" + n_cell_measures + "=" ;
           capt += cm_key[0] + " requires area: or volume:" ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }

         var.attValue[jx][0]="CF failed" ;
         continue ;
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
          std::string capt("non-existing variable declared by " );
          capt += var.name + ":" + n_cell_measures ;
          capt += "=" + cm ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        continue;
      }

      Variable& mvar = pIn->variable[meas_ix];

      mvar.isDataVar=false;

      // must have units
      if( ! mvar.isValidAtt(n_units) )
      {
        if( notes->inq(bKey + "72c", mvar.name) )
        {
          std::string capt("measures variable ") ;
          capt += cm_arg[0];
          capt += " must have units" ;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }

        var.attValue[jx][0]="CF failed" ;
      }
      else
      {
        // a measure variable must have units that are consistent with
        // the measure type, i.e., m2 for area and m3 for volume.
        std::string s(mvar.getAttValue(n_units));
        std::string xpctdU;

        if( cm_key[0].find("area") < std::string::npos )
          xpctdU = "m2";
        else if( cm_key[0].find("volume") < std::string::npos )
          xpctdU = "m3";

        if( ! cmpUnits( s, xpctdU) )
        {
          if( notes->inq(bKey + "72d", mvar.name) )
          {
            std::string capt("!" + mvar.name + " requires units=") ;
            capt += xpctdU ;

            std::string text("Found " + mvar.name) ;
            text += ":units=";
            text += s;

            (void) notes->operate(capt, text) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }

    // The dimensions of the measure variable should be the same as or a subset of
    // the dimensions of the variable to which they are related, but their order
    // is not restricted.
    // There is no clue at present how to check this.

/*
    if( jx > 0 )
    {
      // note that var has declared a cell_measures attribute,
      // get the dimensions of var which are associated to XYZ coordianates
      std::vector<std::string> dims;
      for( size_t a=0 ; a < pIn->varSz ; ++a )
      {
         Variable& vara = pIn->variable[a];
         if( vara.coord.isCoordVar )
            if( vara.coord.isX || vara.coord.isY || vara.coord.isZ )
               dims.push_back( vara.name ) ;
      }

      Variable& mvar = pIn->variable[meas_ix];
    }
*/
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

    // Note that key-words used for climatological statistics are checked in chap7_4()

    int jx = var.getAttIndex(n_cell_methods) ;

    // a special inquiry
    if( jx == -1 )
    {
      std::string n_cm_alt=n_cell_methods.substr(0,n_cell_methods.size()-1) ;
      if( (jx=var.getAttIndex(n_cm_alt)) > -1 )
      {
        if( notes->inq(bKey + "73f", var.name) )
        {
          std::string capt("!" + var.name) ;
          capt += ":" + n_cell_methods + " is misspelled";

          std::string text("expected" ) ;
          text += var.name ;
          text += ":" + n_cell_methods + "\nfound" ;
          text += var.name ;
          text += ":cell_method" ;

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }

        var.attName[jx] = n_cell_methods ;
      }

      continue;
    }

    // note that the loop is in fact gratuitous
    std::string cm ;
    for( size_t i=0 ; i < var.attValue[jx].size() ; ++i )
    {
       if( i )
          cm += " " ;
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
        std::string capt("!" + var.name);
        capt += ":" + n_cell_methods + ": unclosed comment" ;

        std::string text(var.name);
        text += ":" + n_cell_methods +"=" ;
        text += cm;

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // parse 'wordLists', i.e., sequences of
    // dim1: [dim:2: [dim3 ...] method [....].
    // Any dimN: after a non-colon item indicates the start
    // of another wordList, except the phrase within parenthesis.

    Split x_cm(s0);

    if( ! isColonFail )
    {
      // no begin with dim:
      if( x_cm[0].size() && x_cm[0][x_cm[0].size()-1] != ':' )
        isColonFail = true;

      // an end with dim:
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
         std::string capt("!" + var.name);
         capt += ":" + n_cell_methods + " requires blank separated word-list" ;

         std::string text(var.name);
         text += ":" + n_cell_methods + "=" ;
         text += cm;

         (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
      }

      var.attValue[jx][0].clear() ;
      return;
    }

    if( isColonFail )
    {
      if( notes->inq(bKey + "73b", var.name) ) // only once
      {
        std::string capt("!" + var.name + ":" + n_cell_methods);
        capt += " requires <name: method" ;
        if( bracketOpen )
          capt += " (comment)" ;
        capt += ">, found " + cm ;

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
             if( notes->inq(bKey + "733g", var.name) )
             {
               std::string capt("!" + var.name + ":");
               capt += n_cell_methods + ": replacement of " ;
               capt += pIn->variable[i0].name + ": and " ;
               capt += pIn->variable[i1].name + ": by area: is recommended" ;

               (void) notes->operate(capt) ;
               notes->setCheckCF_Str( fail );
             }

             // replace by area:
             cm.clear();
             for( size_t j=0 ; j < x_cm.size() ; ++j )
             {
                if( j )
                   cm += " ";

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
    std::vector<std::string> cm_dim;
    std::vector<std::string> cm_method;
    std::vector<std::string> cm_comment;
    bool isNewList=true;

    for( size_t i=0 ; i < x_cm.size() ; ++i )
    {
       size_t lst=x_cm[i].size();
       if(lst)
          --lst;

       // get comments
       if( x_cm[i][0] == '(' )
       {
          cm_comment.back() = x_cm[i++] ;
          for( ; i < x_cm.size() ; ++i )
          {
             cm_comment.back() += " " + x_cm[i] ;

             if( x_cm[i].size() && x_cm[i][x_cm[i].size()-1] == ')' )
               break;
          }

          isNewList=true;
       }

       // get methods
       else if( isNewList && x_cm[i][lst] == ':')
       {
          cm_dim.push_back( x_cm[i] );
          cm_method.push_back("");
          cm_comment.push_back("");
          isNewList=false;
       }

       // get dim:
       else if( x_cm[i][lst] == ':')
          cm_dim.back() += " " + x_cm[i] ;
       else
       {
          if( isNewList )
             cm_method.back() += " " + x_cm[i] ;
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

    int sz = static_cast<int>(cm_dim.size());
    for( int l=0 ; l < sz-1 ; ++l )
    {
      for( int j=l+1 ; j < sz ; ++j )
      {
        if( cm_dim[l] == cm_dim[j] )
        {
           size_t k;
           for( k=0 ; k < multipleDims.size() ; ++k )
             if( multipleDims[k] == cm_dim[l] )
                break;

           if( k == multipleDims.size() )
             multipleDims.push_back( cm_dim[l] );
        }
      }
    }

    // check for multiple dimensions across the word-lists
    std::string time_colon( timeName + ":" );

    for( size_t i=0 ; i < multipleDims.size() ; ++i )
    {
      if( multipleDims[i] == time_colon )
        continue;
      else if( multipleDims[i] == "comment" )
        continue;
      else if( multipleDims[i] == "interval" )
        continue;

      if( notes->inq(bKey + "733f", var.name) )
      {
        std::string capt("!" + var.name + ":" + n_cell_methods + ": ") ;
        capt += multipleDims[i] + ": has to occur only once" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    for( size_t i=0 ; i < cm_dim.size() ; ++i )
    {
       cellMethods_Name(cm_dim[i], var) ;
       cellMethods_Method(cm_method[i], var) ;

       if( cm_comment[i].size() )
         cellMethods_Comment(cm_comment[i], var) ;

       if( cm_dim[i] != time_colon )
         cellMethods_WhereOver(cm_method[i], var, "where") ;
    }

    cellMethods_Climatology(cm_dim, cm_method, var) ;

    // add wordLists separately
    std::vector<std::string> cm_list;

    if( var.attValue[jx][0].size() == 0 )
    {
      for( size_t i=0 ; i < cm_dim.size() ; ++i )
      {
        cm_list.push_back( cm_dim[i] + " " );
        cm_list.back() += cm_method[i] ;
        if( cm_comment[i].size() )
          cm_list.back() += " " + cm_comment[i] ;

        var.attValue[jx][0] += cm_list[i]  ;
      }
    }

    if( followRecommendations )
    {
      rc_chap7_3b(var, cm_dim);
      rc_chap7_3c(var, cm_dim, cm_method);
    }
  }

  return;
}

bool
CF::chap7_3_4(Variable& var, std::string& name)
{
   // check special cases: area and standard_name instead of
   // coordinate variable or scalar coordinate variable.
   // Return true if such a case is given.

   // rule: name must not be the name of a dimension or a scalar variable
   std::vector<std::string> vs( pIn->nc.getDimNames() );

   for( size_t i=0 ; i < pIn->varSz ; ++i )
     if( pIn->variable[i].isScalar )
        vs.push_back( pIn->variable[i].name );

   for( size_t i=0 ; i < vs.size() ; ++i )
   {
     if( name == vs[i] )
        return false;

     if( name == "area" )
     {
       bool isX=false;
       bool isY=false;

       size_t jx;
       for( jx=0 ; jx < pIn->varSz ; ++jx )
       {
          if( pIn->variable[jx].coord.isX )
             isX=true;
          else if( pIn->variable[jx].coord.isY )
             isY=true;
       }

       if( !(isX && isY) )
       {
         if( notes->inq(bKey + "734a", var.name) )
         {
           std::string capt("if " + var.name);
           capt += ":" + n_cell_methods ;
           capt += " with name=area:, then X-Y-axes variables with dim=1 are recommended" ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }

       return true;
     }

     if( name == n_longitude || name == n_latitude )
     {
       int j;
       for( size_t i=0 ; i < pIn->varSz ; ++i )
       {
         if( (j=pIn->variable[i].getAttIndex(n_standard_name)) > -1 )
         {
           if( pIn->variable[i].attValue[j][0] == n_longitude
                 || pIn->variable[i].attValue[j][0] == n_latitude )
              return true;
         }
         else
         {
           if( notes->inq(bKey + "734b", var.name) )
           {
             std::string capt("!" + var.name);
             capt += ":" + n_cell_methods ;
             capt += " with name=" + name;
             capt += " is neither variable nor supplied " + n_standard_name ;
             capt += "=" + n_longitude +"|" + n_latitude +" of any variable";

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }
       }
     }
   }

   return false;
}

void
CF::chap7_4(void)
{
  if( !isCheck )
    return ;

  // climatological statistics. Try for multiple occurrences
  for( size_t ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix];

    // only time may have a climatology att
    if( var.isClimatology && var.name != timeName )
    {
      if( notes->inq(bKey + "74f", var.name) )
      {
        notes->eraseMapItem(bKey + "71a","",true); // any name

        std::string capt("variable=" + var.name);
        capt += " must not have a " + n_climatology + " attribute" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }
  }

  if( timeIx == -1 )
     return;  // no time axis, hence no climatology

  Variable& timeVar = pIn->variable[timeIx];

  if( ! timeVar.isClimatology )
     return;

  // existance of the climatology variable, i.e. the bounds
  int cIx;

  if( pIn->variable[timeIx].bounds.size() )
  {
    // time.bounds have been set previously
    if( (cIx=pIn->getVarIndex(pIn->variable[timeIx].bounds)) == -1 )
    {
      std::string key( );
      if( notes->inq(bKey + "74g", timeVar.name) )
      {
        std::string capt("!" + timeVar.name + ":");
        capt += n_climatology + " has to declare an existing variable" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }

      return;
    }
  }

  // check dimensions of the climatology_bounds
  Variable& climVar = pIn->variable[cIx];

  // a scalar variable has no dims, which is equivalent to a single dim of size=1
  size_t effTimeDimNameSz(timeVar.dimName.size());
  if( timeVar.dimName.size() == 0 )
     ++effTimeDimNameSz;

  bool isClim=true;
  if( (effTimeDimNameSz + 1) == climVar.dimName.size() )
  {
     std::string &lastDim = climVar.dimName[climVar.dimName.size()-1] ;
     int val = pIn->nc.getDimSize( lastDim );
     if( val == 2 )
       isClim=false;
  }

  if( isClim )
  {
    if( notes->inq(bKey + "74a", climVar.name) )
    {
      std::string capt("climatological bounds require an additional dimension=2") ;
      std::string text("dimensions of bounds=");
      if( climVar.dimName.size() )
        text += climVar.dimName[0] ;
      for( size_t i=1 ; i < climVar.dimName.size() ; ++i )
      {
        text += " " ;
        text += climVar.dimName[i] ;
      }

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  // When there is any climatology, then at least one variable must have
  // a corresponding cell_methods attribute.
  size_t cmIx;
  for( cmIx=0 ; cmIx < pIn->varSz ; ++cmIx )
  {
    if( pIn->variable[cmIx].isValidAtt(n_climatology) )
    {
      for( cmIx=0 ; cmIx < pIn->varSz ; ++cmIx )
        if( pIn->variable[cmIx].isValidAtt(n_cell_methods) )
          break;

      if( cmIx < pIn->varSz )
        break;
    }
  }

  if( cmIx == pIn->varSz )
  {
    if( notes->inq(bKey + "74b", timeVar.name) )
    {
      std::string capt("!" + timeVar.name) ;
      capt += ":" + n_climatology + " has to coincide with attribute=" + n_cell_methods;
      capt += " of a data variable" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  // a climatology variable must not have _FillValue and missing_value
  // checked elsewhere

  // a climatology variable must be a numeric type
  if( climVar.type == NC_CHAR )
  {
    if( notes->inq(bKey + "74i", climVar.name) )
    {
      std::string capt("!" + climVar.name) ;
      capt += " has to be a numeric type";

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
}

void
CF::chap8(void)
{
  // packed data
  if( ! notes->inq(bKey + "8", "", "INQ_ONLY" ) )
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

  nc_type var_type;

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
      attType[i] = pIn->nc.getAttType(attStr[i], var.name) ;
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
        std::string capt("!" + var.name +":") ;
        capt += attStr[0];
        capt += " and ";
        capt += var.name + ":";
        capt += attStr[1];
        capt += " have to be the same type";

        std::string text(var.name +":");
        text += attStr[0] ;
        text += " type=";
        text += pIn->nc.getAttTypeStr(attStr[0], var.name);
        text += "\n";
        text += var.name + ":";
        text += attStr[1] +" type=";
        text += pIn->nc.getAttTypeStr(attStr[1], var.name);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }

      return;
    }
  }

  var_type = pIn->nc.getVarType(var.name) ;

  // var and att of different type
  if( (attBool[att_ix] && (attType[att_ix] != var_type) ) )
  {
    // the variable must be a particular type
    if( ! (var_type == NC_INT || var_type == NC_BYTE || var_type == NC_SHORT) )
    {
      if( notes->inq(bKey + "81b", var.name) )
      {
        std::string capt("type of variable=" + var.name);
        capt += " is different from scale_factor|add_offset," ;
        capt += var.name + " has to be int, short or byte" ;

        std::string text(var.name + ":");
        if( attBool[0] )
        {
          text += attStr[0] + " type=";
          text += pIn->nc.getAttTypeStr(attStr[0], var.name);
        }
        if( attBool[0] && attBool[1] )
        {
          text += ", " ;
          text += var.name + ":";
        }
        if( attBool[1] )
        {
          text += attStr[1] +" type=";
          text += pIn->nc.getAttTypeStr(attStr[1], var.name);
        }
        text += "\n";
        text += var.name;
        text += " type=";
        text += pIn->nc.getVarTypeStr(var.name);

        (void) notes->operate(capt, text) ;
        notes->setCheckCF_Str( fail );
      }
    }

    // var and att still of different type
    if( !( (attType[att_ix] == NC_FLOAT || attType[att_ix] == NC_DOUBLE ) ) )
    {
       if( notes->inq(bKey + "81c", var.name) )
       {
         std::string capt("For different types between variable:" + var.name);
         capt += " and packing attributes " ;
         capt += "type of " + attStr[0] + " and " + attStr[1] ;
         capt += " have to be double of float";

         std::string text("Found: " + var.name + ":");
         if( attBool[0] )
         {
           text += attStr[0] + " type=";
           text += pIn->nc.getAttTypeStr(attStr[0], var.name);
         }
         if( attBool[0] && attBool[1] )
         {
           text += ", " ;
           text += var.name + ":";
         }
         if( attBool[1] )
         {
           text += attStr[1] +" type=";
           text += pIn->nc.getAttTypeStr(attStr[1], var.name);
         }
         text += "\n";
         text += var.name;
         text += " type=";
         text += pIn->nc.getVarTypeStr(var.name);

         (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }

  // recommendation
  // if scale_factor and add_offset are type float, the variable should not be int
  if( attType[0] == NC_FLOAT && attType[1] == NC_FLOAT
         && var_type == NC_INT )
  {
     if( notes->inq(bKey + "81d", var.name) )
     {
       std::string capt("if scale_factor and add_offset are float,");
       capt += "then the variable should not be int" ;

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
           && pIn->nc.getAttType(n_valid_range, var.name) != var_type )
      isNotType[0] = true;
    if( (j=var.getAttIndex(n_valid_min)) > -1
           && pIn->nc.getAttType(n_valid_min, var.name) != var_type )
      isNotType[1] = true;
    if( (j=var.getAttIndex(n_valid_max)) > -1
           && pIn->nc.getAttType(n_valid_max, var.name) != var_type )
      isNotType[2] = true;

    if( isNotType[0] || isNotType[1] || isNotType[2] )
    {
      if( notes->inq(bKey + "81e", var.name) )
      {
        std::string capt("types of variable=" + var.name + " and attribute");
        if( (isNotType[0] && isNotType[1])
              || (isNotType[0] && isNotType[2])
                 || (isNotType[1] && isNotType[2]) )
           capt += "s";
        capt += " ";

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

  std::string compress=("compress");
  std::string compressVal;
  Split x_cmp;

  // any evidence for a compressed-index variable? Tests also for int-type
  bool isIntType=false;
  bool isEvidence=isCompressEvidence(var, &isIntType);
  compressVal = var.getAttValue(compress, lowerCase);
  x_cmp = compressVal;

  if( var.isCompress && isEvidence )
      var.isDataVar=false ;
  else if( var.isCompress  )
  {
    if(isCheck && !isIntType)
    {
      if( notes->inq(bKey + "82b", var.name) )
      {
        std::string capt("!" + var.name + ":") ;
        capt += compress + " is specified, but the data are not of index-type";

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
      }
    }

    if( compressVal.size() )
    {
      for( size_t j=0 ; j < x_cmp.size() ; ++j )
      {
        for( size_t jx=0 ; jx < sz ; ++jx )
           if( pIn->variable[jx].name == x_cmp[j] )
              pIn->variable[jx].isDataVar=false ;

        if( isCheck )
        {
          // compress must only declare dims
          std::vector<std::string> dims( pIn->nc.getDimNames() );

          bool is=true;
          for( size_t k=0 ; k < dims.size() ; ++k )
          {
            if( dims[k] == x_cmp[j] )
            {
               is=false;
               break;
            }
          }

          if( is )
          {
            if( notes->inq(bKey + "83c", var.name) )
            {
              std::string capt("!" + var.name) ;
              capt += ":compress has to declare only dimensions" ;

              (void) notes->operate(capt) ;
              notes->setCheckCF_Str( fail );
            }

            continue;  // skip range test below
          }
        }
      }
    }
  }
  else if( isCheck && isEvidence )
  {
     if( ! ( cFVal > 15
         && pIn->variable[pIn->varSz].isValidAtt("featureType") ) )
     {
       // Note: CF-1.6 allows scalar descriptive variables not in compress context
       if( notes->inq(bKey + "82a", var.name) )
       {
         std::string capt(cFVersion + ": " + var.name) ;
         capt += " is estimated as compressed-index variable, ";
         capt += "but attribute=compress is missing";

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }

  // test for index-range under-|overflow
  if( compressVal.size() )
  {
    // get dimensions
    std::vector<int> vDims;
    for( size_t j=0 ; j < x_cmp.size() ; ++j )
       vDims.push_back( pIn->nc.getDimSize(x_cmp[j]) );

    int d_max=1;
    for(size_t i=0 ; i < vDims.size() ; ++i )
       d_max *= vDims[i] ;

    //get compressed indexes
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
         if( notes->inq(bKey + "82d", var.name) )
         {
           std::string capt("warning for FORTAN: compressed indexes values of ") ;
           capt += var.name + " apply C index convention";

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
         if( notes->inq(bKey + "82e", var.name) )
         {
           std::string capt("warning for C/C++: compressed indexes of " + var.name) ;
           capt += " apply FORTRAN index convention";

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
       else
         is=true;
    }

    if(is)
    {
      if( ! notes->findMapItem(bKey+"12a", var.name) && notes->inq(bKey + "82f", var.name) )
      {
        std::string capt("variable=" + var.name) ;
        capt += ": compressed index exceeds valid range, found index=" ;
        capt += hdhC::itoa(ma_max) + " >= ";
        for(size_t i=0 ; i < vDims.size() ; ++i )
        {
           if( i )
             capt += " x " ;
           capt += hdhC::itoa(vDims[i]);
        }

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
  // discrete sampling geometries
  std::vector<std::string> dsg_att;

  dsg_att.push_back("featureType");
  dsg_att.push_back(n_cf_role);
  dsg_att.push_back("instance_dimension");
  dsg_att.push_back("sample_dimension");

  if( cFVal < 16 )
  {
    std::vector<std::string> coll;

    for( size_t i=0 ; i < pIn->variable.size() ; ++i )
    {
      Variable& var = pIn->variable[i] ;
      for( size_t j=0 ; j < dsg_att.size() ; ++j )
      {
         if( var.isValidAtt(dsg_att[j]) )
         {
            size_t k;
            for( k= 0 ; k < coll.size() ; ++k )
               if( dsg_att[j] == coll[k] )
                  break;
            if( coll.size() == k )
               coll.push_back(dsg_att[j]) ;
         }
      }
    }

    if( coll.size() )
    {
       if( notes->inq(bKey + "0a") )
       {
         std::string capt("warning: usage of CF-1.6 specification in CF-1.4") ;
         std::string text("Found: " + coll[0]);
         for( size_t i=1 ; i < coll.size() ; ++i )
            text += ", " + coll[i] ;

         (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
       }
    }

    return;
  }

  size_t cf_role=1;
  size_t instance_dim=2;
  size_t sample_dim=3;

  // checks are performed for case insensitivity.
  std::vector<std::string> vs_validFeatureType;
  vs_validFeatureType.push_back("point");
  vs_validFeatureType.push_back("timeseries");
  vs_validFeatureType.push_back("trajectory");
  vs_validFeatureType.push_back("profile");
  vs_validFeatureType.push_back("timeseriesprofile");
  vs_validFeatureType.push_back("trajectoryprofile");

  std::string global("global attribute");

  // collection of strings of found attributes

  // >CF-1.5: global attribute
  std::vector<std::string> vs_featureType;

  Variable& glob = pIn->variable[pIn->varSz] ;

  // Get the featureType(s). Note that only a single one is expected by CF-1.6
  chap9_featureType(vs_validFeatureType, vs_featureType) ;

  // warnings for attributes cf_role|instance_dimension|sample_dimension
  // which are confusing when global
  for( size_t i=1 ; i < dsg_att.size() ; ++i )
  {
     if( glob.isValidAtt(dsg_att[i]) )
     {
        if( notes->inq(bKey + "9d", global) )
        {
          std::string capt("warning: " + dsg_att[i]) ;
          capt += " is confusing as a ";
          capt += global;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
     }
  }

  // scan variables for occurrences of attribute cf_role attribute,
  // sample_dimension, and instance_dimension
  std::vector<std::pair<int, int> > vp_cf_role_ix;
  std::vector<std::pair<int, int> > vp_sample_dim_ix;
  std::vector<std::pair<int, int> > vp_instance_dim_ix;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    int j=-1;
    if( (j=var.getAttIndex(dsg_att[instance_dim])) > -1 )
      vp_instance_dim_ix.push_back( std::pair<int, int>(static_cast<int>(i),j ) );

    if( (j=var.getAttIndex(dsg_att[sample_dim])) > -1 )
      vp_sample_dim_ix.push_back( std::pair<int, int>(static_cast<int>(i),j ) );
  }

  std::vector<std::string> vs_cr;
  vs_cr.push_back("timeseries_id") ;
  vs_cr.push_back("profile_id") ;
  vs_cr.push_back("trajectory_id") ;

  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    // look for an unknown cf_role value
    Variable& var = pIn->variable[i] ;

    int j=-1;
    if( (j=var.getAttIndex(dsg_att[cf_role])) > -1 )
    {
      vp_cf_role_ix.push_back( std::pair<int, int>(static_cast<int>(i),j ) );

      bool is=true;
      for( size_t l=0 ; l < vs_cr.size() ; ++l )
        if( var.attValue[j][0] == vs_cr[l] )
           is=false;

      if(is)
      {
        if( notes->inq(bKey + "9g", var.name) )
        {
          std::string capt("!" + var.name + n_cf_role ) ;
          capt += " with invalid value=";
          capt += var.attValue[j][0];

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

// todo
return;

  // Go for the specific featureTypes. If not given, try to estimate the right one.
  // Return true if a featureType could be estimated
  std::string estimatedFT;

  if( chap9_point(vs_featureType) )
    estimatedFT = "point" ;
  else if( chap9_timeSeries(vs_featureType) )
    estimatedFT = "timeseries" ;
  else if( chap9_profile(vs_featureType) )
    estimatedFT = "profile" ;
  else if( chap9_trajectory(vs_featureType) )
    estimatedFT = "trajectory" ;
  else if( chap9_timeSeriesProfile(vs_featureType) )
    estimatedFT = "timeseriesprofile" ;
  else if( chap9_trajectoryProfile(vs_featureType) )
    estimatedFT = "trajectoryprofile" ;
  else
    return;

  if( vs_featureType.size())
  {
     // Is featureType set corresponding to the layout?
     if( vs_featureType[0] != estimatedFT )
     {
       if( notes->inq(bKey + "9e", "global") )
       {
         std::string capt("suspicion that ");
         capt += dsg_att[0] + "=" + vs_featureType[0] ;
         capt += " does not match the estimated value=" + estimatedFT ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }
  else
  {
     // Is featureType missing?
     if( notes->inq(bKey + "9j", "global") )
     {
       std::string capt("suspicion of missing ");
       capt += dsg_att[0] + " with estimated value=" + estimatedFT ;

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }

     return;
  }

  // are cf_role and featureType consistent?
  if( vp_cf_role_ix.size() == 1 )
  {
     std::string str = hdhC::Lower()(
          pIn->variable[vp_cf_role_ix[0].first].attValue[vp_cf_role_ix[0].second][0] ) ;

     if( str.substr(0, str.size() - 3) != vs_featureType[0] )
     {
       if( notes->inq(bKey + "9k", "global") )
       {
         std::string capt("mismatch between " );
         capt += dsg_att[0] + "=" + vs_featureType[0] ;
         capt += " and " + n_cf_role + "=" + str ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
     }
  }

  return;
}

void
CF::chap9_featureType(
  std::vector<std::string> &vs_validFeatureType,
  std::vector<std::string> &vs_featureType)
{
  std::string fT("featureType");
  std::string global_attribute("global attribute");

  Variable& glob = pIn->variable[pIn->varSz] ;

     // for checking misspelling by blanks
  std::string combinedFeatureType;
  std::string str;

  int att_ix;
  if( (att_ix=glob.getAttIndex(fT)) > -1 )
  {
     // check for these invalid cases:
     // 1) misspelling by separation
     // 2) CF-v1.6: multiple featureTypes
     Split x_str;

     for(size_t i=0 ; i < glob.attValue[att_ix].size() ; ++i )
     {
        str = hdhC::Lower()(glob.attValue[att_ix][i]) ;

        // multiples separated by blanks?
        if( str.find(' ') < std::string::npos )
        {
           x_str = str;

           // reassemble
           str = x_str[0];
           for( size_t l=1 ; l < x_str.size() ; ++l)
              str += x_str[l] ;

           for( size_t j=0 ; j < vs_validFeatureType.size() ; ++j)
           {
              if( vs_validFeatureType[j] == str )
              {
                 combinedFeatureType = str;
                 break;
              }
           }

           // check multiple
           for(size_t j=0 ; j < x_str.size() ; ++j )
              vs_featureType.push_back(x_str[j]);
        }
        else
          vs_featureType.push_back(str);
     }
  }

  if( cFVal == 16 && vs_featureType.size() > 1 )
  {
    if( notes->inq(bKey + "9a", "global") )
    {
      std::string capt("CF-1.6 expects only a single value for " + global_attribute) ;
      capt += " " + fT ;

      std::string text("Found:");
      for( size_t i=0 ; i < vs_featureType.size() ; ++i )
      {
        text += ' ';
        text += vs_featureType[i] ;
       }

       if(combinedFeatureType.size())
       {
          text += "\n Note that clearing of blanks would amend the featureType";
          text += "\n to a valid one. Continue checks with that." ;

          vs_featureType.clear();
          vs_featureType.push_back( combinedFeatureType );
       }

       (void) notes->operate(capt, text) ;
       notes->setCheckCF_Str( fail );
    }
  }

  for( size_t j=0 ; j < vs_featureType.size() ; ++j)
  {
    bool is=true;
    for( size_t i=0 ; i < vs_validFeatureType.size() ; ++i)
    {
      if( vs_validFeatureType[i] == vs_featureType[j] )
      {
         is=false;
         break;
      }
    }

    if(is)
    {
      if( notes->inq(bKey + "9f", "global") )
      {
        std::string capt("invalid " + global_attribute ) ;
        capt += " " + fT + "=" ;
        capt += vs_featureType[j] ;

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
    str = var.getAttValue(fT, lowerCase) ;
    if( str.size() )
    {
       if( notes->inq(bKey + "9c", var.name) )
       {
         std::string capt("warning: " + fT) ;
         capt += " is confusing as attribute of variable=";
         capt += var.name;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }

    int j;
    if( (j=var.getAttIndex(n_cf_role)) > -1 )
      cf_role_ix.push_back( std::pair<size_t, int>(i, j) ) ;

    if( (j=var.getAttIndex("instance_dimension")) > -1 )
      instance_dim_ix.push_back( std::pair<size_t, int>(i, j) ) ;

    if( (j=var.getAttIndex("sample_dimension")) > -1 )
      sample_dim_ix.push_back( std::pair<size_t, int>(i, j) ) ;
  }

  // it is recommended to have attribute cf_role included for each label variable
  if( vs_featureType.size() )
  {
    for( size_t i=0 ; i < pIn->varSz ; ++i )
    {
      Variable& var = pIn->variable[i] ;

      if( var.isLabel && ! var.isValidAtt(n_cf_role) )
      {
        if( notes->inq(bKey + "9i", var.name) )
        {
          std::string capt("recommendation: attribute " + n_cf_role) ;
          capt += " should be included for variable=";
          capt += var.name;

          (void) notes->operate(capt) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  return ;
}

bool
CF::chap9_point(std::vector<std::string> &vs_featureType)
{
  std::vector<size_t> var_ix;
  int x_ix;
  int y_ix;
  int t_ix;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );
  }

  // all with a single, identical dimension
  bool is=false;
  std::string dName;

  if( var_ix.size() && pIn->variable[var_ix[0]].dimName.size() == 1 )
  {
    dName = pIn->variable[var_ix[0]].dimName[0] ;
    is=true;
    for( size_t i=1 ; i < var_ix.size() ; ++i )
    {
       if( !(pIn->variable[var_ix[i]].dimName.size() == 1
              && dName == pIn->variable[var_ix[i]].dimName[0]) )
       {
          is=false;
          break;
       }
    }
  }

  // cross-check with geo-located variables and time
  if( is )
  {
     if( x_ix > -1 )
        if( !( pIn->variable[x_ix].dimName.size() == 1
                  &&  pIn->variable[x_ix].dimName[0] == dName ) )
       is=false;

     if( y_ix > -1 )
        if( !( pIn->variable[y_ix].dimName.size() == 1
                  &&  pIn->variable[y_ix].dimName[0] == dName ) )
       is=false;

     if( t_ix > -1 )
        if( !( pIn->variable[t_ix].dimName.size() == 1
                  &&  pIn->variable[t_ix].dimName[0] == dName ) )
       is=false;
  }

  return is;
}

bool
CF::chap9_timeSeries(std::vector<std::string> &vs_featureType)
{
  if( timeIx == -1 )
     return false;

  std::vector<size_t> var_ix;
  int x_ix = -1 ;
  int y_ix = -1 ;
  int t_ix = -1 ;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );
  }

  bool is=true;
  std::string geoDim;

  // different cases, but
  // a) all with identical (also none) dimensions of the geo-located variables
  if( x_ix > -1 && y_ix > -1 )
  {
    size_t sz = pIn->variable[x_ix].dimName.size() ;
    if( sz < 2 && sz == pIn->variable[y_ix].dimName.size() )
    {
      size_t i;
      for( i=0 ; i < sz ; ++i )
        if( pIn->variable[x_ix].dimName[i] != pIn->variable[y_ix].dimName[i])
          break;

      if( i < sz )
        is=false;
      else if( sz )
        geoDim = pIn->variable[x_ix].dimName[0] ;
    }
  }
  else
    is=false;

  // b) dims of time must be different, at least the number
  if( is )
  {
     if( t_ix > -1 )
     {
       if( pIn->variable[t_ix].dimName.size() == 1 )
       {
          if( geoDim == pIn->variable[t_ix].dimName[0] )
             is=false;
       }
       else if( pIn->variable[t_ix].dimName.size() == 2 )
       {
          if( geoDim != pIn->variable[t_ix].dimName[0] )
             is=false;  // the other one doesn't matter
       }
     }
  }

  // c) data variable vs. time variable
  if( is && t_ix > -1)
  {
    Variable& tVar = pIn->variable[t_ix] ;

    bool is2=true;
    for( size_t i=0 ; i < var_ix.size() ; ++i )
    {
      Variable& var = pIn->variable[var_ix[i]] ;

      //    i) dims are the same

      if( var.dimName.size() == tVar.dimName.size() )
      {
        size_t j;
        for( j=0 ; j < var.dimName.size() ; ++j )
          if( var.dimName[j] != tVar.dimName[j] )
             break;
        if( j < var.dimName.size() )
           is2=false;
      }

      //   ii) the only dimensions of time is shared by the data variables
      else
      {
         if( tVar.dimName.size() != 1 )
           is2=false;
         else
         {
           size_t j;
           for( j=0 ; j < var.dimName.size() ; ++j )
             if( var.dimName[j] == tVar.dimName[0] )
                break;
           if( j == var.dimName.size() )
              is2=false;
         }
      }

      if( !is2 )
      {
         is=false;
         break;
      }
    }
  }

  return is;
}

bool
CF::chap9_profile(std::vector<std::string> &vs_featureType)
{
/*
  std::vector<size_t> var_ix;
  int x_ix = -1 ;
  int y_ix = -1 ;
  int z_ix = -1 ;
  int t_ix = -1 ;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isZ )
       z_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );

  }
*/

  return false;
}

bool
CF::chap9_trajectory(std::vector<std::string> &vs_featureType)
{
/*
  std::vector<size_t> var_ix;
  int x_ix = -1 ;
  int y_ix = -1 ;
  int z_ix = -1 ;
  int t_ix = -1 ;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isZ )
       z_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );

  }
*/

  return false;
}

bool
CF::chap9_timeSeriesProfile(std::vector<std::string> &vs_featureType)
{
/*
  std::vector<size_t> var_ix;
  int x_ix = -1 ;
  int y_ix = -1 ;
  int z_ix = -1 ;
  int t_ix = -1 ;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isZ )
       z_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );

  }
*/

  return false;
}

bool
CF::chap9_trajectoryProfile(std::vector<std::string> &vs_featureType)
{
/*
  std::vector<size_t> var_ix;
  int x_ix = -1 ;
  int y_ix = -1 ;
  int z_ix = -1 ;
  int t_ix;

  for(size_t i=0 ; i < pIn->varSz ; ++i )
  {
    if( pIn->variable[i].coord.isX )
       x_ix = i ;
    if( pIn->variable[i].coord.isY )
       y_ix = i ;
    if( pIn->variable[i].coord.isZ )
       z_ix = i ;
    if( pIn->variable[i].coord.isT )
       t_ix = i ;

    if( pIn->variable[i].isDataVar )
       var_ix.push_back( i );
  }
*/

  return false;
}

void
CF::rc_chap(void)
{
   rc_chap2();
   rc_chap3();
   rc_chap4();
   rc_chap5();

   return ;
}

void
CF::rc_chap2(void)
{
   rc_chap2_3();    // naming
   rc_chap2_4();    // dimensions
   rc_chap2_6_2();  // title and history

   return;
}

void
CF::rc_chap2_3(void)
{
   // no two dimensions, variables, nor attributes
   // should be identical when case is ignored

   std::string case_ins;
   std::string case_sens;
   std::string type[3] ;  // 0: dim, 1: var, 2:att
   std::vector<std::string> occ;

   // dimensions
   std::vector<std::string> dims(pIn->nc.getDimNames());
   size_t sz=dims.size();

   for( size_t i=0 ; i < sz ; ++i )
   {
     case_ins = hdhC::Lower()(dims[i]);

     for( size_t j=i+1 ; j < sz ; ++j )
     {
       case_sens = hdhC::Lower()(dims[j]);

       if( case_sens == case_ins )
       {
          type[0] = "dimension" ;
          occ.push_back(dims[i] + " vs. " + dims[j]);
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
          type[1] = "variable" ;
          occ.push_back(var.name + " vs. " + pIn->variable[j].name);
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
            type[2] = "attribute" ;
            occ.push_back(var.attName[ai] + " vs. " + var.attName[aj]);
         }
       }
     }
   }

   for( size_t i=0 ; i < 3 ; ++i )
   {
     if( type[i].size() )
     {
       std::string tag(bKey + "23b");
       if( ! notes->findMapItem(tag, type[i]) && notes->inq(tag, type[i] ) )
       {
         std::string capt("CF recommends to avoid same " );
         capt += type[i] ;
         capt += " names when case is ignored ";

         std::string text("Found: " + occ[0]);
         for( size_t j=1 ; j < occ.size() ; ++j )
            text += ", " + occ[j];

         (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
       }
     }
   }

   return;
}

void
CF::rc_chap2_4(void)
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

//      any_ixMax=0;
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
//           else
//             any_ixMax=i;
         }
      }

      // the sequence of found indexes must ascend
      for(size_t j=1 ; j < dIx.size() ; ++j)
      {
        if( dIx[j-1] > dIx[j] )
        {
          std::string tag(bKey + "24b");
          if( ! notes->findMapItem(tag, var.name) && notes->inq(tag, var.name) )
          {
            std::string capt("recommendation: the order of dimensions of a variable should be T,Z,Y,X") ;
            std::string text("found ");
            text += var.getDimNameStr(true);

            (void) notes->operate(capt, text) ;
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
          std::string capt("recommendation: ");
          capt += var.name + ": other dimensions than T,Z,Y,X should be on the left" ;
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
CF::rc_chap2_6_2(void)
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

     if( var.name == "NC_GLOBAL")
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
     {
       if( p[k] )
       {
          if( notes->inq(bKey + "3a", var.name) )
          {
            std::string capt("warning: attribute ");
            capt += *(p[k]) + " is not global but per variable";

            std::string text("Found: ");
            text += var.name + ":";
            text += *(p[k]) ;

            (void) notes->operate(capt, text) ;
            notes->setCheckCF_Str( fail );
          }

          p[k]=0;
       }
     }
  }

  return;
}

void
CF::rc_chap3(void)
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
        if( ! var.isValidAtt("grid_mapping_name") )
        {
          if( notes->inq(bKey + "32", var.name) )
          {
            std::string capt
            ("recommendation: use standard_name and/or long_name for variable=");
            capt += var.name ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
     }
  }

  rc_chap3_5() ;  // flags
  return;
}

bool
CF::chap3_1_inqDeprecatedUnits(Variable& var, std::string &units)
{
   // units level, layer and sigma_level are deprecated in CF < 1.6

   if( cFVal < 16 )
   {
     if( units == "level" || units == "layer" || units == "sigma_level" )
     {
       if( followRecommendations  )
       {
         std::string tag(bKey+"31a");
         if( ! notes->findMapItem(tag, var.name) && notes->inq(tag, var.name) )
         {
           std::string capt
           ("recommendation: " + var.name + ":" + n_units );
           capt += "=" + units + " is deprecated" ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }

       return true;
     }
   }

   return false;
}

void
CF::rc_chap3_5(void)
{
   // flags
   // when flag_masks and flag_values are both defined, the boolean AND
   // of each entry in flag_values with its corresponding entry in flag_masks
   // should equal the flag_entry vallue

   for( size_t i=0 ; i < pIn->varSz ; ++i )
   {
     Variable& var = pIn->variable[i];

     size_t m_ix, v_ix;

     if( (v_ix=var.isValidAtt("flag_values"))
           && (m_ix=var.isValidAtt("flag_masks")) )
     {

        std::vector<unsigned int> v_uc;
        pIn->nc.getAttValues(v_uc, "flag_values", var.name );

        std::vector<unsigned int> m_uc;
        pIn->nc.getAttValues(m_uc, "flag_masks", var.name );

        if( m_uc.size() == v_uc.size() )
        {
           unsigned int res;
           for( size_t j=0 ; j < m_uc.size() ; ++j )
           {
              res = v_uc[j] & m_uc[j] ;
              if( res != v_uc[j] )
              {
                 if( notes->inq(bKey + "35h", var.name) )
                 {
                    std::string capt
                    ("recommendation: the boolean AND of each flag_values and flag_masks ");
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
CF::rc_chap4(void)
{
   // rc_chap4_4a()   called from chap4_4()
   rc_chap4_4b();    // year and month

   return;
}

void
CF::rc_chap4_4a(Variable& var)
{
   // having called this method means that a ref-date 0-1-1 was found

   for( size_t i=0 ; i < var.attName.size() ; ++i )
   {
      if( var.attName[i] == "calendar" )
      {
         if( hdhC::Upper()(var.attValue[i][0]) == "NONE" )
           return;
         else
         {
           if( notes->inq(bKey + "44j", var.name) )
           {
             std::string capt("reference time in the year 0 to indicate climatological time is deprecated") ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
         }
      }
   }

   return;
}

void
CF::rc_chap4_4b(void)
{
   if( timeIx == -1 )
      return ;

   Split x_units( pIn->variable[timeIx].getAttValue(timeName) );

   if( x_units[0] == "year" || x_units[0] == "month" )
   {
     if( notes->inq(bKey + "44k", timeName) )
     {
       std::string capt("recommendation: units of ") ;
       capt += x_units[0] + " should be used with caution due to the variability" ;

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
   }

   return;
}

void
CF::rc_chap5(void)
{
  size_t ix;

  for( ix=0 ; ix < pIn->varSz ; ++ix )
  {
    Variable& var = pIn->variable[ix] ;

    // the name of multi-coord-var should not match the name of any of
    // its dimensions.
    size_t sz = var.dimName.size();
    if( var.isLabel && sz )
       --sz;

    if( sz > 1 )
    {
      for( size_t i=0 ; i < sz ; ++i )
      {
         if( var.name == var.dimName[i] )
         {
           if( notes->inq(bKey + "5f", var.name) )
           {
             std::string capt("recommendation: variable=") ;
             capt += var.name + " should not match the name of any of its dimensions" ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }
        }
      }
    }

    // the name of a scalar variable should not match the name of any dimension
    else if( var.isScalar || var.isLabel )
    {
       if( pIn->nc.isDimValid(var.name) )
       {
         if( notes->inq(bKey + "57a", var.name) )
         {
           std::string capt("recommendation: scalar variable=") ;
           capt += var.name + " should not match the name of dimension=" ;
           capt += var.name ;

           (void) notes->operate(capt) ;
           notes->setCheckCF_Str( fail );
         }
       }
    }
  }

  return;
}

void
CF::rc_chap7_1(Variable& bvar)
{
  std::vector<std::string> s;
  s.push_back(n_FillValue);
  s.push_back("missing_value");

  for( size_t i=0 ; i < 2 ; ++i )
  {
    if( bvar.isValidAtt(s[i]) )
    {
       if( notes->inq(bKey + "71h", bvar.name) )
       {
         std::string capt("recommendation: ") ;

         int j = pIn->getVarIndex(bvar.boundsOf) ;
         if( pIn->variable[j].isValidAtt(n_bounds) )
           capt += "boundary";
         else
           capt += n_climatology;
         capt += " variable=" + bvar.name + " should not have attribute=" ;
         capt += s[i];

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }
  }

  return;
}

void
CF::rc_chap7_3b(Variable& var, std::vector<std::string> &cm_dim )
{
  // cell_methods

  // data variables (exception of bounds)
  // should have cell_methods attributes

  if( var.boundsOf.size() )
    return;

  if( var.dimName.size() == 0 )
    return;

  if( ! var.isValidAtt(n_cell_methods) )
  {
    if( notes->inq(bKey + "733l", var.name) )
    {
      std::string capt("recommendation: variable ");
      capt += var.name ;
      capt += " should have a " + n_cell_methods +" attribute" ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return;
  }

  std::vector<size_t> missing;
  std::string item;

  // scan over dimensions of a variable and scalar variables
  // supplied by attribute coordinates
  std::vector<std::string> items;

  // dimensions
  size_t jx;
  for( jx=0 ; jx < var.dimName.size() ; ++jx )
    items.push_back( var.dimName[jx] );

  // scalar coordinate variables
  Split x_ca( var.getAttValue(n_coordinates) ) ;
  for( size_t x=0 ; x < x_ca.size() ; ++x )
  {
     int j;
     if( (j=pIn->getVarIndex(x_ca[x])) > -1 )
     {
        if( pIn->variable[j].isScalar && pIn->variable[j].isCoordinate() )
           items.push_back( x_ca[x] );
     }
  }

  for( jx=0 ; jx < item.size() ; ++jx )
  {
    bool is=true;
    for( size_t l=0 ; l < cm_dim.size() ; ++l )
    {
      Split x_cm_dim(cm_dim[l], ": ");

      for( size_t x=0 ; x < x_cm_dim.size() ; ++x )
      {
        if( x_cm_dim[x] == items[jx] || x_cm_dim[x] == "area" )
        {
          is=false;
          break;
        }
      }

      if( is )
      {
        // the current dimension is not listed in cell_methods nor is it a scalar coord var.
        int v ;
        if( (v=pIn->getVarIndex(var.dimName[jx]) > -1 ) )
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
      std::string capt("recommendation: variable ");
      capt += var.name ;
      capt += ":" + n_cell_methods + " should specify an item for each dimension, also for " ;

      std::string text("Found: ") ;
      text += var.getAttValue(n_cell_methods) ;
      text += "\nMissing: " ;

      for( size_t l=0 ; l < missing.size() ; ++l )
      {
         if( l )
         {
           capt += ", " ;
           text += ", " ;
         }

         capt += missing[l] ;
         text += missing[l] ;
      }

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return;
}

void
CF::rc_chap7_3c(Variable& var,
  std::vector<std::string> &cm_dim,
  std::vector<std::string> &cm_method )
{
  // cell_methods
  // except for entries whose cell_method is point, all numeric variables
  // and scalar coordinate vars named by cell_methods should have
  // bounds or climatology attributes.
  std::string dim;

  for( size_t l=0 ; l < cm_dim.size() ; ++l )
  {
    if( cm_method[l].find("point") < std::string::npos )
      continue;

    Split x_cm_dim(cm_dim[l], ": ");  // the dimensions in cell_methods

    for( size_t x=0 ; x < x_cm_dim.size() ; ++x )
    {
      int ix;
      if( (ix=pIn->getVarIndex(x_cm_dim[x])) > -1 )
      {
        Variable& var_cm = pIn->variable[ix];

        if( var_cm.bounds.size() == 0 && !var_cm.isLabel )
        {
          if( notes->inq(bKey + "73h", var.name) )
          {
            std::string capt("recommendation: ") ;
            if( var_cm.isScalar )
               capt += "scalar " ;
            capt += "coordinate variable=";
            capt += var_cm.name + " named by " + var.name + ":" + n_cell_methods;
            capt += " should have bounds or a " + n_climatology + " attribute" ;

            (void) notes->operate(capt) ;
            notes->setCheckCF_Str( fail );
          }
        }
      }
    }
  }

  return;
}

bool
CF::scanStdNameTable(std::vector<struct SN_Entry> &sne)
{
//  if( sne.size() == 0 )  // no standard name
//     return false;

  if( std_name_table.size() == 0 )
     return false;

  std::string file(tablePath + "/");

  ReadLine ifs( file + std_name_table ) ;
  ifs.clearSurroundingSpaces();

  if( ! ifs.isOpen() )
    // could not open
    return false ;

  for( size_t i=0 ; i < sne.size() ; ++i )
  {
     bool isCont=false;

     // any identical standard_names in the race?
     for( size_t k=0 ; k < i ; ++k )
     {
       if( sne[i].name == sne[k].name )
       {
         sne[i].std_name  = sne[k].std_name ;
         sne[i].found     = sne[k].found ;
         sne[i].alias     = sne[k].alias;
         sne[i].amip      = sne[k].amip;
         sne[i].grib      = sne[k].grib;
         sne[i].remainder = sne[k].remainder ;
         sne[i].canonical_units = sne[k].canonical_units;
         isCont=true;
         break;
       }
     }

     if( isCont )
       continue;

     Split x_sn_name(sne[i].name) ;

     if( x_sn_name.size() == 1 )
     {
       if( scanStdNameTable(ifs, sne[i]) )
          sne[i].std_name  = sne[i].name ;
     }
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

         struct SN_Entry tmp_sne(s);

         scanStdNameTable(ifs, tmp_sne) ;

         if( tmp_sne.found )
         {
            if( s.size() < sne[i].name.size() )
              sne[i].remainder = sne[i].name.substr(s.size()+1) ;

            sne[i].std_name  = tmp_sne.name ;
            sne[i].found     = tmp_sne.found ;
            sne[i].alias     = tmp_sne.alias;
            sne[i].amip      = tmp_sne.amip;
            sne[i].grib      = tmp_sne.grib;
            sne[i].canonical_units = tmp_sne.canonical_units;

            break;
         }
       }
     }
  }

  ifs.close();

  return true;
}

bool
CF::scanStdNameTable(ReadLine& ifs, struct SN_Entry &sne)
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

      if( x_line[1] == sne.name )
      {
        if( x_line[0] == beg_entry)
        {
           sne.found=true;

           // exploit entry block
           while( ! ifs.getLine(line) )
           {
              if( line.find(end_entry) < std::string::npos )
                break;

              x_line = line ;
              if( x_line.size() > 1 )
              {
                if( x_line[0] == canonical_units )
                  sne.canonical_units = x_line[1] ;
                else if( x_line[0] == n_grib )
                  sne.grib = x_line[1] ;
                else if( x_line[0] == n_amip )
                  sne.amip = x_line[1] ;
              }
           }

           return true;
        }
        else if( line.find(beg_alias) < std::string::npos )
        {
           // try again later with the real entry
           sne.alias=sne.name;
           --countRewinds;

           if( ifs.getLine(line) )  // only the next line
              break; // should not happen

           x_line = line ;
           if( x_line.size() > 1 )
             sne.name=x_line[1] ;
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
     ;
   else if( s.find("1.5") < std::string::npos )
   {
     if( notes->inq(bKey + "261d") )
     {
        std::string capt("CF-1.5 is not implemented,");
        capt += " falling back to CF-1.4" ;

        (void) notes->operate(capt) ;
        notes->setCheckCF_Str( fail );
     }
   }
   else if( s.find("1.6") < std::string::npos )
     cFVal=16;  // fall back
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
     if(! notes->findMapItem(tag) && notes->inq(tag) )
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
  {
    std::string tag(bKey + "431a");
    if( ! notes->findMapItem(tag, var.name) && notes->inq(tag, var.name) )
    {
      std::string capt("!" + var.name + ":units attribute is missing") ;

      (void) notes->operate(capt) ;
      notes->setCheckCF_Str( fail );
    }

    return false;
  }

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
            rc_chap4_4a(var);

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
        std::string capt("invalid " + var.name + ":units=") ;
        capt += var.units;

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
  def_units.push_back( "s" );  // rotated

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
      if( j == 0 )
        coord = "rot" ;
      else if( j < 6 )
      {
        coord = "lat" ;
        if( var.is_ull_Y )
        {
          ++var.coord.indication_Y;
          var.is_ull_Y=false;
        }
      }
      else
      {
        coord = "lon" ;
        if( var.is_ull_X )
        {
          ++var.coord.indication_X;
          var.is_ull_X=false;
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
      if( sn.substr(0,5) == "grid_" )
      {
        if( sn.substr(5,3) == "lon" )
        {
           coord = "rlon" ;
           if( var.is_ull_rotX )
           {
             ++var.coord.indication_X;
             var.coord.isX=true;
             var.is_ull_rotX=false;
           }
        }
        else if( sn.substr(5,3) == "lat" )
        {
           coord = "rlat" ;
           if( var.is_ull_rotY )
           {
             ++var.coord.indication_Y;
             var.coord.isY=true;
             var.is_ull_rotY=false;
           }
        }
      }
    }
  }

  return coord;
}

void
CF::verify_formula_terms(
  Variable& var,
  int found_ft_ix,
  std::string &valid_ft,
  std::vector<std::string> &fTerms,
  std::vector<std::pair<std::string, std::string> >& p_found_ft)
{
  // formula_term contains 'terms:' and names of variables with the omission of
  // particular terms.

  // Vector fTerms contains a list of term: strings.
  // Note that it will contain existing var-names on return.

  std::vector<std::string> valid_units;
  valid_units.push_back("pa 1");
  valid_units.push_back("1 Pa Pa");
  valid_units.push_back("1 Pa 1 Pa Pa");
  valid_units.push_back("m 1 m");
  valid_units.push_back("1 1 1 m m m");
  valid_units.push_back("1 m m");
  valid_units.push_back("1 m m 1 1 m");
  valid_units.push_back("1 m m m 1 m");
  valid_units.push_back("1 m m m 1 m 1");

  Split fUnits(valid_units[found_ft_ix]);

  std::vector<std::string> assoc;
  std::vector<std::pair<std::string, std::string> > paramVarUnits;

  if( isCheck )
  {
    for( size_t j=0 ; j < p_found_ft.size() ; ++j )
    {
       size_t i;
       for( i=0 ; i < pIn->varSz ; ++i )
       {
          // each variable listed in the formula_terms string must have
          // no _FillValue occurrences. Even when dependent on time.
          if( p_found_ft[j].second == pIn->variable[i].name )
          {
             for( size_t k=0 ; k < fTerms.size() ; ++k )
             {
                if( fTerms[k] == p_found_ft[j].first )
                {
                   // check the units of the parameters
                   int iu = pIn->variable[i].getAttIndex(n_units);
                   if( iu == -1 || pIn->variable[i].attValue[iu][0] == "1" )
                   {
                      if( fUnits[k] != "1" && notes->inq(bKey + "432j", var.name) )
                      {
                         std::string capt("!" + var.name + n_formula_terms);
                         capt += ": parameter variable=" + pIn->variable[i].name ;
                         capt += " requires " + n_units + "=" + fUnits[k] ;
                         capt += ", but found dimensionless" ;

                         (void) notes->operate(capt) ;
                         notes->setCheckCF_Str( fail );
                      }
                   }
                   else
                   {
                      if( cmpUnits(pIn->variable[i].attValue[iu][0], fUnits[k]) )
                        paramVarUnits.push_back(
                          std::pair<std::string, std::string>
                          (pIn->variable[i].name, pIn->variable[i].attValue[iu][0]) );
                      else if( ! chap3_1_inqDeprecatedUnits(
                                  pIn->variable[i], pIn->variable[i].attValue[iu][0] ) )
                      {
                         if( notes->inq(bKey + "432i", var.name) )
                         {
                           std::string capt("!" + var.name + ":" + n_formula_terms );
                           capt += ": parameter variable=" + pIn->variable[i].name ;
                           capt += ":" + n_units + "=";
                           capt += pIn->variable[i].attValue[iu][0] ;
                           if( fUnits[k] == "1" )
                             capt += ", but dimensionless is required";
                           else
                             capt += ", but " + fUnits[k] + " is required";

                           (void) notes->operate(capt) ;
                           notes->setCheckCF_Str( fail );
                         }
                      }
                   }
                }
             }

             // variables associated with the dimless var
             assoc.push_back( p_found_ft[j].second ) ;

             //check also time dependent variables
             checkAuxCoordData(pIn->variable[i]) ;

             var.coord.isZ    = true;
             var.coord.isZ_DL = true;
             var.isDataVar=false;
             var.isChecked=true;
             break;
          }

       }

       // any non-existing variable in the formula_terms string?
       if( i == pIn->varSz && notes->inq(bKey + "432d", var.name) )
       {
         std::string capt("!" + var.name + ":");
         capt += n_formula_terms + " declares undefined variable by " ;
         capt += p_found_ft[j].first + " " + p_found_ft[j].second ;

         (void) notes->operate(capt) ;
         notes->setCheckCF_Str( fail );
       }
    }

    // is to be done here after having looped through all fTerms
    if( paramVarUnits.size() > 1 )
    {
       for( size_t k=1 ; k < paramVarUnits.size() ; ++k )
       {
         if( paramVarUnits[k].second != paramVarUnits[0].second )
         {
           if( notes->inq(bKey + "432k", var.name) )
           {
             std::string capt("warning: " + var.name + ":" + n_formula_terms );
             capt += ": parameter variable=" + paramVarUnits[0].first ;
             capt +=  ":" + n_units + "=";
             capt += paramVarUnits[0].second + " is different from " ;
             capt += paramVarUnits[k].first ;
             capt +=  ":" + n_units + "=" + paramVarUnits[k].second ;

             (void) notes->operate(capt) ;
             notes->setCheckCF_Str( fail );
           }

           break;
         }
       }
    }
  }

  // each term in form_terms must refer to an existing variable
  std::vector<size_t> invalidTermIx;

  for(size_t i=0 ; i < p_found_ft.size() ; ++i )
  {
    size_t j;

    for(j=0 ; j < fTerms.size() ; ++j )
      if( fTerms[j] == p_found_ft[i].first )
        break;

    if( j == fTerms.size() )
       invalidTermIx.push_back(j);
  }

  if( isCheck && invalidTermIx.size() )
  {
    // found invalid formula_term terms:
    if( notes->inq(bKey + "432c", var.name) )
    {
      std::string capt("invalid" + var.name + ":" + n_formula_terms ) ;
      capt += " failed by: " + fTerms[invalidTermIx[0]] ;
      for( size_t i=1 ; i < invalidTermIx.size() ; ++i )
         capt += ", " + fTerms[invalidTermIx[i]] ;

      std::string text(n_formula_terms + " (requested)=");
      text += valid_ft;
      text += "\n" + n_formula_terms + " (file)=";
      for( size_t i=0 ; i < p_found_ft.size() ; ++i )
         text += " " + p_found_ft[i].first ;

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  fTerms.clear();
  for( size_t i=0 ; i < assoc.size() ; ++i )
     fTerms.push_back( assoc[i] );

  // fTerms contains now only associated variables.

  return ;
}

bool
CF::xCheckSNvsFT( Variable& var,
   std::vector<std::string>& valid_sn,
   std::vector<std::string>& valid_ft,
   int& found_sn_ix,
   int& found_ft_ix,
   std::vector<std::pair<std::string, std::string> >& p_found_ft)
{
  // cross-check of standard_name vs. formula_terms (case: dimless vertical coord)
  // return true if no formula_term proceeding is possible

  int att_sn_ix = var.getAttIndex(n_standard_name);

  if( att_sn_ix > -1 )
  {
    std::string str( var.getAttValue(n_standard_name, lowerCase) );

    for( size_t j=0 ; j < valid_sn.size() ; ++j )
    {
      if( str == valid_sn[j] )
      {
        found_sn_ix = j ;
        break;
      }
    }
  }

  int att_ft_ix=var.getAttIndex(n_formula_terms) ;

  if( att_ft_ix == -1 )
  {
     // try to identify the attribute by a proper structure, i.e. 2x (term: var-name)
     // hoping that there is only a mistyped name.
     for( size_t j=0 ; j < var.attName.size() ; ++j )
     {
        if( var.attName[j] == n_cell_methods )
           continue;
        if( var.attName[j] == n_cell_measures )
           continue;
        if( var.attName[j] == n_coordinates )
           continue;
        if( var.attName[j] == n_units )
           continue;

        std::string& currAV = var.attValue[j][0];

        if( currAV.find(':') < std::string::npos )
        {
           Split x_v(currAV);
           int count=0;

           for(size_t k=0 ; k < x_v.size() ; ++k )
           {
              if( x_v[k][x_v[k].size()-1] == ':' )
                 continue;

              for( size_t l=0 ; l < pIn->varSz ; ++l)
                 if( x_v[k] == pIn->variable[l].name )
                    ++count;
           }

           if( count > 1 )
           {

             if( notes->inq(bKey + "432e", var.name) )
             {
               std::string capt("!" + var.name + ":" + var.attName[j]);
               capt += " is invalid, expected attribute name=" + n_formula_terms ;

               (void) notes->operate(capt) ;
               notes->setCheckCF_Str( fail );
             }

             att_ft_ix = j;
             break;
           }
        }
     }
  }

  // no formula_terms attribute is given
  if( att_ft_ix == -1)
     return true;  // apparently no hybrid case

  else if( att_sn_ix == -1 && att_ft_ix > -1)
  {
     if( notes->inq(bKey + "432f", var.name) )
     {
       std::string capt("attribute=" + n_formula_terms);
       capt += " indicates a dimensionless vertical coordinate, " ;
       capt +="but standard_name is missing";

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
  }
  else if( att_sn_ix > -1 && found_sn_ix == -1)
  {
     if( notes->inq(bKey + "432g", var.name) )
     {
       std::string capt("Standard_name=" + var.attValue[att_sn_ix][0]);
       capt += " is invalid for a dimensionless vertical coordinate" ;

       (void) notes->operate(capt) ;
       notes->setCheckCF_Str( fail );
     }
  }
  else if( found_sn_ix > -1 && att_ft_ix == -1)
  {
    if( followRecommendations && notes->inq(bKey + "432a", var.name) )
    {
      std::string capt("recommendation: set attribute=" + n_formula_terms );
      std::string text("suggestion of " + n_formula_terms + "=");
      text += valid_ft[found_sn_ix] ;

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  // formula_terms attribute was found, now get the indexes in the valid vectors.
  // This is more tricky for found_ft_ix than for found_sn_ix above.
  // Take into account the term: strings and
  // find the occurrences of existing variables for each valid_ft element.
  std::vector<int> occurrence;
  std::vector<Split> x_valid_ft;

  for( size_t i=0 ; i < valid_ft.size() ; ++i )
  {
     occurrence.push_back( 0 );
     x_valid_ft.push_back( Split(valid_ft[i]) ) ;
  }

  std::string& found_ft = var.attValue[att_ft_ix][0];
  Split x_found_ft(found_ft);

  int sz=x_found_ft.size()-1 ;
  for(int i=0 ; i < sz ; i+=2 )
  {
    std::string &s0 = x_found_ft[i] ;
    std::string &s1 = x_found_ft[i+1] ;
    if( s0[s0.size()-1] == ':' && s1[s1.size()-1] != ':' )
       p_found_ft.push_back( std::pair<std::string, std::string>
                                 (x_found_ft[i],x_found_ft[i+1]) );
    else
    {
       if( (i+1) < (sz+1) )
         --i; // try for another pair shifted
    }
  }

  for( size_t i=0 ; i < p_found_ft.size() ; ++i )
  {
     for( size_t j=0 ; j < valid_ft.size() ; ++j )
     {
       for( size_t k=0 ; k < x_valid_ft[j].size() ; ++k )
       {
         if( x_valid_ft[j][k] == p_found_ft[i].first )
         {
            for( size_t iv=0 ; iv < pIn->varSz ; ++iv )
               if( pIn->variable[iv].name == p_found_ft[i].second )
                 ++occurrence[j];
         }
       }
     }
  }

  // sort indexes according to the number of occurrences
  size_t v_swap;
  std::vector<int> zx;
  sz=static_cast<int>(occurrence.size());
  for( int i=0 ; i < sz ; ++i )
    zx.push_back(i);

  sz=static_cast<int>(zx.size());
  for( int i=0 ; i < sz ; ++i )
  {
    for( int j=i+1 ; j < sz ; ++j )
    {
       if( occurrence[zx[j]] >  occurrence[zx[i]] )
       {
         v_swap = zx[i] ;
         zx[i] = zx[j];
         zx[j] = v_swap;
       }
    }
  }

  bool is=false;
  if( occurrence[zx[0]] && occurrence[zx[0]] == occurrence[zx[1]] )
  {
     // could not identify unambiguously the valid_ft index. Thus, if one
     // of them matches found_sn_ix, then take it.
     if( zx[0] == found_sn_ix )
        found_ft_ix = zx[0] ;
     else if( zx[1] == found_sn_ix )
        found_ft_ix = zx[1] ;
     else
       is=true;
  }
  else
    found_ft_ix = zx[0] ;

  if( found_ft_ix > -1 && found_ft_ix != found_sn_ix)
  {
    if( notes->inq(bKey + "432h", var.name) )
    {
      std::string capt("!" + var.name + ":" + n_standard_name);
      capt += " and " + var.name + ":" + n_formula_terms + " do not match" ;

      std::string text("found: " + var.name + ":" + n_standard_name);
      text += "=" + var.attValue[att_sn_ix][0] + " and " ;
      text += var.name + ":" + n_formula_terms + "=" + var.attValue[att_ft_ix][0] ;

      (void) notes->operate(capt, text) ;
      notes->setCheckCF_Str( fail );
    }
  }

  return is;
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
};

std::vector<std::string> CF::attName(CF_AttName, CF_AttName + 37);
std::vector<char>        CF::attType(CF_AttType, CF_AttType + 37);
