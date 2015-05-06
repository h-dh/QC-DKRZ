#include "oper.h"

Oper::Oper()
{
  initDefaults();
}

void
Oper::applyOptions(void)
{
  Base::applyOptions();

   return;
}

void
Oper::help(void)
{
  std::cerr << "Additional to Oper:\n" ;
//  std::cerr << "   \n" ;
  std::cerr << std::endl;
  return;
}

bool
Oper::init(void)
{
  //re-set the execution pointer from init to entry
  execPtr = &IObj::entry ;
  return false;
}

void
Oper::initDefaults(void)
{
  setObjName("OP");

  // set pointer to member function init()
  execPtr = &IObj::entry ;

  notes=0;
  cF=0;
  pIn=0;
  fDI=0;
  pOper=0;
  pOut=0;
  qA=0;
  tC=0;

  return;
}

void
Oper::linkObject(IObj *p)
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

