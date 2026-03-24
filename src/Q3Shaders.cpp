//
//  Quake3Shaders.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 23.11.24.
//

#include <iostream>
#include <filesystem>

#include <fstream>
#include <sstream>

#include "Q3Shaders.h"

namespace fs = std::filesystem;

void Quake3Shaders::initFromDir(const std::string& path)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".shader")
        {
            readFile(entry.path());
//            break;
        }
    }
}



std::vector<Q3ShaderItem> parseShaderFile(const std::string& filename);
void printEntities(const std::vector<Q3ShaderItem>& entities);

void Quake3Shaders::readFile(const std::string& filename)
{
//    std::cout << "------- READ FILE -------" << std::endl;
//    std::cout << filename << std::endl << std::endl;
    
    try
    {
        auto entities = parseShaderFile(filename);
        
        for(const auto& item : entities)
        {
            this->entries[item.name] = item;
        }
        
//        printEntities(entities);
    } 
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}

bool Quake3Shaders::getBaseTextureName(const std::string& shaderName, std::string& textureName, BlendMode& mode) const
{
    auto it = entries.find(shaderName);
    
    if (it == entries.end()) 
    {
//        std::cerr << "Нет шейдера " << shaderName << std::endl;
        return false;
    }
    
    const auto& shader = it->second;
    
    bool success = false;
    
    for (const auto& stage : shader.stages)
    {
        auto it = stage.parameters.find("map"); 
        
        if (it == stage.parameters.end()) continue;
        if (it->second == "$lightmap") continue;
        
        textureName = it->second;
        
        mode = BlendMode::Opaque;
        
        auto blendFunc = stage.parameters.find("blendFunc");
        if (blendFunc != stage.parameters.end())
        {
            if (blendFunc->second == "GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA")
            {
                mode = BlendMode::Transparent;
            }
            else if (blendFunc->second == "GL_SRC_ALPHA GL_ONE")
            {
                mode = BlendMode::Add;
            }
            
        }
        
        auto alphaFunc = stage.parameters.find("alphaFunc");
        if (alphaFunc != stage.parameters.end())
        {
            mode = BlendMode::Mask;
        }
        
        success = true;
        break;
    }
    
    return success;
}




// Функция для проверки, является ли строка комментарием или пустой
bool isCommentOrEmpty(const std::string& line) {
    return line.empty() || line.rfind("//", 0) == 0;
}

// Функция для очистки строки
void trim(std::string& line) {
    line.erase(0, line.find_first_not_of(" \t\r\n")); // Удаление начальных пробелов и переносов
    line.erase(line.find_last_not_of(" \t\r\n") + 1); // Удаление конечных пробелов и переносов
}

// Функция для обработки строки параметра
void parseParameter(const std::string& line, std::unordered_map<std::string, std::string>& parameters) {
    std::istringstream iss(line);
    std::string key;
    std::string value;

    iss >> key; // Первый токен — это ключ
    std::getline(iss, value); // Остальное — значение
    trim(value); // Убираем пробелы в начале и конце значения

    if (!key.empty() && !value.empty()) {
        parameters[key] = value;
    }
}

// Функция для парсинга всего файла
std::vector<Q3ShaderItem> parseShaderFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    std::vector<Q3ShaderItem> entities;
    std::string line;
    Q3ShaderItem* currentEntity = nullptr; // Указатель на текущую сущность
    Q3ShaderStage* currentStage = nullptr;  // Указатель на текущий stage
    bool inBody = false;            // Флаг, указывающий на обработку тела сущности
    bool inStage = false;           // Флаг, указывающий на обработку stage
    
    bool hasCandidate = false;
    std::string candidateName;

    while (std::getline(file, line))
    {
        trim(line); // Удаляем лишние символы
        
        if (isCommentOrEmpty(line)) {
            continue; // Пропускаем комментарии и пустые строки
        }
        
        if (hasCandidate && line == "{")
        {
            currentEntity = &entities.emplace_back();
            currentEntity->name = candidateName;
            inBody = true; // Переходим в тело сущности
            
            hasCandidate = false;
            continue;
        }
        
        if (!inBody && !inStage)
        {
            hasCandidate = true;
            candidateName = line;
            continue;
        }

        // Внутри тела сущности
        if (line == "{") {
//                if (inStage) {
//                    throw std::runtime_error("Неверный формат: вложенные фигурные скобки внутри stage");
//                }
            inStage = true; // Начало stage
            currentStage = &currentEntity->stages.emplace_back();
        } else if (line == "}") {
            if (inStage) {
                inStage = false; // Завершаем stage
                currentStage = nullptr;
            } else {
                inBody = false; // Завершаем тело сущности
                currentEntity = nullptr;
            }
        } else if (inStage) {
            // Если внутри stage, добавляем параметры
            parseParameter(line, currentStage->parameters);
        } else {
            // Если вне stage, добавляем параметры в тело сущности
            parseParameter(line, currentEntity->parameters);
        }
    }

    return entities;
}

// Функция для вывода данных
void printEntities(const std::vector<Q3ShaderItem>& entities) {
    for (const auto& entity : entities) {
        std::cout << "Entity: " << entity.name << "\n";
        std::cout << "Parameters:\n";
        for (const auto& [key, value] : entity.parameters) {
            std::cout << "  " << key << ": " << value << "\n";
        }
        std::cout << "Stages:\n";
        for (const auto& stage : entity.stages) {
            std::cout << "  Stage:\n";
            for (const auto& [key, value] : stage.parameters) {
                std::cout << "    " << key << ": " << value << "\n";
            }
        }
        std::cout << "-------------------\n";
    }
}
