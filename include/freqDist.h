#ifndef _FREQDIST_H
#define _FREQDIST_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <valarray>
#include <vector>

//#include <assert.h>
#include "matrix_array.h"
#include "readline.h"

/*! \file freqDist.h
 \brief Frequency distribution: building and operation.
*/

//! Frequency distribution of an ensemble of type T.

/*! Frequency distributions are calculated for fixed sized
 bins (optional of different size) as well as for adjusting width.
 A FreqDist is optionally built with weighting.
 While feeding data, the range spanned by the classes may expand.
 FreqDist knows about data marked as 'missing|filling value'.
 The class properties may be determined at construction or be
 prescribed by a file. An object can save its properties, so that
 a session may be resumed by another object; the extension of
 such a saved file is '.build'. Example:\n
    Symmetry about Zero=0\n
    Class Width= 1.\n
    Minimum of Range=-0.5\n
    Maximum of Range=500.\n
    Filling Value=-9.000000000e+33\n
    Number of Ensembles= 1.\n
    'number of classes' 'lower boundary of first bin'\n
 Then on each line:\n
    'upper bnd' '(weighted) freq' 'count' 'accum. value'*/

template<typename T>
class FreqDist {
public:
   struct ExceptionStruct
   {
      std::ofstream *ofsError;
      std::string strError;
   } ;

//! Copy constructor.
  FreqDist();

//! Constructor sets the basic properties of the distribution.
/*! A constant bin (or class) 'width' is set for the 'start' value.
    The first bin is centred about the value of 'start', but
    the value is not taken into account in the frequency distribution.
    Unfortunately, exact centering is not possible, but the borders
    are aligned (human-)conveniently.*/
  FreqDist(double width, const bool isCentric=true, T start=0.);

//! Destructor
//  ~FreqDist();

//! Add a frequency ditribution from 'file'.
/*! If file is empty, then automatic determination of properties.
    If the current bin-width equals 0, then init from file (if classes
    are available, also read).
    If the freqDist from file shall really be added to the one in use,
    the properties of both are checked, and the programm exits, if
    they are different.*/
  void
     addBuild(std::string file="");

//! Increase the number of ensembles.
/*! The number of ensembles currently contained is increased [default: 1].
    E.g.: Area-weighted data from a geographical region is applied and
    the sum of all weights of the region is 1. Then, each time step
    would require to increase the ensemble count by 1. Eventually,
    the frequencies of the distribution would be 1 (if missing values
    were absent).*/
  void
     addEnsembleCount( int i=1){ensembleCount += i;}

  void
     clear(void);

//! Adjust the widths of the classes automaticly.
/*! If enabled, the initial bin-width is increased by a factor
    [default: 2], if the number of classes exceeds a given
    threshold [default:50].
    See setAutomResizeFactor() and setAutomResizeAtClassSize().*/
  void
     disableAutomResize(void)
        {isAutomResizeEnabled=false;}

  void
     disableFillingValue(void){isFillingValueEnabled=false;}

  void
     enableAutomResize(void)
        {isCentric=true; isAutomResizeEnabled=true;}
  void
     enableFillingValue(void){isFillingValueEnabled=true;}

//! Error messages are written to a file. Exits immediately.
  void
     exceptionError(std::string);

//! Get a vector with mean values of the fequency dist's bins.
  std::vector<double>
     getHistVal(void);

//! Get vector of the fequencies of the bins.
  std::vector<double>
     getHistFreq(void);

//! Get vector of std::pair of boundaries for each class.
/*! Note: lower bound = std::pair.first \n
          upper bound = std::pair.second*/
std::vector<std::pair<double,double> >
     getHistBorder(void);

//! Print histogram to std::cout.
/*! Numbers have by default a width of 17 digits with precison==6.
    Note: this is not the build-file, but the real histogram.*/
  void
     print(std::string f="", int formW=17, int formP=6);

//! Print histogram to std::cout.
/*! Note: this is not the build-file, but the real histogram.
    Additionally, the shape of a histogram bar is given.*/
  void
     printHist(std::string f="", int formW=17, int formP=6);

//! Add a vector of un-weighted values to the freq dist.
  void
     push_val(std::vector<T> &v );

//! Add a vector of values with weights to the freq dist.
  void
     push_val(std::vector<T> &v, std::vector<double> &w);

//! Add a vector of values with equal weights to the freq dist.
  void
     push_val(std::valarray<T> &v);

//! Add a vector of values with weights to the freq dist.
  void
     push_val(std::valarray<T> &v, std::valarray<double> &w);

//! Add values from an array of size 'size' to the frequency distribution.
/*! Without weights by default.*/
  void
      push_val(size_t size, const T *v, const double *w=0);

//! Add MtrxArr objs of values  to the freq dist.
  void
     push_val(const MtrxArr<T> &v);

//! Add MtrxArr objs of values and weights to the freq dist.
  void
     push_val(const MtrxArr<T> &v, const MtrxArr<double> &w);

//! Rest current distribution for another start.
  void
     reset(void);

//! Save the frequency distribution as build-file.
/*! Only this kind of file can be used for a later continuation.
    If 'sFile' is emtpy, then the name for output will be used,
    extended by '.build'. No outputfilename? Print to std::cout.*/
  void
     saveBuild( std::string sFile="");

//! Save only the header of a build-file.
/*! This can be used to give subsequent calculations of seperate
    distributions the same class properties, e.g. for a particular
    temperature from different scenarios.
    If 'file' is empty, properties are given to std::cout.*/
  void
     saveProperties( std::string file="");

//! Resize factor for automatic bin-width adjustment
  void
     setAutomResizeFactor(double f) {automResizeFactor = f;}

//! Threshhold of maximum bin number for automatic bin-width adjustment
  void
     setAutomResizeAtClassSize(size_t f) {automResizeThreshold = f;}

//! A string is added to the header of a save-type file.
  void
     setInfo(std::string);

//! Set missing|filling value.
  void
     setFillingValue(T v) {fillingValue=v;}

//! Set a filename for output.
/*! If the name has one of the extensions: ".hist", ".Hist",
    ".prop", "build", the extension is casted away and the
    appropriate is selected (depending on other options).
    If no name was set, histograms/savings are printed to std::cout.*/
  void
     setOutputFilename(std::string f);

//! Printed histograms frquency will be given in per cent.
  void
     setPrintPerCent(void){isPrintPerCent=true;}

//! Set bin width and alignment around or at 'start' value. 
/*! Note: isCentric will be set to false, whenever automatic
    resizing is enabled (even if not performed).*/
  void
     setProperties(double width,
           T start, const bool isCentric=true);

//! Set bin width. 
/*! Note: isCentric will be set to false, whenever automatic
    resizing is enabled (even if not performed).*/
  void
     setProperties(double width, const bool isCentric=true);

//! Read only the properties, even if a complete build-file is given
  void
     setReadOnlyProperties(void){isReadOnlyProperties=true;}

  std::vector<std::string> infoLine; // additional info in files

private:
  bool isAutomResizeEnabled;
  bool isCentric;
  bool isFirstReadBuild;
  bool isPrintHistogram;
  bool isPrintPerCent;
  bool isReadOnlyProperties;

  T      rangeMin, rangeMax;
  double binWidth;
  double automResizeFactor;
  size_t automResizeThreshold;

  int lastUsedIndex;
  
  double NUMERICAL_TOLERANCE ;
  double ensembleCount;

  bool isFillingValueEnabled;
  T fillingValue;
  size_t numFV;
  
  std::string outputFilenameBase;
  std::string outputFilename;

  std::vector<double> binBorder;
  std::vector<double> binVal;
  std::vector<double> binFreq;  // possibly weighted
  std::vector<double> binCount; // unweighted

  // messaging in case of exceptions.
  struct ExceptionStruct xcptn;

//  std::vector<std::vector<double> > *associate ;

  void   init(void);
  void   install(T val=0.);
  double getAverage(int i);
  double getAverage(int i, double val);
  void   push_val(double val, double weight=1., double counts=1.,
                     double accum_val=0.);
  bool   readBuild( ReadLine & );
  void   reInstall(std::string f);
  void   reInstall( ReadLine &);
  void   resize(void);
  template<typename TE>
  bool   testEquality(TE val, TE comp);
  double updateBinWidth(T); //return new class boundary
};

#endif
