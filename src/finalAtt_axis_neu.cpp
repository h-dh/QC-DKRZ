  std::vector<int> xv_ix; // variable with axis attr
  std::vector<int> xv_jx; // axis-att [-1]

  std::vector<int> acv_ix; // aux coord variable
  std::vector<int> acv_jx; // axis-att [-1]
  std::vector<std::vector<int> >cv_ix;  // corresponding coordinate var(s)
  std::vector<std::vector<int> > cv_jx;

  int vSz = static_cast<int>(pIn->varSz) ;

  for( int i=0 ; i <vSz ; ++i )
  {
    Variable& aux = pIn->variable[i] ;
    int cType;

    int jx ;

    if( (jx=aux.getAttIndex(n_axis)) > -1 )
    {
      xv_ix.push_back(i);
      xv_jx.push_back(jx) ;
    }

    if( (cType=aux.getCoordinateType()) > -1 && ! aux.coord.isCoordVar )
    {
       acv_ix.push_back(i) ;
       acv_jx.push_back(jx) ;
       cv_ix.push_back(std::vector<int>());
       cv_jx.push_back(std::vector<int>()) ;

       // find associated coordinate variables
       for( int j=0 ; j < pIn->varSz ; ++j )
       {
         if( i == j )
            continue;

         Variable& var = pIn->variable[j] ;

         if( var.coord.isCoordVar )
         {
            if( cType == var.getCoordinateType() )
            {
               cv_ix.back().push_back(j);
               cv_jx.back().push_back(var.getAttIndex(n_axis)) ;
            }
         }
       }
    }
  }

  // legal values of axis are T,Z,Y,X (case insensitive)
  for( size_t j=0 ; j < xv_ix.size() ; ++j )
  {
    Variable& var = pIn->variable[xv_ix[j]] ;

    std::string axis(var.attValue[xv_jx[j]][0]);
    std::string l_axis( hdhC::Lower()(axis) );

    if( ! (l_axis == "x" || l_axis == "y" || l_axis == "z" || l_axis == "t")  )
    {
      if( notes->inq(bKey + "4a", var.name) )
      {
        std::string capt("!" + var.name);
        capt += ":axis is not X, Y, Z, or T (case insensitive)" ;

        std::string text("Found ");
        text += var.name + ":axis=" + axis;

        (void) notes->operate(capt, text) ;
         notes->setCheckCF_Str( fail );
      }
    }
  }

  // axis must be consistent with the coordinate variable
  for( size_t j=0 ; j < xv_ix.size() ; ++j )
  {
    Variable& var = pIn->variable[xv_ix[j]] ;

    std::string& axis = var.attValue[xv_jx[j]][0] ;
    std::string l_axis( hdhC::Lower()(axis) );

    if( var.coord.isCoordVar || cFVal > 15 )
    {
      if( l_axis == "t" )
        ++var.coord.indication_T;
      else if( l_axis == "x" )
         ++var.coord.indication_X;
      else if( l_axis == "y" )
        ++var.coord.indication_Y;
      else if( l_axis == "z" )
        ++var.coord.indication_Z;

      // axis must be consistent with the coordinate variable
      std::string capt("found ");

      if( axis != "x" && var.coord.isX )
        capt += var.name +":" + n_axis + "=" + axis + ", expected axis=X";
      else if( axis != "y" && var.coord.isY )
        capt += var.name +":" + n_axis + "=" + axis + ", expected axis=Y";
      else if( axis != "z" && var.coord.isZ )
        capt += var.name +":" + n_axis + "=" + axis + ", expected axis=Z";
      else if( axis != "t" && var.coord.isT )
        capt += var.name +":" + n_axis + "=" + axis + ", expected axis=T";

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

  // no axis att for auxiliary coordinate variables (<CF-1.6)
  if( cFVal < 16 )
  {
    for( size_t j=0 ; j < acv_ix.size() ; ++j )
    {
      if( acv_jx[j] > -1 )
      {
        Variable& aux = pIn->variable[acv_ix[j]] ;

        if( notes->inq(bKey + "5a", aux.name) )
        {
          std::string capt("variable=" + aux.name);
          capt += " was estimated as auxiliary coordinate, ";
          capt += "i.e. attribute:axis is not allowed when " ;
          capt += cFVersion ;

          std::string text("Found ");
          text += aux.name + ":axis=";
          text += aux.attValue[acv_jx[j]][0];

          (void) notes->operate(capt, text) ;
          notes->setCheckCF_Str( fail );
        }
      }
    }
  }

  for( size_t i=0 ; i < acv_ix.size() ; ++i )
  {
    if( acv_jx[i] > - 1 ) // aux with axis
    {
       if( cFVal < 16 )
       {
         Variable& aux = pIn->variable[acv_ix[i]] ;

         if( notes->inq(bKey + "5a", aux.name) )
         {
           std::string capt("variable=" + aux.name);
           capt += " was estimated as auxiliary coordinate, ";
           capt += "i.e. attribute:axis is not allowed when " ;
           capt += cFVersion ;

           std::string text("Found ");
           text += aux.name + ":axis=";
           text += aux.attValue[acv_jx[i]][0];

           (void) notes->operate(capt, text) ;
           notes->setCheckCF_Str( fail );
         }
       }

       else
       {
          if( cv_ix[i].size() )
          {
             for(size_t k=0 ; k < cv_jx[i].size() ; ++k )
             {
                if( cv_jx[i][k] > -1 )
                {
                   Variable& aux = pIn->variable[acv_ix[i]] ;

                   // both auxiliary and pure coordinate variable
                   // are taken into account
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
       }
    }

    Variable& var = pIn->variable[i] ;

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

  char xs[]={'X', 'Y', 'Z', 'T'};

  // relations between data variables and corresponding auxiliary and pure
  // coordinate variables.
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( !var.isDataVar || var.coord.isCoordVar || var.isCoordinate() )
       continue;

    // collection of indexes of all var-reps of the current variable's dims
    // and of existing variables declared in the coordinates attribute.
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
    }
  }

  // coordinate variable should have axis attribute
  for( size_t i=0 ; i < pIn->varSz ; ++i )
  {
    Variable& var = pIn->variable[i] ;

    if( var.coord.isCoordVar )
    {
      if( followRecommendations )
      {
      }
    }

    if( !var.coord.isCoordVar || var.coord.isX || var.coord.isY )
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
  }

