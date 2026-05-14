#ifndef COMPILER_H
#define COMPILER_H

#include "MemoryManager.h"
#include "TuringMachine.h"
#include <string>
#include <vector>

class Compiler {
private:
    TuringMachine& tm;
    MemoryManager& mem;
    int stateCounter = 0;
    int tempVarCounter = 0;

    std::string NextState();
    
    struct ASTNode {
        std::string value;
        ASTNode* left = nullptr;
        ASTNode* right = nullptr;
        ~ASTNode() { delete left; delete right; }
    };

    std::vector<std::string> Tokenize(const std::string& expr);
    ASTNode* BuildAST(const std::vector<std::string>& tokens);
    
    // Generates TM states to evaluate the AST node and leaves the result in a memory variable.
    // Returns the memory variable name holding the result.
    std::string CompileAST(ASTNode* node, std::string& currentState);

public:
    Compiler(TuringMachine& tm, MemoryManager& mem) : tm(tm), mem(mem) {}
    void Compile(const std::vector<std::string>& sourceCode);
    void Execute();
};

#endif // COMPILER_H