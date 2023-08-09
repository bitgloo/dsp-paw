/**
 * @file gui_help.cpp
 * @brief Defines the "Help" menu and provides its functionality.
 *
 * Copyright (C) 2022 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

void log(const std::string& str);

static bool showDownloadFirmware = false;
static bool showHelp = false;
static std::string firmwareFile;

static void helpDownloadThread();

void helpRenderMenu()
{
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Open wiki...")) {
#ifdef STMDSP_WIN32
            system("start "
#else
            system("xdg-open "
#endif
                "https://code.bitgloo.com/clyne/stmdspgui/wiki");
        }

        if (ImGui::MenuItem("Download firmware...")) {
            showDownloadFirmware = true;
        }

        ImGui::Separator();
        if (ImGui::MenuItem("About")) {
            showHelp = true;
        }

        ImGui::EndMenu();
    }
}

void helpRenderDialog()
{
    if (showDownloadFirmware) {
	showDownloadFirmware = false;
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseFileFW", "Choose Firmware File", ".hex", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileFW",
                                             ImGuiWindowFlags_NoCollapse,
                                             ImVec2(460, 540)))
    {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            firmwareFile = ImGuiFileDialog::Instance()->GetFilePathName();
#ifdef STMDSP_WIN32
            size_t i = 0;
            while ((i = firmwareFile.find('\\', i)) != std::string::npos) {
                firmwareFile.replace(i, 1, "\\\\");
                i += 2;
            }
#endif

	    std::thread(helpDownloadThread).detach();
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (showHelp) {
        ImGui::Begin("help");

        ImGui::Text("stmdspgui\nCompiled on " __DATE__ ".\n\nWritten by Clyne Sullivan.\n");

        if (ImGui::Button("Close")) {
            showHelp = false;
        }

        ImGui::End();
    }

    if (!firmwareFile.empty()) {
    	ImGui::Begin("Downloading");

	ImGui::Text("Downloading firmware to device...");

	ImGui::End();
    }
}

void helpDownloadThread()
{
    std::string command (
#ifdef STMDSP_WIN32
        "openocd\\bin\\openocd.exe"
#else
	"openocd"
#endif
	" -f openocd.cfg -c \"program $0 reset exit\"");

    command.replace(command.find("$0"), 2, firmwareFile);

    std::cout << "Run: " << command << std::endl;

    if (system(command.c_str()) == 0) {
	log("Programming finished.");    
    } else {
    	log("Error while programming device!");
    }

    firmwareFile.clear();
}

