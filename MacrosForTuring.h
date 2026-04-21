#ifndef MACROS_FOR_TURING
#define MACROS_FOR_TURING

#include "TuringMachine.h"
#include <string>

// Единый алфавит символов, которые каретка должна уметь игнорировать при
// перемещении. m - для сравнения, c - для сложения.
const std::string ALPHABET = "01_:#abcdefghijklmnopqrstuvwxyzcm";

// Базовый макрос: Идет ВПРАВО, пропуская все символы, пока не встретит target
void GenerateMoveRightUntil(TuringMachine &tm, const std::string &startState,
                            char target, const std::string &endState) {
  for (char s : ALPHABET) {
    if (s != target) {
      tm.AddRule(startState, s, startState, s, Direction::Right);
    }
  }
  tm.AddRule(startState, target, endState, target, Direction::Stay);
}

// Базовый макрос: Идет ВЛЕВО, пропуская все символы, пока не встретит target
void GenerateMoveLeftUntil(TuringMachine &tm, const std::string &startState,
                           char target, const std::string &endState) {
  for (char s : ALPHABET) {
    if (s != target) {
      tm.AddRule(startState, s, startState, s, Direction::Left);
    }
  }
  tm.AddRule(startState, target, endState, target, Direction::Stay);
}

// Базовый макрос: Возвращает каретку в самое начало ленты (до левого края, где
// пустота '_')
void GenerateReturnToStart(TuringMachine &tm, const std::string &state,
                           const std::string &nextState) {
  for (char s : ALPHABET) {
    if (s != '_') {
      tm.AddRule(state, s, state, s, Direction::Left);
    }
  }
  // Уперлись в левую границу, делаем шаг вправо на первый символ и передаем
  // управление
  tm.AddRule(state, '_', nextState, '_', Direction::Right);
}

// ====================================================================================
// --- Макрос: Инкремент (var++) ---
// ====================================================================================
void GenerateIncrement(TuringMachine &tm, const std::string &startState,
                       char varName, const std::string &nextState) {
  std::string s1 = "inc_seek_end_" + std::string(1, varName);
  std::string s2 = "inc_write_" + std::string(1, varName);
  std::string s3 = "inc_return_" + std::string(1, varName);

  // 1. Ищем переменную вправо
  GenerateMoveRightUntil(tm, startState, varName, s1);

  // 2. Нашли. Пропускаем имя переменной, ':' и все '1'
  tm.AddRule(s1, varName, s1, varName, Direction::Right);
  tm.AddRule(s1, ':', s1, ':', Direction::Right);
  tm.AddRule(s1, '1', s1, '1', Direction::Right);

  // Уперлись в '#'. Заменяем на '1'
  tm.AddRule(s1, '#', s2, '1', Direction::Right);

  // 3. Ставим новый '#' на свободное место правее
  tm.AddRule(s2, '_', s3, '#', Direction::Stay);
  tm.AddRule(s2, '#', s3, '#',
             Direction::Stay); // На случай, если между переменными нет пробела

  // 4. Откидываем каретку в начало ленты для следующей инструкции
  GenerateReturnToStart(tm, s3, nextState);
}

// ====================================================================================
// --- Макрос: Сравнение (X ? Y) ---
// ====================================================================================
void GenerateCompare(TuringMachine &tm, const std::string &startState,
                     char xName, char yName, const std::string &stateGreater,
                     const std::string &stateLess,
                     const std::string &stateEqual) {

  std::string prefix =
      "cmp_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";
  std::string sFindX = prefix + "find_x";
  std::string sCheckX = prefix + "check_x";
  std::string sGoY = prefix + "go_y";
  std::string sCheckY = prefix + "check_y";
  std::string sBackX = prefix + "back_x";
  std::string sCheckYEmpty = prefix + "check_y_empty";

  // 1. Ищем X
  GenerateMoveRightUntil(tm, startState, xName, sFindX);

  // 2. Ищем '1' в X
  tm.AddRule(sFindX, xName, sFindX, xName, Direction::Right);
  tm.AddRule(sFindX, ':', sFindX, ':', Direction::Right);
  tm.AddRule(sFindX, 'm', sFindX, 'm',
             Direction::Right); // Пропускаем уже помеченные

  tm.AddRule(sFindX, '1', sGoY, 'm',
             Direction::Right); // Нашли '1' -> 'm', идем к Y
  tm.AddRule(sFindX, '#', sCheckYEmpty, '#',
             Direction::Right); // X пуст, идем проверять Y

  // 3. Идем к Y
  GenerateMoveRightUntil(tm, sGoY, yName, sCheckY);

  // 4. Ищем '1' в Y
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, 'm', sCheckY, 'm', Direction::Right);

  tm.AddRule(sCheckY, '1', sBackX, 'm',
             Direction::Left); // Нашли пару -> 'm', возврат к X
  tm.AddRule(sCheckY, '#', prefix + "cleanup_G", '#',
             Direction::Left); // В Y пусто, а в X только что была '1'. X > Y.

  // 5. Возврат к X
  GenerateMoveLeftUntil(tm, sBackX, xName,
                        sFindX); // Идем влево до xName и зацикливаемся

  // 6. Проверка Y (вызывается, если X закончился первым)
  std::string sYFind = sCheckYEmpty + "_find";
  GenerateMoveRightUntil(tm, sCheckYEmpty, yName, sYFind);

  tm.AddRule(sYFind, yName, sYFind, yName, Direction::Right);
  tm.AddRule(sYFind, ':', sYFind, ':', Direction::Right);
  tm.AddRule(sYFind, 'm', sYFind, 'm', Direction::Right);

  tm.AddRule(sYFind, '1', prefix + "cleanup_L", '1',
             Direction::Left); // В Y есть остаток. X < Y
  tm.AddRule(sYFind, '#', prefix + "cleanup_E", '#',
             Direction::Left); // В Y тоже пусто. X = Y

  // 7. Очистка (восстанавливаем 'm' -> '1' и возвращаемся в начало)
  for (std::string res : {"_G", "_L", "_E"}) {
    std::string state = prefix + "cleanup" + res;
    std::string finalState =
        (res == "_G" ? stateGreater : (res == "_L" ? stateLess : stateEqual));

    // Превращаем 'm' обратно в '1' на ходу
    tm.AddRule(state, 'm', state, '1', Direction::Left);
    for (char s : ALPHABET) {
      if (s != 'm' && s != '_') {
        tm.AddRule(state, s, state, s, Direction::Left);
      }
    }
    // Уперлись в левый край -> передаем управление дальше
    tm.AddRule(state, '_', finalState, '_', Direction::Right);
  }
}

// ====================================================================================
// --- Макрос: Сложение (X = X + Y) ---
// ====================================================================================
void GenerateAdd(TuringMachine &tm, const std::string &startState, char xName,
                 char yName, const std::string &nextState) {
  std::string prefix =
      "add_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";
  std::string sFindY = prefix + "find_y";
  std::string sCheckY = prefix + "check_y";
  std::string sGoX = prefix + "go_x";
  std::string sWriteX = prefix + "write_x";
  std::string sGoNextY = prefix + "go_next_y";
  std::string sRestore = prefix + "restore";

  // 1. Ищем Y
  GenerateMoveRightUntil(tm, startState, yName, sCheckY);

  // 2. Ищем '1' в Y
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, 'c', sCheckY, 'c', Direction::Right);

  tm.AddRule(sCheckY, '1', sGoX, 'c',
             Direction::Left); // Взяли '1', пометили 'c', бежим к X
  tm.AddRule(sCheckY, '#', sRestore, '#',
             Direction::Left); // Y кончился, идем восстанавливать

  // 3. Возврат к X
  GenerateMoveLeftUntil(tm, sGoX, xName, sWriteX);

  // 4. Запись в X
  tm.AddRule(sWriteX, xName, sWriteX, xName, Direction::Right);
  tm.AddRule(sWriteX, ':', sWriteX, ':', Direction::Right);
  tm.AddRule(sWriteX, '1', sWriteX, '1', Direction::Right);

  tm.AddRule(sWriteX, '#', sGoNextY, '1',
             Direction::Right); // Заменяем '#' на '1'

  // 5. Ставим новый '#' и идем обратно к Y
  tm.AddRule(sGoNextY, '_', sGoNextY + "_search", '#', Direction::Right);
  tm.AddRule(sGoNextY, '#', sGoNextY + "_search", '#', Direction::Right);

  GenerateMoveRightUntil(tm, sGoNextY + "_search", yName,
                         sCheckY); // Зацикливаемся

  // 6. Очистка Y (восстанавливаем 'c' -> '1' и возвращаемся в начало)
  tm.AddRule(sRestore, 'c', sRestore, '1', Direction::Left);
  for (char s : ALPHABET) {
    if (s != 'c' && s != '_') {
      tm.AddRule(sRestore, s, sRestore, s, Direction::Left);
    }
  }
  tm.AddRule(sRestore, '_', nextState, '_', Direction::Right);
}

#endif // MACROS_FOR_TURING