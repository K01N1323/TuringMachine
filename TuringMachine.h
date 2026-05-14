#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <array>

enum class Direction { Left = -1, Stay = 0, Right = 1 };

struct Rule {
    std::array<char, 3> readSymbols;
    std::string nextState;
    std::array<char, 3> writeSymbols;
    std::array<Direction, 3> moveDirections;
};

class TuringMachine {
private:
    std::array<std::map<int, char>, 3> tapes;
    std::array<int, 3> heads;
    std::string currentState;
    
    // Key: Current state. Value: List of rules for this state.
    // O(1) lookup of state, then iterating small vector of rules.
    std::unordered_map<std::string, std::vector<Rule>> rules;

    char ReadTape(int tapeIndex);

public:
    TuringMachine(const std::string& startState = "start");

    void AddRule(const std::string& state, 
                 char r1, char r2, char r3,
                 const std::string& nextState, 
                 char w1, char w2, char w3, 
                 Direction d1, Direction d2, Direction d3);

    void SetTapeContent(int tapeIndex, int position, char symbol);
    
    const std::map<int, char>& GetTape(int tapeIndex) const;
    int GetHeadPosition(int tapeIndex) const;
    
    bool Step();
    std::string GetCurrentState() const;
    void Run();
};

#endif // TURING_MACHINE_H