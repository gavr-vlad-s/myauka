/*
    Файл:    unwrap_commands.h
    Создан:  24 января 2017г. в 15:58 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
             vladimir.gavrilov@itmm.unn.ru
*/

#ifndef UNWRAP_COMMANDS_H
#define UNWRAP_COMMANDS_H
#include "../include/unwrapped_command.h"
#include "../include/command.h"

Unwrapped_commands unwrap_commands(const Command_buffer& buf);
/* Данная функция преобразует буфер с записанными командами, разворачивая
 * классы символов (кроме классов [:nsq:] и [:ndq:]). */
#endif
