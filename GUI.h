#ifndef GUI_H
#define GUI_H

#include "TuringMachine.h"
#include "MemoryManager.h"
#include "Compiler.h"
#include <string>
#include <vector>

class GUI {
public:
    GUI(TuringMachine& tm, MemoryManager& mem, Compiler& compiler);
    ~GUI();

    void Run();

private:
    TuringMachine& tm;
    MemoryManager& mem;
    Compiler& compiler;

    bool isPlaying = false;
    int executionSpeed = 1;
    float scrollOffset[3] = {0.0f, 0.0f, 0.0f};

    char codeBuffer[4096];
    bool codeModified = false;
    
    void RenderUI();
    void RenderTape(const char* title, int tapeIndex, uint32_t color);
    void RenderMemory();
    void RenderControls();
    void RenderStatus();
    void RenderEditor();
    
    void LoadCodeFromFile();
    void Recompile();
    void ResetSimulation();
    
    // Style constants
    const float CELL_SIZE = 40.0f;
    const float CELL_PADDING = 4.0f;
};

#endif // GUI_H