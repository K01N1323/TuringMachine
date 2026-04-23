#ifndef COMPILER_H
#define COMPILER_H

#include "MacrosForTuring.h"
#include "MemoryManager.h"
#include "TuringMachine.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Структура для отслеживания вложенности циклов
struct BlockScope {
  enum Type { WHILE, FOR } type;
  std::string startState; // Куда возвращаться (начало цикла)
  std::string endState;   // Куда прыгать при выходе (конец цикла)
  std::vector<std::string>
      stepTokens; // Инструкция шага для цикла for (например, x--)
};

class Compiler {
private:
  TuringMachine &tm_;
  MemoryManager &mem_;
  int stateCounter_ = 0;
  std::vector<BlockScope> scopes_; // Стек областей видимости

  std::string NextState() {
    return "q_auto_" + std::to_string(++stateCounter_);
  }

  // Продвинутый лексер с поддержкой скобок и точек с запятой
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
      } else if (std::string("=+-*(){};<>").find(line[i]) !=
                 std::string::npos) {
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
    if (IsNumber(token))
      return mem_.GetAddress("_c" + token);
    return mem_.GetAddress(token);
  }

  // Генератор инструкций (AST-узел для одной строки)
  void CompileStatement(const std::vector<std::string> &tokens,
                        const std::string &currentState,
                        const std::string &nextState) {
    if (tokens.size() == 2 && tokens[1] == "++") {
      GenerateIncrement(tm_, currentState, GetAddressFor(tokens[0]), nextState);
    } else if (tokens.size() == 2 && tokens[1] == "--") {
      GenerateDecrement(tm_, currentState, GetAddressFor(tokens[0]), nextState);
    } else if (tokens.size() == 3 && tokens[1] == "=") {
      if (tokens[0] != tokens[2]) {
        GenerateAssign(tm_, currentState, GetAddressFor(tokens[0]),
                       GetAddressFor(tokens[2]), nextState);
      } else {
        tm_.AddRule(currentState, '^', nextState, '^',
                    Direction::Stay); // Фиктивный JUMP (x = x)
      }
    } else if (tokens.size() == 5 && tokens[1] == "=") {
      std::string dest = tokens[0], arg1 = tokens[2], op = tokens[3],
                  arg2 = tokens[4];
      std::string midState = currentState + "_mid";

      if (op == "+") {
        if (dest == arg1)
          GenerateAdd(tm_, currentState, GetAddressFor(dest),
                      GetAddressFor(arg2), nextState);
        else if (dest == arg2)
          GenerateAdd(tm_, currentState, GetAddressFor(dest),
                      GetAddressFor(arg1), nextState);
        else {
          GenerateAssign(tm_, currentState, GetAddressFor(dest),
                         GetAddressFor(arg1), midState);
          GenerateAdd(tm_, midState, GetAddressFor(dest), GetAddressFor(arg2),
                      nextState);
        }
      } else if (op == "-") {
        if (dest == arg1)
          GenerateSubtract(tm_, currentState, GetAddressFor(dest),
                           GetAddressFor(arg2), nextState);
        else {
          GenerateAssign(tm_, currentState, GetAddressFor(dest),
                         GetAddressFor(arg1), midState);
          GenerateSubtract(tm_, midState, GetAddressFor(dest),
                           GetAddressFor(arg2), nextState);
        }
      } else if (op == "*") {
        if (dest == arg1 ||
            dest == arg2) { // Само-умножение через временный регистр
          std::string t1 = currentState + "_t1", t2 = currentState + "_t2";
          GenerateClear(tm_, currentState, mem_.GetAddress("_temp"), t1);
          GenerateMultiply(tm_, t1, mem_.GetAddress("_temp"),
                           GetAddressFor(arg1), GetAddressFor(arg2), t2);
          GenerateAssign(tm_, t2, GetAddressFor(dest), mem_.GetAddress("_temp"),
                         nextState);
        } else {
          GenerateClear(tm_, currentState, GetAddressFor(dest), midState);
          GenerateMultiply(tm_, midState, GetAddressFor(dest),
                           GetAddressFor(arg1), GetAddressFor(arg2), nextState);
        }
      }
    } else {
      std::cerr << "[Compiler Error] Неизвестная инструкция.\n";
    }
  }

public:
  Compiler(TuringMachine &tm, MemoryManager &mem) : tm_(tm), mem_(mem) {}

  void Compile(const std::vector<std::string> &sourceCode) {
    try {
      mem_.Allocate("_temp", 0);
    } catch (...) {
    }
    try {
      mem_.Allocate("_c0", 0);
    } catch (...) {
    }

    // ПАС 1: Выделение памяти (без изменений)
    for (const auto &line : sourceCode) {
      for (const auto &t : Tokenize(line)) {
        if (IsNumber(t)) {
          try {
            mem_.Allocate("_c" + t, std::stoi(t));
          } catch (...) {
          }
        } else if (isalpha(t[0]) && t != "while" && t != "for") {
          try {
            mem_.Allocate(t, 0);
          } catch (...) {
          }
        }
      }
    }

    // ПАС 2: Генерация потока управления
    std::string currentState = "start";

    for (const auto &line : sourceCode) {
      auto tokens = Tokenize(line);
      if (tokens.empty())
        continue;

      if (tokens[0] == "while") {
        std::string condState = NextState() + "_cond";
        std::string bodyState = NextState() + "_body";
        std::string endState = NextState() + "_end";

        // JUMP в начало проверки
        tm_.AddRule(currentState, '^', condState, '^', Direction::Stay);
        // Сравниваем: если > 0 -> в тело, иначе -> выход
        GenerateCompare(tm_, condState, GetAddressFor(tokens[2]),
                        GetAddressFor("0"), bodyState, endState, endState);

        scopes_.push_back({BlockScope::WHILE, condState, endState, {}});
        currentState = bodyState; // Тело начинается здесь
      } else if (tokens[0] == "}") {
        if (scopes_.empty())
          continue;
        BlockScope scope = scopes_.back();
        scopes_.pop_back();

        if (scope.type == BlockScope::FOR) {
          std::string stepState = NextState() + "_step";
          CompileStatement(scope.stepTokens, currentState, stepState);
          currentState = stepState;
        }

        // КРИТИЧЕСКИЙ МОМЕНТ: Замыкаем тело цикла на проверку условия
        tm_.AddRule(currentState, '^', scope.startState, '^', Direction::Stay);

        // Будущие инструкции пойдут из состояния выхода из цикла
        currentState = scope.endState;
      } else {
        // Обычная инструкция: x++, fact = fact * x и т.д.
        std::string nextState = NextState();
        CompileStatement(tokens, currentState, nextState);
        currentState = nextState; // Теперь цепочка не рвется!
      }
    }

    // Финальный переход в halt
    tm_.AddRule(currentState, '^', "halt", '^', Direction::Stay);
  }
};

#endif // COMPILER_H
