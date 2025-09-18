//
// Created by Oleg Shetak on 09.09.2025.
//
#ifndef SEARCH_ENGINE_CONVERTER_H
#define SEARCH_ENGINE_CONVERTER_H

#include "nlohmann/json.hpp"
#include "search.h"

using json = nlohmann::json;

// Класс для работы со всеми JSON файлами
class ConverterJSON {

private:
    std::string config_path;
    std::string request_path;
    std::string answers_path;

    std::string name;
    std::string version;

    std::vector<std::string> file_names;

    // Загружает JSON из файла
    json loadJsonFromFile(const std::string& filePath);
    // Валидация полей в JSON объекте
    void validateJsonFields(const json& j, const std::unordered_map<std::string, json::value_t>& requiredFields, const std::string& context = "");

public:
    ConverterJSON(const std::string& configPath = "config.json",
                  const std::string& requestPath = "requests.json",
                  const std::string& answersPath = "answers.json");

    // Загрузка и валидация config.json
    json loadAndValidateConfig();

    // Получение текстовых документов из файлов, указанных в config.json
    std::vector<std::string> getTextDocuments();

    // Получение лимита ответов из config.json
    int getResponsesLimit();

    // Получение запросов из requests.json
    std::vector<std::string> getRequests();

    // Метод для записи результатов поисковых запросов в файл answers.json
    void putAnswers(std::vector<std::vector<RelativeIndex>> answers);

    //Геттеры для имени и версии
    const std::string& getName() const;
    const std::string& getVersion() const;
};

#endif //SEARCH_ENGINE_CONVERTER_H
