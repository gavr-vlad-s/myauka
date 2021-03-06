/*
    File:    categories.h
    Created: 10 January 2015 at 10:32 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef CATEGORIES_H
#define CATEGORIES_H

#include "../include/generalized_char.h"
#include <set>

using Set_of_char = std::set<char32_t>;

enum Category_kind{
    All_chars, Not_single_quote, Not_double_quote, Set_of_cs
};

struct Category{
    Category_kind kind = Set_of_cs;
    Set_of_char   s;
};

Category gc2category(const Generalized_char& gc);

bool operator < (const Category& c1, const Category& c2);

bool operator == (const Category& c1, const Category& c2);

/* Calculation of the union of character categories. */
Category operator + (const Category& c1, const Category& c2);

/* If categories c1 and c2 intersect, then the returned value is true;
 * else the returned value is false. */
bool operator * (const Category& c1, const Category& c2);

/* If the category c1 is contained in the category c2, then the returned value
 * is true, else the returned value is false.
 */
bool is_subcategory(const Category& c1, const Category& c2);

void print_category(const Category& c);
#endif