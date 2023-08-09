/**
 * @file file.cpp
 * @brief Contains code for file-management-related UI elements and logic.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"
#include "ImGuiFileDialog.h"
#include "TextEditor.h"

#include "stmdsp_code.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

extern TextEditor editor;
extern void log(const std::string& str);

enum class FileAction {
    None,
    Open,
    Save,
    SaveAs
};

static FileAction fileAction = FileAction::None;
static std::string fileCurrentPath;
static std::vector<std::filesystem::path> fileExampleList;

static void saveCurrentFile()
{
    if (std::ofstream ofs (fileCurrentPath, std::ios::binary); ofs.good()) {
        const auto& text = editor.GetText();
        ofs.write(text.data(), text.size());
        log("Saved.");
    }
}

static void openCurrentFile()
{
    if (std::ifstream ifs (fileCurrentPath); ifs.good()) {
        std::ostringstream sstr;
        sstr << ifs.rdbuf();
        editor.SetText(sstr.str());
    }
}

static void openNewFile()
{
    fileCurrentPath.clear();
    editor.SetText(stmdsp::file_content);
}

static std::vector<std::filesystem::path> fileScanExamples()
{
    const auto path = std::filesystem::current_path() / "examples";
    const std::filesystem::recursive_directory_iterator rdi (path);

    std::vector<std::filesystem::path> list;
    std::transform(
        std::filesystem::begin(rdi),
        std::filesystem::end(rdi),
        std::back_inserter(list),
        [](const auto& file) { return file.path(); });
    std::sort(list.begin(), list.end());
    return list;
}

void fileInit()
{
    fileExampleList = fileScanExamples();
    openNewFile();
}

void fileRenderMenu()
{
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
            // TODO modified?
            openNewFile();
            log("Ready.");
        }

        if (ImGui::MenuItem("Open")) {
            fileAction = FileAction::Open;
            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileOpenSave", "Choose File", ".cpp", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
        }

        if (ImGui::BeginMenu("Open Example")) {
            for (const auto& file : fileExampleList) {
                if (ImGui::MenuItem(file.filename().string().c_str())) {
                    fileCurrentPath = file.string();
                    openCurrentFile();

                    // Treat like new file.
                    fileCurrentPath.clear();
                    log("Ready.");
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Save")) {
            if (fileCurrentPath.size() > 0) {
                saveCurrentFile();
            } else {
                fileAction = FileAction::SaveAs;
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ChooseFileOpenSave", "Choose File", ".cpp", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
            }
        }

        if (ImGui::MenuItem("Save As")) {
            fileAction = FileAction::SaveAs;
            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileOpenSave", "Choose File", ".cpp", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Quit")) {
            SDL_Event quitEvent (SDL_QUIT);
            SDL_PushEvent(&quitEvent);
        }

        ImGui::EndMenu();
    }
}

void fileRenderDialog()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseFileOpenSave",
                                             ImGuiWindowFlags_NoCollapse,
                                             ImVec2(460, 540)))
    {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

	    if (fileAction == FileAction::Open) {
                fileCurrentPath = filePathName;
                openCurrentFile();
                log("Ready.");
	    } else if (fileAction == FileAction::SaveAs) {
                fileCurrentPath = filePathName;
                saveCurrentFile();
            }
        }
        
        ImGuiFileDialog::Instance()->Close();
    }
}

