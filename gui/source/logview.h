#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <string>

#include "imgui.h"

// Adapted from ExampleAppLog from imgui_demo.cpp
class LogView
{
public:
    LogView();

    void Clear();
    void AddLog(const std::string& str);
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void Draw(const char* title, bool* p_open = NULL, ImGuiWindowFlags flags = 0);

private:
    ImGuiTextBuffer Buf;
    ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool updated;
};

#endif // LOGVIEW_H

