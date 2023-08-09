#include "logview.h"

LogView::LogView()
{
    Clear();
}

void LogView::Clear()
{
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
    updated = false;
}

void LogView::AddLog(const std::string& str)
{
    AddLog(str.c_str());
}

void LogView::AddLog(const char* fmt, ...)
{
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    Buf.appendfv("\n", args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
        if (Buf[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
    updated = true;
}

void LogView::Draw(const char* title, bool* p_open, ImGuiWindowFlags flags)
{
    if (!ImGui::Begin(title, p_open, flags))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Log ");
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    if (ImGui::Button("Clear"))
        Clear();
    ImGui::SameLine();
    if (ImGui::Button("Copy"))
        ImGui::LogToClipboard();
    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);


    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* buf = Buf.begin();
    const char* buf_end = Buf.end();

    ImGuiListClipper clipper;
    clipper.Begin(LineOffsets.Size);

    while (clipper.Step())
    {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
        {
            const char* line_start = buf + LineOffsets[line_no];
            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
            ImGui::TextUnformatted(line_start, line_end);
        }
    }
    clipper.End();

    ImGui::PopStyleVar();

    if (updated) {
        ImGui::SetScrollHereY();
        updated = false;
    }

    ImGui::EndChild();
    ImGui::End();
}

