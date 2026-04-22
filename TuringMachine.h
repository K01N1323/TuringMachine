#include "MacrosForTuring.h"
#include "MemoryManager.h"
#include "TuringMachine.h"
#include <iostream>

// =========================================================
// ТЕСТ 1: Сложение (X = X + Y)
// Ожидаем: 3 + 4 = 7
// =========================================================
void TestAddition() {
  std::cout << "--- [TEST 1] Addition (3 + 4) ---\n";
  TuringMachine tm("start");
  MemoryManager mem;

  mem.Allocate("x", 3);
  mem.Allocate("y", 4);
  mem.Deploy(tm);

  GenerateAdd(tm, "start", mem.GetAddress("x"), mem.GetAddress("y"), "halt");
  tm.Run();

  int result = mem.GetDecimalValue(tm, "x");
  std::cout << "x (Expected 7) = " << result
            << (result == 7 ? " [PASSED]" : " [FAILED]") << "\n";
  std::cout << "y (Expected 4) = " << mem.GetDecimalValue(tm, "y") << "\n\n";
}

// =========================================================
// ТЕСТ 2: Усеченное вычитание (X = X - Y)
// Ожидаем: 5 - 2 = 3
// =========================================================
void TestSubtraction() {
  std::cout << "--- [TEST 2] Subtraction (5 - 2) ---\n";
  TuringMachine tm("start");
  MemoryManager mem;

  mem.Allocate("x", 5);
  mem.Allocate("y", 2);
  mem.Deploy(tm);

  GenerateSubtract(tm, "start", mem.GetAddress("x"), mem.GetAddress("y"),
                   "halt");
  tm.Run();

  int result = mem.GetDecimalValue(tm, "x");
  std::cout << "x (Expected 3) = " << result
            << (result == 3 ? " [PASSED]" : " [FAILED]") << "\n";
  std::cout << "y (Expected 2) = " << mem.GetDecimalValue(tm, "y") << "\n\n";
}

// =========================================================
// ТЕСТ 3: Умножение (Result = X * Y)
// Ожидаем: 4 * 5 = 20
// =========================================================
void TestMultiplication() {
  std::cout << "--- [TEST 3] Multiplication (4 * 5) ---\n";
  TuringMachine tm("start");
  MemoryManager mem;

  mem.Allocate("result", 0);
  mem.Allocate("x", 4);
  mem.Allocate("y", 5);
  mem.Deploy(tm);

  // Передаем адреса: куда писать (result), что прибавлять (x), сколько раз (y)
  GenerateMultiply(tm, "start", mem.GetAddress("result"), mem.GetAddress("x"),
                   mem.GetAddress("y"), "halt");
  tm.Run();

  int result = mem.GetDecimalValue(tm, "result");
  std::cout << "result (Expected 20) = " << result
            << (result == 20 ? " [PASSED]" : " [FAILED]") << "\n\n";
}

// =========================================================
// ТЕСТ 4: Копирование / Присваивание (X = Y)
// Ожидаем: X станет равно 9, Y останется 9
// =========================================================
void TestAssign() {
  std::cout << "--- [TEST 4] Assignment (X = Y) ---\n";
  TuringMachine tm("start");
  MemoryManager mem;

  mem.Allocate("x", 2); // Изначально тут мусор
  mem.Allocate("y", 9);
  mem.Deploy(tm);

  GenerateAssign(tm, "start", mem.GetAddress("x"), mem.GetAddress("y"), "halt");
  tm.Run();

  int resultX = mem.GetDecimalValue(tm, "x");
  int resultY = mem.GetDecimalValue(tm, "y");
  std::cout << "x (Expected 9) = " << resultX
            << (resultX == 9 ? " [PASSED]" : " [FAILED]") << "\n";
  std::cout << "y (Expected 9) = " << resultY << "\n\n";
}

int main() {
  std::cout << "Starting Turing Machine Hardware "
               "Tests...\n=========================================\n\n";

  TestAddition();
  TestSubtraction();
  TestMultiplication();
  TestAssign();

  std::cout
      << "=========================================\nAll tests completed.\n";
  return 0;
}