#ifndef COMPILER_H
#define COMPILER_H

#include "MacrosForTuring.h"
#include "MemoryManager.h"
#include "TuringMachine.h"
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

class Compiler {
private:
  TuringMachine &tm_;
  MemoryManager &mem_;
  int stateCounter_ = 0;

  // Генератор уникальных состояний конечного автомата
  std::string NextState() {
    return "q_auto_" + std::to_string(++stateCounter_);
  }

  // Примитивный лексер: разбивает строку на токены по пробелам
  std::vector<std::string> Tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
      tokens.push_back(token);
    }
    return tokens;
  }

  // Проверка, является ли токен числом
  bool IsNumber(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
  }

  // Безопасное получение адреса: если это число, обращаемся к ROM (скрытой
  // переменной)
  char GetAddressFor(const std::string &token) {
    if (IsNumber(token)) {
      return mem_.GetAddress("_c" + token);
    }
    return mem_.GetAddress(token);
  }

public:
  Compiler(TuringMachine &tm, MemoryManager &mem) : tm_(tm), mem_(mem) {}

  void Compile(const std::vector<std::string> &sourceCode) {
    // ====================================================================
    // ПАС 1: Выделение памяти (Memory Mapping & ROM Allocation)
    // ====================================================================
    for (const auto &line : sourceCode) {
      auto tokens = Tokenize(line);
      for (const auto &t : tokens) {
        if (IsNumber(t)) {
          // Размещаем константы в памяти как скрытые переменные
          std::string constName = "_c" + t;
          try {
            mem_.Allocate(constName, std::stoi(t));
          } catch (...) { /* Константа уже выделена, игнорируем */
          }
        } else if (isalpha(t[0])) {
          // Размещаем пользовательские переменные (изначально равны 0)
          try {
            mem_.Allocate(t, 0);
          } catch (...) { /* Переменная уже существует */
          }
        }
      }
    }

    // ====================================================================
    // ПАС 2: Генерация Микрокода (Syntax-Directed Translation)
    // ====================================================================
    std::string currentState = "start";

    for (size_t i = 0; i < sourceCode.size(); ++i) {
      auto tokens = Tokenize(sourceCode[i]);
      if (tokens.empty())
        continue;

      // Если это последняя инструкция, прыгаем в halt, иначе генерируем
      // промежуточное состояние
      std::string nextState =
          (i == sourceCode.size() - 1) ? "halt" : NextState();

      // Парсинг: x ++
      if (tokens.size() == 2 && tokens[1] == "++") {
        GenerateIncrement(tm_, currentState, GetAddressFor(tokens[0]),
                          nextState);
      }
      // Парсинг: x --
      else if (tokens.size() == 2 && tokens[1] == "--") {
        GenerateDecrement(tm_, currentState, GetAddressFor(tokens[0]),
                          nextState);
      }
      // Парсинг: x = y
      else if (tokens.size() == 3 && tokens[1] == "=") {
        GenerateAssign(tm_, currentState, GetAddressFor(tokens[0]),
                       GetAddressFor(tokens[2]), nextState);
      }
      // Парсинг бинарных операций: x = y + z
      else if (tokens.size() == 5 && tokens[1] == "=") {
        std::string dest = tokens[0];
        std::string arg1 = tokens[2];
        std::string op = tokens[3];
        std::string arg2 = tokens[4];

        std::string midState = currentState + "_mid";

        if (op == "+") {
          // x = y + z  =>  1) x = y; 2) x = x + z
          GenerateAssign(tm_, currentState, GetAddressFor(dest),
                         GetAddressFor(arg1), midState);
          GenerateAdd(tm_, midState, GetAddressFor(dest), GetAddressFor(arg2),
                      nextState);
        } else if (op == "-") {
          // x = y - z  =>  1) x = y; 2) x = x - z
          GenerateAssign(tm_, currentState, GetAddressFor(dest),
                         GetAddressFor(arg1), midState);
          GenerateSubtract(tm_, midState, GetAddressFor(dest),
                           GetAddressFor(arg2), nextState);
        } else if (op == "*") {
          // Умножение X = X + Y * Z. Чтобы сделать X = Y * Z, нужно сначала
          // обнулить X.
          GenerateClear(tm_, currentState, GetAddressFor(dest), midState);
          GenerateMultiply(tm_, midState, GetAddressFor(dest),
                           GetAddressFor(arg1), GetAddressFor(arg2), nextState);
        }
      } else {
        std::cerr << "Синтаксическая ошибка в строке: " << sourceCode[i]
                  << "\n";
      }

      currentState = nextState; // Двигаем цепочку состояний
    }
  }
};

#endif // COMPILER_H