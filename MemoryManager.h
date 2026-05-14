#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "TuringMachine.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Variable {
    std::string name;
    int varIndex;     // Logical index (0, 1, 2...)
    int initialValue;
};

class MemoryManager {
private:
    std::vector<Variable> variables;
    std::unordered_map<std::string, int> symbolTable;
    
    // Each variable takes 34 cells: 
    // Cell 0: '#'
    // Cell 1: '+' or '-'
    // Cell 2-33: 32 binary digits (MSB at 2, LSB at 33)
    static constexpr int VAR_SIZE = 34;

public:
    void Allocate(const std::string& name, int initialValue = 0);
    void Clear() { variables.clear(); symbolTable.clear(); }
    std::vector<std::string> GetVariableNames() const {
        std::vector<std::string> names;
        for (const auto& var : variables) names.push_back(var.name);
        return names;
    }
    int GetVarIndex(const std::string& name) const;
    void Deploy(TuringMachine& tm) const;
    int GetDecimalValue(const TuringMachine& tm, const std::string& name) const;
    
    // Return physical tape start position for a variable
    int GetAddress(const std::string& name) const { return GetVarIndex(name) * VAR_SIZE; }
};

#endif // MEMORY_MANAGER_H