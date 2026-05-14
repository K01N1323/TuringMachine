#include "Compiler.h"
#include "GUI.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
  TuringMachine tm("start");
  MemoryManager mem;
  Compiler compiler(tm, mem);

  std::string filename = "Program.txt";
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "[Ошибка] Не удалось открыть файл: " << filename << "\n";
    return 1;
  }

  std::vector<std::string> sourceCode;
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      sourceCode.push_back(line);
    }
  }
  file.close();

  std::cout << "[Compiler] Считывание кода завершено. Трансляция...\n";
  compiler.Compile(sourceCode);
  mem.Deploy(tm);

#ifdef HEADLESS
  std::cout << "[Processor] Запуск в консольном режиме (Headless)...\n";
  
  auto PrintTapes = [&](int step) {
      std::cout << "\033[H\033[J"; // Clear screen
      std::cout << "Step: " << step << " | State: " << tm.GetCurrentState() << "\n";
      std::cout << "------------------------------------------------\n";
      for (int t = 0; t < 3; ++t) {
          std::cout << "Tape " << t << ": ";
          int hp = tm.GetHeadPosition(t);
          const auto& tape = tm.GetTape(t);
          // Show a window around the head
          for (int i = hp - 15; i <= hp + 15; ++i) {
              char c = (tape.count(i) ? tape.at(i) : '_');
              if (i == hp) std::cout << "\033[1;31m[" << c << "]\033[0m"; // Red head
              else std::cout << " " << c << " ";
          }
          std::cout << "\n";
      }
      std::cout << "------------------------------------------------\n";
      std::cout << "Memory Variables:\n";
      for (const auto& varName : {"x", "y", "z"}) {
          try {
              int val = mem.GetDecimalValue(tm, varName);
              std::cout << "  " << varName << " = " << val << "\n";
          } catch (...) {}
      }
      std::cout << std::flush;
  };

  int stepCount = 0;
  while (tm.Step()) {
      stepCount++;
      if (stepCount % 50 == 0) { // Update every 50 steps for speed
          PrintTapes(stepCount);
          // usleep(10000); // 10ms delay
      }
      if (stepCount > 100000) break; // Safety break
  }
  PrintTapes(stepCount); // Final state

  std::cout << "[Processor] Выполнение завершено.\n";
#else
  std::cout << "[Processor] Запуск GUI симулятора...\n";
  GUI gui(tm, mem, compiler);
  gui.Run();
#endif

  return 0;
}