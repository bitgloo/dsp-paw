#include "circular.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

#include "stmdsp.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

namespace ImGui
{
    void PushDisabled()
    {
        ImGuiContext& g = *GImGui;
        bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
        if (!was_disabled)
            PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.6f);
        PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    void PopDisabled()
    {
        ImGuiContext& g = *GImGui;
        bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
        PopItemFlag();
        if (was_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
            PopStyleVar();
    }
}

// Used for status queries and buffer size configuration.
extern std::shared_ptr<stmdsp::device> m_device;

void deviceAlgorithmUnload();
void deviceAlgorithmUpload();
bool deviceConnect();
void deviceGenLoadFormula(const std::string& list);
void deviceGenLoadList(std::string_view list);
bool deviceGenStartToggle();
void deviceLoadAudioFile(const std::string& file);
void deviceLoadLogFile(const std::string& file);
void deviceSetSampleRate(unsigned int index);
void deviceSetInputDrawing(bool enabled);
void deviceStart(bool logResults, bool drawSamples);
void deviceStartMeasurement();
void deviceUpdateDrawBufferSize(double timeframe);
std::size_t pullFromDrawQueue(
    CircularBuffer<std::vector, stmdsp::dacsample_t>& circ);
std::size_t pullFromInputDrawQueue(
    CircularBuffer<std::vector, stmdsp::dacsample_t>& circ);

static std::string sampleRatePreview = "?";
static bool measureCodeTime = false;
static bool logResults = false;
static bool drawSamples = false;
static bool popupRequestBuffer = false;
static bool popupRequestSiggen = false;
static bool popupRequestLog = false;
static double drawSamplesTimeframe = 1.0; // seconds

static std::string getSampleRatePreview(unsigned int rate)
{
    return std::to_string(rate / 1000) + " kHz";
}

static std::string connectLabel ("Connect");
void deviceRenderDisconnect()
{
    connectLabel = "Connect";
    measureCodeTime = false;
    logResults = false;
    drawSamples = false;
}

void deviceRenderMenu()
{
    auto addMenuItem = [](const std::string& label, bool enable, auto action) {
        if (ImGui::MenuItem(label.c_str(), nullptr, false, enable)) {
            action();
        }
    };

    if (ImGui::BeginMenu("Device")) {
        addMenuItem(connectLabel, !m_device || !m_device->is_running(), [&] {
                if (deviceConnect()) {
                    connectLabel = "Disconnect";
                    sampleRatePreview =
                        getSampleRatePreview(m_device->get_sample_rate());
                    deviceUpdateDrawBufferSize(drawSamplesTimeframe);
                } else {
                    deviceRenderDisconnect();
                }
            });

        const bool isConnected = m_device ? true : false;
        const bool isRunning = isConnected && m_device->is_running();

        ImGui::Separator();

        static std::string startLabel ("Start");
        addMenuItem(startLabel, isConnected, [&] {
                startLabel = isRunning ? "Start" : "Stop";
                deviceStart(logResults, drawSamples);
                if (logResults && isRunning)
                    logResults = false;
            });
        addMenuItem("Upload algorithm", isConnected && !isRunning,
            deviceAlgorithmUpload);
        addMenuItem("Unload algorithm", isConnected && !isRunning,
            deviceAlgorithmUnload);
        addMenuItem("Measure Code Time", isRunning, deviceStartMeasurement);

        ImGui::Separator();
        if (!isConnected || isRunning)
            ImGui::PushDisabled(); // Hey, pushing disabled!

        ImGui::Checkbox("Draw samples", &drawSamples);
        if (ImGui::Checkbox("Log results...", &logResults)) {
            if (logResults)
                popupRequestLog = true;
        }
        addMenuItem("Set buffer size...", true, [] { popupRequestBuffer = true; });

        if (!isConnected || isRunning)
            ImGui::PopDisabled();
        ImGui::Separator();

        addMenuItem("Load signal generator",
            isConnected && !m_device->is_siggening() && !m_device->is_running(),
            [] { popupRequestSiggen = true; });

        static std::string startSiggenLabel ("Start signal generator");
        addMenuItem(startSiggenLabel, isConnected, [&] {
                const bool running = deviceGenStartToggle();
                startSiggenLabel = running ? "Stop signal generator"
                                           : "Start signal generator";
            });

        ImGui::EndMenu();
    }
}

void deviceRenderToolbar()
{
    ImGui::SameLine();
    if (ImGui::Button("Upload"))
        deviceAlgorithmUpload();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);

    const bool enable =
        m_device && !m_device->is_running() && !m_device->is_siggening();
    if (!enable)
        ImGui::PushDisabled();

    if (ImGui::BeginCombo("", sampleRatePreview.c_str())) {
        extern std::array<unsigned int, 6> sampleRateInts;

        for (const auto& r : sampleRateInts) {
            const auto s = getSampleRatePreview(r);
            if (ImGui::Selectable(s.c_str())) {
                sampleRatePreview = s;
                deviceSetSampleRate(r);
                deviceUpdateDrawBufferSize(drawSamplesTimeframe);
            }
        }

        ImGui::EndCombo();
    }

    if (!enable)
        ImGui::PopDisabled();
}

void deviceRenderWidgets()
{
    static std::string siggenInput (32768, '\0');
    static int siggenOption = 0;

    if (popupRequestSiggen) {
        popupRequestSiggen = false;
        ImGui::OpenPopup("siggen");
    } else if (popupRequestBuffer) {
        popupRequestBuffer = false;
        ImGui::OpenPopup("buffer");
    } else if (popupRequestLog) {
        popupRequestLog = false;
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseFileLog", "Choose File", ".csv", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    if (ImGui::BeginPopup("siggen")) {
        if (ImGui::RadioButton("List", &siggenOption, 0)) {
            siggenInput.resize(32768);
            siggenInput[0] = '\0';
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Formula", &siggenOption, 1)) {
            siggenInput.resize(1024);
            siggenInput[0] = '\0';
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Audio File", &siggenOption, 2))
            siggenInput.clear();

        if (siggenOption == 2) {
            if (ImGui::Button("Choose File")) {
                // This dialog will override the siggen popup, closing it.
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ChooseFileGen", "Choose File", ".wav", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
            }
        } else {
            ImGui::Text(siggenOption == 0 ? "Enter a list of numbers:"
                                          : "Enter a formula. x = sample #, y = -1 to 1.\nf(x) = ");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, {.8, .8, .8, 1});
            ImGui::InputText("", siggenInput.data(), siggenInput.size());
            ImGui::PopStyleColor();
        }

        if (ImGui::Button("Save")) {
            switch (siggenOption) {
            case 0:
                deviceGenLoadList(siggenInput.substr(0, siggenInput.find('\0')));
                break;
            case 1:
                deviceGenLoadFormula(siggenInput.substr(0, siggenInput.find('\0')));
                break;
            case 2:
                break;
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            siggenInput.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("buffer")) {
        static std::string bufferSizeInput ("4096");
        ImGui::Text("Please enter a new sample buffer size (100-4096):");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {.8, .8, .8, 1});
        ImGui::InputText("",
            bufferSizeInput.data(),
            bufferSizeInput.size(),
            ImGuiInputTextFlags_CharsDecimal);
        ImGui::PopStyleColor();
        if (ImGui::Button("Save")) {
            if (m_device) {
                int n = std::clamp(std::stoi(bufferSizeInput), 100, 4096);
                m_device->continuous_set_buffer_size(n);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileLog",
                                             ImGuiWindowFlags_NoCollapse,
                                             ImVec2(460, 540)))
    {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            const auto filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            deviceLoadLogFile(filePathName);
        } else {
            logResults = false;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileGen",
                                             ImGuiWindowFlags_NoCollapse,
                                             ImVec2(460, 540)))
    {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            const auto filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            deviceLoadAudioFile(filePathName);
        }

        ImGuiFileDialog::Instance()->Close();
    }
}

void deviceRenderDraw()
{
    if (drawSamples) {
        static std::vector<stmdsp::dacsample_t> buffer;
        static std::vector<stmdsp::dacsample_t> bufferInput;
        static auto bufferCirc = CircularBuffer(buffer);
        static auto bufferInputCirc = CircularBuffer(bufferInput);

        static bool drawSamplesInput = false;
        static unsigned int yMinMax = 4095;

        ImGui::Begin("draw", &drawSamples);
        ImGui::Text("Draw input ");
        ImGui::SameLine();
        if (ImGui::Checkbox("", &drawSamplesInput)) {
            deviceSetInputDrawing(drawSamplesInput);
            if (drawSamplesInput) {
                bufferCirc.reset(2048);
                bufferInputCirc.reset(2048);
            }
        }
        ImGui::SameLine();
        ImGui::Text("Time: %0.3f sec", drawSamplesTimeframe);
        ImGui::SameLine();
        if (ImGui::Button("-", {30, 0})) {
            drawSamplesTimeframe = std::max(drawSamplesTimeframe / 2., 0.0078125);
            deviceUpdateDrawBufferSize(drawSamplesTimeframe);
        }
        ImGui::SameLine();
        if (ImGui::Button("+", {30, 0})) {
            drawSamplesTimeframe = std::min(drawSamplesTimeframe * 2, 32.);
            deviceUpdateDrawBufferSize(drawSamplesTimeframe);
        }
        ImGui::SameLine();
        ImGui::Text("Y: +/-%1.2fV", 3.3f * (static_cast<float>(yMinMax) / 4095.f));
        ImGui::SameLine();
        if (ImGui::Button(" - ", {30, 0})) {
            yMinMax = std::max(63u, yMinMax >> 1);
        }
        ImGui::SameLine();
        if (ImGui::Button(" + ", {30, 0})) {
            yMinMax = std::min(4095u, (yMinMax << 1) | 1);
        }

        auto newSize = pullFromDrawQueue(bufferCirc);
        if (newSize > 0) {
            buffer.resize(newSize);
            bufferCirc = CircularBuffer(buffer);
            pullFromDrawQueue(bufferCirc);
        }

        if (drawSamplesInput) {
            auto newSize = pullFromInputDrawQueue(bufferInputCirc);
            if (newSize > 0) {
                bufferInput.resize(newSize);
                bufferInputCirc = CircularBuffer(bufferInput);
                pullFromInputDrawQueue(bufferInputCirc);
            }
        }

        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        auto size = ImGui::GetWindowSize();
        p0.y += 65;
        size.y -= 70;
        drawList->AddRectFilled(p0, {p0.x + size.x, p0.y + size.y}, IM_COL32_BLACK);

        const auto lcMinor = ImGui::GetColorU32(IM_COL32(40, 40, 40, 255));
        const auto lcMajor = ImGui::GetColorU32(IM_COL32(140, 140, 140, 255));

        {
            const float yinc = (3. / 3.3) * size.y / 12.f;
            const float center = p0.y + size.y / 2;
            drawList->AddLine({p0.x, center}, {p0.x + size.x, center}, ImGui::GetColorU32(IM_COL32_WHITE));
            for (int i = 1; i < 7; ++i) {
                drawList->AddLine({p0.x, center + i * yinc}, {p0.x + size.x, center + i * yinc}, (i % 2) ? lcMinor : lcMajor);
                drawList->AddLine({p0.x, center - i * yinc}, {p0.x + size.x, center - i * yinc}, (i % 2) ? lcMinor : lcMajor);
            }
        }
        {
            const float xinc = size.x / 16.f;
            const float center = p0.x + size.x / 2;
            drawList->AddLine({center, p0.y}, {center, p0.y + size.y}, ImGui::GetColorU32(IM_COL32_WHITE));
            for (int i = 1; i < 8; ++i) {
                drawList->AddLine({center + i * xinc, p0.y}, {center + i * xinc, p0.y + size.y}, (i % 2) ? lcMinor : lcMajor);
                drawList->AddLine({center - i * xinc, p0.y}, {center - i * xinc, p0.y + size.y}, (i % 2) ? lcMinor : lcMajor);
            }
        }

        const float di = static_cast<float>(buffer.size()) / size.x;
        const float dx = std::ceil(size.x / static_cast<float>(buffer.size()));
        ImVec2 pp = p0;
        float i = 0;
        while (pp.x < p0.x + size.x) {
            unsigned int idx = i;
            float n = std::clamp((buffer[idx] - 2048.) / yMinMax, -0.5, 0.5);
            i += di;

            ImVec2 next (pp.x + dx, p0.y + size.y * (0.5 - n));
            drawList->AddLine(pp, next, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
            pp = next;
        }

        if (drawSamplesInput) {
            ImVec2 pp = p0;
            float i = 0;
            while (pp.x < p0.x + size.x) {
                unsigned int idx = i;
                float n = std::clamp((bufferInput[idx] - 2048.) / yMinMax, -0.5, 0.5);
                i += di;

                ImVec2 next (pp.x + dx, p0.y + size.y * (0.5 - n));
                drawList->AddLine(pp, next, ImGui::GetColorU32(IM_COL32(0, 0, 255, 255)));
                pp = next;
            }
        }

        const auto mouse = ImGui::GetMousePos();
        if (mouse.x > p0.x && mouse.x < p0.x + size.x &&
            mouse.y > p0.y && mouse.y < p0.y + size.y)
        {
            char buf[16];
            drawList->AddLine({mouse.x, p0.y}, {mouse.x, p0.y + size.y}, IM_COL32(255, 255, 0, 255));

            {
                const std::size_t si = (mouse.x - p0.x) / size.x * buffer.size();
                const float s = buffer[si] / 4095.f * 6.6f - 3.3f;
                snprintf(buf, sizeof(buf), "   %1.3fV", s);
                drawList->AddText(mouse, IM_COL32(255, 0, 0, 255), buf);
            }

            if (drawSamplesInput) {
                const std::size_t si = (mouse.x - p0.x) / size.x * bufferInput.size();
                const float s = bufferInput[si] / 4095.f * 6.6f - 3.3f;
                snprintf(buf, sizeof(buf), "   %1.3fV", s);
                drawList->AddText({mouse.x, mouse.y + 20}, IM_COL32(0, 0, 255, 255), buf);
            }
        }

        ImGui::End();
    }
}

