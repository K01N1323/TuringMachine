#ifndef MACROS_FOR_TURING
#define MACROS_FOR_TURING

#include "TuringMachine.h"
#include <string>

// Единый алфавит.
// * - флаг для сложения, @ - флаг для умножения/сравнения/вычитания
// ^ - МАРКЕР АБСОЛЮТНОГО НАЧАЛА ЛЕНТЫ (The Wall)
const std::string ALPHABET = "01_:#abcdefghijklmnopqrstuvwxyz*@^";

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

// Базовый макрос: Возвращает каретку в самое начало ленты (до физической стены
// '^')
void GenerateReturnToStart(TuringMachine &tm, const std::string &state,
                           const std::string &nextState) {
  for (char s : ALPHABET) {
    // Идем влево по всему алфавиту, ВКЛЮЧАЯ '_', пока не найдем стену '^'
    if (s != '^') {
      tm.AddRule(state, s, state, s, Direction::Left);
    }
  }
  // Уперлись в левую стену '^', делаем шаг вправо и передаем управление
  tm.AddRule(state, '^', nextState, '^', Direction::Right);
}

// ====================================================================================
// --- Макрос: Инкремент (var++) ---
// ====================================================================================
void GenerateIncrement(TuringMachine &tm, const std::string &startState,
                       char varName, const std::string &nextState) {
  std::string s1 = "inc_seek_end_" + std::string(1, varName);
  std::string s2 = "inc_write_" + std::string(1, varName);
  std::string s3 = "inc_return_" + std::string(1, varName);

  GenerateMoveRightUntil(tm, startState, varName, s1);

  tm.AddRule(s1, varName, s1, varName, Direction::Right);
  tm.AddRule(s1, ':', s1, ':', Direction::Right);
  tm.AddRule(s1, '1', s1, '1', Direction::Right);

  tm.AddRule(s1, '#', s2, '1', Direction::Right);

  tm.AddRule(s2, '_', s3, '#', Direction::Stay);
  tm.AddRule(s2, '#', s3, '#', Direction::Stay);

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

  GenerateMoveRightUntil(tm, startState, xName, sFindX);

  tm.AddRule(sFindX, xName, sFindX, xName, Direction::Right);
  tm.AddRule(sFindX, ':', sFindX, ':', Direction::Right);
  tm.AddRule(sFindX, '@', sFindX, '@', Direction::Right);

  tm.AddRule(sFindX, '1', sGoY, '@', Direction::Right);
  tm.AddRule(sFindX, '#', sCheckYEmpty, '#', Direction::Right);

  GenerateMoveRightUntil(tm, sGoY, yName, sCheckY);

  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, '@', sCheckY, '@', Direction::Right);

  tm.AddRule(sCheckY, '1', sBackX, '@', Direction::Left);
  tm.AddRule(sCheckY, '#', prefix + "cleanup_G", '#', Direction::Left);

  GenerateMoveLeftUntil(tm, sBackX, xName, sFindX);

  std::string sYFind = sCheckYEmpty + "_find";
  GenerateMoveRightUntil(tm, sCheckYEmpty, yName, sYFind);

  tm.AddRule(sYFind, yName, sYFind, yName, Direction::Right);
  tm.AddRule(sYFind, ':', sYFind, ':', Direction::Right);
  tm.AddRule(sYFind, '@', sYFind, '@', Direction::Right);

  tm.AddRule(sYFind, '1', prefix + "cleanup_L", '1', Direction::Left);
  tm.AddRule(sYFind, '#', prefix + "cleanup_E", '#', Direction::Left);

  for (std::string res : {"_G", "_L", "_E"}) {
    std::string state = prefix + "cleanup" + res;
    std::string finalState =
        (res == "_G" ? stateGreater : (res == "_L" ? stateLess : stateEqual));

    tm.AddRule(state, '@', state, '1', Direction::Left);
    for (char s : ALPHABET) {
      if (s != '@' && s != '^') {
        tm.AddRule(state, s, state, s, Direction::Left);
      }
    }
    tm.AddRule(state, '^', finalState, '^', Direction::Right);
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

  GenerateReturnToStart(tm, startState, sStartLoop);

  GenerateMoveRightUntil(tm, sStartLoop, yName, sCheckY);
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, '*', sCheckY, '*', Direction::Right);

  tm.AddRule(sCheckY, '1', sGoStartX, '*', Direction::Stay);

  tm.AddRule(sCheckY, '#', sRestore, '#', Direction::Stay);

  GenerateReturnToStart(tm, sGoStartX, sFindX);

  GenerateMoveRightUntil(tm, sFindX, xName, sWriteX);
  tm.AddRule(sWriteX, xName, sWriteX, xName, Direction::Right);
  tm.AddRule(sWriteX, ':', sWriteX, ':', Direction::Right);
  tm.AddRule(sWriteX, '1', sWriteX, '1', Direction::Right);

  tm.AddRule(sWriteX, '#', sGoNext, '1', Direction::Right);

  tm.AddRule(sGoNext, '_', prefix + "loop_back", '#', Direction::Stay);
  tm.AddRule(sGoNext, '#', prefix + "loop_back", '#', Direction::Stay);

  GenerateReturnToStart(tm, prefix + "loop_back", sStartLoop);

  GenerateReturnToStart(tm, sRestore, prefix + "clean_y");
  GenerateMoveRightUntil(tm, prefix + "clean_y", yName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", yName, prefix + "cleaning", yName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '*', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);

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

  GenerateMoveRightUntil(tm, startState, xName, sFind);

  tm.AddRule(sFind, xName, sFind, xName, Direction::Right);
  tm.AddRule(sFind, ':', sErase, ':', Direction::Right);

  tm.AddRule(sErase, '1', sErase, '_', Direction::Right);

  tm.AddRule(sErase, '#', sBack, '_', Direction::Left);

  tm.AddRule(sBack, '_', sBack, '_', Direction::Left);
  tm.AddRule(sBack, ':', sWrite, ':', Direction::Right);

  tm.AddRule(sWrite, '_', prefix + "return", '#', Direction::Stay);

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

  GenerateMoveRightUntil(tm, startState, xName, sFind);
  tm.AddRule(sFind, xName, sFind, xName, Direction::Right);
  tm.AddRule(sFind, ':', sScan, ':', Direction::Right);

  tm.AddRule(sScan, '1', sScan, '1', Direction::Right);

  tm.AddRule(sScan, '#', sCheck, '#', Direction::Left);

  tm.AddRule(sCheck, '1', sErase, '#', Direction::Right);

  tm.AddRule(sErase, '#', prefix + "return", '_', Direction::Stay);

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

  GenerateMoveRightUntil(tm, startState, yName, sCheckY);
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, '@', sCheckY, '@', Direction::Right);

  tm.AddRule(sCheckY, '1', sGoStartX, '@', Direction::Stay);

  tm.AddRule(sCheckY, '#', sRestore, '#', Direction::Stay);

  std::string sDecX = prefix + "do_dec_x";
  GenerateReturnToStart(tm, sGoStartX, sDecX);

  std::string sLoopBack = prefix + "loop_back";
  GenerateDecrement(tm, sDecX, xName, sLoopBack);

  GenerateReturnToStart(tm, sLoopBack, startState);

  GenerateReturnToStart(tm, sRestore, prefix + "clean_y");
  GenerateMoveRightUntil(tm, prefix + "clean_y", yName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", yName, prefix + "cleaning", yName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '@', prefix + "cleaning", '1',
             Direction::Right);

  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);
  GenerateReturnToStart(tm, prefix + "finish", nextState);
}

// ====================================================================================
// --- Макрос: Умножение с накоплением (X = X + Y * Z) ---
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

  GenerateReturnToStart(tm, startState, sLoopZ);
  GenerateMoveRightUntil(tm, sLoopZ, zName, sCheckZ);

  tm.AddRule(sCheckZ, zName, sCheckZ, zName, Direction::Right);
  tm.AddRule(sCheckZ, ':', sCheckZ, ':', Direction::Right);
  tm.AddRule(sCheckZ, '@', sCheckZ, '@', Direction::Right);

  tm.AddRule(sCheckZ, '1', sDoAdd, '@', Direction::Stay);

  tm.AddRule(sCheckZ, '#', sRestoreZ, '#', Direction::Stay);

  GenerateAdd(tm, sDoAdd, xName, yName, sLoopZ);

  GenerateReturnToStart(tm, sRestoreZ, prefix + "clean_z");
  GenerateMoveRightUntil(tm, prefix + "clean_z", zName, prefix + "cleaning");

  tm.AddRule(prefix + "cleaning", zName, prefix + "cleaning", zName,
             Direction::Right);
  tm.AddRule(prefix + "cleaning", ':', prefix + "cleaning", ':',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '1', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '@', prefix + "cleaning", '1',
             Direction::Right);
  tm.AddRule(prefix + "cleaning", '#', prefix + "finish", '#', Direction::Stay);

  GenerateReturnToStart(tm, prefix + "finish", nextState);
}

// ====================================================================================
// --- Макрос: Присваивание / Копирование (Dest = Src) ---
// ====================================================================================
void GenerateAssign(TuringMachine &tm, const std::string &startState,
                    char destName, char srcName, const std::string &nextState) {
  std::string sClear = "assign_clear_" + std::string(1, destName) + "_" +
                       std::string(1, srcName);

  GenerateClear(tm, startState, destName, sClear);
  GenerateAdd(tm, sClear, destName, srcName, nextState);
}

#endif // MACROS_FOR_TURING