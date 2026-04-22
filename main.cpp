#include "Compiler.h"
#include <iostream>

int main() {
  TuringMachine tm("start");
  MemoryManager mem;
  Compiler compiler(tm, mem);

  // НАШ ПСЕВДОКОД!
  std::vector<std::string> sourceCode = {
      "x = 5",              // Инициализируем x
      "y = 3",              // Инициализируем y
      "x++",                // x становится 6
      "y--",                // y становится 2
      "result = x * y",     // result = 6 * 2 = 12
      "result = result - x" // result = 12 - 6 = 6
  };

  std::cout << "[Compiler] Translating pseudocode to MT instructions...\n";
  compiler.Compile(sourceCode);

  std::cout << "[Memory] Deploying layout onto the tape...\n";
  mem.Deploy(tm);

  std::cout << "[Processor] Executing microcode on Turing Machine...\n";
  tm.Run(); // Исполняем на полной скорости

  std::cout << "\n=== FINAL REGISTERS ===\n";
  std::cout << "x = " << mem.GetDecimalValue(tm, "x") << "\n";
  std::cout << "y = " << mem.GetDecimalValue(tm, "y") << "\n";
  std::cout << "result = " << mem.GetDecimalValue(tm, "result") << "\n";
  std::cout << "=======================\n";

  return 0;
}