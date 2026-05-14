#include "TuringMachine.h"
#include <iostream>

TuringMachine::TuringMachine(const std::string& startState) {
    currentState = startState;
    heads = {0, 0, 0};
}

void TuringMachine::AddRule(const std::string& state, 
                            char r1, char r2, char r3,
                            const std::string& nextState, 
                            char w1, char w2, char w3, 
                            Direction d1, Direction d2, Direction d3) {
    rules[state].push_back({
        {r1, r2, r3},
        nextState,
        {w1, w2, w3},
        {d1, d2, d3}
    });
}

void TuringMachine::SetTapeContent(int tapeIndex, int position, char symbol) {
    if (tapeIndex >= 0 && tapeIndex < 3) {
        tapes[tapeIndex][position] = symbol;
    }
}

const std::map<int, char>& TuringMachine::GetTape(int tapeIndex) const {
    return tapes[tapeIndex];
}

int TuringMachine::GetHeadPosition(int tapeIndex) const {
    return heads[tapeIndex];
}

char TuringMachine::ReadTape(int tapeIndex) {
    auto& tape = tapes[tapeIndex];
    int head = heads[tapeIndex];
    if (tape.find(head) == tape.end()) {
        return '_'; // Default empty symbol
    }
    return tape[head];
}

bool TuringMachine::Step() {
    if (currentState == "halt") {
        return false;
    }

    // Read current symbols from all 3 tapes
    char c1 = ReadTape(0);
    char c2 = ReadTape(1);
    char c3 = ReadTape(2);

    // O(1) Lookup: find the list of rules for the current state
    auto it = rules.find(currentState);
    if (it == rules.end()) {
        return false;
    }

    // Iterate through the rules for this specific state to find a match (wildcard support)
    for (const auto& rule : it->second) {
        bool match = true;
        if (rule.readSymbols[0] != '?' && rule.readSymbols[0] != c1) match = false;
        if (rule.readSymbols[1] != '?' && rule.readSymbols[1] != c2) match = false;
        if (rule.readSymbols[2] != '?' && rule.readSymbols[2] != c3) match = false;

        if (match) {
            // Apply side effects: Write
            for (int i = 0; i < 3; ++i) {
                if (rule.writeSymbols[i] != '?') {
                    tapes[i][heads[i]] = rule.writeSymbols[i];
                }
                // Apply side effects: Move
                heads[i] += static_cast<int>(rule.moveDirections[i]);
            }
            // Transition state
            currentState = rule.nextState;
            return true;
        }
    }

    if (currentState != "halt") {
        std::cerr << "TM Error: No matching rule for state '" << currentState 
                  << "' with symbols (" << c1 << ", " << c2 << ", " << c3 << ")\n";
    }

    return false;
}

std::string TuringMachine::GetCurrentState() const {
    return currentState;
}

void TuringMachine::Run() {
    while (Step()) {}
}