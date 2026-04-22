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

  std::string NextState() {
    return "q_auto_" + std::to_string(++stateCounter_);
  }

  std::vector<std::string> Tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::string token;

    for (size_t i = 0; i < line.length(); ++i) {
      if (std::isspace(line[i])) {
        if (!token.empty()) {
          tokens.push_back(token);
          token.clear();
        }
      } else if (i + 1 < line.length() &&
                 (line.substr(i, 2) == "++" || line.substr(i, 2) == "--")) {
        if (!token.empty()) {
          tokens.push_back(token);
          token.clear();
        }
        tokens.push_back(line.substr(i, 2));
        ++i;
      } else if (line[i] == '=' || line[i] == '+' || line[i] == '-' ||
                 line[i] == '*') {
        if (!token.empty()) {
          tokens.push_back(token);
          token.clear();
        }
        tokens.push_back(std::string(1, line[i]));
      } else {
        token += line[i];
      }
    }
    if (!token.empty())
      tokens.push_back(token);
    return tokens;
  }

  bool IsNumber(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
  }

  char GetAddressFor(const std::string &token) {
    if (IsNumber(token)) {
      return mem_.GetAddress("_c" + token);
    }
    return mem_.GetAddress(token);
  }

public:
  Compiler(TuringMachine &tm, MemoryManager &mem) : tm_(tm), mem_(mem) {}

  void Compile(const std::vector<std::string> &sourceCode) {
    // Выделяем скрытую временную переменную (Регистр ALU процессора)
    try {
      mem_.Allocate("_temp", 0);
    } catch (...) {
    }

    // ====================================================================
    // ПАС 1: Выделение памяти (Memory Mapping)
    // ====================================================================
    for (const auto &line : sourceCode) {
      auto tokens = Tokenize(line);
      for (const auto &t : tokens) {
        if (IsNumber(t)) {
          std::string constName = "_c" + t;
          try {
            mem_.Allocate(constName, std::stoi(t));
          } catch (...) {
          }
        } else if (isalpha(t[0])) {
          try {
            mem_.Allocate(t, 0);
          } catch (...) {
          }
        }
      }
    }

    // ====================================================================
    // ПАС 2: AST Оптимизация и Генерация Микрокода
    // ====================================================================
    std::string currentState = "start";

    for (size_t i = 0; i < sourceCode.size(); ++i) {
      auto tokens = Tokenize(sourceCode[i]);
      if (tokens.empty())
        continue;

      std::string nextState =
          (i == sourceCode.size() - 1) ? "halt" : NextState();

      if (tokens.size() == 2 && tokens[1] == "++") {
        GenerateIncrement(tm_, currentState, GetAddressFor(tokens[0]),
                          nextState);
      } else if (tokens.size() == 2 && tokens[1] == "--") {
        GenerateDecrement(tm_, currentState, GetAddressFor(tokens[0]),
                          nextState);
      } else if (tokens.size() == 3 && tokens[1] == "=") {
        if (tokens[0] != tokens[2]) {
          GenerateAssign(tm_, currentState, GetAddressFor(tokens[0]),
                         GetAddressFor(tokens[2]), nextState);
        } else {
          // Защита от x = x: ничего не делаем, фиктивный переход
          tm_.AddRule(currentState, '^', nextState, '^', Direction::Stay);
        }
      } else if (tokens.size() == 5 && tokens[1] == "=") {
        std::string dest = tokens[0];
        std::string arg1 = tokens[2];
        std::string op = tokens[3];
        std::string arg2 = tokens[4];

        std::string midState = currentState + "_mid";

        if (op == "+") {
          if (dest == arg1) { // Оптимизация x = x + y
            GenerateAdd(tm_, currentState, GetAddressFor(dest),
                        GetAddressFor(arg2), nextState);
          } else if (dest == arg2) { // Оптимизация x = y + x
            GenerateAdd(tm_, currentState, GetAddressFor(dest),
                        GetAddressFor(arg1), nextState);
          } else {
            GenerateAssign(tm_, currentState, GetAddressFor(dest),
                           GetAddressFor(arg1), midState);
            GenerateAdd(tm_, midState, GetAddressFor(dest), GetAddressFor(arg2),
                        nextState);
          }
        } else if (op == "-") {
          if (dest == arg1) { // Оптимизация x = x - y (Именно здесь был баг!)
            GenerateSubtract(tm_, currentState, GetAddressFor(dest),
                             GetAddressFor(arg2), nextState);
          } else if (dest == arg2) { // Сложный кейс x = y - x (нужен _temp)
            std::string t1 = currentState + "_t1", t2 = currentState + "_t2";
            GenerateAssign(tm_, currentState, mem_.GetAddress("_temp"),
                           GetAddressFor(arg1), t1);
            GenerateSubtract(tm_, t1, mem_.GetAddress("_temp"),
                             GetAddressFor(arg2), t2);
            GenerateAssign(tm_, t2, GetAddressFor(dest),
                           mem_.GetAddress("_temp"), nextState);
          } else {
            GenerateAssign(tm_, currentState, GetAddressFor(dest),
                           GetAddressFor(arg1), midState);
            GenerateSubtract(tm_, midState, GetAddressFor(dest),
                             GetAddressFor(arg2), nextState);
          }
        } else if (op == "*") {
          if (dest == arg1 ||
              dest == arg2) { // Само-умножение (x = x * y) идет через _temp
            std::string t1 = currentState + "_t1", t2 = currentState + "_t2";
            GenerateClear(tm_, currentState, mem_.GetAddress("_temp"), t1);
            GenerateMultiply(tm_, t1, mem_.GetAddress("_temp"),
                             GetAddressFor(arg1), GetAddressFor(arg2), t2);
            GenerateAssign(tm_, t2, GetAddressFor(dest),
                           mem_.GetAddress("_temp"), nextState);
          } else {
            GenerateClear(tm_, currentState, GetAddressFor(dest), midState);
            GenerateMultiply(tm_, midState, GetAddressFor(dest),
                             GetAddressFor(arg1), GetAddressFor(arg2),
                             nextState);
          }
        }
      } else {
        std::cerr << "Синтаксическая ошибка в строке: " << sourceCode[i]
                  << "\n";
      }

      currentState = nextState;
    }
  }
};

#endif // COMPILER_H