//
// Created by Oleg Shestak on 12.09.2025.
//
#include "search.h"
#include "index.h"
#include <algorithm>
#include <cmath>

#define EPS 1e-6f

// Разбивает строку на уникальные слова (только буквы и цифры в нижнем регистре)
std::vector<std::string> splitIntoUniqueWords(const std::string& text) {
    std::unordered_map<std::string, int> unique_words_map;
    std::string word;
    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];
        if (isalpha(ch) || isdigit(ch)) {
            word += tolower(ch);
        } else {
            if (!word.empty()) {
                unique_words_map[word] = 1;     // сохраняем как уникальное
                word.clear();
            }
        }
    }
    // После цикла — если в буфере осталось слово, добавляем его
    if (!word.empty()) {
        unique_words_map[word] = 1;
    }
    // Копируем ключи словаря в вектор для результата
    std::vector<std::string> unique_words;
    for (const auto& p : unique_words_map) {
        unique_words.push_back(p.first);
    }
    return unique_words;
}

// Обрабатывает один поисковый запрос — возвращает релевантные документы с рангами
std::vector<RelativeIndex> SearchServer::processQuery(const std::string& query) {
    std::vector<std::string> unique_words = splitIntoUniqueWords(query);  // выделяем слова из запроса
    if (unique_words.empty()) {
        return {};  // пустой запрос — нет результатов
    }
    std::unordered_map<size_t, int> doc_to_abs_relevance;
    // Для каждого уникального слова запроса ищем в индексе все документы, где оно встречается
    for (const auto& word : unique_words) {
        std::vector<Entry> entries = index.getWordCount(word);
        for (const Entry& e : entries) {
            doc_to_abs_relevance[e.doc_id] += e.count;
        }
    }

    if (doc_to_abs_relevance.empty()) {
        return {};  // нет документов с этими словами
    }

    // Определяем максимальную абсолютную релевантность среди всех документов
    int max_abs_relevance = 0;
    for (const auto& [doc_id, abs_rel] : doc_to_abs_relevance) {
        if (abs_rel > max_abs_relevance) {
            max_abs_relevance = abs_rel;
        }
    }

    std::vector<RelativeIndex> ranked_results;
    ranked_results.reserve(doc_to_abs_relevance.size());

    // Заполняем вектор результатов, нормируя релевантности относительно максимальной
    for (const auto& [doc_id, abs_rel] : doc_to_abs_relevance) {
        RelativeIndex ri;
        // Нормируем релевантность относительно максимальной
        ri.rank = max_abs_relevance > 0 ? static_cast<float>(abs_rel) / max_abs_relevance : 0.0f;
        ri.doc_id = doc_id;
        ranked_results.push_back(ri);
    }

    // Сортируем результаты: сначала по убыванию ранга, при равенстве – по doc_id
    sort(ranked_results.begin(), ranked_results.end(), [](const RelativeIndex& a, const RelativeIndex& b) {
        if (std::fabs(a.rank - b.rank) < EPS) {
            return a.doc_id < b.doc_id;
        }
        return a.rank > b.rank;
    });

    return ranked_results;
}

// Обрабатываем список запросов, возвращаем вектор с результатами для каждого
std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input, size_t max_responses) {
    std::vector<std::vector<RelativeIndex>> results;
    for (const auto& query : queries_input) {
    auto ranked = processQuery(query);

    // Ограничиваем количество результатов max_responses
    if (ranked.size() > max_responses) {
        ranked.resize(max_responses);
    }

    results.push_back(std::move(ranked));
}
    return results;
}