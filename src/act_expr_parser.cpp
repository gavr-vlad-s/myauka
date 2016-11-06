/*
    Файл:    act_expr_parser.cpp
    Создан:  13 декабря 2015г. в 09:05 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/act_expr_parser.h"
#include "../include/belongs.h"
// #include "../include/idx_to_string.h"
//#include "test_expr_scaner.h"
#include <map>
#include <cstdio>
#include <cstdlib>

Act_expr_parser::Act_expr_parser(Expr_scaner_ptr         esc,
                                 const Errors_and_tries& et,
                                 std::shared_ptr<Scope>  scope){
    esc_         = esc;
    scope_       = scope;
    et_          = et;
    parser_stack = Multipop_stack<Stack_elem>();
}

Terminal lexem2terminal(const Expr_lexem_info& l){
    switch(l.code){
        case Nothing: case UnknownLexem:
            return End_of_text;
        case Action:
            return Term_a;
        case Or:
            return Term_b;
        case Kleene_closure ... Optional_member:
            return Term_c;
        case Class_Latin ... Class_nsq: case Character:
            return Term_d;
        case Begin_expression:
            return Term_p;
        case End_expression:
            return Term_q;
        case Opened_round_brack:
            return Term_LP;
        case Closed_round_brack:
            return Term_RP;
        default:
            ;
    }
    return Term_d;
}

/* Правила грамматики:
 *
 * ---------------------------------------------
 * | Номер правила | Правило    | Имя правила  |
 * |--------------------------------------------
 * | (0)           | S -> pTq   | S_is_pTq     |
 * | (1)           | T -> TbE   | T_is_TbE     |
 * | (2)           | T -> E     | T_is_E       |
 * | (3)           | E -> EF    | E_is_EF      |
 * | (4)           | E -> F     | E_is_F       |
 * | (5)           | F -> Gc    | F_is_Gc      |
 * | (6)           | F -> G     | F_is_G       |
 * | (7)           | G -> Ha    | G_is_Ha      |
 * | (8)           | G -> H     | G_is_H       |
 * | (9)           | H -> d     | H_is_d       |
 * | (10)          | H -> (T)   | H_is_LP_T_RP |
 * ---------------------------------------------
 *
 * В этой грамматике под a понимается $имя_действия,
 * под b --- оператор |, под c --- унарные операторы ? * +,
 * под d --- символ или класс символов, под p --- { (открывающая
 * фигурная скобка), под q --- } (закрывающая фигурная скобка).
 */

Act_expr_parser::Rule_info Act_expr_parser::rules[] = {
    {Nt_S, 3}, {Nt_T, 3}, {Nt_T, 1}, {Nt_E, 2}, {Nt_E, 1}, {Nt_F, 2},
    {Nt_F, 1}, {Nt_G, 2}, {Nt_G, 1}, {Nt_H, 1}, {Nt_H, 3}
};

#define ANY ((uint8_t)(-1))
struct GOTO_entry{
    uint8_t from;
    uint8_t to;
};

GOTO_entry goto_S[] = {
    {ANY, 1}
};

GOTO_entry goto_T[] = {
    {9, 15}, {ANY, 3}
};

GOTO_entry goto_E[] = {
    {10, 16}, {ANY, 4}
};

GOTO_entry goto_F[] = {
    {4, 12}, {16, 12}, {ANY, 5}
};

GOTO_entry goto_G[] = {
    {ANY, 6}
};

GOTO_entry goto_H[] = {
    {ANY, 7}
};

GOTO_entry* goto_table[] = {
    goto_S, goto_T, goto_E, goto_F, goto_G, goto_H
};

void Act_expr_parser::shift(size_t shifted_state, Expr_lexem_info e){
    Stack_elem selem;
    selem.st_num   = shifted_state;
    selem.attr.eli = e;
    parser_stack.push(selem);
    (this->*checker)(e);
//     printf("shift %zu\n", shifted_state); /* отладочная печать*/
}

void Act_expr_parser::reduce(Rule r){
    reduce_without_back(r);
    esc_->back();
}

size_t next_state(size_t s, Non_terminal n){
    size_t cs;
    GOTO_entry  current_entry;
    GOTO_entry* goto_for_n = goto_table[n];
    while((cs = (current_entry = *goto_for_n++).from) != ANY){
        if(cs == s){
            return current_entry.to;
        }
    }
    goto_for_n--;
    return goto_for_n -> to;
}

// static const char* rule_str[] = {
    // "S->pTq", "T->TbE", "T->E",
    // "E->EF",  "E->F",   "F->Gc",
    // "F->G",   "G->Ha",  "G->H",
    // "H->d",   "H->(T)"
// };
// static const size_t states_for_shift = 1 << 2  | 1 << 8  | 1 << 9  |
                                       // 1 << 10 | 1 << 11 | 1 << 13 |
                                       // 1 << 14 | 1 << 17;
// void print_stack_elem(Stack_elem se){
    // if(belongs(se.st_num, states_for_shift)){
        // printf("{st_num = %zu, attr : eli = {", se.st_num);
        // print_expr_lexem(se.attr.eli); printf("}}");
    // }else{
        // printf("{st_num = %zu, attr : indeces = {begin_index = "
               // "%zu, end_index = %zu}}", se.st_num,
               // se.attr.indeces.begin_index, se.attr.indeces.end_index);
    // }
// }

void Act_expr_parser::reduce_without_back(Rule r){
    size_t rule_len = rules[r].len;
    parser_stack.get_elems_from_top(rule_body, rule_len);
    generate_command(r);

    Stack_elem se;
    se.attr    = (this->*attrib_calc[r])();
    parser_stack.multi_pop(rule_len);
    Stack_elem top_elem = parser_stack.top();
    se.st_num           = next_state(top_elem.st_num, rules[r].nt);
    // puts("Calculated stack elem is");
    // print_stack_elem(se);
    parser_stack.push(se);
    // printf("\n");
    // printf("reduce %s\n", rule_str[r]);
}

#define ERROR     {Act_error, 0}
#define SHIFT(t)  {Act_shift, t}
#define REDUCE(r) {Act_reduce, r}
#define ACCESS    {Act_OK, 0}

//using Terminal_to_action = std::map<Terminal, Parser_action_info>;
using State_and_terminal  = std::pair<size_t, Terminal>;
using Parser_action_table = std::map<State_and_terminal, Parser_action_info>;

const Parser_action_table action_table = {
    {{0,Term_p},SHIFT(2)},              {{1,End_of_text},ACCESS},
    {{2,Term_d},SHIFT(8)},              {{2,Term_LP},SHIFT(9)},
    {{3,Term_b},SHIFT(10)},             {{3,Term_q},SHIFT(11)},
    {{4,Term_d},SHIFT(8)},              {{4,Term_LP},SHIFT(9)},
    {{4,Term_b},REDUCE(T_is_E)},        {{4,Term_q},REDUCE(T_is_E)},
    {{4,Term_RP},REDUCE(T_is_E)},       {{5,Term_b},REDUCE(E_is_F)},
    {{5,Term_d},REDUCE(E_is_F)},        {{5,Term_q},REDUCE(E_is_F)},
    {{5,Term_LP},REDUCE(E_is_F)},       {{5,Term_RP},REDUCE(E_is_F)},
    {{6,Term_b},REDUCE(F_is_G)},        {{6,Term_d},REDUCE(F_is_G)},
    {{6,Term_q},REDUCE(F_is_G)},        {{6,Term_LP},REDUCE(F_is_G)},
    {{6,Term_RP},REDUCE(F_is_G)},       {{6,Term_c},SHIFT(13)},
    {{7,Term_a},SHIFT(14)},             {{7,Term_b},REDUCE(G_is_H)},
    {{7,Term_c},REDUCE(G_is_H)},        {{7,Term_d},REDUCE(G_is_H)},
    {{7,Term_q},REDUCE(G_is_H)},        {{7,Term_LP},REDUCE(G_is_H)},
    {{7,Term_RP},REDUCE(G_is_H)},       {{8,Term_a},REDUCE(H_is_d)},
    {{8,Term_b},REDUCE(H_is_d)},        {{8,Term_c},REDUCE(H_is_d)},
    {{8,Term_d},REDUCE(H_is_d)},        {{8,Term_q},REDUCE(H_is_d)},
    {{8,Term_LP},REDUCE(H_is_d)},       {{8,Term_RP},REDUCE(H_is_d)},
    {{9,Term_d},SHIFT(8)},              {{9,Term_LP},SHIFT(9)},
    {{10,Term_d},SHIFT(8)},             {{10,Term_LP},SHIFT(9)},
    {{11,End_of_text},REDUCE(S_is_pTq)},{{12,Term_b},REDUCE(E_is_EF)},
    {{12,Term_d},REDUCE(E_is_EF)},      {{12,Term_q},REDUCE(E_is_EF)},
    {{12,Term_LP},REDUCE(E_is_EF)},     {{12,Term_RP},REDUCE(E_is_EF)},
    {{13,Term_b},REDUCE(F_is_Gc)},      {{13,Term_d},REDUCE(F_is_Gc)},
    {{13,Term_q},REDUCE(F_is_Gc)},      {{13,Term_LP},REDUCE(F_is_Gc)},
    {{13,Term_RP},REDUCE(F_is_Gc)},     {{14,Term_b},REDUCE(G_is_Ha)},
    {{14,Term_d},REDUCE(G_is_Ha)},      {{14,Term_q},REDUCE(G_is_Ha)},
    {{14,Term_LP},REDUCE(G_is_Ha)},     {{14,Term_RP},REDUCE(G_is_Ha)},
    {{14,Term_c},REDUCE(G_is_Ha)},      {{15,Term_b},SHIFT(10)},
    {{15,Term_RP},SHIFT(17)},           {{16,Term_d},SHIFT(8)},
    {{16,Term_LP},SHIFT(9)},            {{16,Term_b},REDUCE(T_is_TbE)},
    {{16,Term_q},REDUCE(T_is_TbE)},     {{16,Term_RP},REDUCE(T_is_TbE)},
    {{17,Term_a},REDUCE(H_is_LP_T_RP)}, {{17,Term_b},REDUCE(H_is_LP_T_RP)},
    {{17,Term_c},REDUCE(H_is_LP_T_RP)}, {{17,Term_d},REDUCE(H_is_LP_T_RP)},
    {{17,Term_q},REDUCE(H_is_LP_T_RP)}, {{17,Term_LP},REDUCE(H_is_LP_T_RP)},
    {{17,Term_RP},REDUCE(H_is_LP_T_RP)}
};

void Act_expr_parser::checker_for_number_expr(Expr_lexem_info e){
    if(belongs(e.code, 1ULL << Class_ndq | 1ULL << Class_nsq)){
        printf("Ошибка в строке %zu: в регулярном выражении для чисел "
               "недопустимы классы символов [:nsq:] и [:ndq:].\n",
               esc_->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }
}

void Act_expr_parser::checker_for_string_expr(Expr_lexem_info e){
}

void Act_expr_parser::compile(Command_buffer& buf, Number_or_string kind_of_expr){
    checker =
        (kind_of_expr == Number_expr) ? &Act_expr_parser::checker_for_number_expr :
        &Act_expr_parser::checker_for_string_expr;
    buf_ = buf;

    Stack_elem initial_elem;
    initial_elem.st_num                   = 0;
    initial_elem.attr.indeces.begin_index = 0;
    initial_elem.attr.indeces.end_index   = 0;
    parser_stack.push(initial_elem);

    for( ; ; ){
        eli_ = esc_->current_lexem();
        t = lexem2terminal(eli_);
        // printf("Current lexem is\n");
        // print_expr_lexem(eli_);
        //system("pause");
        current_state = parser_stack.top().st_num;
//         printf("\nCurrent state is %zu\n", current_state);
        //system("pause");
        // Terminal_to_action ta = action_table[current_state];
        // Terminal_to_action::iterator it = ta.find(t);
        auto it = action_table.find({current_state, t});
        Parser_action_info pai;
        if(it != action_table.end()){
            pai = it->second;
        }else{
            pai = (this->*error_hadler[current_state])();
        }
        switch(pai.kind){
            case Act_reduce:
                reduce(static_cast<Rule>(pai.arg));
                break;
            case Act_shift:
                shift(pai.arg, eli_);
                break;
            case Act_reduce_without_back:
                reduce_without_back(static_cast<Rule>(pai.arg));
                break;
            case Act_OK:
//                 puts("access");
                buf = buf_;
                esc_->back();
                return;
        }
//         puts("*******************");
    }
}

Act_expr_parser::Attrib_calculator Act_expr_parser::attrib_calc[] = {
    &Act_expr_parser::attrib_by_S_is_pTq,
    &Act_expr_parser::attrib_by_T_is_TbE,
    &Act_expr_parser::attrib_by_T_is_E,
    &Act_expr_parser::attrib_by_E_is_EF,
    &Act_expr_parser::attrib_by_E_is_F,
    &Act_expr_parser::attrib_by_F_is_Gc,
    &Act_expr_parser::attrib_by_F_is_G,
    &Act_expr_parser::attrib_by_G_is_Ha,
    &Act_expr_parser::attrib_by_G_is_H,
    &Act_expr_parser::attrib_by_H_is_d,
    &Act_expr_parser::attrib_by_H_is_LP_T_RP
};

void Act_expr_parser::generate_command(Rule r){
    Command            com;
    Id_scope::iterator it;
    size_t             act_index;
    size_t             min_index;
    size_t             max_index;
    switch(r){
        case T_is_TbE:
            com.name        = Cmd_or;
            com.args.first  = rule_body[0].attr.indeces.end_index;
            com.args.second = rule_body[2].attr.indeces.end_index;
            com.action_name = 0;
            buf_.push_back(com);
            break;

        case E_is_EF:
            com.name        = Cmd_concat;
            com.args.first  = rule_body[0].attr.indeces.end_index;
            com.args.second = rule_body[1].attr.indeces.end_index;
            com.action_name = 0;
            buf_.push_back(com);
            break;

        case F_is_Gc:
            com.name =
                static_cast<Command_name>(rule_body[1].attr.eli.code -
                                          Kleene_closure + Cmd_Kleene);
            com.args.first = rule_body[0].attr.indeces.end_index;
            com.args.second = 0;
            com.action_name = 0;
            buf_.push_back(com);
            break;

        case H_is_d:
            if(Character == rule_body[0].attr.eli.code){
                com.name        = Cmd_char_def;
                com.c           = rule_body[0].attr.eli.c;
            }else{
                com.name = Cmd_char_class_def;
                com.cls  = static_cast<Char_class>(
                    rule_body[0].attr.eli.code - Class_Latin);
            }
            com.action_name = 0;
            buf_.push_back(com);
            break;

        case G_is_Ha:
            /* Если действие a ещё не определено, то выдаём сообщение
             * об ошибке и считаем, что никакого действия не задано.
             * В противном случае записываем индекс имени действия. */
            act_index = rule_body[1].attr.eli.action_name_index;
            it        = scope_->idsc.find(act_index);
            if(it == scope_->idsc.end()){
                printf("Действие ");
                et_.ids_trie->print(act_index);
                printf(" в строке %zu не определено.\n",
                       esc_->lexem_begin_line_number());
                et_.ec -> increment_number_of_errors();
                return;
            } else if(it->second.kind != Action_name){
                printf("Идентификатор ");
                et_.ids_trie->print(act_index);
                printf(" в строке %zu именем действия не является.\n",
                       esc_->lexem_begin_line_number());
                et_.ec -> increment_number_of_errors();
                return;
            };
            min_index = rule_body[0].attr.indeces.begin_index;
            max_index = rule_body[0].attr.indeces.end_index + 1;
            for(size_t i = min_index; i < max_index; i++){
                buf_[i].action_name = act_index;
            }
            break;

        default:
            ;
    }
}

Attributes Act_expr_parser::attrib_by_S_is_pTq(){
    return rule_body[1].attr;
}

Attributes Act_expr_parser::attrib_by_T_is_TbE(){
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_T_is_E(){
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_E_is_EF(){
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_E_is_F(){
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_F_is_Gc(){
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_F_is_G(){
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_G_is_Ha(){
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_G_is_H(){
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_H_is_d(){
    Attributes s;
    s.indeces.begin_index = s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_H_is_LP_T_RP(){
    return rule_body[1].attr;
}

Act_expr_parser::Error_handler Act_expr_parser::error_hadler[] = {
    &Act_expr_parser::state00_error_handler, // 0  +
    &Act_expr_parser::state01_error_handler, // 1  +
    &Act_expr_parser::state02_error_handler, // 2  +
    &Act_expr_parser::state03_error_handler, // 3  +
    &Act_expr_parser::state04_error_handler, // 4  +
    &Act_expr_parser::state04_error_handler, // 5  +
    &Act_expr_parser::state06_error_handler, // 6  +
    &Act_expr_parser::state07_error_handler, // 7  +
    &Act_expr_parser::state07_error_handler, // 8  +
    &Act_expr_parser::state02_error_handler, // 9  +
    &Act_expr_parser::state02_error_handler, // 10 +
    &Act_expr_parser::state11_error_handler, // 11 +
    &Act_expr_parser::state04_error_handler, // 12 +
    &Act_expr_parser::state04_error_handler, // 13 +
    &Act_expr_parser::state06_error_handler, // 14 +
    &Act_expr_parser::state15_error_handler, // 15 +
    &Act_expr_parser::state04_error_handler, // 16 +
    &Act_expr_parser::state07_error_handler  // 17 +
};

/* В этом массиве собраны правила, по которым выполняется свёртка в
 * функциях обработки ошибок. Номер элемента массива --- номер
 * текущего состояния синтаксического анализатора. Если для
 * какого--либо состояния в соответствующей функции обработки
 * ошибок выполняется не свёртка, то элемент данного массива с
 * соотвествующим индексом равен (-1). */
char reduce_rules[] = {
    -1,       -1,          -1,      -1,
    T_is_E,   E_is_F,      F_is_G,  G_is_H,
    H_is_d,   -1,          -1,      S_is_pTq,
    E_is_EF,  F_is_Gc,     G_is_Ha, -1,
    T_is_TbE, H_is_LP_T_RP
};

Parser_action_info Act_expr_parser::state00_error_handler(){
    printf("В строке %zu ожидается открывающая фигурная скобка.\n",
           esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(eli_.code != Closed_round_brack){
        esc_->back();
    }
    eli_.code = Begin_expression;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 2;
    return pa;
}

Parser_action_info Act_expr_parser::state01_error_handler(){
    Parser_action_info pa;
    pa.kind = Act_OK; pa.arg = 0;
    return pa;
}

Parser_action_info Act_expr_parser::state02_error_handler(){
    printf("В строке %zu ожидается символ, класс символов, или "
           "открывающая круглая скобка.\n",
           esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    esc_->back();
    eli_.code = Character;
    eli_.c    = 'a';
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 8;
    return pa;
}

Parser_action_info Act_expr_parser::state03_error_handler(){
    printf("В строке %zu ожидается оператор | либо закрывающая "
           "фигурная скобка.\n", esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Term_p){
        esc_->back();
    }
    eli_.code = Or;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 10;
    return pa;
//     shift(10, eli_);
}

Parser_action_info Act_expr_parser::state04_error_handler(){
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Term_a:
            printf("Неожиданное действие в строке %zu.\n",
                   esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case Term_c:
            printf("Неожиданный постфиксный оператор в строке %zu.\n",
                   esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case End_of_text:
            printf("Неожиданный конец текста в строке %zu.\n",
                   esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case Term_p:
            printf("Неожиданная открывающая фигурная скобка в "
                   "строке %zu.\n", esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info Act_expr_parser::state06_error_handler(){
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Term_a:
            printf("Неожиданное действие в строке %zu.\n",
                   esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        case Term_p:
            printf("Неожиданная открывающая фигурная скобка в "
                   "строке %zu.\n", esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        case End_of_text:
            printf("Неожиданный конец текста в строке %zu.\n",
                   esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info Act_expr_parser::state07_error_handler(){
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    if(Term_p == t){
        printf("Неожиданная открывающая фигурная скобка в "
               "строке %zu.\n", esc_->lexem_begin_line_number());
        pa.kind = Act_reduce_without_back; pa.arg = r;
    }else{
        printf("Неожиданный конец текста в строке %zu.\n",
               esc_->lexem_begin_line_number());
        pa.kind = Act_reduce_without_back; pa.arg = r;
    }
    et_.ec->increment_number_of_errors();
    return pa;
}

Parser_action_info Act_expr_parser::state11_error_handler(){
    Parser_action_info pa;
    pa.kind = Act_reduce; pa.arg = S_is_pTq;
    return pa;
}

Parser_action_info Act_expr_parser::state15_error_handler(){
    printf("В строке %zu ожидается оператор | либо закрывающая "
           "круглая скобка.\n", esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Term_p){
        esc_->back();
    }
    eli_.code = Or;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 10;
    return pa;
}