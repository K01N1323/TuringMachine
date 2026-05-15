#include "Compiler.h"
#include "MacrosForTuring.h"
#include <sstream>
#include <iostream>
#include <stack>
#include <cctype>
#include <algorithm>

std::string Compiler::NextState() {
    return "q_auto_" + std::to_string(++stateCounter);
}

std::vector<std::string> Compiler::Tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < line.length(); ++i) {
        if (std::isspace(line[i])) {
            if (!token.empty()) { tokens.push_back(token); token.clear(); }
        } else if (std::string("=+-*/(){};<>").find(line[i]) != std::string::npos) {
            if (!token.empty()) { tokens.push_back(token); token.clear(); }
            tokens.push_back(std::string(1, line[i]));
        } else {
            token += line[i];
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

// Simple Shunting-yard algorithm
Compiler::ASTNode* Compiler::BuildAST(const std::vector<std::string>& tokens) {
    std::stack<ASTNode*> values;
    std::stack<std::string> ops;

    auto precedence = [](const std::string& op) {
        if (op == "<" || op == ">") return 1;
        if (op == "+" || op == "-") return 2;
        if (op == "*" || op == "/") return 3;
        return 0;
    };

    auto applyOp = [&]() {
        if (ops.empty()) return;
        std::string op = ops.top(); ops.pop();
        if (values.size() < 2) {
            // Syntax error: not enough values. Just clean up if needed.
            return;
        }
        ASTNode* right = values.top(); values.pop();
        ASTNode* left = values.top(); values.pop();
        ASTNode* node = new ASTNode{op};
        node->left = left;
        node->right = right;
        values.push(node);
    };

    for (const auto& token : tokens) {
        if (token == "(") {
            ops.push(token);
        } else if (token == ")") {
            while (!ops.empty() && ops.top() != "(") applyOp();
            if (!ops.empty()) ops.pop();
        } else if (token == "+" || token == "-" || token == "*" || token == "/" || token == "<" || token == ">") {
            while (!ops.empty() && precedence(ops.top()) >= precedence(token)) {
                applyOp();
            }
            ops.push(token);
        } else {
            values.push(new ASTNode{token});
        }
    }
    while (!ops.empty()) {
        applyOp();
    }
    return values.empty() ? nullptr : values.top();
}

std::string Compiler::CompileAST(ASTNode* node, std::string& currentState) {
    if (!node) return "";
    
    // Leaf node: constant or variable
    if (!node->left && !node->right) {
        bool isNumber = !node->value.empty();
        for (size_t i = 0; i < node->value.size(); ++i) {
            char c = node->value[i];
            if (i == 0 && c == '-' && node->value.size() > 1) continue;
            if (!std::isdigit(c)) { isNumber = false; break; }
        }
        
        if (isNumber) {
            std::string constName = "_c_" + node->value;
            mem.Allocate(constName, std::stoi(node->value));
            return constName;
        }
        return node->value;
    }

    // Evaluate children
    std::string leftVar = CompileAST(node->left, currentState);
    std::string rightVar = CompileAST(node->right, currentState);

    // Create a temporary variable to hold the result
    std::string resultVar = "_t_" + std::to_string(++tempVarCounter);
    mem.Allocate(resultVar, 0);

    // Generate states:
    // 1. Seek leftVar
    std::string s1 = NextState();
    GenerateRewind(tm, currentState, s1);
    std::string s2 = NextState();
    GenerateAlgorithmicSeek(tm, s1, mem.GetVarIndex(leftVar), s2);
    // 2. Load to Tape 1
    std::string s3 = NextState();
    GenerateLoadTape0ToTape1(tm, s2, s3);
    
    // 3. Seek rightVar
    std::string s4 = NextState();
    if (node->value == "+" || node->value == "-") {
        GenerateRewindTape0(tm, s3, s4);
    } else {
        GenerateRewind(tm, s3, s4);
    }
    std::string s5 = NextState();
    GenerateAlgorithmicSeek(tm, s4, mem.GetVarIndex(rightVar), s5);
    // 4. Load to Tape 2
    std::string s6 = NextState();
    GenerateLoadTape0ToTape2(tm, s5, s6);

    // 5. Math operation (Tape 1 op Tape 2 -> Tape 1)
    std::string s7 = NextState();
    if (node->value == "+") GenerateBinaryAdd(tm, s6, s7);
    else if (node->value == "-") GenerateBinarySub(tm, s6, s7);
    else if (node->value == "*") GenerateBinaryMul(tm, s6, s7);
    else if (node->value == "/") GenerateBinaryDiv(tm, s6, s7);
    else if (node->value == "<") GenerateBinaryLess(tm, s6, s7);
    else if (node->value == ">") GenerateBinaryGreater(tm, s6, s7);

    // 6. Seek resultVar
    std::string s8 = NextState();
    GenerateRewind(tm, s7, s8);
    std::string s9 = NextState();
    GenerateAlgorithmicSeek(tm, s8, mem.GetVarIndex(resultVar), s9);
    // 7. Store Tape 1 to Tape 0
    std::string s10 = NextState();
    GenerateStoreTapeToTape0(tm, s9, 1, s10);

    currentState = s10;
    return resultVar;
}

void Compiler::Execute() {
    while (tm.Step()) {
        if (tm.GetCurrentState() == "print_val") {
            int val = 0;
            const auto& tape = tm.GetTape(1);
            
            char sign = '+';
            auto itSign = tape.find(1);
            if (itSign != tape.end()) sign = itSign->second;

            for (int i = 0; i < 32; ++i) {
                auto itBit = tape.find(2 + i);
                if (itBit != tape.end() && itBit->second == '1') {
                    val |= (1 << (31 - i));
                }
            }
            if (sign == '-') val = -val;
            std::cout << val << std::endl;
        }
    }
}

void Compiler::Compile(const std::vector<std::string>& sourceCode) {
    std::string currentState = "start";
    
    // ПЕРВЫЙ ПРОХОД: Собираем все переменные и константы
    for (const auto& line : sourceCode) {
        auto tokens = Tokenize(line);
        if (tokens.size() >= 2 && tokens[0] == "var") {
            std::string varName = tokens[1];
            int initVal = 0;
            if (tokens.size() >= 4 && tokens[2] == "=") {
                bool isNum = true;
                std::string valStr = tokens[3];

                if (valStr == "-" && tokens.size() >= 5) {
                    valStr += tokens[4];
                }

                for(size_t i=0; i<valStr.size(); ++i) {
                    if (i==0 && valStr[i] == '-' && valStr.size() > 1) continue;
                    if (!std::isdigit(valStr[i])) { isNum = false; break; }
                }
                if (isNum) initVal = std::stoi(valStr);
            }
            mem.Allocate(varName, initVal);
        }
    }

    // ВТОРОЙ ПРОХОД: Генерируем правила для вычислений
    for (size_t i = 0; i < sourceCode.size(); ++i) {
        auto tokens = Tokenize(sourceCode[i]);
        if (tokens.empty()) continue;

        if (tokens[0] == "var" && tokens.size() >= 4 && tokens[2] == "=") {
            std::string destVar = tokens[1];
            std::vector<std::string> exprTokens(tokens.begin() + 3, tokens.end());
            if (!exprTokens.empty() && exprTokens.back() == ";") exprTokens.pop_back();
            
            bool isJustNegativeNumber = (exprTokens.size() == 2 && exprTokens[0] == "-" && std::isdigit(exprTokens[1][0]));

            if (exprTokens.size() > 1 && !isJustNegativeNumber) {
                ASTNode* ast = BuildAST(exprTokens);
                if (ast) {
                    std::string resultVar = CompileAST(ast, currentState);
                    
                    std::string s1 = NextState(); GenerateRewind(tm, currentState, s1);
                    std::string s2 = NextState(); GenerateAlgorithmicSeek(tm, s1, mem.GetVarIndex(resultVar), s2);
                    std::string s3 = NextState(); GenerateLoadTape0ToTape1(tm, s2, s3);
                    
                    std::string s4 = NextState(); GenerateRewind(tm, s3, s4);
                    std::string s5 = NextState(); GenerateAlgorithmicSeek(tm, s4, mem.GetVarIndex(destVar), s5);
                    std::string s6 = NextState(); GenerateStoreTapeToTape0(tm, s5, 1, s6);
                    
                    currentState = s6;
                    delete ast;
                }
            }
        }
    }
    
    tm.AddRule(currentState, '?', '?', '?', "halt", '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}
