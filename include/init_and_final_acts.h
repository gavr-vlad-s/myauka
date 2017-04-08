/*
    File:    init_and_final_acts.h
    Created: 04 February 2017 at 14:39 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef INIT_AND_FINAL_ACTS_H
#define INIT_AND_FINAL_ACTS_H
/* Пользовательские действия, выполняемые при инициализации и при завершении
   автомата обработки строк Moscow time, порождённого из
   описания сканера.
*/

struct Init_and_final_acts{
    size_t init_acts = 0;
    size_t fin_acts  = 0;
};
#endif