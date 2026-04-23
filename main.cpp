#include "Compiler.h"
#include <iostream>

int main() {
  TuringMachine tm("start");
  MemoryManager mem;
  Compiler compiler(tm, mem);

  // Псевдокод: Вычисление факториала числа 5
  // 5! = 5 * 4 * 3 * 2 * 1 = 120
  std::vector<std::string> sourceCode = {
      "x = 5",    // Инициализируем число
      "fact = 1", // База факториала

      "while ( x ) {",     // Пока x > 0
      "  fact = fact * x", // Накапливаем факториал
      "  x--",             // Уменьшаем x
      "}"};

  std::cout << "[Compiler] Translating FACTORIAL program...\n";
  compiler.Compile(sourceCode);
  mem.Deploy(tm);

  std::cout
      << "[Processor] Executing on MT. Please wait, computing 5! is heavy...\n";
  tm.Run();

  std::cout << "\n=== FINAL REGISTERS ===\n";
  std::cout << "x = " << mem.GetDecimalValue(tm, "x") << "\n";
  std::cout << "fact (Expected 120) = " << mem.GetDecimalValue(tm, "fact")
            << "\n";
  std::cout << "=======================\n";

  return 0;
}