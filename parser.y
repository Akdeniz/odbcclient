%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines
//%define api.namespace {yy}
%define parser_class_name {Parser}

%code requires{
  struct MyState;
  class Scanner;

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

}

%parse-param { Scanner  &scanner  }
%parse-param { MyState  &state  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   #include "scanner.hpp"

#undef yylex
#define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token               END    0     "end of file"
%token               LEFT_BRACKET
%token               RIGHT_BRACKET
%token               SEMICOLON
%token               IGNOREERROR
%token <std::string> SQL_SYNTAX
%token               NEWLINE

%locations

%%

list_option : END | list END;

list
  : item
  | list item
  ;

item
  : ignore_error_list
  | NEWLINE
  ;

ignore_error_list : ignore_error | ignore_error_list ignore_error;

ignore_error : IGNOREERROR LEFT_BRACKET SQL_SYNTAX RIGHT_BRACKET

%%


void 
yy::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
