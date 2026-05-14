#ifndef MACROS_FOR_TURING_H
#define MACROS_FOR_TURING_H

#include "TuringMachine.h"
#include <string>

// Rewind all tapes to 0 position (for simplicity, we assume heads don't move past 0 to the left)
void GenerateRewind(TuringMachine& tm, const std::string& startState, const std::string& endState);

// Rewind ONLY Tape 0 to 0 position
void GenerateRewindTape0(TuringMachine& tm, const std::string& startState, const std::string& endState);

// Algorithmic Seek: Counts '#' symbols dynamically on Tape 0 to reach varIndex
void GenerateAlgorithmicSeek(TuringMachine& tm, const std::string& startState, int varIndex, const std::string& endState);

// Load/Store (reads 34 cells from Tape 0 and copies to Tape 1/2)
void GenerateLoadTape0ToTape1(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateLoadTape0ToTape2(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateStoreTapeToTape0(TuringMachine& tm, const std::string& startState, int sourceTape, const std::string& endState);

// Binary Math Macros (operates on Tape 1 and Tape 2, output to Tape 1)
void GenerateBinaryAdd(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateBinarySub(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateBinaryMul(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateBinaryDiv(TuringMachine& tm, const std::string& startState, const std::string& endState);

// Binary comparison (Tape 1 < Tape 2 -> Tape 1 has '1' if true, '0' if false)
void GenerateBinaryLess(TuringMachine& tm, const std::string& startState, const std::string& endState);
void GenerateBinaryGreater(TuringMachine& tm, const std::string& startState, const std::string& endState);

#endif // MACROS_FOR_TURING_H