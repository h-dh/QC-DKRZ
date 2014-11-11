#ifndef _BRACE_OP_H
#define _BRACE_OP_H

/*! \file brace_op.h
 \brief Hierarchical text from nested braces.
*/

//! Operation on hierachical text given by nested braces.
/*!
  A nested brace inherits the contents of the one it is embedded. The
  hierarchy of braces is tree-like with unlimited number of parallel
  branches as well as vertical branching. Thus, the tip of a branch
  inherits every item from the parent down the crotches to the root.
  The braces {} of the lowest level may be omitted as well as a comma around braces.
  Each higher-level branch, i.e. contents of a brace,
  is a specialisation to its parent. A brace may contain multiple
  comma-separated items. If the L-value, i.e. the word 'key'
  in 'key=value', is identical to a lower level one, then the
  lower one is not inherited. Inheritance of an item may also be inhibited by
  the macro "D(item). The macro is applied solely to L-values.
  The hierarchy of braces is eventually expanded into unrelated strings.\n
  Example:\n
     simple: given: a,{b,c},{d,e},f \n
             result: 'a,f', 'a,b,c,f', 'd,e,a,f' \n
  elaborate: given: a,b=1{x{D(x),y,b=2}},{u,v},w \n
             result: 'a,b=1,w', 'x,a,b=1,w', 'y,b=2,a,w', 'u,v,a,b=1,w,'
*/

struct Branch
{
   Branch(){prev=0; level=0;}
   Branch( Branch *p){prev=p;}
   ~Branch(){;}

   Branch*   back(void);
   Branch*   branch(void);
   void      deleteTree(void);
   // recursive
   void      getSequence(std::vector<std::vector<Branch*> >&);
   // first call
   std::vector<std::vector<Branch*> >
             getSequence(void);
   void      printTree(void);
   void      setLevel(int i){level=i;}
   void      setStr(std::string &s){ str=s;}

   Branch *prev;
   std::vector<Branch*> next;
   std::string str;
   int level;
} ;

class BraceOP
{
  public:
  BraceOP();
  BraceOP(std::string &s) ;
  BraceOP(std::vector<std::string> &) ;
  ~BraceOP(){;}

  void  add(std::vector<std::string> &) ;
  void  add(std::string);
  void  clear();
  void  findBraces(bool enablePrintTree=false);
  void  getGroups(std::vector<std::vector<Branch*> > &);
  bool  next(std::string&);
  void  printGroups(void);
  void  printTree(void){ findBraces(true); }
  void  rmDiscardToken(std::string &);
  void  set(std::vector<std::string> &) ;
  void  set(std::string);
  void  setDiscard(std::string);

  size_t                   nextCounter;
  std::string              discardToken;
  std::string              discardClose;

  std::string              str;
  std::vector<std::string> groups;
};

#endif
