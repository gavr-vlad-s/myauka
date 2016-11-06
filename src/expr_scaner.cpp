/*
    Файл:    expr_scaner.cpp
    Создан:  13 декабря 2015г. в 09:05 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/expr_scaner.h"
#include "../include/belongs.h"
#include <cstdlib>
#include <cstdio>
#include "../include/search_char.h"
#include "../include/get_init_state.h"

enum Category : uint16_t {
    Spaces,            Other,             Action_name_begin,
    Action_name_body,  Delimiters,        Dollar,
    Backslash,         Opened_square_br,  After_colon,
    After_backslash,   Begin_expr,        End_expr
};

static const std::map<char32_t, uint16_t> categories_table = {
    {   1,    1},  {   2,    1},  {   3,    1},  {   4,    1},
    {   5,    1},  {   6,    1},  {   7,    1},  {   8,    1},
    {   9,    1},  {  10,    1},  {  11,    1},  {  12,    1},
    {  13,    1},  {  14,    1},  {  15,    1},  {  16,    1},
    {  17,    1},  {  18,    1},  {  19,    1},  {  20,    1},
    {  21,    1},  {  22,    1},  {  23,    1},  {  24,    1},
    {  25,    1},  {  26,    1},  {  27,    1},  {  28,    1},
    {  29,    1},  {  30,    1},  {  31,    1},  {  32,    1},
    {U'$',  544},  {U'(',  528},  {U')',  528},  {U'*',  528},
    {U'+',  528},  {U'0',    8},  {U'1',    8},  {U'2',    8},
    {U'3',    8},  {U'4',    8},  {U'5',    8},  {U'6',    8},
    {U'7',    8},  {U'8',    8},  {U'9',    8},  {U'?',  528},
    {U'A',   12},  {U'B',   12},  {U'C',   12},  {U'D',   12},
    {U'E',   12},  {U'F',   12},  {U'G',   12},  {U'H',   12},
    {U'I',   12},  {U'J',   12},  {U'K',   12},  {U'L',  268},
    {U'M',   12},  {U'N',   12},  {U'O',   12},  {U'P',   12},
    {U'Q',   12},  {U'R',  268},  {U'S',   12},  {U'T',   12},
    {U'U',   12},  {U'V',   12},  {U'W',   12},  {U'X',   12},
    {U'Y',   12},  {U'Z',   12},  {U'[',  640},  {U'\\', 576},
    {U']',  512},  {U'_',   12},  {U'a',   12},  {U'b',  268},
    {U'c',   12},  {U'd',  268},  {U'e',   12},  {U'f',   12},
    {U'g',   12},  {U'h',   12},  {U'i',   12},  {U'j',   12},
    {U'k',   12},  {U'l',  268},  {U'm',   12},  {U'n',  780},
    {U'o',  268},  {U'p',   12},  {U'q',   12},  {U'r',  268},
    {U's',   12},  {U't',   12},  {U'u',   12},  {U'v',   12},
    {U'w',   12},  {U'x',  268},  {U'y',   12},  {U'z',   12},
    {U'{', 1552},  {U'|',  528},  {U'}', 2576},
};

uint64_t Expr_scaner::get_categories_set(char32_t c){
    auto it = categories_table.find(c);
    if(it != categories_table.end()){
        return it->second;
    }else{
        return 1ULL << Other;
    }
}

/**
 * Элемент таблицы переходов автомата обработки классов символов
 */
struct Elem {
    /** Указатель на строку , состоящую из символов , по которым
        возможен переход. */
    char32_t*       symbols;
    /** код лексемы */
    Expr_lexem_code code;
    /** Если текущий символ совпадает с symbols[0], то
        выполняется переход в состояние first_state;
        если текущий символ совпадает с symbols[1], то
        выполняется переход в состояние first_state+1;
        если текущий символ совпадает с symbols[2], то
        выполняется переход в состояние first_state+2,
        и так далее. */
    uint16_t        first_state;
};

/* Для автомата обработки классов символов член state класса
 * Expr_scaner является индексом элемента в таблице переходов,
 * обозначеннойниже как a_classes_jump_table. */
static const Elem a_classes_jump_table[] = {
    {const_cast<char32_t*>(U"ae"),M_Class_Latin,  1},           // 0:   [:L...
    {const_cast<char32_t*>(U"t"), M_Class_Latin,  3},           // 1:   [:La...
    {const_cast<char32_t*>(U"t"), M_Class_Letter, 4},           // 2:   [:Le...
    {const_cast<char32_t*>(U"i"), M_Class_Latin,  5},           // 3:   [:Lat...
    {const_cast<char32_t*>(U"t"), M_Class_Letter, 6},           // 4:   [:Let...
    {const_cast<char32_t*>(U"n"), M_Class_Latin,  7},           // 5:   [:Lati...
    {const_cast<char32_t*>(U"e"), M_Class_Letter, 8},           // 6:   [:Lett...
    {const_cast<char32_t*>(U":"), M_Class_Latin,  9},           // 7:   [:Latin...
    {const_cast<char32_t*>(U"r"), M_Class_Letter, 10},          // 8:   [:Lette...
    {const_cast<char32_t*>(U"]"), M_Class_Latin,  11},          // 9:   [:Latin:...
    {const_cast<char32_t*>(U":"), M_Class_Letter, 12},          // 10:  [:Letter...
    {const_cast<char32_t*>(U""),  Class_Latin,    0},           // 11:  [:Latin:]
    {const_cast<char32_t*>(U"]"), M_Class_Letter, 13},          // 12:  [:Letter:...
    {const_cast<char32_t*>(U""),  Class_Letter,   0},           // 13:  [:Letter:]

    {const_cast<char32_t*>(U"u"), M_Class_Russian,15},          // 14:  [:R...
    {const_cast<char32_t*>(U"s"), M_Class_Russian,16},          // 15:  [:Ru...
    {const_cast<char32_t*>(U"s"), M_Class_Russian,17},          // 16:  [:Rus...
    {const_cast<char32_t*>(U"i"), M_Class_Russian,18},          // 17:  [:Russ...
    {const_cast<char32_t*>(U"a"), M_Class_Russian,19},          // 18:  [:Russi...
    {const_cast<char32_t*>(U"n"), M_Class_Russian,20},          // 19:  [:Russia...
    {const_cast<char32_t*>(U":"), M_Class_Russian,21},          // 20:  [:Russian...
    {const_cast<char32_t*>(U"]"), M_Class_Russian,22},          // 21:  [:Russian:...
    {const_cast<char32_t*>(U""),  Class_Russian,  0},           // 22:  [:Russian:]

    {const_cast<char32_t*>(U"d"), M_Class_bdigits,24},          // 23:  [:b...
    {const_cast<char32_t*>(U"i"), M_Class_bdigits,25},          // 24:  [:bd...
    {const_cast<char32_t*>(U"g"), M_Class_bdigits,26},          // 25:  [:bdi...
    {const_cast<char32_t*>(U"i"), M_Class_bdigits,27},          // 26:  [:bdig...
    {const_cast<char32_t*>(U"t"), M_Class_bdigits,28},          // 27:  [:bdigi...
    {const_cast<char32_t*>(U"s"), M_Class_bdigits,29},          // 28:  [:bdigit...
    {const_cast<char32_t*>(U":"), M_Class_bdigits,30},          // 29:  [:bdigits...
    {const_cast<char32_t*>(U"]"), M_Class_bdigits,31},          // 30:  [:bdigits:...
    {const_cast<char32_t*>(U""),  Class_bdigits,  0},           // 31:  [:bdigits:]

    {const_cast<char32_t*>(U"i"), M_Class_digits, 33},          // 32:  [:d...
    {const_cast<char32_t*>(U"g"), M_Class_digits, 34},          // 33:  [:di...
    {const_cast<char32_t*>(U"i"), M_Class_digits, 35},          // 34:  [:dig...
    {const_cast<char32_t*>(U"t"), M_Class_digits, 36},          // 35:  [:digi...
    {const_cast<char32_t*>(U"s"), M_Class_digits, 37},          // 36:  [:digit...
    {const_cast<char32_t*>(U":"), M_Class_digits, 38},          // 37:  [:digits...
    {const_cast<char32_t*>(U"]"), M_Class_digits, 39},          // 38:  [:digits:...
    {const_cast<char32_t*>(U""),  Class_digits,   0},           // 39:  [:digits:]

    {const_cast<char32_t*>(U"ae"),M_Class_latin,  41},          // 40:  [:l...
    {const_cast<char32_t*>(U"t"), M_Class_latin,  43},          // 41:  [:la...
    {const_cast<char32_t*>(U"t"), M_Class_letter, 44},          // 42:  [:le...
    {const_cast<char32_t*>(U"i"), M_Class_latin,  45},          // 43:  [:lat...
    {const_cast<char32_t*>(U"t"), M_Class_letter, 46},          // 44:  [:let...
    {const_cast<char32_t*>(U"n"), M_Class_latin,  47},          // 45:  [:lati...
    {const_cast<char32_t*>(U"e"), M_Class_letter, 48},          // 46:  [:lett...
    {const_cast<char32_t*>(U":"), M_Class_latin,  49},          // 47:  [:latin...
    {const_cast<char32_t*>(U"r"), M_Class_letter, 50},          // 48:  [:lette...
    {const_cast<char32_t*>(U"]"), M_Class_latin,  51},          // 49:  [:latin:...
    {const_cast<char32_t*>(U":"), M_Class_letter, 52},          // 50:  [:letter...
    {const_cast<char32_t*>(U""),  Class_latin,    0},           // 51:  [:latin:]
    {const_cast<char32_t*>(U"]"), M_Class_letter, 53},          // 52:  [:letter:...
    {const_cast<char32_t*>(U""),  Class_letter,   0},           // 53:  [:letter:]

    {const_cast<char32_t*>(U"ds"),M_Class_ndq,    55},          // 54:  [:n...
    {const_cast<char32_t*>(U"q"), M_Class_ndq,    57},          // 55:  [:nd...
    {const_cast<char32_t*>(U"q"), M_Class_nsq,    58},          // 56:  [:ns...
    {const_cast<char32_t*>(U":"), M_Class_ndq,    59},          // 57:  [:ndq...
    {const_cast<char32_t*>(U":"), M_Class_nsq,    60},          // 58:  [:nsq...
    {const_cast<char32_t*>(U"]"), M_Class_ndq,    61},          // 59:  [:ndq:...
    {const_cast<char32_t*>(U"]"), M_Class_nsq,    62},          // 60:  [:nsq:...
    {const_cast<char32_t*>(U""),  Class_ndq,      0},           // 61:  [:ndq:]
    {const_cast<char32_t*>(U""),  Class_nsq,      0},           // 62:  [:nsq:]

    {const_cast<char32_t*>(U"d"), M_Class_odigits,64},          // 63:  [:o...
    {const_cast<char32_t*>(U"i"), M_Class_odigits,65},          // 64:  [:od...
    {const_cast<char32_t*>(U"g"), M_Class_odigits,66},          // 65:  [:odi...
    {const_cast<char32_t*>(U"i"), M_Class_odigits,67},          // 66:  [:odig...
    {const_cast<char32_t*>(U"t"), M_Class_odigits,68},          // 67:  [:odigi...
    {const_cast<char32_t*>(U"s"), M_Class_odigits,69},          // 68:  [:odigit...
    {const_cast<char32_t*>(U":"), M_Class_odigits,70},          // 69:  [:odigits...
    {const_cast<char32_t*>(U"]"), M_Class_odigits,71},          // 70:  [:odigits:...
    {const_cast<char32_t*>(U""),  Class_odigits,  0,},          // 71:  [:odigits:]

    {const_cast<char32_t*>(U"u"), M_Class_russian,73},          // 72:  [:r...
    {const_cast<char32_t*>(U"s"), M_Class_russian,74},          // 73:  [:ru...
    {const_cast<char32_t*>(U"s"), M_Class_russian,75},          // 74:  [:rus...
    {const_cast<char32_t*>(U"i"), M_Class_russian,76},          // 75:  [:russ...
    {const_cast<char32_t*>(U"a"), M_Class_russian,77},          // 76:  [:russi...
    {const_cast<char32_t*>(U"n"), M_Class_russian,78},          // 77:  [:russia...
    {const_cast<char32_t*>(U":"), M_Class_russian,79},          // 78:  [:russian...
    {const_cast<char32_t*>(U"]"), M_Class_russian,80},          // 79:  [:russian:...
    {const_cast<char32_t*>(U""),  Class_russian,  0},           // 80:  [:russian:]

    {const_cast<char32_t*>(U"d"), M_Class_xdigits,82},          // 81:  [:x...
    {const_cast<char32_t*>(U"i"), M_Class_xdigits,83},          // 82:  [:xd...
    {const_cast<char32_t*>(U"g"), M_Class_xdigits,84},          // 83:  [:xdi...
    {const_cast<char32_t*>(U"i"), M_Class_xdigits,85},          // 84:  [:xdig...
    {const_cast<char32_t*>(U"t"), M_Class_xdigits,86},          // 85:  [:xdigi...
    {const_cast<char32_t*>(U"s"), M_Class_xdigits,87},          // 86:  [:xdigit...
    {const_cast<char32_t*>(U":"), M_Class_xdigits,88},          // 87:  [:xdigits...
    {const_cast<char32_t*>(U"]"), M_Class_xdigits,89},          // 88:  [:xdigits:...
    {const_cast<char32_t*>(U""),  Class_xdigits,  0}            // 89:  [:xdigits:]
};

Expr_scaner::Automaton_proc Expr_scaner::procs[] = {
    &Expr_scaner::start_proc,     &Expr_scaner::unknown_proc,
    &Expr_scaner::action_proc,    &Expr_scaner::delimiter_proc,
    &Expr_scaner::classes_proc,   &Expr_scaner::char_proc
};

Expr_scaner::Final_proc Expr_scaner::finals[] = {
    &Expr_scaner::none_final_proc,
    &Expr_scaner::unknown_final_proc,
    &Expr_scaner::action_final_proc,
    &Expr_scaner::delimiter_final_proc,
    &Expr_scaner::classes_final_proc,
    &Expr_scaner::char_final_proc
};

bool Expr_scaner::start_proc() {
    bool t = true;
    state = -1;
    /* Для автомата, обрабатывающего какую-либо лексему, состояние
     * с номером (-1) является состоянием, в котором происходит
     * инициализация этого автомата. */
    if(belongs(Spaces, char_categories)){
        loc->current_line += U'\n' == ch;
        return t;
    }
    lexem_begin_line = loc->current_line;
    if(belongs(Delimiters, char_categories)){
        automaton = A_delimiter; token.code = UnknownLexem;
        (loc->pcurrent_char)--;
    }else if(belongs(Dollar, char_categories)){
        automaton = A_action;  token.code = Action;
        buffer = U"";
    }else if(belongs(Opened_square_br, char_categories)){
        automaton = A_class, token.code = Character;
        token.c = U'[';
    }else if(belongs(Backslash, char_categories)){
         automaton = A_char; token.code = Character;
    }else if(belongs(Begin_expr, char_categories)){
        token.code = Begin_expression; t = false;
        (loc->pcurrent_char)++;
    }else if(belongs(End_expr, char_categories)){
        token.code = End_expression; t = false;
        (loc->pcurrent_char)++;
    }else{
        token.code = Character; token.c = ch; t = false;
        (loc->pcurrent_char)++;
    }
    return t;
}

static const char* class_strings[] = {
    "[:Latin:]",    "[:Letter:]",       "[:Russian:]",
    "[:bdigits:]",  "[:digits:]",       "[:latin:]",
    "[:letter:]",   "[:ndq:]",          "[:nsq:]",
    "[:odigits:]",  "[:russian:]",      "[:xdigits:]"
};

void Expr_scaner::correct_class(){
    /* Данная функция корректирует код лексемы, скорее всего
     * являющейся классом символов, и выводит необходимую
     * диагностику. */
    if(token.code >= M_Class_Latin){
        int y = token.code - M_Class_Latin;
        printf("В строке %zu ожидается %s.\n",
               loc->current_line,class_strings[y]);
        token.code = static_cast<Expr_lexem_code>(y + Class_Latin);
        en -> increment_number_of_errors();
    }
}

Expr_lexem_info Expr_scaner::current_lexem(){
    automaton = A_start; token.code = Nothing;
    lexem_begin = loc->pcurrent_char;
    bool t = true;
    while((ch = *(loc->pcurrent_char)++)){
        char_categories = get_categories_set(ch); // categories_table[ch];
        t = (this->*procs[automaton])();
        if(!t){
            /* Сюда попадаем, лишь если лексема уже прочитана При этом текущим автоматом.
             * уже прочитан символ, идущий сразу за концом прочитанной лексемы на
             *,основании этого символа принято решение о том, что лексема прочитана, и
             * совершён переход к следующему за ним символу. Поэтому для того, чтобы
             * не пропустить первый символ следующей лексемы, нужно уменьшить на
             * единицу указатель pcurrent_char. */
            (loc->pcurrent_char)--;
            if(Action == token.code){
                /* Если текущая лексема является идентификатором, то этот идентификатор
                 * нужно записать в таблицу идентификаторов. */
                token.action_name_index = ids -> insert(buffer);
            }else if(A_class == automaton){
                /* Если закончили обрабатывать класс символов, то нужно скорректировать
                 * его код, и, возможно, вывести диагностику. */
                correct_class();
            }
            return token;
        }
    }
    /* Здесь можем оказаться, только если уже прочли весь обрабатываемый текст. При этом
     * указатель на текущий символ указывает на символ, который находится сразу же после
     * нулевого символа, являющегося признаком конца текста. Чтобы не выйти при
     * последующих вызовах за пределы текста, нужно перейти обратно к нулевому
     * символу. */
    (loc->pcurrent_char)--;
    /* Далее, поскольку мы находимся здесь, то конец текущей лексемы (возможно,
     * неожиданный) ещё не обработан. Надо эту обработку выполнить, и, возможно, вывести
     * какую-то диагностику.*/
    (this->*finals[automaton])();
    return token;
}

bool Expr_scaner::unknown_proc(){
    return belongs(Other, char_categories);
}

/* Данный массив состоит из пар вида (состояние, символ) и используется для инициализации
 * автомата обработки классов символов. Смысл элемента массива таков: если в состоянии
 * инициализации текущий символ совпадает со второй компонентой элемента, то работа
 * начинается с состояния, которое является первой компонентой элемента. Рассмотрим,
 * например, элемент {54, U'n'}. Если текущий символ совпадает со второй компонентой
 * этого элемента, то работа начинается с состояния, являющегося первой компонентой,
 * т.е. с состояния 54. Массив должен быть отсортирован по возрастанию
 * второй компоненты.*/
static const State_for_char init_table_for_classes[] = {
    {0, U'L'},  {14, U'R'}, {23, U'b'}, {32, U'd'}, {40, U'l'},
    {54, U'n'}, {63, U'o'}, {72, U'r'}, {81, U'x'}
};

bool Expr_scaner::classes_proc(){
    bool t = false;
    switch(state){
        case -1:
            if(U':' == ch){
                state = -2; t = true;
            }
            break;
        case -2:
            if(belongs(After_colon, char_categories)){
                state = get_init_state(ch, init_table_for_classes,
                                       sizeof(init_table_for_classes)/
                                       sizeof(State_for_char));
                token.code = a_classes_jump_table[state].code;
                t = true;
            }else{
                printf("В строке %zu ожидается один из следующих"
                       "символов: L, R, b, d, l, n, o, r, x.\n",
                       loc->current_line);
                en -> increment_number_of_errors();
            }
            break;
        default:
            Elem elem = a_classes_jump_table[state];
            token.code = elem.code;
            int y = search_char(ch, elem.symbols);
            if(y != THERE_IS_NO_CHAR){
                state = elem.first_state + y; t = true;
            }
    }
    return t;
}

bool Expr_scaner::char_proc(){
    if(belongs(After_backslash, char_categories)){
        token.c = (U'n' == ch) ? U'\n' : ch;
        (loc->pcurrent_char)++;
    }else{
        token.c = U'\\';
    }
    return false;
}

bool Expr_scaner::delimiter_proc(){
    bool t = false;
    switch((char)ch){
        case U'{':
            token.code = Begin_expression;
            break;
        case U'}':
            token.code = End_expression;
            break;
        case U'(':
            token.code = Opened_round_brack;
            break;
        case U')':
            token.code = Closed_round_brack;
            break;
        case U'|':
            token.code = Or;
            break;
        case U'*':
            token.code = Kleene_closure;
            break;
        case U'+':
            token.code = Positive_closure;
            break;
        case U'?':
            token.code = Optional_member;
            break;
    }
    (loc->pcurrent_char)++;
    return t;
}

bool Expr_scaner::action_proc(){
    bool t = true;
    /* Переменная t равна true, если имя действия полностью
     * ещё не прочитано, и false в противном случае. */
    if(-1 == state){
        if(belongs(Action_name_begin, char_categories)){
            buffer += ch; state = 0;
        }else{
            printf("В строке %zu ожидается латинская буква или знак подчёркивания.\n",
                    loc->current_line);
            en -> increment_number_of_errors();
            t = false;
        }
        return t;
    }
    t = belongs(Action_name_body, char_categories);
    if(t){
        buffer += ch;
    }
    return t;
}

void Expr_scaner::none_final_proc(){
    /* Данная подпрограмма будет вызвана, если по прочтении входного текста оказались
     * в автомате A_start. Тогда ничего делать не нужно. */
}

void Expr_scaner::unknown_final_proc(){
    /* Данная подпрограмма будет вызвана, если по прочтении входного текста оказались
     * в автомате A_unknown. Тогда ничего делать не нужно. */
}

void Expr_scaner::action_final_proc(){
    /* Данная функция будет вызвана, если по прочтении входного потока оказались
     * в автомате обработки имён действий, автомате A_action. Тогда это имя
     * нужно записать в префиксное дерево идентификаторов. */
    token.action_name_index = ids -> insert(buffer);
}

void Expr_scaner::delimiter_final_proc(){
}

void Expr_scaner::classes_final_proc(){
    token.code = a_classes_jump_table[state].code;
    correct_class();
}

void Expr_scaner::char_final_proc(){
    token.c = U'\\';
}