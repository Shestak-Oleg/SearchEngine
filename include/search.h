//
// Created by Oleg Shetak on 12.09.2025.
//
#ifndef SEARCH_ENGINE_SEARCH_H
#define SEARCH_ENGINE_SEARCH_H

#include "index.h"

struct RelativeIndex {
    size_t doc_id;
    float rank;
    bool operator == (const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

class SearchServer {
public:
    //В конструктор класса передаётся ссылка на класс InvertedIndex, чтобы SearchServer мог узнать частоту слов встречаемых в запросе
    SearchServer(InvertedIndex& idx) : index(idx){ };

    //Метод обработки поисковых запросов, который возвращает отсортированный список релевантных ответов для заданных запросов
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input, size_t max_responses);

private:
    InvertedIndex index;
    std::vector<RelativeIndex> processQuery(const std::string& query);
};

#endif //SEARCH_ENGINE_SEARCH_H
