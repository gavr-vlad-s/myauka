/*
    File:    get_init_state.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef GET_INIT_STATE_H
#define GET_INIT_STATE_H

struct State_for_char{
    unsigned st;
    char32_t c;
};

/* Функция get_init_state инициализирует конечный автомат. Делает она это так: ищет
 * символ sym в таблице sts, состоящей из пар (состояние, символ) и имеющей размер
 * n, двоичным поиском по второму компоненту пары. После нахождения выдаётся
 * первая компонента пары. В качестве алгоритма двоичного поиска используется
 * алгоритм B из раздела 6.2.1 монографии "Кнут Д.Э. Искусство программирования.
 * Т.3. Сортировка и поиск. 2-е изд.: Пер. с англ. --- М.: Вильямс, 2008.". При
 * этом в нашем случае не может быть, чтобы нужный элемент в таблице sts
 * отсутствовал. */
int get_init_state(char32_t sym, const State_for_char sts[], int n);
#endif