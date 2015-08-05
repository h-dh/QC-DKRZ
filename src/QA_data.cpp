DataOutputBuffer::DataOutputBuffer()
{
  // might be changed with initBuffer()
  maxBufferSize=1500;
  bufferCount=0;

  // pointers
  min=max=ave=std_dev=0;
  fill_count=0;
  checksum=0;

  nc=0;
}

void
DataOutputBuffer::clear()
{
  bufferCount=0;

  if(min)
    delete [] min;

  if(max)
    delete [] max;

  if(ave)
    delete [] ave;

  if(std_dev)
    delete [] std_dev;

  if(fill_count)
    delete [] fill_count;

  if(checksum)
    delete [] checksum;

  min=max=ave=std_dev=0;
  fill_count=0;
  checksum=0;

  return;
}

void
DataOutputBuffer::flush(void)
{
   std::string t;

   if( min )
   {
     t = name + "_min";
     nc->putData(nextFlushBeg, bufferCount, t, min );
   }

   if( max )
   {
     t = name + "_max";
     nc->putData(nextFlushBeg, bufferCount, t, max );
   }

   if( ave )
   {
     t = name + "_ave";
     nc->putData(nextFlushBeg, bufferCount, t, ave );
   }

   if( std_dev )
   {
     t = name + "_std_dev";
     nc->putData(nextFlushBeg, bufferCount, t, std_dev );
   }

   if( fill_count )
   {
     t = name + "_fill_count";
     nc->putData(nextFlushBeg, bufferCount, t, fill_count );
   }

   if( checksum )
   {
     t = name + "_checksum";
     nc->putData(nextFlushBeg, bufferCount, t, checksum );
   }

   nextFlushBeg += bufferCount;
   bufferCount=0;

   return;
}

void
DataOutputBuffer::initBuffer(NcAPI *n, size_t nxt, size_t mx)
{
  nc = n;
  maxBufferSize=mx;
  nextFlushBeg=nxt;

  // for a change on the fly
  if( min )
  {
    if( bufferCount )
      flush();

    clear() ;
  }

  // container for results temporary holding the data
  min=        new double [maxBufferSize] ;
  max=        new double [maxBufferSize] ;
  ave=        new double [maxBufferSize] ;
  std_dev=    new double [maxBufferSize] ;
  fill_count= new int    [maxBufferSize] ;
  checksum=   new uint32_t [maxBufferSize] ;

  bufferCount=0;

  return;
}

void
DataOutputBuffer::store(hdhC::FieldData &fA)
{
   // flush collected qa results to the qa-results netCDF file
   if( bufferCount == maxBufferSize )
     flush();  // resets countTime, too

   if( fA.isValid )
   {
     min[bufferCount]       =fA.min;
     max[bufferCount]       =fA.max;

     ave[bufferCount]       =fA.areaWeightedAve;
     fill_count[bufferCount]=fA.fillingValueNum;

     if( fA.isStndDevValid )
       std_dev[bufferCount]=fA.stndDev;
     else
       std_dev[bufferCount]=1.E+20;

     checksum[bufferCount] = fA.checksum;
   }
   else
   {
     min[bufferCount]       =1.E+20;
     max[bufferCount]       =1.E+20;

     ave[bufferCount]       =1.E+20;
     fill_count[bufferCount]=-1;
     std_dev[bufferCount]   =1.E+20;
     checksum[bufferCount]  =0;
   }

   fill_count[bufferCount]=fA.fillingValueNum;

   ++bufferCount;

   return;
}

SharedRecordFlag::SharedRecordFlag()
{
  buffer=0;
  currFlag=0;
  bufferCount=0;
  maxBufferSize=1500;
}

SharedRecordFlag::~SharedRecordFlag()
{
  if( buffer )
     delete[] buffer;
}

void
SharedRecordFlag::adjustFlag(int num, size_t rec, int erase)
{
  // adjust coded error flags written already to qa_<filename>.nc
  if( nc->getNumOfRecords(name) == 0 )
    return;

  MtrxArr<int> imv;
  nc->getData(imv, name , rec );

  std::vector<int> decomp;

  if( imv[0] > 0 )
  {
    // check whether the identical error flag is already available
    int flags[] = { 0, 1, 2, 4, 8, 16, 32, 64,
                  100, 200, 400, 800, 1600, 3200, 6400};
    int sz = 15;
    int iv=imv[0] ;

    for( int i=sz-1 ; i > 0 ; --i )
    {
       if( iv < flags[i] )
         continue;

       decomp.push_back( flags[i] );
       iv -= flags[i] ;

       if( iv == 0 )
         break;
    }

    for( size_t i=0 ; i < decomp.size() ; ++i )
    {
      if( num == decomp[i] )
      {
        num=0;
        break;
      }
    }
  }

  int iv=num + imv[0] - erase;  // note that erase equals zero by default
  nc->putData(rec, 1, name, &iv );

  return;
}

void
SharedRecordFlag::flush(void)
{
   nc->putData(nextFlushBeg, bufferCount, name, buffer );

   nextFlushBeg += bufferCount;
   bufferCount=0;

   return;
}

void
SharedRecordFlag::initBuffer(NcAPI *n, size_t nxt, size_t mx)
{
  nc = n;
  maxBufferSize=mx;
  nextFlushBeg=nxt;

  if( buffer )
  {
    // for a change on the fly
    if( bufferCount )
      flush();

    delete [] buffer ;
  }

  buffer = new int [maxBufferSize] ;
  bufferCount=0;

  return;
}

void
SharedRecordFlag::store(void)
{
   if( bufferCount == maxBufferSize )
      flush();

   buffer[bufferCount++] = currFlag;

   currFlag=0;
   return;
}


Outlier::Outlier( QA *p, size_t vi, std::string nm)
{
  pQA=p;
  vMDix=vi;
  name=nm;
}

bool
Outlier::isSelected(
     std::vector<std::string> &options,
     std::string &vName,
     bool isQA_enabledPostProc, int effDim )
{
  if( options.size() == 1 && options[0] == "t" )
    return true; // default for all

  bool isThis  =true;
  bool isZeroDim=false;

  // dismember options
  for( size_t k=0 ; k < options.size() ; ++k )
  {
    bool isVar  =false;
    isThis      =true;
    isZeroDim   =false;

    Split cvs(options[k],",");
    for( size_t i=0 ; i < cvs.size() ; ++i )
    {
      if( cvs[i] == "POST" )
      {
        if( ! isQA_enabledPostProc )
         isThis=false ;
      }
      else if( cvs[i] == "no_0-D" && effDim < 1 )
        isZeroDim=true;
      else if( cvs[i] == vName )
        isVar=true;  // valid for this specific variable
    }

    if( isVar )
    {
      // this dedicated variable
      if( isZeroDim )
         return false;

      return isThis ;
    }
  }

  // all variables
  if( isZeroDim )
    return false;

  return isThis ;
}

void
Outlier::parseOption(std::vector<std::string> &opts)
{
  if( opts.size() == 0 )
     return;

  BraceOP groups ;
  std::string s0;

  Split csv;
  csv.setSeparator(',');

  for( size_t k=0 ; k < opts.size() ; ++k )
  {
    s0=opts[k];

    groups.set( s0 );
    while ( groups.next(s0) )
    {
       csv = s0;

       bool isGeneral=true;
       for( size_t l=0 ; l < csv.size() ; ++l )
       {
         s0 = csv[l] ;

         if( s0[0] == 'L' )
           continue;
         else if( s0[0] == 'M' )
           continue;
         else if( s0[0] == 'P' )
           continue;
         else if( s0[0] == 'N' )
           continue;

         isGeneral = false;
       }

       if( isGeneral )
       {
         for( size_t l=0 ; l < csv.size() ; ++l )
           options.push_back( csv[l] ) ;
         continue;
       }

       bool isVarSpecific=false;
       for( size_t l=0 ; l < csv.size() ; ++l )
       {
         if( name == csv[l] )
         {
            isVarSpecific = true;
            break;
         }
       }

       if( isVarSpecific )
       {
         for( size_t l=0 ; l < csv.size() ; ++l )
           options.push_back( csv[l] ) ;

         return;
       }
    }
  }

  return;
}

bool
Outlier::test(QA_Data *pQAD)
{
  // Acknowledgement: Dr. Andreas Chlond, MPI-M Hamburg, suggested to
  //                  exploit the function N(sigma) as criterion.*/

  bool retCode = false;

  if( ! pQA->isCheckData )
    return retCode;

  std::vector<std::string> names;

  if( names.size() == 0 )
    return retCode;  // probably a fixed variable

  names.push_back( name + "_min"  );
  names.push_back( name + "_max"  );

  // get mean and standard deviation
  double ave[2];
  double sigma[2];

  size_t sampleSize=pQAD->statMin.getSampleSize();
  if( sampleSize < 2 )
    return retCode;

  if( ! pQAD->statMin.getSampleAverage( &ave[0] ) )
    return retCode;

  if( ! pQAD->statMin.getSampleStdDev( &sigma[0] ) )
    return retCode;

  if( ! pQAD->statMax.getSampleAverage( &ave[1] ) )
    return retCode;

  if( ! pQAD->statMax.getSampleStdDev( &sigma[1] ) )
    return retCode;

  for( size_t i=0 ; i < 2 ; ++i )
    sigma[i] /= 4.;

  // init outlier options
  size_t outOptL=5;
  size_t outOptM=5;
  size_t outOptN=0;
  double outOptPrcnt=0.;

  for( size_t i=0 ; i < options.size() ; ++i )
  {
     std::string &t0 = options[i] ;

     if( t0[0] == 'L' )
       outOptL = static_cast<size_t>(hdhC::string2Double(t0));
     else if( t0[0] == 'M' )
       outOptM = static_cast<size_t>(hdhC::string2Double(t0));
     else if( t0[0] == 'P' )
       outOptPrcnt = hdhC::string2Double(t0) / 100.;
     else if( t0[0] == 'N' )
       outOptN = static_cast<size_t>(hdhC::string2Double(t0)) ;
  }

  // default: both are specified
  if( outOptN == 0 && outOptPrcnt == 0. )
  {
     outOptN=5;
     outOptPrcnt=0.01;
  }

  // read min / max from the qa-nc file
  std::vector<std::string> extStr;
  extStr.push_back( "minimum" );
  extStr.push_back( "maximum" );
  int errNum[2];
  errNum[0]=400;
  errNum[1]=800;

  std::vector<size_t> outRec;
  std::vector<double> outVal;

  size_t currRecEnd=pQA->nc->getNumOfRecords();
//  size_t subTempRecs=pQA->pIn->nc.getNumOfRecords();
  size_t prevRecEnd ;

//  if( pQA->enabledPostProc )
    prevRecEnd=0  ;  //forces re-reading of all
//  else
//    prevRecEnd = currRecEnd - subTempRecs ;

  size_t currRecNum = currRecEnd - prevRecEnd ;

  // adjustment of outOptPrcnt: the effective percentage must be related
  // to the total number of data points in a way that 10 outlier must
  // be possible in principle.
  if( outOptPrcnt > 0. )
  {
    while ( static_cast<double>(currRecNum) * outOptPrcnt < .1 )
     outOptPrcnt *= 10. ;
  }

  double extr[currRecNum];

  MtrxArr<double> mv;
  size_t N[100]; // counter for exceeding extreme values

  for( size_t i=0 ; i < 2 ; ++i )
  {
    outRec.clear();
    outVal.clear();

    bool isOut = false;
    size_t rec=0;
    for( size_t j=prevRecEnd ; j < currRecEnd ; ++j )
    {
       pQA->nc->getData(mv, names[i] , j );
       extr[rec++] = mv[0] ;
    }

    // find number of extreme values exceeding N*sigma
    size_t n=0;
    size_t countConst=1;
    double n_sigma;
    size_t nLimit = 100;

    do
    {
      sigma[i] *= 2.;

      for(  n=0 ; n < nLimit ; ++n )
      {
        n_sigma = sigma[i] + static_cast<double>(n) * sigma[i];
        N[n]=0;

        outRec.clear();
        outVal.clear();

        for( size_t j=0 ; j < currRecNum ; ++j )
        {
          if( fabs(extr[j] - ave[i]) > n_sigma )
          {
            ++N[n] ;
            outRec.push_back( prevRecEnd + j );
            outVal.push_back( extr[j] );
          }
        }

        // no detection
        if( N[n] == 0 )
          break;

        if( N[n] == N[n-1] )
          ++countConst ;
        else
          countConst=1 ;

        if( countConst == outOptM )
        {
          isOut = true;
          break;
        }
      }
    } while( n == nLimit );

    // if there are too many peaks/breaks, then consider this as regular.
    // default: > 1%
    if( isOut )
    {
      // if only one of the two is specified or both
      bool isTooManyA ;
      bool isTooManyB ;

      if( outOptPrcnt > 0. )
      {
        double a =
           static_cast<double>(N[n]) / static_cast<double>(currRecEnd) ;
        isTooManyA =  a > outOptPrcnt ;
      }
      else
        isTooManyA = false;

      if( outOptN )
        isTooManyB = outRec.size() > outOptN ;
      else
        isTooManyB = false;

      if( isTooManyA && isTooManyB )
      {
        isOut = false;

        // too many, thus check the magnitude
        double outOptMagOrder=1.;
        for( size_t j=0 ; j < outOptL ; ++j )
          outOptMagOrder *= 10.;

        double scale=0.1;
        double a;
        double aveSave = a = fabs( ave[i] );
        do
        {
          scale *= 10.;
          a /= 10.;
        } while( a > 1. );
        scale *= outOptMagOrder ;

        std::vector<size_t> outRec_n;
        std::vector<double> outVal_n;
        for( size_t j=0 ; j < outRec.size() ; ++j )
        {
          if( fabs( aveSave - outVal[j] ) > scale )
          {
            outRec_n.push_back( outRec[j] );
            outVal_n.push_back( outVal[j] );
          }
        }

        if( outRec_n.size() )
        {
           outRec = outRec_n ;
           outVal = outVal_n ;
           outRec_n.clear();
           outVal_n.clear();
           isOut = true;
        }
      }
    }

    if( isOut )
    {
      double currTime;
      std::string cTime;

      std::string key("R");
      key += hdhC::itoa(errNum[i]);
      if( notes->inq( key, name, "NO_MT") )
      {
        pQAD->sharedRecordFlag.currFlag += errNum[i];

        // fine for absolute and relative Dates
        // search the maximum outlier

        size_t outRecMax;
        double outValMax;
        std::string currDateStrMax;

        outRecMax = outRec[0] ;
        outValMax = outVal[0] ;

        double cT=pQA->pIn->nc.getData(tmp_mv, "time", outRec[0]) ;
        currDateStrMax        = pQA->qaTime.getDateStr(cT);

        for( size_t k=1 ; k < outRec.size() ; ++k )
        {
          if ( outVal[k] < outValMax )
            continue;

          cT=pQA->pIn->nc.getData(tmp_mv, "time", outRec[k]) ;
          currDateStrMax        = pQA->qaTime.getDateStr(cT);

          outValMax = outVal[k] ;
          outRecMax = outRec[k] ;
        }

        std::ostringstream ostr(std::ios::app);
        ostr.setf(std::ios::scientific, std::ios::floatfield);

        if(pQA->varMeDa.size() > 1 )
          ostr << name << ", ";
        ostr << "rec# ";

        ostr << outRecMax<< ", questionable " << extStr[i];
        ostr << ", value=" ;
        ostr << std::setw(12) << std::setprecision(5) << outValMax;
        ostr << ", date="<< currDateStrMax << ".";

        std::string capt(ostr.str());

        ostr.str("");  // clear previous contents
        ostr << "variable=" << pQA->varMeDa[vMDix].var->name;
        ostr << ", " << pQA->varMeDa[vMDix].var->std_name;
        ostr << ", units=" << pQA->varMeDa[vMDix].var->units;

        MtrxArr<int> imv;
        for( size_t k=0 ; k < outRec.size() ; ++k )
        {
          // adjust coded flags
          pQAD->sharedRecordFlag.adjustFlag(errNum[i], outRec[k] ) ;

          currTime = pQA->pIn->nc.getData(tmp_mv, "time", outRec[k]) ;

          ostr << "\nrec#=" << outRec[k];
          ostr << ", date=" << pQA->qaTime.getDateStr(currTime);
          ostr << ", value=";
          ostr << std::setw(12) << std::setprecision(5) << outVal[k];
        }

        if( (retCode=notes->operate(capt, ostr.str())) )
        {
          notes->setCheckDataStr(pQA->fail);
        }
      }
    }
  }

  return retCode;
}


ReplicatedRecord::ReplicatedRecord( QA *p, size_t i, std::string nm)
{
  pQA=p;
  vMDix=i;
  name=nm;

  groupSize=0;
  enableReplicationOnlyGroups = false ;
}

bool
ReplicatedRecord::isSelected(
     std::vector<std::string> &options,
     std::string &vName,
     bool isQA_enabledPostProc, int effDim )
{
  if( options.size() == 1 && options[0] == "t" )
    return true; // default for all

  bool isThis  =true;
  bool isZeroDim=false;

  // dismember options
  for( size_t k=0 ; k < options.size() ; ++k )
  {
    bool isVar  =false;
    isThis      =true;
    isZeroDim   =false;

    Split cvs(options[k],",");
    for( size_t i=0 ; i < cvs.size() ; ++i )
    {
      if( cvs[i] == "POST" )
      {
        if( ! isQA_enabledPostProc )
         isThis=false ;
      }
      else if( cvs[i].substr(0,10) == "clear_bits" )
        ;
      else if( cvs[i] == "no_0-D" && effDim < 1 )
        isZeroDim=true;
      else if( cvs[i].substr(0,11) == "only_groups" )
        ;
      else if( cvs[i] == vName )
        isVar=true;  // valid for this specific variable
    }

    if( isVar )
    {
      // this dedicated variable
      if( isZeroDim )
         return false;

      return isThis ;
    }
  }

  // all variables
  if( isZeroDim )
    return false;

  return isThis ;
}

void
ReplicatedRecord::parseOption(std::vector<std::string> &options)
{
  // dismember options
  for( size_t k=0 ; k < options.size() ; ++k )
  {
    Split cvs(options[k],",");
    for( size_t i=0 ; i < cvs.size() ; ++i )
    {
      if( cvs[i].substr(0,10) == "clear_bits" )
      {
        size_t numOfClearBits=2; //default

        if( cvs[i].size() > 10 )
        {
          // note: from index 10
          if( hdhC::isDigit( cvs[i].substr(10) ) )
            numOfClearBits=static_cast<size_t>(
              hdhC::string2Double( cvs[i].substr(10) ) );
        }

        // enable clearing of the least significant bits for R32 flags
        if( numOfClearBits )
        {
           pQA->varMeDa[vMDix].var->pDS->enableChecksumWithClearedBits(
                 numOfClearBits ) ;
           pQA->varMeDa[vMDix].qaData.numOfClearedBitsInChecksum = numOfClearBits;
        }
      }
      else if( cvs[i].substr(0,11) == "only_groups" )
      {
        // return zero in case of no digit
        setGroupSize( static_cast<size_t>(
           hdhC::string2Double( cvs[i], 1, true ) ) ) ;

        enableReplicationOnlyGroups = true;
      }
    }
  }

  return;
}

void
ReplicatedRecord::test(int nRecs, size_t bufferCount, size_t nextFlushBeg,
                       bool isMultiple, bool isFirst)
{
  // Test for replicated records.
  // Before an array of values (e.g. ave, max ,min)
  // is flushed to the qa_<filename>.nc file, the values
  // in the array are tested for replicated records in
  // the priviously written qa_<filename>.nc as well as
  // in the array itself.*/
  std::string t_checksum;

  // temporary arrays
  double arr_curr_time[bufferCount] ;
  double arr_prev_time[bufferCount] ;
  size_t arr_curr_index[bufferCount] ;
  size_t arr_prev_index[bufferCount] ;

  std::vector<double> tmp_curr_time ;
  std::vector<double> tmp_prev_time ;
  std::vector<size_t> tmp_curr_index ;
  std::vector<size_t> tmp_prev_index ;

  size_t status [bufferCount] ;
  size_t currStatus=1;
  size_t totalIndex=0;

  // 0: no, 1: singleton, 2: group (beg), 3: group(end)
  // 4: filling and constant values

  for( size_t j=0 ; j < bufferCount ; ++j )
  {
    int n ;
    n = pQA->varMeDa[vMDix].qaData.sharedRecordFlag.buffer[j];

    // exclude code numbers 100 and 200
    if( n > 99 && n < 400 )
      status[j]=4 ;
    else
      status[j]=0 ;
  }

  t_checksum = name + "_checksum";

  if( ! isFirst )
  { // not for the first sub-temporal file,
    // search only in the qa_<filename>.nc, when
    // values have previously been written

//    int nRecs=static_cast<int>( nc->getNumOfRecords() );

    // constrain memory allocation to a buffer size
    size_t sz_max=10000;
    size_t start[] = {0};
    size_t count[] = {0};

    // buffer for netCDF data storage
    uint32_t cs[sz_max];
    double   pTime[sz_max];

    // read data from file, chunk by chunk
    while (nRecs > 0)
    {
      size_t nnRecs=static_cast<size_t>(nRecs);
      size_t sz = nnRecs > sz_max ? sz_max : nnRecs;
      nRecs -= static_cast<int>(sz);

      count[0]=sz;

      nc_get_vara_int(
         pQA->nc->getNcid(), pQA->nc->getVarID(t_checksum),
            start, count, reinterpret_cast<int*>(cs) );
      nc_get_vara_double(
         pQA->nc->getNcid(), pQA->nc->getVarID("time"), start, count, pTime );

      start[0] += sz ;

      for( size_t j=0 ; j < bufferCount ; ++j )
      { // loop over the temporarily stored values
        if( status[j] )
          continue;

        for( size_t i=0 ; i < sz ; ++i )
        { // loop over the qa_<filename>.nc file

          if( pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[j] == cs[i] )
          {
             // we suspect identity.
             // is it a group?
             size_t i0=i++;
             size_t j0=j++;
             currStatus=1;

             while( i < sz && j < bufferCount &&
                   pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[j] == cs[i] )
             {  // yes
                status[j] = 2;
                currStatus=2;
                arr_curr_time[j]  = pQA->qaTime.timeOutputBuffer.buffer[j] ;
                arr_prev_time[j]  = pTime[i] ;
                arr_curr_index[j] = nextFlushBeg + j;
                arr_prev_index[j++] = totalIndex + i++;
             }

             --i;
             --j;

             arr_curr_time[j0]  = pQA->qaTime.timeOutputBuffer.buffer[j0] ;
             arr_prev_time[j0]  = pTime[i0] ;
             arr_curr_index[j0] = nextFlushBeg + j0;
             arr_prev_index[j0] = totalIndex + i0;
             if( currStatus == 2 )
             {
               status[j]  = 3;
               status[j0] = 2;
             }
             else
               status[j0] = 1;

             break;
          }
        }
      }

      totalIndex += sz;
    } // end: not the first sub-temporal file
  }

  size_t j=0;

  for( size_t i=1 ; i < bufferCount ; ++i, j=0 )
  {
    if( status[i] )
      continue;

    if( pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[i] ==
                pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[j] )
    {
       // we suspect identity.
       if( status[j] && status[j] < 3)
       {
         // it was already resolved previously
         size_t i0=i++;
         size_t j0=j++;

         while( i < bufferCount && j < bufferCount &&
               pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[j]
                     == pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[i] )
         {  // yes
            arr_curr_time[i]  = pQA->qaTime.timeOutputBuffer.buffer[j] ;
            arr_prev_time[i]  = pQA->qaTime.timeOutputBuffer.buffer[j] ;
            arr_curr_index[i] = arr_curr_index[j];
            arr_prev_index[i] = arr_prev_index[j];
            status[i++] = status[j++];
         }

         --i;
         --j;

         arr_curr_time[i0]  = arr_curr_time[j0] ;
         arr_prev_time[i0]  = arr_prev_time[j0] ;
         arr_curr_index[i0] = arr_curr_index[j0];
         arr_prev_index[i0] = arr_prev_index[j0];
         status[i0] = status[j0];
         continue;
       }

       // is it another replication of a group?
       size_t i0=i++;
       size_t j0=j++;
       currStatus=1;

       while( i < bufferCount && j < bufferCount &&
             pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[j]
                 == pQA->varMeDa[vMDix].qaData.dataOutputBuffer.checksum[i] )
       {  // yes
          status[i] = 2;
          currStatus=2;
          arr_curr_index[i] = nextFlushBeg + i;
          arr_prev_index[i] = nextFlushBeg + j;
          arr_curr_time[i]  = pQA->qaTime.timeOutputBuffer.buffer[i] ;
          arr_prev_time[i++]  = pQA->qaTime.timeOutputBuffer.buffer[j++] ;
       }

       --i;
       --j;

       arr_curr_time[i0]  = pQA->qaTime.timeOutputBuffer.buffer[i0] ;
       arr_prev_time[i0]  = pQA->qaTime.timeOutputBuffer.buffer[j0] ;
       arr_curr_index[i0] = nextFlushBeg + i0;
       arr_prev_index[i0] = nextFlushBeg + j0;
       if( currStatus == 2 )
       {
         status[i] = 3;
         status[i0] = 2;
       }
       else
         status[i0] = 1;
    }
  }

  // And now the error flag
  for( size_t i=0 ; i < bufferCount ; ++i )
  {
    if( status[i] && status[i] < 4 )
      pQA->varMeDa[vMDix].qaData.sharedRecordFlag.buffer[i] += 3200;
  }

  // if we've found replicated records report them
  bool is=true;
  for( size_t i=0 ; i < bufferCount ; ++i )
  {
    if( status[i] && status[i] < 4 )
    {
      is=false;
      break ;
    }
  }

  if( is )
    return;  // no replication found

  bool isGroup=false;
  std::vector<std::string> range;
  double cT;
  std::string sCT;

  for( size_t i=0 ; i < bufferCount ; ++i )
  {
    if( status[i] == 0   ||  status[i] == 4 || status[i] == 3 )
    {
       if( isGroup )
       {
         range.push_back( hdhC::double2String(arr_prev_index[i] ) );
         range.push_back( hdhC::double2String(arr_curr_index[i] ) );
         cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_prev_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
         cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_curr_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
         isGroup = false;
       }
       else
         continue;
    }
    else if( status[i] == 1 )
    {
       if( isGroup )
          isGroup=false;  // should not happen
       else
       {
         range.push_back( hdhC::double2String(arr_prev_index[i] ) );
         range.push_back( hdhC::double2String(arr_curr_index[i] ) );
         cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_prev_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
         cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_curr_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
       }

    }
    else if( status[i] == 2 )
    {
       if( ! isGroup )
       {
         range.push_back( hdhC::double2String(arr_prev_index[i] ) );
         range.push_back( hdhC::double2String(arr_curr_index[i] ) );
         cT=pQA->pIn->nc.getData(tmp_mv, "time", arr_prev_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
         cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_curr_index[i]) ;
         sCT = hdhC::double2String(cT) ;
         range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
         isGroup=true;
       }

       continue;
    }

    // issue message
    report(range, isMultiple );
    range.clear();
  }

  // finalisation for a group ending on the last record
  if( isGroup )
  {
    size_t i = bufferCount -1 ;
    range.push_back( hdhC::double2String(arr_prev_index[i-1] ) );
    range.push_back( hdhC::double2String(arr_curr_index[i-1] ) );
    cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_prev_index[i]) ;
    sCT = hdhC::double2String(cT) ;
    range.push_back(pQA->qaTime.refDate.getDate( sCT ) );
    cT  = pQA->pIn->nc.getData(tmp_mv, "time", arr_curr_index[i]) ;
    sCT = hdhC::double2String(cT) ;
    range.push_back(pQA->qaTime.refDate.getDate( sCT ) );

    report(range, isMultiple );
  }

  return;
}

void
ReplicatedRecord::report(std::vector<std::string> &range, bool isMultiple)
{
   std::string capt ;
   std::string text ;

   if( isMultiple )
     capt = name + ", ";

   capt += "rec# ";

   // Is it a group?
   bool isIsolated=false;
   size_t beg = static_cast<size_t>(
                    hdhC::string2Double( range[1] ) );

   if( range.size() > 4 && range[0] != range[4] )
   {
     if( groupSize )
     {
       size_t diff=static_cast<size_t>(hdhC::string2Double(range[4]) );
       diff -= static_cast<size_t>(hdhC::string2Double(range[0]) );
       if( diff < groupSize )
         return;
     }

     capt += range[0];
     capt += "-";
     capt += range[4];
     capt += " --> ";
     capt += range[1];
     capt += "-";
     capt += range[5] + "=";

     // adjust coded error flags
     size_t end = static_cast<size_t>( hdhC::string2Double( range[5] ) );

     for( size_t k=beg ; k <= end ; ++k )
       pQA->varMeDa[vMDix].qaData.sharedRecordFlag.adjustFlag(3200, k ) ;
   }
   else
   {
     if( enableReplicationOnlyGroups )
       return;

     capt += range[0];
     capt += " --> " ;
     capt += range[1] + "=";
     isIsolated=true;

     // adjust coded error flags
     pQA->varMeDa[vMDix].qaData.sharedRecordFlag.adjustFlag(3200, beg ) ;
   }

   std::ostringstream ostr(std::ios::app);

   std::string key("R3200");
   if( notes->inq( key, name, "NO_MT") )
   {
     pQA->varMeDa[vMDix].qaData.sharedRecordFlag.currFlag += 3200 ;

     if( enableReplicationOnlyGroups )
       capt += "suspicion of replicated group(s) of records" ;
     else
     {
       if( isIsolated )
         capt += "suspicion of replicated isolated record(s)" ;
       else
         capt += "suspicion of replicated group(s) of records" ;
     }

     ostr << "variable=" << name;
     ostr.setf(std::ios::fixed, std::ios::floatfield);

     // Is it a group?
     if( isIsolated )
     {
       ostr << "\nrec# " << range[0] ;
       ostr << ", date=" << range[2] ;
       ostr << " with suspected identity to record #" << range[1];
       ostr << ", date=" << range[3] ;
     }
     else
     {
       ostr << "\nrec# (begin) " << range[0] << " --> " << range[1] ;
       ostr << ", date=" ;
       ostr << range[2] << " --> " << range[3] ;

       ostr << "\nrec# (end) " << range[4] << " --> " << range[5] ;
       ostr << ", date=" ;
       ostr << range[6] << " --> " << range[7] ;
     }

     // note: no update of record_flag variable; already done
     (void) notes->operate(capt, ostr.str()) ;
     notes->setCheckDataStr(pQA->fail);
   }

   return;
}

QA_Data::QA_Data()
{
   // only used in context of enabled replication test
   allRecordsAreIdentical=true;

   enableConstValueTest = true ;
   enableFillValueTest = true ;
   enableOutlierTest = true;
   enableReplicationTest = true;
   enableStdDevTest = true;

   isEntirelyConst=true;
   isEntirelyFillValue=true;
   isForkedAnnotation=false;

   numOfClearedBitsInChecksum = 0;
   bufferCount=0;
   maxBufferSize=1500;

   outlier = 0;
   replicated = 0;
}

QA_Data::~QA_Data()
{
   if( replicated )
     delete replicated;
   if( outlier )
     delete outlier;

}

void
QA_Data::applyOptions(bool isPost)
{
   return;
}

void
QA_Data::checkFinally(Variable *var)
{
  if( isEntirelyConst && ! var->isScalar )
  {
     std::string key("60_1");
     if( notes->inq( key, name, ANNOT_NO_MT) )
     {
       std::string capt("entire file of const value=");
       capt += hdhC::double2String( currMin );

       std::string text("variable=") ;
       text += name;

       if( notes->operate(capt, text) )
         notes->setCheckDataStr(pQA->fail);

       // erase redundant map entries
       notes->eraseAnnotation("R200");
       notes->eraseAnnotation("R3200");

       return;
    }
  }

  if( isEntirelyFillValue )
  {
     std::string key("60_2");
     if( notes->inq( key, name, ANNOT_NO_MT) )
     {
       std::string capt("data set entirely of _FillValue");

       std::string text("variable=") ;
       text += name;

       if( notes->operate(capt, text) )
         notes->setCheckDataStr(pQA->fail);

       // erase redundant map entries
       notes->eraseAnnotation("R100");

       return;
    }
  }

   if( replicated && allRecordsAreIdentical && ! pIn->isRecSingle )
   {
      std::string key("60_3");
      if( notes->inq( key, name, ANNOT_NO_MT) )
      {
        std::string capt("all records (data at given time steps) are identical") ;

        std::string text("Variable=") ;
        text +=  name;

        if( notes->operate(capt, text) )
          notes->setCheckDataStr(pQA->fail);

        // erase redundant map entries
        notes->eraseAnnotation("R3200");
      }
   }

  return;
}

void
QA_Data::disableTests(std::string name)
{
   if( notes == 0 )
      return;

   // only for tests on data
   std::string key;

   // note: exceptipon() returns false, when no
   //       directive was specified in PROJECT_flags.conf

   key="R100";
   if( ! notes->inq( key, name, "INQ_ONLY") )
   {
     enableFillValueTest=false;
     isEntirelyFillValue=false;
   }

   key="R200";
   if( ! notes->inq( key, name, "INQ_ONLY") )
   {
     isEntirelyConst=false;
     enableConstValueTest=false;
   }

   key="R400";
   bool is = notes->inq( key, name, "INQ_ONLY") ;

   key="R800";
   if( ! (is || notes->inq( key, name, "INQ_ONLY")) )
     enableOutlierTest=false;

   key="R1600";
//   if( ! notes->inq( key, name, "INQ_ONLY") )
//     enableStdDevTest=false;

   key="R3200";
   if( ! notes->inq( key, name, "INQ_ONLY") )
     enableReplicationTest=false;

   // note: R6400 will be disabled in the InFile object. This is
   // invoked by the qaExecutor script.

   key="60_1";
   if( ! notes->inq( key, name, "INQ_ONLY") )
     isEntirelyConst=false;

   key="60_2";
   if( ! notes->inq( key, name, "INQ_ONLY") )
     isEntirelyFillValue=false;

   key="60_3";
   if( ! notes->inq( key, name, "INQ_ONLY") )
     allRecordsAreIdentical=false;

  return;
}

int
QA_Data::finally(int eCode)
{
  // write pending results to qa-file.nc. Modes are considered there
  flush();

  // annotation obj forked by the parent VMD
  notes->printFlags();

  int rV = notes->getExitValue();
  eCode = ( eCode > rV ) ? eCode : rV ;

  return eCode ;
}

void
QA_Data::forkAnnotation(Annotation *n)
{
//   if( isForkedAnnotation )
//      delete notes;

   notes = new Annotation(n);

   isForkedAnnotation=true;
   return;
}

void
QA_Data::flush(void)
{
  if( dataOutputBuffer.min )
  {
     // test for entirely identical records
     if( allRecordsAreIdentical )
     {
       for( size_t i=1 ; i < bufferCount ; ++i )
       {
         if( dataOutputBuffer.checksum[0]
                   != dataOutputBuffer.checksum[i] )
         {
           allRecordsAreIdentical=false;
           break;
         }
       }
     }

     // Test for replicated records.
     if( replicated )
     {
         int nRecs=static_cast<int>( pQA->nc->getNumOfRecords() );

         replicated->test(nRecs, bufferCount, nextFlushBeg,
                            pQA->varMeDa.size() > 1 ? true : false,
                                       nextFlushBeg ? true : false );
     }

     for( size_t i=0 ; i < bufferCount ; ++i)
     {
       statMin.add( dataOutputBuffer.min[i] );
       statMax.add( dataOutputBuffer.max[i] );
       statAve.add( dataOutputBuffer.ave[i] );
       statStdDev.add( dataOutputBuffer.std_dev[i] );
     }

     dataOutputBuffer.flush();
     sharedRecordFlag.flush();
  }

  bufferCount = 0;

  return ;
}

void
QA_Data::init(InFile *in, QA *q, std::string nm)
{
   pIn = in;
   pQA =q;
   name = nm;

   // apply parsed command-line args
   applyOptions();

   dataOutputBuffer.setName(name) ;
   sharedRecordFlag.setName(nm + "_flag") ;

   ANNOT_NO_MT="NO_MT";
   isForkedAnnotation=false;

   return ;
}

void
QA_Data::initBuffer(NcAPI *nc, size_t nxt, size_t mx)
{
  maxBufferSize=mx;
  nextFlushBeg=nxt;

  dataOutputBuffer.initBuffer(nc, nxt, mx);
  sharedRecordFlag.initBuffer(nc, nxt, mx);

  bufferCount=0;

  return;
}

void
QA_Data::initResumeSession(void)
{
  // read data values from the previous QA run
  std::vector<double> dv;
  std::string statStr;
  std::string s0;

  s0 = name + "_max";
  pQA->nc->getAttValues( dv, "valid_range", s0);
  statStr  ="sampleMin=" ;
  statStr += hdhC::double2String( dv[0] );
  statStr +=", sampleMax=" ;
  statStr += hdhC::double2String( dv[1] );
  statStr += ", ";
  statStr += pQA->nc->getAttString("statistics", s0) ;
  statMax.setSampleProperties( statStr );

  s0 = name + "_min";
  pQA->nc->getAttValues( dv, "valid_range", s0);
  statStr  ="sampleMin=" ;
  statStr += hdhC::double2String( dv[0] );
  statStr +=", sampleMax=" ;
  statStr += hdhC::double2String( dv[1] );
  statStr += ", ";
  statStr += pQA->nc->getAttString("statistics", s0) ;
  statMin.setSampleProperties( statStr );

  s0 = name + "_ave";
  pQA->nc->getAttValues( dv, "valid_range", s0);
  statStr  ="sampleMin=" ;
  statStr += hdhC::double2String( dv[0] );
  statStr +=", sampleMax=" ;
  statStr += hdhC::double2String( dv[1] );
  statStr += ", ";
  statStr += pQA->nc->getAttString("statistics", s0) ;
  statAve.setSampleProperties( statStr );

  s0 = name + "_std_dev";
  pQA->nc->getAttValues( dv, "valid_range", s0);
  statStr  ="sampleMin=" ;
  statStr += hdhC::double2String( dv[0] );
  statStr +=", sampleMax=" ;
  statStr += hdhC::double2String( dv[1] );
  statStr += ", ";
  statStr += pQA->nc->getAttString("statistics", s0) ;
  statStdDev.setSampleProperties( statStr );

  return;
}

void
QA_Data::openQA_NcContrib(NcAPI *nc, Variable *var)
{
  // a multi-purpose vector
  std::vector<std::string> vs;

  // variable name in inFile
  std::string &vName = name;

  // Define a dummy variable for no data, but information.
  // Define variable explicitly, because of no dimensions
  // and an additional att.
  nc->defineVar(vName, pIn->nc.getVarType(vName), vs);
  nc->setAtt(vName, "about",
                          "original attributes of the checked variable");

  // get original dimensions and convert names into a string
  vs = pIn->nc.getDimName(vName);
  std::string str;

  Split splt( var->units );

  if( splt.size() > 0 )
  {
    str = "(" + vs[0] ;
    for( size_t k=1 ; k < vs.size() ; ++k)
      str += ", " + vs[k];

    str += ")";
  }
  else
    str = "None";

  nc->setAtt(vName, "original_dimensions", str );
  nc->copyAtts(pIn->nc, vName, vName);

  // define qa-variables
  std::string dimStr;
  if( var->isFixed )
    dimStr = "fixed" ;
  else
    dimStr = pQA->qaTime.timeName.c_str() ;

  // different, but derived, varnames
  vs.clear();
  std::string str0( name + "_min" );
  vs.push_back(dimStr);
  nc->defineVar( str0, NC_DOUBLE, vs);
  vs[0] = "global_minimum";
  nc->setAtt( str0, "long_name", vs[0]);
  nc->setAtt( str0, "units", var->units);
  nc->setAtt( str0, "_FillValue", static_cast<double>(1.E+20));

  vs.clear();
  str0 = name + "_max" ;
  vs.push_back(dimStr);
  nc->defineVar( str0, NC_DOUBLE, vs);
  vs[0] = "global_maximum";
  nc->setAtt( str0, "long_name", vs[0]);
  nc->setAtt( str0, "units", var->units);
  nc->setAtt( str0, "_FillValue", static_cast<double>(1.E+20));

  vs.clear();
  str0 = name + "_ave" ;
  vs.push_back(dimStr);
  nc->defineVar( str0, NC_DOUBLE, vs);
  vs[0] = "global_average";
  nc->setAtt( str0, "long_name", vs[0]);
  nc->setAtt( str0, "units", var->units);
  nc->setAtt( str0, "_FillValue", static_cast<double>(1.E+20));

/*
  if( in.variable[m].pGM->isRegularGrid() )
    nc->setAtt( str0, "comment",
      "Note: weighted by latitude-parallel bounded areas");
  else
    nc->setAtt( str0, "comment",
      "Note: weighted by areas of spherical triangles");
*/

  vs.clear();
  str0 = name + "_std_dev" ;
  vs.push_back(dimStr);
  nc->defineVar( str0, NC_DOUBLE, vs);
  vs[0] = "standard_deviation";
  nc->setAtt( str0, "long_name", vs[0]);
  nc->setAtt( str0, "units", var->units);
  nc->setAtt( str0, "_FillValue", static_cast<double>(1.E+20));

  vs.clear();
  vs.push_back(dimStr);
  str0 = name + "_fill_count" ;
  nc->defineVar( str0, NC_INT, vs);
  vs[0] = "number of cells with _FillValue";
  nc->setAtt( str0, "long_name", vs[0]);
  nc->setAtt( str0, "units", "1");
  nc->setAtt( str0, "_FillValue", static_cast<int>(-1));

  vs.clear();
  vs.push_back(dimStr);
  str0 = name + "_checksum" ;
  nc->defineVar( str0, NC_INT, vs);
  vs[0] = "Fletcher32 checksum of data";
  nc->setAtt( str0, "long_name", vs[0]);
  if( numOfClearedBitsInChecksum )
  {
    std::string st("last ");
    st += hdhC::itoa(numOfClearedBitsInChecksum) ;
    st += " insignificant bits cleared";
    nc->setAtt( str0, "comment", st);
  }

  nc->setAtt( str0, "units", "1");
  nc->setAtt( str0, "_FillValue", static_cast<int>(0));

  vs.clear();
  str0 = name + "_flag" ;
  vs.push_back(dimStr);
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
QA_Data::setAnnotation(Annotation *n)
{
//   if( isForkedAnnotation )
//      delete notes;

   notes = n;

   isForkedAnnotation=false;
   return;
}

void
QA_Data::setNextFlushBeg(size_t n)
{
   nextFlushBeg=n;
   sharedRecordFlag.setNextFlushBeg(n);

   return;
}

void
QA_Data::setStatisticsAttribute(NcAPI *nc)
{
  // store the statistics' properties in qa_<variable>.nc
  std::vector<std::string> vs;
  std::string t;
  size_t sz=2;
  double vals[2];

  // overall range
  vals[0]=statMin.getSampleMin();
  vals[1]=statMax.getSampleMax();

  nc->setAtt( name, "valid_range", vals, static_cast<size_t>(2) );

  // attributes of special interest
  t = name + "_max";
  vals[0] = statMax.getSampleMin();
  vals[1] = statMax.getSampleMax();
  nc->setAtt( t, "valid_range", vals, sz);
  vs.clear();
  vs.push_back( statMax.getSampleProperties() );
  // strip leading part indicating valid_range
  size_t pos = vs[0].find("sampleSize");
  nc->setAtt( t, "statistics", vs[0].substr(pos));

  t = name + "_min";
  vals[0] = statMin.getSampleMin();
  vals[1] = statMin.getSampleMax();
  nc->setAtt( t, "valid_range", vals, sz);
  vs.clear();
  vs.push_back( statMin.getSampleProperties() );
  // strip leading part indicating valid_range
  pos = vs[0].find("sampleSize");
  nc->setAtt( t, "statistics", vs[0].substr(pos));

  t = name + "_ave";
  vals[0] = statAve.getSampleMin();
  vals[1] = statAve.getSampleMax();
  nc->setAtt( t, "valid_range", vals, sz);
  vs.clear();
  vs.push_back( statAve.getSampleProperties() );
  // strip leading part indicating valid_range
  pos = vs[0].find("sampleSize");
  nc->setAtt( t, "statistics", vs[0].substr(pos));

  t = name + "_std_dev";
  vals[0] = statStdDev.getSampleMin();
  vals[1] = statStdDev.getSampleMax();
  nc->setAtt( t, "valid_range", vals, sz);
  vs.clear();
  vs.push_back( statStdDev.getSampleProperties() );
  // strip leading part indicating valid_range
  pos = vs[0].find("sampleSize");
  nc->setAtt( t, "statistics", vs[0].substr(pos));

  return;
}

void
QA_Data::store(hdhC::FieldData &fA)
{
   if( bufferCount == maxBufferSize )
      flush();

   dataOutputBuffer.store(fA) ;
   sharedRecordFlag.store();

   ++bufferCount;

   return;
}

void
QA_Data::test(int i, hdhC::FieldData &fA)
{
  // all tests return true for finding something no
  if( testValidity(fA) )
  {
    currMin=fA.min;
    currMax=fA.max;

    testInfNaN(fA);

    testConst(fA) ;

    if( enableOutlierTest )
    {
      if( ! fA.isValid )
      {  // update the statistics
         statMax.add( currMax );
         statMin.add( currMin );
      }
    }

    if( enableStdDevTest )
      (void) testStndDev(fA) ;
  }

  return ;
}

bool
QA_Data::testConst(hdhC::FieldData &fA)
{
  if( ! fA.isValid )
    return false;

  if( ! enableConstValueTest )
    return true;

  if( currMin != currMax || isSingleValueField  )
  {
    isEntirelyConst=false;
    return false;
  }

  std::string key=("R200");
  std::string val=hdhC::double2String(currMin);

  // add a constraint to the inquiry, if specified
  notes->setConstraint(val);

  if( notes->inq( key, name, ANNOT_NO_MT) )
  {
    sharedRecordFlag.currFlag += 200;

    std::string capt("entire record of constant value=");
    capt += val ;

    std::ostringstream ostr(std::ios::app);
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr << "rec#=" << pIn->currRec;
    ostr << ", variable=" << name;
    ostr << ", date=" << pQA->qaTime.currDateStr;

    (void) notes->operate(capt, ostr.str()) ;
    notes->setCheckDataStr(pQA->fail);
  }
  return true;
}

bool
QA_Data::testInfNaN(hdhC::FieldData &fA)
{
  if( !fA.infCount && !fA.nanCount )
    return false;

  std::string key=("R6400");
  if( notes->inq( key, name, ANNOT_NO_MT) )
  {
    sharedRecordFlag.currFlag += 6400;

    std::string capt("Inf or NaN value(s) found");

    std::ostringstream ostr(std::ios::app);
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr << "rec#=" << pIn->currRec;
    ostr << ", variable=" << name;
    ostr << ", date=" << pQA->qaTime.currDateStr;

    (void) notes->operate(capt, ostr.str()) ;
    notes->setCheckDataStr(pQA->fail);
  }

  return true ;
}

bool
QA_Data::testStndDev(hdhC::FieldData &fA)
{
  if( ! fA.isValid )
    return false;


  if( fA.isStndDevValid )
    return false;

  if( isSingleValueField )
    return false;

  std::string key=("R1600");

  if( notes->inq( key, name, ANNOT_NO_MT) )
  {
    sharedRecordFlag.currFlag += 1600 ;

    std::string capt("undefined standard deviation");

    std::ostringstream ostr(std::ios::app);
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr << "rec#=" << pIn->currRec;
    ostr << ", variable=" << name;
    ostr << ", date=" << pQA->qaTime.currDateStr;

    (void) notes->operate(capt, ostr.str()) ;
    notes->setCheckDataStr(pQA->fail);
  }
  return true ;
}

bool
QA_Data::testValidity(hdhC::FieldData &fA)
{
  if( fA.size < 2 )
  {
    isSingleValueField = true ;
    isEntirelyFillValue=false;
  }
  else
    isSingleValueField=false;

  if( fA.isValid )
  {
    isEntirelyFillValue=false;
    return true;
  }

  // the flag is always stored in the file and it is used
  // to prevent false replicated record detection
  if( ! enableFillValueTest )
    return true;

  std::string key=("R100");
  if( notes->inq( key, name, ANNOT_NO_MT) )
  {
    sharedRecordFlag.currFlag += 100 ;

    std::string capt("entire record with filling value");

    std::ostringstream ostr(std::ios::app);
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr << "rec#=" << pIn->currRec;
    ostr << ", variable=" << name;
    ostr << ", date=" << pQA->qaTime.currDateStr;

    (void) notes->operate(capt, ostr.str()) ;
    notes->setCheckDataStr(pQA->fail);
  }

  return false ;
}
