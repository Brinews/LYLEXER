/*
 * Parser
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"

#include <stdio.h>

using namespace std;

#ifdef DBG
#define DEBUG() do { \
    printf("%d %20s\n", __LINE__, __FUNCTION__); \
} while (0)
#else
#define DEBUG()
#endif

void Parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

void Parser::type_error(const char *err_msg)
{
    cout << err_msg << endl;
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type) {
        //cout << t.token_type << " , " << t.lexeme << ": " << expected_type << endl;
        DEBUG();
        syntax_error();
    }
    return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

// Parsing

void Parser::parse_program()
{
    // program -> scope
    
    parse_scope();
}

void Parser::parse_scope()
{
    // scope -> { scope_list }
    
    expect(LBRACE);
    parse_scope_list();
    expect(RBRACE);
}


void Parser::parse_scope_list()
{
    // scope_list -> stmt
    // scope_list -> scope
    // scope_list -> declaration
    // scope_list -> stmt scope_list
    // scope_list -> scope scope_list
    // scope_list -> declaration scope_list

    // TODO
    Token t = peek();

    if (t.token_type == WHILE || t.token_type == ID) {
        parse_stmt();
        parse_scope_list();
    }
    else if (t.token_type == TYPE || t.token_type == VAR) {
        parse_declaration();
        parse_scope_list();
    } 
    else if (t.token_type == LBRACE) {
        parse_scope();
        parse_scope_list();
    }
}

void Parser::parse_declaration()
{
    // declaration -> type_decl
    // declaration -> var_decl

    Token t = peek();
    if (t.token_type == TYPE)
    	parse_type_decl();
    if (t.token_type == VAR)
    	parse_var_decl();
}

void Parser::parse_type_decl()
{
    // type_decl -> TYPE id_list COLON type_name SEMICOLON

    isDclType = true;
    top = dclTypeIds.size(); // record the current size

    expect(TYPE);
    parse_id_list();
    expect(COLON);
    parse_type_name();
    expect(SEMICOLON);

    // check new added dcl types
    for (int i = top; i < dclTypeIds.size(); i++) {
        Token t = dclTypeIds[i];
        for (int j = 0; j < i; j++) {
            // check type declared more than once, code 1.1
            if (t.lexeme == dclTypeIds[j].lexeme) {
                string err = "ERROR CODE 1.1 " + t.lexeme;
                type_error(err.c_str());
            }
        }
        for (int k = 0; k < dclVarIds.size(); k++) {
            if (dclTypeIds[i].lexeme == dclVarIds[k].lexeme) {
                string err = "ERROR CODE 2.2 " + dclTypeIds[i].lexeme;
                type_error(err.c_str());
            }
        }
        if (def_type(types[i]) == ERROR) {
            string err = "ERROR CODE 1.4 " + dclTypeIds[i].lexeme;
            type_error(err.c_str());
        }
    }
}

void Parser::parse_type_name()
{
    // type_name -> REAL
    // type_name -> INT
    // type_name -> BOOLEAN
    // type_name -> STRING
    // type_name -> LONG
    // type_name -> ID

    // TODO
    Token t = peek();

    if (t.token_type == REAL
          || t.token_type == INT
          || t.token_type == BOOLEAN
          || t.token_type == STRING
          || t.token_type == LONG
          || t.token_type == ID) {
        // read a token
        lexer.GetToken();
        if (isDclType) { // types
            for (int i = top; i < dclTypeIds.size(); i++) {
                types.push_back(t);
            }
        } else { // vars
            for (int i = top; i < dclVarIds.size(); i++) {
                vars.push_back(t);
            }
        }
    } else {
        DEBUG();
        syntax_error();
    }
}

void Parser::parse_var_decl()
{
    // var_decl -> VAR id_list COLON type_name SEMICOLON

    isDclType = false;
    top = dclVarIds.size();

    expect(VAR);
    parse_id_list();
    expect(COLON);
    parse_type_name();
    expect(SEMICOLON);

    // check new added vars
    for (int i = top; i < dclVarIds.size(); i++) {
        for (int j = 0; j < dclTypeIds.size(); j++) {
            if (dclVarIds[i].lexeme == dclTypeIds[j].lexeme) {
                string err = "ERROR CODE 1.2 " + dclVarIds[i].lexeme;
                type_error(err.c_str());
            }
        }
        for (int k = 0; k < i; k++) {
            if (dclVarIds[i].lexeme == dclVarIds[k].lexeme) {
                string err = "ERROR CODE 2.1 " + dclVarIds[i].lexeme;
                type_error(err.c_str());
            }
        }
        if (vars[i].token_type == ID) {
            if (def_type(vars[i]) == ERROR) {
                string err = "ERROR CODE 1.4 " + dclTypeIds[i].lexeme;
                type_error(err.c_str());
            }

            bool foundVar = false;
            for (int m = 0; m < top; m++) {
                if (dclVarIds[m].lexeme == vars[i].lexeme) {
                    foundVar = true;
                    break;
                }
            }
            if (foundVar == true) {
                string err = "ERROR CODE 2.3 " + dclVarIds[i].lexeme;
                type_error(err.c_str());
            }
        }
    }

    // record used type
    for (int i = top; i < top+1; i++) {
        if (vars[i].token_type == ID) {
            used.push_back(vars[i]);
        }
    }
}

void Parser::parse_id_list()
{
    // id_list -> ID
    // id_list -> ID COMMA id_list

    // TODO
    Token t = expect(ID);
    if (isDclType == true) {
        dclTypeIds.push_back(t);
    } else {
        dclVarIds.push_back(t);
    }

    t = peek();
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
}

void Parser::parse_stmt_list()
{
    // stmt_list -> stmt
    // stmt_list -> stmt stmt_list
    
    parse_stmt();
    Token t = peek();
    if (t.token_type == WHILE || t.token_type == ID)
    {
        // stmt_list -> stmt stmt_list
        parse_stmt_list();
    }
    else if (t.token_type == RBRACE)
    {
        // stmt_list -> stmt
    }
    else
    {
        DEBUG();
        syntax_error();
    }
}

void Parser::parse_stmt()
{
    // stmt -> assign_stmt
    // stmt -> while_stmt
 
    // TODO
    Token t = peek();

    if (t.token_type == ID) {
        parse_assign_stmt();
    }
    else if (t.token_type == WHILE) {
        parse_while_stmt();
    }
    else {
        DEBUG();
        syntax_error();
    }
    
}

TokenType Parser::def_type(Token type)
{
    if (type.token_type != ID) return type.token_type;

    for (int i = 0; i < dclTypeIds.size(); i++) {
        if (dclTypeIds[i].lexeme == type.lexeme) {
            if (types[i].token_type != ID) return types[i].token_type;
            else {
                return def_type(types[i]);
            }
        }
    }

    return ERROR;
}

TokenType Parser::get_type(Token tok)
{
    if (tok.token_type == NUM) return INT;
    if (tok.token_type == REALNUM) return REAL;

    if (tok.token_type != ID) return tok.token_type;

    for (int i = 0; i < dclVarIds.size(); i++) {
        if (dclVarIds[i].lexeme == tok.lexeme) {
            return def_type(vars[i]);
        }
    }

    return ERROR;
}

void Parser::resolve_variable(Token tok)
{
    if (tok.token_type != ID) return;

    bool isType = false;
    for (int i = 0; i < dclTypeIds.size(); i++) {
        if (dclTypeIds[i].lexeme == tok.lexeme) {
            isType = true;
            break;
        }
    }
    if (isType == true) {
        string err = "ERROR CODE 1.3 " + tok.lexeme;
        type_error(err.c_str());
    }
}

void Parser::parse_assign_stmt()
{
    // assign_stmt -> ID EQUAL expr SEMICOLON

    // TODO
    left = expect(ID); // get left side of an assignment
    expect(EQUAL);

    right.clear(); // prepare right side
    parse_expr();

    expect(SEMICOLON);

    // variable resolving error
    resolve_variable(left);

    for (int i = 0; i < right.size(); i++) {
        resolve_variable(right[i]);
        
        if (get_type(right[i]) == ERROR) {
            string err = "ERROR CODE 2.4 " + right[i].lexeme;
            type_error(err.c_str());
        }
    }

    // type mismatch checker
    for (int i = 0; i < right.size(); i++) {
        for (int j = i+1; j < right.size(); j++) {
            if (get_type(right[i]) != get_type(right[j])) {
                char buf[10];
                sprintf(buf, "%d", left.line_no);
                string err = "TYPE MISMATCH ";
                err += buf;
                err += " C2";
                type_error(err.c_str());
            }
        }
    }

    TokenType ty = get_type(left);

    if (ty == ERROR) {
        string err = "ERROR CODE 2.4 " + left.lexeme;
        type_error(err.c_str());
    }

    for (int i = 0; i < right.size(); i++) {
        if (get_type(right[i]) != ty) {
            char buf[10];
            sprintf(buf, "%d", left.line_no);
            string err = "TYPE MISMATCH ";
            err += buf;
            err += " C1";
            type_error(err.c_str());
        }
    }

    // record used asgn
    if (left.token_type == ID) {
        used.push_back(left);
    }

    for (int i = 0; i < right.size(); i++) {
        if (right[i].token_type == ID) {
            used.push_back(right[i]);
        }
    }
}

void Parser::parse_while_stmt()
{
   // while_stmt -> WHILE condition LBRACE stmt list RBRACE

    // TODO
    expect(WHILE);
    parse_condition();
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

void Parser::parse_expr()
{
    // expr -> term 
    // expr -> term + expr

    // TODO
    parse_term();
    Token t = peek();

    if (t.token_type == PLUS) {
        expect(PLUS);
        parse_expr();
    }
}

void Parser::parse_term()
{
    // term -> factor
    // term -> factor MULT term

    // TODO
    parse_factor();
    Token t = peek();
    if (t.token_type == MULT) {
        expect(MULT);
        parse_term();
    }
}

void Parser::parse_factor()
{
    // factor -> LPAREN expr RPAREN
    // factor -> NUM
    // factor -> REALNUM
    // factor -> ID

    // TODO
    Token t = peek();
    if (t.token_type == NUM 
            || t.token_type == REALNUM
            || t.token_type == ID) {
        lexer.GetToken();
        right.push_back(t);
    }
    else if (t.token_type == LPAREN) {
        expect(LPAREN);
        parse_expr();
        expect(RPAREN);
    }
    else {
        DEBUG();
        syntax_error();
    }
}

void Parser::parse_condition()
{
    // condition -> ID
    // condition -> primary relop primary

    // TODO
    Token t = peek();
    Token cond;
    bool relop = false;
    rel.clear();

    if (t.token_type == ID) {
        cond = expect(ID);
        t = peek();
        if (t.token_type != LBRACE) {
            rel.push_back(cond);
            parse_relop();
            parse_primary();
            relop = true;
        }
    }
    else {
        parse_primary();
        parse_relop();
        parse_primary();
        relop = true;
    }

    // C3 check
    if (relop == true) {
        resolve_variable(rel[0]);
        resolve_variable(rel[1]);

        if (get_type(rel[0]) != get_type(rel[1])) {
            char buf[10];
            sprintf(buf, "%d", rel[0].line_no);
            string err = "TYPE MISMATCH ";
            err += buf;
            err += " C3";
            type_error(err.c_str());
        }

        // record used cond
        if (rel[0].token_type == ID) {
            used.push_back(rel[0]);
        }
        if (rel[0].token_type == ID) {
            used.push_back(rel[0]);
        }
    } else { // C4 check
        resolve_variable(cond);

        if (cond.token_type != BOOLEAN) {
            char buf[10];
            sprintf(buf, "%d", cond.line_no);
            string err = "TYPE MISMATCH ";
            err += buf;
            err += " C4";
            type_error(err.c_str());
        }

        // record used cond
        if (cond.token_type == ID) {
            used.push_back(cond);
        }
    }
}

void Parser::parse_primary()
{
    // primary -> ID
    // primary -> NUM
    // primary -> REALNUM
    // TODO

    Token t = peek();

    if (t.token_type == ID
            || t.token_type == NUM
            || t.token_type == REALNUM) {
        lexer.GetToken();
        rel.push_back(t);
    }
    else {
        DEBUG();
        syntax_error();
    }
}

void Parser::parse_relop()
{
    // relop -> GREATER
    // relop -> GTEQ
    // relop -> LESS
    // relop -> NOTEQUAL
    // relop -> LTEQ

    // TODO
    Token t = peek();

    if (t.token_type == GREATER
            || t.token_type == GTEQ
            || t.token_type == LESS
            || t.token_type == NOTEQUAL
            || t.token_type == LTEQ) {
        lexer.GetToken();
    } else {
        DEBUG();
        syntax_error();
    }
}

int Parser::get_def(Token ref)
{
    for (int i = 0; i < dclTypeIds.size(); i++) {
        if (ref.lexeme == dclTypeIds[i].lexeme) {
            return dclTypeIds[i].line_no;
        }
    }

    for (int i = 0; i < dclVarIds.size(); i++) {
        if (ref.lexeme == dclVarIds[i].lexeme) {
            return dclVarIds[i].line_no;
        }
    }

    return -1;
}

void Parser::show_used_list()
{
    for (int i = 0; i < used.size(); i++) {
        cout << used[i].lexeme << " " << used[i].line_no << " " 
             << get_def(used[i]) << endl;
    }
}

void Parser::ParseInput()
{
    parse_program();
    expect(END_OF_FILE);

    show_used_list();
}

int main()
{
    Parser parser;

    parser.ParseInput();
}

