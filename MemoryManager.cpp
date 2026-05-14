#include "MemoryManager.h"
#include <stdexcept>
#include <iostream>

void MemoryManager::Allocate(const std::string& name, int initialValue) {
    if (symbolTable.find(name) != symbolTable.end()) return;
    int index = variables.size();
    variables.push_back({name, index, initialValue});
    symbolTable[name] = index;
}

int MemoryManager::GetVarIndex(const std::string& name) const {
    auto it = symbolTable.find(name);
    if (it != symbolTable.end()) {
        return it->second;
    }
    throw std::runtime_error("Variable not found: " + name);
}

void MemoryManager::Deploy(TuringMachine& tm) const {
    tm.SetTapeContent(0, -1, '!');
    tm.SetTapeContent(1, -1, '!');
    tm.SetTapeContent(2, -1, '!');
    for (const auto& var : variables) {
        int basePos = var.varIndex * VAR_SIZE;
        tm.SetTapeContent(0, basePos, '#'); // Delimiter
        
        int val = var.initialValue;
        if (val < 0) {
            tm.SetTapeContent(0, basePos + 1, '-');
            val = -val;
        } else {
            tm.SetTapeContent(0, basePos + 1, '+');
        }

        // 32-bit binary digits (MSB at basePos+2, LSB at basePos+33)
        // Note: the example provided in prompt has MSB first
        for (int i = 0; i < 32; ++i) {
            int bit = (val >> (31 - i)) & 1;
            tm.SetTapeContent(0, basePos + 2 + i, bit ? '1' : '0');
        }
    }
    // Final delimiter so that search macros know where memory ends (optional but safe)
    if (!variables.empty()) {
        tm.SetTapeContent(0, variables.size() * VAR_SIZE, '#');
    }
}

int MemoryManager::GetDecimalValue(const TuringMachine& tm, const std::string& name) const {
    int index = GetVarIndex(name);
    int basePos = index * VAR_SIZE;
    
    const auto& tape = tm.GetTape(0);
    
    auto getTapeChar = [&](int pos) {
        auto it = tape.find(pos);
        return it != tape.end() ? it->second : '_';
    };

    char sign = getTapeChar(basePos + 1);
    
    int val = 0;
    for (int i = 0; i < 32; ++i) {
        char bit = getTapeChar(basePos + 2 + i);
        if (bit == '1') {
            val |= (1 << (31 - i));
        }
    }

    if (sign == '-') {
        val = -val;
    }
    return val;
}