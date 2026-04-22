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
// --- Макрос: Безопасное Сложение (X = X + Y) ---
// ====================================================================================
void GenerateAdd(TuringMachine &tm, const std::string &startState, char xName,
                 char yName, const std::string &nextState) {
  std::string prefix =
      "add_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";
  std::string sStartLoop = prefix + "start_loop";
  std::string sCheckY = prefix + "check_y";
  std::string sGoStartX = prefix + "go_start_x";
  std::string sFindX = prefix + "find_x";
  std::string sWriteX = prefix + "write_x";
  std::string sGoNext = prefix + "go_next";
  std::string sRestore = prefix + "restore";

  // 1. Абсолютный возврат в начало перед циклом
  GenerateReturnToStart(tm, startState, sStartLoop);

  // 2. Идем вправо, ищем Y
  GenerateMoveRightUntil(tm, sStartLoop, yName, sCheckY);
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, 'c', sCheckY, 'c',
             Direction::Right); // Пропускаем уже скопированные

  // 3. Берем одну '1' из Y и помечаем её как 'c'
  tm.AddRule(sCheckY, '1', sGoStartX, 'c', Direction::Stay);

  // Если уперлись в '#', значит Y закончился — идем в блок восстановления
  tm.AddRule(sCheckY, '#', sRestore, '#', Direction::Stay);

  // 4. Возвращаемся в начало перед поиском X (Абсолютная адресация)
  GenerateReturnToStart(tm, sGoStartX, sFindX);

  // 5. Ищем X
  GenerateMoveRightUntil(tm, sFindX, xName, sWriteX);
  tm.AddRule(sWriteX, xName, sWriteX, xName, Direction::Right);
  tm.AddRule(sWriteX, ':', sWriteX, ':', Direction::Right);
  tm.AddRule(sWriteX, '1', sWriteX, '1', Direction::Right);

  // 6. Дописываем '1' в конец X, сдвигаем правый маркер '#'
  tm.AddRule(sWriteX, '#', sGoNext, '1', Direction::Right);

  // Благодаря MemoryManager здесь точно будет пустое место ('_') под новый '#'
  tm.AddRule(sGoNext, '_', prefix + "loop_back", '#', Direction::Stay);
  tm.AddRule(sGoNext, '#', prefix + "loop_back", '#',
             Direction::Stay); // Резерв на случай склейки

  // 7. Возврат в начало и повтор челночного бега
  GenerateReturnToStart(tm, prefix + "loop_back", sStartLoop);

  // 8. Восстановление Y (чистим 'c' обратно в '1')
  GenerateReturnToStart(tm, sRestore, prefix + "clean_y");
  GenerateMoveRightUntil(tm, prefix + "clean_y", yName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", yName, prefix + "cleaning", yName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", 'c', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);

  // 9. Окончательный возврат в начало и передача управления дальше
  GenerateReturnToStart(tm, prefix + "finish", nextState);
}

// ====================================================================================
// --- Макрос: Обнуление (X = 0) ---
// ====================================================================================
void GenerateClear(TuringMachine &tm, const std::string &startState, char xName,
                   const std::string &nextState) {
  std::string prefix = "clr_" + std::string(1, xName) + "_";
  std::string sFind = prefix + "find";
  std::string sErase = prefix + "erase";
  std::string sBack = prefix + "back";
  std::string sWrite = prefix + "write";

  // 1. Ищем переменную
  GenerateMoveRightUntil(tm, startState, xName, sFind);

  // 2. Пропускаем имя и двоеточие
  tm.AddRule(sFind, xName, sFind, xName, Direction::Right);
  tm.AddRule(sFind, ':', sErase, ':', Direction::Right);

  // 3. Стираем все '1', превращая их в пустой Padding '_'
  tm.AddRule(sErase, '1', sErase, '_', Direction::Right);

  // 4. Наткнулись на закрывающий '#'. Затираем и его, затем делаем шаг влево
  tm.AddRule(sErase, '#', sBack, '_', Direction::Left);

  // 5. Идем обратно влево по свежим пустотам '_', пока не упремся в двоеточие
  tm.AddRule(sBack, '_', sBack, '_', Direction::Left);
  tm.AddRule(sBack, ':', sWrite, ':', Direction::Right);

  // 6. Ставим новый закрывающий '#' сразу после ':'
  tm.AddRule(sWrite, '_', prefix + "return", '#', Direction::Stay);

  // 7. Возврат в начало
  GenerateReturnToStart(tm, prefix + "return", nextState);
}

// ====================================================================================
// --- Макрос: Декремент (X--) ---
// ====================================================================================
void GenerateDecrement(TuringMachine &tm, const std::string &startState,
                       char xName, const std::string &nextState) {
  std::string prefix = "dec_" + std::string(1, xName) + "_";
  std::string sFind = prefix + "find";
  std::string sScan = prefix + "scan";
  std::string sCheck = prefix + "check";
  std::string sErase = prefix + "erase";

  // 1. Ищем переменную
  GenerateMoveRightUntil(tm, startState, xName, sFind);
  tm.AddRule(sFind, xName, sFind, xName, Direction::Right);
  tm.AddRule(sFind, ':', sScan, ':', Direction::Right);

  // 2. Пробегаем все '1' до конца значения
  tm.AddRule(sScan, '1', sScan, '1', Direction::Right);

  // 3. Уперлись в '#', делаем шаг назад (на последнюю единицу)
  tm.AddRule(sScan, '#', sCheck, '#', Direction::Left);

  // 4. Проверка: если под кареткой '1', мы усекаем её (меняем на новый '#')
  tm.AddRule(sCheck, '1', sErase, '#', Direction::Right);

  // Переходим вправо на старый '#' и превращаем его в пустой Padding '_'
  tm.AddRule(sErase, '#', prefix + "return", '_', Direction::Stay);

  // 5. Альтернатива: если под кареткой ':', значит переменная уже равна 0
  // (underflow) Просто игнорируем декремент.
  tm.AddRule(sCheck, ':', prefix + "return", ':', Direction::Stay);

  GenerateReturnToStart(tm, prefix + "return", nextState);
}

// ====================================================================================
// --- Макрос: Усеченное Вычитание (X = max(0, X - Y)) ---
// ====================================================================================
void GenerateSubtract(TuringMachine &tm, const std::string &startState,
                      char xName, char yName, const std::string &nextState) {
  std::string prefix =
      "sub_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";
  std::string sFindY = prefix + "find_y";
  std::string sCheckY = prefix + "check_y";
  std::string sGoStartX = prefix + "go_start_x";
  std::string sRestore = prefix + "restore";

  // 1. Ищем Y
  GenerateMoveRightUntil(tm, startState, yName, sCheckY);
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, 'm', sCheckY, 'm',
             Direction::Right); // Пропускаем отмеченные

  // 2. Берем одну '1' из Y (помечаем как 'm')
  tm.AddRule(sCheckY, '1', sGoStartX, 'm', Direction::Stay);

  // Если в Y пусто (уперлись в '#'), идем в блок восстановления
  tm.AddRule(sCheckY, '#', sRestore, '#', Direction::Stay);

  // 3. Возвращаемся в абсолютное начало перед поиском X (Архитектурно
  // безопасно!)
  std::string sDecX = prefix + "do_dec_x";
  GenerateReturnToStart(tm, sGoStartX, sDecX);

  // 4. Декрементируем X. Вызываем наш готовый макрос!
  std::string sLoopBack = prefix + "loop_back";
  GenerateDecrement(tm, sDecX, xName, sLoopBack);

  // 5. Возвращаемся в начало и повторяем цикл
  GenerateReturnToStart(tm, sLoopBack, startState);

  // 6. Очистка Y (восстанавливаем 'm' обратно в '1')
  GenerateReturnToStart(tm, sRestore, prefix + "clean_y");
  GenerateMoveRightUntil(tm, prefix + "clean_y", yName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", yName, prefix + "cleaning", yName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", 'm', prefix + "cleaning", '1',
             Direction::Right);

  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);
  GenerateReturnToStart(tm, prefix + "finish", nextState);
}

// ====================================================================================
// --- Макрос: Умножение с накоплением (X = X + Y * Z) ---
// --- Демонстрирует паттерн "Композиция конечных автоматов"
// ====================================================================================
void GenerateMultiply(TuringMachine &tm, const std::string &startState,
                      char xName, char yName, char zName,
                      const std::string &nextState) {
  std::string prefix = "mul_" + std::string(1, xName) + "_" +
                       std::string(1, yName) + "_" + std::string(1, zName) +
                       "_";
  std::string sLoopZ = prefix + "loop_z";
  std::string sCheckZ = prefix + "check_z";
  std::string sDoAdd = prefix + "do_add";
  std::string sRestoreZ = prefix + "restore_z";

  // 1. Идем в абсолютное начало, затем ищем множитель Z
  GenerateReturnToStart(tm, startState, sLoopZ);
  GenerateMoveRightUntil(tm, sLoopZ, zName, sCheckZ);

  // 2. Ищем свободную '1' в Z
  tm.AddRule(sCheckZ, zName, sCheckZ, zName, Direction::Right);
  tm.AddRule(sCheckZ, ':', sCheckZ, ':', Direction::Right);
  tm.AddRule(
      sCheckZ, 'm', sCheckZ, 'm',
      Direction::Right); // 'm' - пометка, что эту единицу мы уже обработали

  // 3. Берем '1' из Z (заменяем на 'm') и переходим к фазе сложения
  tm.AddRule(sCheckZ, '1', sDoAdd, 'm', Direction::Stay);

  // Если Z кончился (уперлись в '#'), идем восстанавливать Z
  tm.AddRule(sCheckZ, '#', sRestoreZ, '#', Direction::Stay);

  // ========================================================================
  // 4. МАГИЯ КОМПОЗИЦИИ: Вызываем макрос сложения!
  // Он возьмет состояние sDoAdd, честно прибавит Y к X,
  // и когда закончит — вернет управление нашему циклу (в состояние sLoopZ).
  // Мы буквально вшили один конечный автомат внутрь другого.
  // ========================================================================
  GenerateAdd(tm, sDoAdd, xName, yName, sLoopZ);

  // 5. Очистка Z (восстанавливаем 'm' обратно в '1')
  GenerateReturnToStart(tm, sRestoreZ, prefix + "clean_z");
  GenerateMoveRightUntil(tm, prefix + "clean_z", zName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", zName, prefix + "cleaning", zName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", 'm', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);

  GenerateReturnToStart(tm, prefix + "finish", nextState);
}

// ====================================================================================
// --- Макрос: Присваивание / Копирование (Dest = Src) ---
// --- Архитектура: Очищаем целевую переменную, затем прибавляем к ней исходную.
// ====================================================================================
void GenerateAssign(TuringMachine &tm, const std::string &startState,
                    char destName, char srcName, const std::string &nextState) {
  std::string sClear = "assign_clear_" + std::string(1, destName) + "_" +
                       std::string(1, srcName);

  // 1. Dest = 0
  GenerateClear(tm, startState, destName, sClear);

  // 2. Dest = Dest + Src
  GenerateAdd(tm, sClear, destName, srcName, nextState);
}

#endif // MACROS_FOR_TURING