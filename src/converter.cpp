//
// Created by Oleg Shestak on 09.09.2025.
//
#include "converter.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <stdexcept>

using ordered_json = nlohmann::ordered_json;

#define APP_VERSION "0.1"
#define MAX_REQUESTS 1000 // максимальное число запросов
#define MAX_WORDS_REQ 10 // максимальное количесво слов в запросе
#define MAX_WORDS_DOC 1000 // максимальное число слов в документе

// Загружает JSON из файла
json ConverterJSON::loadJsonFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("File '" + filePath + "' is missing or cannot be opened");
    }

    json jsonData;
    try {
        file >> jsonData;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse '" + filePath + "': " + std::string(e.what()));
    }
    return jsonData;
}

// Валидация полей в JSON объекте
void ConverterJSON::validateJsonFields(const json& j, const std::unordered_map<std::string, json::value_t>& requiredFields, const std::string& context) {
    for (const auto& [field, type] : requiredFields) {
        if (!j.contains(field)) {
            throw std::runtime_error(context + " missing required field '" + field + "'");
        }
        if (j[field].type() != type) {
            throw std::runtime_error(context + " field '" + field + "' has incorrect type");
        }
    }
}

ConverterJSON::ConverterJSON(const std::string& configPath,
                             const std::string& requestPath,
                             const std::string& answersPath)
        : config_path(configPath),
          request_path(requestPath),
          answers_path(answersPath),
          name(""),
          version(""),
          file_names()
{}

// Загрузка и валидация config.json
json ConverterJSON::loadAndValidateConfig() {
    json fullConfig = loadJsonFromFile(config_path);

    // Проверяем, что есть "config" (объект) и "files" (массив)
    validateJsonFields(fullConfig, {{"config", json::value_t::object}, {"files", json::value_t::array}}, config_path);

    // Проверяем поля внутри "config"
    validateJsonFields(fullConfig["config"], {{"name", json::value_t::string}, {"version", json::value_t::string}}, config_path + "->config");

    name = fullConfig["config"]["name"].get<std::string>();
    version = fullConfig["config"]["version"].get<std::string>();

    // Проверка версии приложения
    if (fullConfig["config"]["version"] != APP_VERSION) {
        throw std::runtime_error(config_path + " has incorrect file version");
    }

    file_names.clear();

    for (const auto &fileJson : fullConfig["files"]) {
        if (fileJson.is_string()) {
            file_names.push_back(fileJson.get<std::string>());
        } else {
            file_names.push_back("");
        }
    }

    return fullConfig;
}

// Получение текстовых документов из файлов, указанных в config.json
// Если файл невалиден, вектор документов получает пустую строку на месте этого файла
std::vector<std::string> ConverterJSON::getTextDocuments() {
    if (file_names.empty()) {
        loadAndValidateConfig();
    }

    // Регулярное выражение для проверки слов: только строчные буквы, длина 1-100
    std::regex wordRegex("^[a-z]{1,100}$");
    std::vector<std::string> documents;

    for (const auto& filename : file_names) {
        if (filename.empty()) {
            documents.emplace_back("");
            continue;
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file " << filename << "\n";
            documents.emplace_back("");
            continue;
        }

        // Читаем файл целиком через итераторы
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // Разбиваем содержимое на слова
        std::istringstream iss(content);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // Проверяем ограничение на количество слов
        if (words.size() > MAX_WORDS_DOC) {
            std::cerr << "File " << filename << " contains more than " << MAX_WORDS_DOC << " words, skipping\n";
            documents.emplace_back("");
            continue;
        }

        // Проверяем каждое слово на соответствие регулярному выражению
        bool valid = all_of(words.begin(), words.end(), [&](const std::string& w) {
            return regex_match(w, wordRegex);
        });

        if (!valid) {
            std::cerr << "File " << filename << " contains invalid words, skipping\n";
            documents.emplace_back("");
            continue;
        }

        // Если все проверки пройдены, добавляем содержимое файла
        documents.push_back(content);
    }

    return documents;
}

// Получение лимита ответов из config.json
int ConverterJSON::getResponsesLimit() {
    json configJson = loadAndValidateConfig();

    const uint8_t defaultValue = 5;

    if (configJson["config"].contains("max_responses")) {
        const auto& maxResp = configJson["config"]["max_responses"];
        int value;

        if (maxResp.is_number_integer()) {
            value = maxResp.get<int>();
        } else if (maxResp.is_number_float()) {
            value = static_cast<int>(maxResp.get<double>());
        } else {
            std::cerr << "'max_responses' is not a number. Using default " << defaultValue << "\n";
            return defaultValue;
        }

        return value;
    } else {
        std::cerr << "'max_responses' field is missing inside 'config'. Using default value: " << defaultValue << "\n";
        return defaultValue;
    }
}

// Получение запросов из requests.json
std::vector<std::string> ConverterJSON::getRequests() {
    json requestsJson = loadJsonFromFile(request_path);

    // Проверяем, что "requests" (массив)
    validateJsonFields(requestsJson, {{"requests", json::value_t::array}}, request_path);

    const auto& requestsArray = requestsJson["requests"];

    if (requestsArray.size() > MAX_REQUESTS) {
        throw std::runtime_error("Requests file contains more than " + std::to_string(MAX_REQUESTS) + " requests");
    }

    std::regex wordRegex("^[a-z]+$");
    std::vector<std::string> requests;

    for (const auto& req : requestsArray) {
        if (!req.is_string()) {
            throw std::runtime_error("Each request must be a string");
        }

        std::string request = req.get<std::string>();
        std::istringstream iss(request);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // Проверяем количество слов в запросе
        if (words.empty() || words.size() > MAX_WORDS_REQ) {
            throw std::runtime_error("Each query must contain from 1 to " + std::to_string(MAX_WORDS_REQ) + " words");
        }

        // Проверяем каждое слово на валидность
        for (const auto& w : words) {
            if (!regex_match(w, wordRegex)) {
                throw std::runtime_error("Word '" + w + "' contains invalid characters");
            }
        }

        requests.push_back(request);
    }

    return requests;
}

// Метод для записи результатов поисковых запросов в файл answers.json
void ConverterJSON::putAnswers(std::vector<std::vector<RelativeIndex>> answers) {
    //Для определенного порядка полей в json
    ordered_json jAnswers;
    ordered_json jRoot;

    for (size_t i = 0; i < answers.size(); ++i) {
        // Формируем id запроса с ведущими нулями: request001, request002, ...
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "request%03zu", i + 1);
        std::string requestId(buffer);

        const auto& relevances = answers[i];

        if (relevances.empty()) {
            jAnswers[requestId] = ordered_json {
                    {"result", "false"}
            };
        } else {
            ordered_json jRelevance = ordered_json::array();
            for (const auto& [docid, rank] : relevances) {
                jRelevance.push_back({
                                             {"docid", docid},
                                             {"rank", rank}
                                     });
            }

            ordered_json jAnswer;
            jAnswer["result"] = "true";
            jAnswer["relevance"] = jRelevance;

            jAnswers[requestId] = jAnswer;
        }
    }

    jRoot["answers"] = jAnswers;

    // Открываем файл для записи (перезаписываем содержимое)
    std::ofstream outFile("answers.json", std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Cannot open or create answers.json for writing");
    }

    // Записываем JSON с отступами для удобства чтения
    outFile << jRoot.dump(4) << std::endl;
    outFile.close();
}

const std::string& ConverterJSON::getName() const {
    return name;
}

const std::string& ConverterJSON::getVersion() const {
    return version;
}