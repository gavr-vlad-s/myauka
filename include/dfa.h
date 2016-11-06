/*
    Файл:    dfa.h
    Создан:  13 декабря 2015г. в 09:05 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef DFA_H
#define DFA_H

#include "../include/ndfa.h"
#include "../include/command.h"
#include "../include/generalized_char.h"
#include <vector>
#include <map>
#include <set>
#include <utility>

struct DFA_state_with_action{
    size_t st;
    size_t action_idx;
};

using State_and_gen_char = std::pair<size_t, Generalized_char>;
using DFA_jumps = std::map<State_and_gen_char, DFA_state_with_action>;

struct DFA{
    DFA_jumps        jumps;
    size_t           begin_state      = 0;
    size_t           number_of_states = 0;
    std::set<size_t> final_states;

    DFA()                = default;
    ~DFA()               = default;
    DFA(const DFA& orig) = default;
};


/* Данная функция по НКА (недетерминированному конечному автомату) ndfa
 * строит соответствующий ДКА a. */
void convert_NDFA_to_DFA(DFA& a, const NDFA& ndfa);

using Min_DFA_state_jumps = std::map<Generalized_char, DFA_state_with_action>;
using Min_DFA_jumps       = std::vector<Min_DFA_state_jumps>;
/* В Min_DFA_jumps элемент с индексом j представляет собой переходы для
 * состояния с номером j. */

struct Min_DFA{
    Min_DFA_jumps    jumps;
    size_t           begin_state = 0;
    std::set<size_t> final_states;

    Min_DFA()                    = default;
    ~Min_DFA()                   = default;
    Min_DFA(const Min_DFA& orig) = default;
};

void minimize_DFA(Min_DFA& minimized, const DFA& minimizing);
#endif