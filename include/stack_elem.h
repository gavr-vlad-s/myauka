/*
    Файл:    stack_elem.h
    Создан:  13 декабря 2015г. в 09:05 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef STACK_ELEM_H
#define STACK_ELEM_H

#include "../include/expr_scaner.h"

struct Attributes{
    union{
        Expr_lexem_info eli;
        struct{
            size_t begin_index;
            size_t end_index;
        } indeces;
    };
};

struct Stack_elem{
    size_t     st_num; /* Номер состояния SLR(1)--парсера. */
    Attributes attr;   /* Атрибуты символа грамматики, соответствующие состоянию. */
};
#endif