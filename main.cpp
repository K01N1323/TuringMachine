#include "MacrosForTuring.h"
#include "MemoryManager.h"
#include "TuringMachine.h"
#include <iostream>

int main() {
  TuringMachine tm("start");
  MemoryManager mem;

  // 1. Выделение памяти (ОС)
  mem.Allocate("x", 5);      // x = 5
  mem.Allocate("y", 3);      // y = 3
  mem.Allocate("result", 0); // result = 0
  mem.Deploy(tm);

  // 2. Компиляция микрокода (result = x * y)
  // Генерируем макрос умножения, передавая физические адреса переменных
  GenerateMultiply(tm, "start", mem.GetAddress("result"), mem.GetAddress("x"),
                   mem.GetAddress("y"), "halt");

  // 3. Выполнение на "железе" без графического спама (скорость!)
  std::cout << "[TM] Executing microcode...\n";
  tm.Run(); // Используем Run() вместо RunAnimated() для мгновенного результата

  // 4. Получение результата обратно в C++ пространство
  int decimalResult = mem.GetDecimalValue(tm, "result");

  std::cout << "====================================\n";
  std::cout << "Calculation finished successfully.\n";
  std::cout << "x = " << mem.GetDecimalValue(tm, "x") << "\n";
  std::cout << "y = " << mem.GetDecimalValue(tm, "y") << "\n";
  std::cout << "result = " << decimalResult << "\n";
  std::cout << "====================================\n";

  return 0;
}