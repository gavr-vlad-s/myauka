/*
    File:    trie_for_set_of_char.h
    Created: 06 November 2016 at 12:20 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef TRIE_FOR_SET_OF_CHAR_H
#define TRIE_FOR_SET_OF_CHAR_H
#include "../include/trie.h"
#include <set>
class Trie_for_set_of_char : public Trie<char32_t>{
public:
    virtual ~Trie_for_set_of_char() { };
    Trie_for_set_of_char(){};
    Trie_for_set_of_char(const Trie_for_set_of_char& orig) = default;

    /**
     *  \brief The function get_set on the index idx of the set of characters of
     *         type char32_t builds the same set, but already as std::set < char32_t >.
     *  \param [in] idx The index of the set of states in the prefix tree of such sets.
     *  \return         The same set, but already as std::set < char32_t >.
     */
    std::set<char32_t> get_set(size_t idx);
    size_t insertSet(const std::set<char32_t>& s);
private:
    virtual void post_action(const std::basic_string<char32_t>& s, size_t n);
};
#endif