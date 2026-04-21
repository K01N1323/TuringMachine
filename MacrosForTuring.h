#ifndef MACROS_FOR_TURING
#define MACROS_FOR_TURING

#include "TuringMachine.h"

// Параметр alphabet содержит все допустимые символы, которые машина может
// встретить
void GenerateMoveRightUntil(
    TuringMachine &tm, const std::string &startState, char target,
    const std::string &endState,
    const std::string &alphabet =
        "01_:#abcdefghigklmnopqrstuvwxyz") // Алфавит по умолчанию
{
  // 1. Генерируем правила для пропуска всех "лишних" символов
  for (char s : alphabet) {
    if (s != target) {
      // Если видим символ s (и он не target) -> пишем его же, идем вправо,
      // остаемся в поиске
      tm.AddRule(startState, s, startState, s, Direction::Right);
    }
  }

  // 2. Генерируем победное правило
  // Если видим target -> оставляем его, никуда не двигаемся (Stay), переходим в
  // endState
  tm.AddRule(startState, target, endState, target, Direction::Stay);
}

void GenerateIncrement(TuringMachine &tm, std::string varName,
                       std::string nextInstructionState) {
  std::string s1 = "find_" + varName;
  std::string s2 = "seek_end_" + varName;
  std::string s3 = "write_new_" + varName;
  std::string s4 = "shift_delim_" + varName;
  std::string s5 = "return_" + varName;

  // 1. Ищем начало переменной
  // Если видим не имя переменной — идем вправо
  tm.AddRule(s1, '#', s1, '#', Direction::Right);
  tm.AddRule(s1, '_', s1, '_', Direction::Right);
  // Нашли имя (например, 'x')
  tm.AddRule(s1, varName[0], s2, varName[0], Direction::Right);

  // 2. Перематываем значение (пропускаем ':' и все '1')
  tm.AddRule(s2, ':', s2, ':', Direction::Right);
  tm.AddRule(s2, '1', s2, '1', Direction::Right);
  // Уперлись в разделитель '#'
  tm.AddRule(s2, '#', s3, '1', Direction::Right); // Пишем 1 вместо #

  // 3. Ставим новый разделитель правее
  tm.AddRule(s3, '_', s4, '#', Direction::Left);
  tm.AddRule(
      s3, '1', s4, '#',
      Direction::Left); // Если там было что-то другое (другая переменная)

  // 4. Возвращаемся в начало ленты (до самого первого '#' или начала)
  tm.AddRule(s4, '1', s4, '1', Direction::Left);
  tm.AddRule(s4, ':', s4, ':', Direction::Left);
  tm.AddRule(s4, varName[0], s4, varName[0], Direction::Left);
  tm.AddRule(s4, '#', s5, '#', Direction::Left); // Встретили левый край

  // 5. Когда дошли до упора — передаем управление следующей команде псевдокода
  tm.AddRule(s5, '_', nextInstructionState, '_', Direction::Right);
}

void GenerateCompare(TuringMachine &tm, char xName, char yName,
                     std::string stateGreater, std::string stateLess,
                     std::string stateEqual) {

  std::string prefix =
      "cmp_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";

  // Состояния
  std::string sFindX = prefix + "find_x";
  std::string sGoY = prefix + "go_y";
  std::string sFindY = prefix + "find_y";
  std::string sBackX = prefix + "back_x";
  std::string sCheckY = prefix + "check_y";
  std::string sCleanup = prefix + "cleanup";

  // 1. Ищем '1' в переменной X
  // Сначала находим саму букву переменной
  tm.AddRule("start_cmp", xName, sFindX, xName, Direction::Right);

  tm.AddRule(sFindX, ':', sFindX, ':', Direction::Right);
  tm.AddRule(sFindX, 'm', sFindX, 'm',
             Direction::Right); // Пропускаем уже помеченные
  tm.AddRule(sFindX, '1', sGoY, 'm',
             Direction::Right); // Нашли! Пометили 'm', идем к Y
  tm.AddRule(sFindX, '#', sCheckY, '#',
             Direction::Right); // X кончился, идем проверять Y

  // 2. Идем к переменной Y
  tm.AddRule(sGoY, '#', sGoY, '#', Direction::Right);
  tm.AddRule(sGoY, '_', sGoY, '_', Direction::Right);
  tm.AddRule(sGoY, yName, sFindY, yName, Direction::Right);

  // 3. Ищем '1' в Y для пары
  tm.AddRule(sFindY, ':', sFindY, ':', Direction::Right);
  tm.AddRule(sFindY, 'm', sFindY, 'm', Direction::Right);
  tm.AddRule(sFindY, '1', sBackX, 'm',
             Direction::Left); // Нашли пару! Возвращаемся к X

  // Если в Y нет '1' (встретили #), а в X мы только что пометили единицу —
  // значит X > Y
  tm.AddRule(sFindY, '#', sCleanup + "_G", '#', Direction::Left);

  // 4. Возврат к X (после нахождения пары)
  tm.AddRule(sBackX, '1', sBackX, '1', Direction::Left);
  tm.AddRule(sBackX, 'm', sBackX, 'm', Direction::Left);
  tm.AddRule(sBackX, ':', sBackX, ':', Direction::Left);
  tm.AddRule(sBackX, yName, sBackX, yName, Direction::Left);
  tm.AddRule(sBackX, xName, sBackX, xName, Direction::Left);
  tm.AddRule(sBackX, '#', sFindX, '#', Direction::Right); // Снова в поиск X

  // 5. Проверка Y (если X уже пуст)
  tm.AddRule(sCheckY, '#', sCheckY, '#', Direction::Right);
  tm.AddRule(sCheckY, yName, sCheckY, yName, Direction::Right);
  tm.AddRule(sCheckY, ':', sCheckY, ':', Direction::Right);
  tm.AddRule(sCheckY, 'm', sCheckY, 'm', Direction::Right);
  tm.AddRule(sCheckY, '1', sCleanup + "_L", '1',
             Direction::Left); // В Y еще есть 1 -> X < Y
  tm.AddRule(sCheckY, '#', sCleanup + "_E", '#',
             Direction::Left); // В Y тоже пусто -> X = Y

  // 6. Очистка (Cleanup) - нужно вернуть все 'm' в '1'
  // Для краткости: машина бежит влево до упора, меняя 'm' на '1'
  for (std::string res : {"_G", "_L", "_E"}) {
    std::string finalState =
        (res == "_G" ? stateGreater : (res == "_L" ? stateLess : stateEqual));
    tm.AddRule(sCleanup + res, 'm', sCleanup + res, '1', Direction::Left);
    tm.AddRule(sCleanup + res, '1', sCleanup + res, '1', Direction::Left);
    tm.AddRule(sCleanup + res, ':', sCleanup + res, ':', Direction::Left);
    tm.AddRule(sCleanup + res, xName, sCleanup + res, xName, Direction::Left);
    tm.AddRule(sCleanup + res, yName, sCleanup + res, yName, Direction::Left);
    tm.AddRule(sCleanup + res, '#', sCleanup + res, '#', Direction::Left);
    tm.AddRule(sCleanup + res, '_', finalState, '_',
               Direction::Right); // Конец очистки
  }
}

void GenerateAdd(TuringMachine &tm, char xName, char yName,
                 std::string nextState) {
  std::string prefix =
      "add_" + std::string(1, xName) + "_" + std::string(1, yName) + "_";

  std::string sFindY = prefix + "find_y";
  std::string sGrab = prefix + "grab_one";
  std::string sGoX = prefix + "go_x";
  std::string sWrite = prefix + "write_x";
  std::string sBackY = prefix + "back_y";
  std::string sRestore = prefix + "restore_y";

  // 1. Идем к переменной Y, чтобы "взять" единицу
  tm.AddRule("start_add", yName, sGrab, yName, Direction::Right);

  tm.AddRule(sGrab, ':', sGrab, ':', Direction::Right);
  tm.AddRule(sGrab, 'c', sGrab, 'c',
             Direction::Right); // Пропускаем уже скопированные
  tm.AddRule(sGrab, '1', sGoX, 'c',
             Direction::Left); // Взяли 1! Пометили 'c', идем к X
  tm.AddRule(sGrab, '#', sRestore, '#',
             Direction::Left); // В Y больше нет 1. Идем чистить 'c'

  // 2. Несем единицу к X
  tm.AddRule(sGoX, '#', sGoX, '#', Direction::Left);
  tm.AddRule(sGoX, yName, sGoX, yName, Direction::Left);
  tm.AddRule(sGoX, xName, sWrite, xName, Direction::Right);

  // 3. Записываем единицу в конец X (используем логику инкремента)
  tm.AddRule(sWrite, ':', sWrite, ':', Direction::Right);
  tm.AddRule(sWrite, '1', sWrite, '1', Direction::Right);
  tm.AddRule(sWrite, '#', sBackY, '1', Direction::Right); // Вместо # пишем 1

  // 4. Сдвигаем разделитель # (важно!)
  tm.AddRule(sBackY, '_', sBackY + "_return", '#', Direction::Left);

  // 5. Возвращаемся к Y за следующей
  tm.AddRule(sBackY + "_return", '1', sBackY + "_return", '1', Direction::Left);
  tm.AddRule(sBackY + "_return", xName, sBackY + "_return", xName,
             Direction::Left);
  tm.AddRule(sBackY + "_return", '#', sGrab, '#', Direction::Right);

  // 6. Восстановление Y (меняем 'c' обратно на '1')
  tm.AddRule(sRestore, 'c', sRestore, '1', Direction::Left);
  tm.AddRule(sRestore, ':', sRestore, ':', Direction::Left);
  tm.AddRule(sRestore, yName, sRestore, yName, Direction::Left);
  tm.AddRule(sRestore, '#', sRestore, '#', Direction::Left);
  tm.AddRule(sRestore, '_', nextState, '_', Direction::Right);
}

#endif // MACROS_FOR_TURING
