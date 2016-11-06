/*
    Файл:    codes_and_newline.cpp
    Создан:  11 августа 2016г. в 7:40 (по Москве)
    Автор:   Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/codes_and_newline.h"
#include <cstdio>

const char32_t* newline_lexem_str = U"Newline";

void Codes_and_newline::newline_is_lexem_sec(){
    li = msc->current_lexem();
    // newline_is_lexem = true;
    size_t idx       = et_.ids_trie->insert(newline_lexem_str);
    add_new_lexem_code(idx);
    // return true;
}

void Codes_and_newline::add_new_lexem_code(size_t idx){
    auto s = scope_->idsc.find(idx);
    Id_attributes iattr;
    if(s != scope_->idsc.end()){
        printf("В строке %zu повторно определён идентификатор ",
               msc->lexem_begin_line_number());
        et_.ids_trie->print(idx); printf("\n");
        et_.ec -> increment_number_of_errors();
    }else{
        iattr.kind = Code_of_lexem;
        iattr.code = ++last_code_val;
        scope_->idsc[idx] = iattr;
        codes.push_back(idx);
    }
}

Codes_and_newline::State_proc Codes_and_newline::procs[] = {
    &Codes_and_newline::codes_id_proc, &Codes_and_newline::codes_comma_sep_proc
};

void Codes_and_newline::codes_sec(std::vector<size_t>& codes_, size_t& last_code){
    codes = codes_; last_code_val = last_code;
    codes_sec_();
    codes_ = codes; last_code = last_code_val;
}

void Codes_and_newline::codes_sec_(){
    lc = (li = msc-> current_lexem()).code;
    //li = msc->current_lexem();
    if(lc != Kw_codes){
        printf("В строке %zu ожидается %%codes.\n", msc->lexem_begin_line_number());
        et_.ec -> increment_number_of_errors();
        msc->back();
        return;
    }

    state = Codes_id;
    for( ; ; ){
        lc = (li = msc-> current_lexem()).code;
        bool t = (this->*procs[state])();
        if(!t){return;};
    }
}

bool Codes_and_newline::codes_id_proc(){
    bool t = true;
    //Main_lexem_code c = li.code;
    if(Id == lc){
        add_new_lexem_code(li.ident_index);
        state = Codes_comma_sep;
        return t;
    }
    printf("В строке %zu пропущен идентификатор.\n", msc->lexem_begin_line_number());
    et_.ec -> increment_number_of_errors();
    if(Comma == lc){
        state = Codes_comma_sep;
    }else{
        msc->back(); t = false;
    }
    return t;
}

bool Codes_and_newline::codes_comma_sep_proc(){
    bool t = true;
    switch(lc){
        case Id:
            printf("В строке %zu ожидается запятая.\n", msc->lexem_begin_line_number());
            et_.ec -> increment_number_of_errors();
            add_new_lexem_code(li.ident_index);
            state = Codes_comma_sep;
            break;

        case Comma:
            state = Codes_id;
            break;

        default:
            msc->back(); t = false;
    }
    return t;
}