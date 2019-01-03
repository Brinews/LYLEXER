/*
 * Parser
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

class Parser {
  private:
    LexicalAnalyzer lexer;

    void type_error(const char *mesg);
    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();

    TokenType def_type(Token type);
    TokenType get_type(Token tok);
    void resolve_variable(Token tok);

    bool isDclType;
    std::vector<Token> dclTypeIds; // store dcl ids
    std::vector<Token> types;  // store dcl types
    std::vector<Token> dclVarIds; // store dcl ids
    std::vector<Token> vars; // store dcl types
    int top;

    Token left; // type mismatch checker
    std::vector<Token> right;

    std::vector<Token> rel;// relop

    std::vector<Token> used; // all variables used record

    int get_def(Token ref);
    void show_used_list();

    void parse_program();
    void parse_scope();
    void parse_scope_list();
    void parse_declaration();
    void parse_type_decl();
    void parse_type_name();
    void parse_var_decl();
    void parse_id_list();
    void parse_stmt_list();
    void parse_stmt();
    void parse_assign_stmt();
    void parse_while_stmt();
    void parse_expr();
    void parse_term();
    void parse_factor();
    void parse_condition();
    void parse_primary();
    void parse_relop();

  public:
    void ParseInput();
};

#endif

