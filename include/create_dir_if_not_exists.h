/*
    Файл:    create_dir_if_not_exists.h
    Создан:  5 ноября 2016г. в 12:33 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef CREATE_DIR_IF_NOT_EXISTS_H
#define CREATE_DIR_IF_NOT_EXISTS_H
#include <boost/filesystem/operations.hpp>
/**
    Следующая функция создаёт каталог с путём p к нему, если каталог не существует.
    Если p существует, но не является каталогом, то возвращается false.
    Во всех остальных случаях возвращается true.
*/
bool create_dir_if_not_exists(const boost::filesystem::path& p);
#endif