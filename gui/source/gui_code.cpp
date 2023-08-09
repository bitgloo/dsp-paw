/**
 * @file code.cpp
 * @brief Contains code for algorithm-code-related UI elements and logic.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "code.hpp"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"
#include "TextEditor.h"

#include <string>

TextEditor editor; // file.cpp

static std::string editorCompiled;

static void codeCompile();
static void codeDisassemble();

void codeEditorInit()
{
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetPalette(TextEditor::GetLightPalette());
}

void codeRenderMenu()
{
    if (ImGui::BeginMenu("Code")) {
        if (ImGui::MenuItem("Compile"))
            codeCompile();
        if (ImGui::MenuItem("Disassemble"))
            codeDisassemble();

        ImGui::EndMenu();
    }
}

void codeRenderToolbar()
{
    if (ImGui::Button("Compile"))
        codeCompile();
}

void codeRenderWidgets(const ImVec2& size)
{
    editor.Render("code", size, true);
}

static void codeCompile()
{
    compileEditorCode(editor.GetText());
    editorCompiled = editor.GetText().compare(editorCompiled);
}

static void codeDisassemble()
{
    if (editor.GetText().compare(editorCompiled) != 0)
        codeCompile();
    disassembleCode();
}

