#include "MacrosForTuring.h"
#include "TuringMachine.h"
#include <chrono>
#include <iostream>
#include <thread>

void RunAnimated(TuringMachine &tm, int delayMs = 50) {
  std::cout << "Запуск симуляции...\n";
  tm.PrintTape();

  while (tm.Step()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    std::cout << "\033[2J\033[1;1H";
    tm.PrintTape();
  }
  std::cout << "\nПрограмма МТ завершена!\n";
}

int main() {
  TuringMachine tm("start");

  // Инициализируем ленту с отступами (_) чтобы переменным было куда расти
  std::string data = "_#x:11#____#y:111#____";
  for (size_t i = 0; i < data.length(); i++) {
    tm.SetTapeContent(i, data[i]);
  }

  // Собираем программу x = x + y
  // Передаем стартовое состояние "start", и финальное "halt"
  GenerateAdd(tm, "start", 'x', 'y', "halt");

  RunAnimated(tm, 40);

  return 0;
}