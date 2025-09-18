//
// Created by Oleg Shetak on 12.09.2025.
//
#ifndef SEARCH_ENGINE_INDEX_H
#define SEARCH_ENGINE_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>

// Структура для хранения информации о слове в документе
struct Entry {
    size_t doc_id; // номер документа в коллекции
    size_t count; // количество вхождений слова в документа

    bool operator==(const Entry& other) const {
        return doc_id == other.doc_id && count == other.count;
    }
};

class InvertedIndex {
public:

    // Обновить или заполнить базу документов, по которой будем совершать поиск
    void updateDocumentBase(std::vector<std::string> input_docs);

    //Метод определяет количество вхождений слова word в загруженной базе документов и возвращает подготовленный список с частотой слов
    std::vector<Entry> getWordCount(const std::string& word);

private:
    std::vector<std::string> docs; // список содержимого документов
    std::unordered_map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь

    //Вспомогательный метод разбивает текст документа на слова и подсчитывает для каждого уникального слова количество вхождений
    static void tokenizeAndCount(const std::string& text, std::unordered_map<std::string, size_t>& word_counts);
};

#endif //SEARCH_ENGINE_INDEX_H
