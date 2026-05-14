#include "GUI.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>

// Вспомогательный макрос для преобразования hex в ImU32
#define IM_COL32_HEX(hex) IM_COL32((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255)

GUI::GUI(TuringMachine& tm, MemoryManager& mem, Compiler& compiler) : tm(tm), mem(mem), compiler(compiler) {
    memset(codeBuffer, 0, sizeof(codeBuffer));
    LoadCodeFromFile();
}

GUI::~GUI() {}

void GUI::LoadCodeFromFile() {
    std::ifstream file("Program.txt");
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();
        strncpy(codeBuffer, content.c_str(), sizeof(codeBuffer) - 1);
    }
}

void GUI::Recompile() {
    std::string code(codeBuffer);
    std::stringstream ss(code);
    std::vector<std::string> source;
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) source.push_back(line);
    }

    ResetSimulation();
    compiler.Compile(source);
    mem.Deploy(tm);
}

void GUI::ResetSimulation() {
    isPlaying = false;
    tm = TuringMachine("start");
    mem.Clear();
}

void GUI::Run() {
    if (!glfwInit()) {
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return;
    }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1440, 900, "Эмулятор 3-ленточной Машины Тьюринга", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не удалось создать окно GLFW" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Загрузка шрифта с поддержкой кириллицы
    ImFontConfig font_config;
    font_config.OversampleH = 2;
    font_config.OversampleV = 2;
    io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Supplemental/Arial.ttf", 18.0f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
    
    // Современный темный стиль
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.35f, 0.45f, 1.00f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (isPlaying) {
            for (int i = 0; i < executionSpeed; ++i) {
                if (!tm.Step()) {
                    isPlaying = false;
                    break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void GUI::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(400, ImGui::GetIO().DisplaySize.y));
    ImGui::Begin("Панель управления", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    RenderStatus();
    ImGui::Separator();
    RenderControls();
    ImGui::Separator();
    RenderEditor();
    ImGui::Separator();
    RenderMemory();
    
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(400, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 400, ImGui::GetIO().DisplaySize.y));
    ImGui::Begin("Вид лент", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "  ВИЗУАЛИЗАЦИЯ ВЫПОЛНЕНИЯ");
    ImGui::Separator();
    ImGui::Spacing();

    RenderTape("ЛЕНТА 0: ХРАНИЛИЩЕ ПАМЯТИ", 0, IM_COL32(50, 150, 255, 180));
    ImGui::Spacing();
    RenderTape("ЛЕНТА 1: ОПЕРАНД A / РЕЗУЛЬТАТ", 1, IM_COL32(50, 200, 100, 180));
    ImGui::Spacing();
    RenderTape("ЛЕНТА 2: ОПЕРАНД B / ПЕРЕНОС", 2, IM_COL32(255, 180, 50, 180));

    ImGui::End();
}

void GUI::RenderStatus() {
    ImGui::TextDisabled("СТАТУС СИСТЕМЫ");
    ImGui::Text("Состояние: "); ImGui::SameLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", tm.GetCurrentState().c_str());
    
    if (tm.GetCurrentState() == "halt") {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "ВЫПОЛНЕНИЕ ЗАВЕРШЕНО");
    } else if (isPlaying) {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "ЗАПУЩЕНО...");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "ПАУЗА");
    }
}

void GUI::RenderControls() {
    ImGui::Spacing();
    ImGui::TextDisabled("УПРАВЛЕНИЕ СИМУЛЯЦИЕЙ");
    
    if (ImGui::Button(isPlaying ? " ПАУЗА " : " ПУСК ", ImVec2(100, 40))) {
        isPlaying = !isPlaying;
    }
    ImGui::SameLine();
    if (ImGui::Button(" ШАГ ", ImVec2(100, 40))) {
        tm.Step();
    }
    ImGui::SameLine();
    if (ImGui::Button(" СБРОС ", ImVec2(100, 40))) {
        ResetSimulation();
        mem.Deploy(tm);
    }
    
    ImGui::PushItemWidth(-1);
    ImGui::Text("Скорость симуляции:");
    if (ImGui::SliderInt("##speed", &executionSpeed, 1, 500)) {
        // Если скорость 1, это позволит видеть каждый шаг
    }
    if (executionSpeed > 50) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Высокая скорость: головки могут мерцать");
    }
    ImGui::PopItemWidth();
}

void GUI::RenderEditor() {
    ImGui::Spacing();
    ImGui::TextDisabled("РЕДАКТОР ПРОГРАММЫ");
    
    float editorHeight = ImGui::GetIO().DisplaySize.y * 0.3f;
    if (ImGui::InputTextMultiline("##editor", codeBuffer, sizeof(codeBuffer), ImVec2(-1, editorHeight))) {
        codeModified = true;
    }
    
    if (ImGui::Button(" СКОМПИЛИРОВАТЬ И ЗАГРУЗИТЬ ", ImVec2(-1, 35))) {
        Recompile();
        codeModified = false;
    }
    
    if (codeModified) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Код изменен, требуется перекомпиляция");
    }
}

void GUI::RenderMemory() {
    ImGui::Spacing();
    ImGui::TextDisabled("ПЕРЕМЕННЫЕ ПАМЯТИ (ЛЕНТА 0)");
    
    if (ImGui::BeginTable("MemTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Переменная");
        ImGui::TableSetupColumn("Значение (Dec)");
        ImGui::TableHeadersRow();

        auto varNames = mem.GetVariableNames();
        for (const auto& varName : varNames) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            
            // Отделяем временные переменные визуально
            if (varName.size() > 3 && varName.substr(0, 3) == "_t_") {
                ImGui::TextDisabled("%s (temp)", varName.c_str());
            } else if (varName.size() > 3 && varName.substr(0, 3) == "_c_") {
                ImGui::TextDisabled("%s (const)", varName.c_str());
            } else {
                ImGui::Text("%s", varName.c_str());
            }

            ImGui::TableSetColumnIndex(1);
            try {
                int val = mem.GetDecimalValue(tm, varName);
                ImGui::Text("%d", val);
            } catch (...) {
                ImGui::TextDisabled("Н/Д");
            }
        }

        ImGui::EndTable();
    }
}

void GUI::RenderTape(const char* title, int tapeIndex, uint32_t color) {
    ImGui::Text(" %s", title);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x - 20;
    float height = 80.0f;

    // Отрисовка фона
    draw_list->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), IM_COL32(30, 30, 35, 255), 4.0f);
    draw_list->AddRect(pos, ImVec2(pos.x + width, pos.y + height), color, 4.0f, 0, 2.0f);

    int headPos = tm.GetHeadPosition(tapeIndex);
    const auto& tape = tm.GetTape(tapeIndex);

    // Центрирование ленты на головке
    float centerX = width / 2.0f;
    int viewRange = (int)(width / CELL_SIZE) / 2 + 1;

    for (int i = headPos - viewRange; i <= headPos + viewRange; ++i) {
        float x = centerX + (i - headPos) * CELL_SIZE - CELL_SIZE/2.0f;
        if (x < 0 || x > width - CELL_SIZE) continue;

        ImVec2 cellPos = ImVec2(pos.x + x, pos.y + 10);
        ImVec2 cellEnd = ImVec2(cellPos.x + CELL_SIZE - CELL_PADDING, cellPos.y + CELL_SIZE + 20);

        char c = '_';
        auto it = tape.find(i);
        if (it != tape.end()) c = it->second;

        // Фон ячейки
        uint32_t cellCol = (i == headPos) ? IM_COL32(200, 50, 50, 255) : IM_COL32(45, 45, 50, 255);
        draw_list->AddRectFilled(cellPos, cellEnd, cellCol, 4.0f);
        if (i == headPos) {
            draw_list->AddRect(cellPos, cellEnd, IM_COL32(255, 255, 255, 255), 4.0f, 0, 2.0f);
        }

        // Текст ячейки
        char buf[2] = {c, '\0'};
        ImVec2 textSize = ImGui::CalcTextSize(buf);
        draw_list->AddText(ImVec2(cellPos.x + (CELL_SIZE - CELL_PADDING - textSize.x)/2, cellPos.y + (CELL_SIZE + 20 - textSize.y)/2), 
                           IM_COL32(255, 255, 255, 255), buf);
        
        // Индекс позиции
        char posBuf[16];
        snprintf(posBuf, 16, "%d", i);
        draw_list->AddText(nullptr, 10.0f, ImVec2(cellPos.x + 2, cellPos.y + 2), IM_COL32(150, 150, 150, 255), posBuf);
    }

    // Отрисовка указателя головки (треугольник)
    ImVec2 tri1 = ImVec2(pos.x + centerX, pos.y + height - 5);
    ImVec2 tri2 = ImVec2(pos.x + centerX - 8, pos.y + height - 15);
    ImVec2 tri3 = ImVec2(pos.x + centerX + 8, pos.y + height - 15);
    draw_list->AddTriangleFilled(tri1, tri2, tri3, IM_COL32(255, 255, 255, 255));

    ImGui::Dummy(ImVec2(width, height));
}
