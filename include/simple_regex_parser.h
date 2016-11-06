/* 
    Файл:    simple_regex_parser.h
    Создан:  14 декабря 2015г. в 15:25 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef SIMPLE_REGEX_PARSER_H
#define SIMPLE_REGEX_PARSER_H

#include "../include/expr_scaner.h"
#include "../include/command.h"
// #include "scope.h"

class Simple_regex_parser{
public:
    Simple_regex_parser(Expr_scaner_ptr esc,
                        const Errors_and_tries& et) :
        et_(et), esc_(esc) {};
    Simple_regex_parser() = default;
    Simple_regex_parser(const Simple_regex_parser& orig) = default;
    ~Simple_regex_parser() = default;

    void compile(Command_buffer& buf);
private:
    Errors_and_tries et_;
    Expr_scaner_ptr  esc_;

    size_t number_of_ors;
    enum Parser_state{
        State_begin_expr, State_begin_concat,
        State_concat,     State_end_expr
    };
    Parser_state state;
    
    typedef void (Simple_regex_parser::*Proc)(Command_buffer& buf);
    
    static Proc state_proc[];

    void state_begin_expr_proc(Command_buffer& buf);
    void state_begin_concat_proc(Command_buffer& buf);
    void state_concat_proc(Command_buffer& buf);
    void state_end_expr_proc(Command_buffer& buf);

    void write_char_or_char_class(Command_buffer& buf);
    void write_or_command(Command_buffer& buf);
    size_t arg1, arg2;
    size_t last_index;
    
    Expr_lexem_info eli;
    Expr_lexem_code elc;
};
#endif