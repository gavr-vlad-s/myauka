/*
    Файл:    automata_repres.h
    Создан:  04 февраля 2017г. в 14:59 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef AUTOMATA_REPRES_H
#define AUTOMATA_REPRES_H
#include <string>
#include "../include/info_for_constructing.h"
#include "../include/groupped_dfa.h"

struct Str_data_for_automaton {
    std::string automata_name;
    std::string proc_name;
    std::string category_name_prefix;
    std::string diagnostic_msg;
    std::string final_actions;
    std::string final_states_set_name;
};

std::string automata_repres(Info_for_constructing& info, const G_DFA& aut,
                            const Str_data_for_automaton& f);
#endif