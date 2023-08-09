/**
 * @file gui.cpp
 * @brief Contains code for GUI-related logic.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

bool guiInitialize();
void guiRender();
bool guiHandleEvents();
void guiShutdown();

static SDL_Window *window = nullptr;
static SDL_GLContext gl_context;

bool guiInitialize()
{
    if (SDL_Init(0) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    window = SDL_CreateWindow("stmdsp gui",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 700,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_ALLOW_HIGHDPI*/);

    if (window == nullptr) {
        puts("Error: Could not create the window!");
        return false;
    }

    SDL_SetWindowMinimumSize(window, 320, 320);

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5;
    style.FrameRounding = 3;
    style.ScrollbarRounding = 1;

//#define ACCENT1 0.26f, 0.59f, 0.98f
#define ACCENT1 0.6f, 0.6f, 0.6f
#define ACCENT2 0.4f, 0.4f, 0.4f

    style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(ACCENT1, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(ACCENT1, 0.67f);
    style.Colors[ImGuiCol_CheckMark]              = ImVec4(ACCENT1, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]             = ImVec4(ACCENT1, 0.78f);
    style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    style.Colors[ImGuiCol_Button]                 = ImVec4(ACCENT1, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(ACCENT1, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]           = ImVec4(ACCENT2, 1.00f);
    style.Colors[ImGuiCol_Header]                 = ImVec4(ACCENT1, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(ACCENT1, 0.80f);
    style.Colors[ImGuiCol_HeaderActive]           = ImVec4(ACCENT1, 1.00f);
    style.Colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    style.Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(ACCENT1, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(ACCENT1, 0.95f);
    style.Colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(ACCENT1, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget]         = ImVec4(ACCENT1, 0.95f);

    style.Colors[ImGuiCol_Tab]                    = ImLerp(style.Colors[ImGuiCol_Header],       style.Colors[ImGuiCol_TitleBgActive], 0.90f);
    style.Colors[ImGuiCol_TabHovered]             = style.Colors[ImGuiCol_HeaderHovered];
    style.Colors[ImGuiCol_TabActive]              = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.60f);
    style.Colors[ImGuiCol_TabUnfocused]           = ImLerp(style.Colors[ImGuiCol_Tab],          style.Colors[ImGuiCol_TitleBg], 0.80f);
    style.Colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(style.Colors[ImGuiCol_TabActive],    style.Colors[ImGuiCol_TitleBg], 0.40f);
    style.Colors[ImGuiCol_NavHighlight]           = style.Colors[ImGuiCol_HeaderHovered];

    return true;
}

void guiRender()
{
    ImGui::Render();

    const auto& displaySize = ImGui::GetIO().DisplaySize;
    const int sizeX = static_cast<int>(displaySize.x);
    const int sizeY = static_cast<int>(displaySize.y);

    glViewport(0, 0, sizeX, sizeY);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

bool guiHandleEvents()
{
    bool done = false;

    for (SDL_Event event; SDL_PollEvent(&event);) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            done = true;
        } else if (event.type == SDL_WINDOWEVENT) {
            const auto& ew = event.window;
            const auto wid = SDL_GetWindowID(window);
            if (ew.event == SDL_WINDOWEVENT_CLOSE && ew.windowID == wid)
                done = true;
        }
    }

    return done;
}

void guiShutdown()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

