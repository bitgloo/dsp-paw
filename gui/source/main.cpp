/**
 * @file main.cpp
 * @brief Program entry point and main loop.
 *
 * Copyright (C) 2022 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"

#include "gui_help.hpp"
#include "logview.h"
#include "main.hpp"
#include "stmdsp.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

void codeEditorInit();
void codeRenderMenu();
void codeRenderToolbar();
void codeRenderWidgets(const ImVec2& size);
void deviceRenderDraw();
void deviceRenderMenu();
void deviceRenderToolbar();
void deviceRenderWidgets();
void fileRenderMenu();
void fileRenderDialog();
void fileInit();
bool guiInitialize();
bool guiHandleEvents();
void guiShutdown();
void guiRender();

static LogView logView;
static ImFont *fontSans = nullptr;
static ImFont *fontMono = nullptr;

template<bool first = false>
static void renderWindow();

int main(int, char **)
{
    if (!guiInitialize())
        return -1;

    auto& io = ImGui::GetIO();
    fontSans = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 20);
    fontMono = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Regular.ttf", 20);
    if (fontSans == nullptr || fontMono == nullptr) {
        std::cout << "Failed to load fonts!" << std::endl;
        return -1;
    }

    codeEditorInit();
    fileInit();

    renderWindow<true>();

    while (1) {
        constexpr std::chrono::duration<double> fpsDelay (1. / 60.);
        const auto endTime = std::chrono::steady_clock::now() + fpsDelay;

        const bool isDone = guiHandleEvents();
        if (!isDone) {
            renderWindow();
            std::this_thread::sleep_until(endTime);
        } else {
            break;
        }
    }

    guiShutdown();
    return 0;
}

void log(const std::string& str)
{
    logView.AddLog(str);
}

template<bool first>
void renderWindow()
{
    // Start the new window frame and render the menu bar.
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        fileRenderMenu();
        deviceRenderMenu();
        codeRenderMenu();
	helpRenderMenu();

        ImGui::EndMainMenuBar();
    }

    if constexpr (first) {
        ImGui::SetNextWindowSize({550, 440});
    }

    constexpr int MainTopMargin = 22;
    const auto& displaySize = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos({0, MainTopMargin});
    ImGui::SetNextWindowSizeConstraints({displaySize.x, 150}, {displaySize.x, displaySize.y - 150});
    ImGui::Begin("main", nullptr,
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    const float mainWindowHeight = ImGui::GetWindowHeight();

    ImGui::PushFont(fontSans);
    codeRenderToolbar();
    deviceRenderToolbar();
    fileRenderDialog();
    helpRenderDialog();
    deviceRenderWidgets();
    ImGui::PopFont();

    ImGui::PushFont(fontMono);
    codeRenderWidgets({displaySize.x - 16, mainWindowHeight - MainTopMargin - 24});
    ImGui::PopFont();

    ImGui::End();

    // The log window is kept separate from "main" to support panel resizing.
    ImGui::PushFont(fontMono);
    ImGui::SetNextWindowPos({0, mainWindowHeight + MainTopMargin});
    ImGui::SetNextWindowSize({displaySize.x, displaySize.y - mainWindowHeight - MainTopMargin});
    logView.Draw("log", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopFont();

    deviceRenderDraw();

    // Draw everything to the screen.
    guiRender();
}

