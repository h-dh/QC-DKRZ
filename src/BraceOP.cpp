#include "brace_op.h"

BraceOP::BraceOP()
{
  nextCounter=0;
  setDiscard("D()");
}

BraceOP::BraceOP(std::string &s)
{
  str=s;
  nextCounter=0;
  setDiscard("D()");
 }

BraceOP::BraceOP(std::vector<std::string> &s)
{
  setDiscard("D()");
  set(s);
}

void
BraceOP::add(std::vector<std::string> &vs)
{
  if( nextCounter )
    str.clear();

  for(size_t i=0 ; i < vs.size() ; ++i )
    str += vs[i];

  nextCounter=0;

  return;
}

void
BraceOP::add(std::string s)
{
  if( nextCounter )
    str.clear();

  nextCounter=0;
  str += s;
  return;
}

void
BraceOP::clear(void)
{
  str.clear();
  return;
}

void
BraceOP::findBraces(bool isPrintTree)
{
  // find { or } positions and corresponding levels.
  // due to nesting there might be multiple braces or
  // nested braces with a same level count.

  str = hdhC::clearSpaces( str );

  // split something like: ..{..,{{..}..},{..}..},{..}

  // Note: leftLevel points to the first char inside a brace
  //       rightLevel points to the last char in a brace

  // insert { before and } behind, respectively, to ensure a zero-th level
  std::string tmp("{");
  tmp += str ;
  tmp += '}';
  str = tmp;

  size_t countLeft=1;
  size_t countRight=0;

  std::string s;
  int lC=0 ; // level counter
  size_t last=0;

  Branch tree;
  Branch *currB=&tree ;

  for( size_t pos=0 ; pos < str.size(); ++pos )
  {
    s.clear();

    if( str[pos] == '{' )
    {
      ++countLeft;

      currB = currB->branch();
      currB->setLevel(++lC);

      if( ! ( (last +1 ) > pos) )
      {
        // brace is open right && non-brace item is given
        s = str.substr(last+1, pos - last - 1);

        if( s.size() && s != "," )
        {
          // item could be placed at a brace without comma
          size_t sz = currB->prev->str.size() ;

       	  if( s[0] != ',' )
            if( sz && currB->prev->str[sz-1] != ',' )
              currB->prev->str += ',';
          currB->prev->str += s;
        }
      }

      last=pos;
    }
    else if( str[pos] == '}' )
    {
      ++countRight;

      if( (last+1) != pos)
      {
        // close an open brace expr
        s = str.substr(last+1, pos - last -1) ;

       	if( ! (s.size() == 0 || s == ",") )
        {
          // item could be placed at a brace without comma
          size_t sz = currB->prev ? currB->prev->str.size() : 0 ;

	         if( s[0] != ',' )
            if( sz && currB->str[sz-1] != ',' )
              currB->str += ',';
          currB->str += s ;
        }
      }

      last = pos;
      --lC;
      currB = currB->prev;
    }
  }

//  if( countLeft != countRight )
//    return ; // error

  if( isPrintTree )
  {
     tree.printTree();
     return;
  }

  // a sequence of strings from the top most leaf down the crotches
  // to the root (order reversed).
  std::vector<std::vector<Branch*> > sequences = tree.getSequence();

  // perform brace operation: add lower level items to higher ones, if
  // there is no identical L-value in the higher ones. Then, resolve
  // each sequence into the components. The resulting vector of groups
  // is member of the class.
  getGroups(sequences);

  // disable sequences which is composed out of the tree branches
  for( size_t i=0 ; i < sequences.size() ; ++i )
    for( size_t j=0 ; j < sequences[i].size() ; ++j )
       sequences[i][j] = 0;

  tree.deleteTree();

  return ;
}

void
BraceOP::getGroups(std::vector<std::vector<Branch*> > &sequences)
{
  // a group is formed by all tokens within a given brace-level
  // plus those from lower levels whose L-value does not match any
  // in the current group.

  Split c_low;
  Split c_high;
  c_low.setSeparator(',');
  c_high.setSeparator(',');

  Split a_low;
  Split a_high;
  a_low.setSeparator('=');
  a_high.setSeparator('=');
  bool is=false;

  std::string dStr;

  for(size_t i=0 ; i < sequences.size() ; ++i )
  {
    for(size_t j=1 ; j < sequences[i].size() ; ++j )
    {
       // comma-split of low and high -level string
       c_low = sequences[i][j-1]->str ;
       c_high = sequences[i][j]->str ;

       // test for low-level assignments
       for( size_t l=0 ; l < c_low.size() ; ++l)
       {
         for( size_t h=0 ; h < c_high.size() ; ++h)
         {
           is=true;

           // identical L-value: discard the lower level one
           if( c_low[l][0] == c_high[h][0] )
           {
              a_low  = c_low[l];
              a_high = c_high[h];
              if( a_low[0] == a_high[0] )  // only L-value
                is=false;
           }

           // discard lower level items
           if( c_high[h].size() > discardToken.size() )
           {
             dStr = c_high[h].substr(0, discardToken.size()) ;

             if( dStr == discardToken )
             {
                a_low  = c_low[l];
                a_high = c_high[h];

                dStr  = discardToken + a_low[0] ;
                dStr += discardClose ;

                if( dStr == a_high[0] )  // only L-value
                {
                  is=false;
                  rmDiscardToken( sequences[i][j]->str ) ;
                  break;
                }
             }
           }
         }

         if( is && c_low[l].size() )  // an item to assign
         {
            sequences[i][j]->str += ',' ;
            sequences[i][j]->str += c_low[l] ;
         }
       }
    }

//      std::cout << sequences[i][j]->str << "\t" ;
//    std::cout << std::endl;
  }

  // remove discard-item
  for(size_t i=0 ; i < sequences.size() ; ++i )
    for(size_t j=0 ; j < sequences[i].size() ; ++j )
       if( sequences[i][j]->str.find(discardToken) < std::string::npos )
          rmDiscardToken( sequences[i][j]->str ) ;

  // remove identical strings

  // 2-D for storing array indexes
  std::vector<std::vector<int> > ix;
  for(size_t i=0 ; i < sequences.size() ; ++i )
  {
    ix.push_back( std::vector<int>() );
    for(size_t j=0 ; j < sequences[i].size() ; ++j )
      ix[i].push_back( j );
  }

  for(size_t i0=0 ; i0 < sequences.size() ; ++i0 )
  {
    for(size_t i1=0 ; i1 < sequences[i0].size() ; ++i1 )
    {
       if( sequences[i0][i1]->str.size() == 0
           || sequences[i0][i1]->str == "," )
       {
         ix[i0][i1] = -1 ;
         continue;
       }

       for(size_t j0=i0+1 ; j0 < sequences.size() ; ++j0 )
       {
         for(size_t j1=0 ; j1 < sequences[j0].size() ; ++j1 )
         {
           if( i0 == j0 && i1 == j1 )
             continue; //note: ... ; i0 < sequences.size()-1 ; ...
                       //      doesn't work for only a single item
           if( sequences[i0][i1]->str == sequences[j0][j1]->str )
             ix[j0][j1] = -1 ;
         }
       }
    }
  }

  groups.clear();

  for(size_t i=0 ; i < sequences.size() ; ++i )
  {
    for(size_t j=0 ; j < sequences[i].size() ; ++j )
    {
      if( ix[i][j] > -1 )
      {
       std::string t( hdhC::stripSides( sequences[i][j]->str, "," ) ) ;
       groups.push_back("");
       groups.back() += t[0];
       for( size_t k=1 ; k < t.size() ; ++k )
         if( ! ( t[k] == ',' && t[k-1] == ',' ) )
           groups.back() += t[k] ;
      }
    }
  }

  return ;
}

bool
BraceOP::next(std::string &s)
{
   if( nextCounter == 0  )
      findBraces();

   if( nextCounter == groups.size() )
     return false;

   s = groups[nextCounter++];

   return true;
}

void
BraceOP::printGroups(void)
{
  for(size_t j=0 ; j < groups.size() ; ++j )
    std::cout << groups[j] << std::endl;

  return ;
}

void
BraceOP::rmDiscardToken(std::string &s0)
{
  Split splt(s0,",");

  s0.clear();

  for( size_t k=0 ; k < splt.size() ; ++k )
  {
    if( splt.size() > discardToken.size() )
    {
      std::string dStr( splt[k].substr(0, discardToken.size()) ) ;

      if( dStr == discardToken )
        continue;

      if( s0.size() )
        s0 += ',' ;
      s0 += splt[k];
    }
  }

  return ;
}

void
BraceOP::set(std::vector<std::string> &vs)
{
  str.clear();
  for(size_t i=0 ; i < vs.size() ; ++i )
    str += vs[i];

  nextCounter=0;

  return;
}

void
BraceOP::set(std::string s)
{
  nextCounter=0;
  str = s;
  return;
}

void
BraceOP::setDiscard(std::string s)
{
  if( s.size() )
    s="D()";

  discardClose = s[ s.size()-1 ];
  discardToken = s.substr(0, s.size()-1);

  return;
}


// ----------------

Branch*
Branch::branch(void)
{
  next.push_back(new Branch(this));
  return next.back();
}

void
Branch::deleteTree(void)
{
  for( size_t i=0 ; i < next.size() ; ++i )
  {
    next[i]->deleteTree();
    delete next[i];
  }

  return;
}

std::vector<std::vector<Branch*> >
Branch::getSequence(void)
{
  std::vector<std::vector<Branch*> > sequences;

  if( next.size() == 0 )
    return sequences; // no single branch is defined

  sequences.push_back( * new std::vector<Branch*> );
  sequences.back().push_back( this ) ;
  next[0]->getSequence(sequences);

  return sequences;
}

void
Branch::getSequence(
    std::vector<std::vector<Branch*> > &sequences)
{
  sequences.back().push_back( this ) ;
  size_t last=sequences.back().size();
  size_t curr=sequences.size()-1;

  for( size_t i=0 ; i < next.size() ; ++i )
  {
    if( i )  // clone parent sequence
    {
       sequences.push_back( * new std::vector<Branch*> );
       for( size_t j=0 ; j < last ; ++j )
	        sequences.back().push_back( sequences[curr][j] );
    }

    next[i]->getSequence(sequences);
  }

  return;
}

void
Branch::printTree(void)
{
  for( int l=1 ; l < level-1 ; ++l )
     std::cout << "|\t" ;

  if( level > 1 )
    std::cout << "|____" ;
  std::cout << str << std::endl;

  for( size_t i=0 ; i < next.size() ; ++i )
    next[i]->printTree();

  return;
}

