//
// Created by Oleg Shestak on 12.09.2025.
//
#include "index.h"
#include <sstream>
#include <thread>

// Метод обновления базы документов и построения инвертированного индекса
void InvertedIndex::updateDocumentBase(std::vector<std::string> input_docs) {
    docs = move(input_docs);
    freq_dictionary.clear();
    std::vector<std::unordered_map<std::string, size_t>> local_indexes(docs.size());
    std::vector<std::thread> threads;
    // Запуск по одному потоку на каждый документ
    for (size_t i = 0; i < docs.size(); ++i) {
        threads.emplace_back([this, &local_indexes, i]() {
            tokenizeAndCount(docs[i], local_indexes[i]);
        });
    }

    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Объединяем результаты из всех локальных индексов в общий инвертированный индекс freq_dictionary
    for (size_t doc_id = 0; doc_id < local_indexes.size(); ++doc_id) {
        for (const auto& [word, count] : local_indexes[doc_id]) {
            freq_dictionary[word].push_back({doc_id, count});
        }
    }
}
// Метод возвращает список записей для заданного слова (в каких документах и сколько раз оно встречается)
std::vector<Entry> InvertedIndex::getWordCount(const std::string& word) {
    std::string word_lc = word;
    // Ищем слово в словаре индекса
    auto it = freq_dictionary.find(word_lc);
    if (it == freq_dictionary.end()) {
        // Если слово не найдено — возвращаем пустой вектор
        return {};
    }
    return it->second;
}

void InvertedIndex::tokenizeAndCount(const std::string& text, std::unordered_map<std::string, size_t>& word_counts) {
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        word_counts[word]++;
    }
}