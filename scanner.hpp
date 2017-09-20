#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "sql_parser.hpp"

struct MyState
{
  int wordcount;
};

class Scanner : public yyFlexLexer{
public:
   
   Scanner(std::istream *in) : yyFlexLexer(in)
   {
   };
   virtual ~Scanner() {
   };

   using FlexLexer::yylex;

   virtual int yylex( yy::Parser::semantic_type *const lval, yy::Parser::location_type *location );

private:
   yy::Parser::semantic_type *yylval = nullptr;
};

