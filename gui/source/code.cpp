/**
 * @file code.cpp
 * @brief Functionality for compiling and disassembling source code.
 *
 * Copyright (C) 2022 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "main.hpp"
#include "stmdsp.hpp"
#include "stmdsp_code.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

extern std::shared_ptr<stmdsp::device> m_device;

// Stores the temporary file name currently used for compiling the algorithm.
static std::string tempFileName;

/**
 * Generates a new temporary file name.
 * @return A string containing the path and file name.
 */
static std::string newTempFileName();

/**
 * Executes the given command using system(), collecting the text output in the
 * given file.
 * @param command The command to be executed.
 * @param file The file to write command output to.
 * @return True if the command was successful (i.e. returned zero).
 */
static bool codeExecuteCommand(const std::string& command, const std::string& file);

/**
 * Does an in-place replacement of all occurances of "what" with "with".
 * @param str The text string to operate on.
 * @param what The text to search for.
 * @param with The text that will replace occurances of "what".
 */
static void stringReplaceAll(std::string& str, const std::string& what,
    const std::string& with);

std::ifstream compileOpenBinaryFile()
{
    if (!tempFileName.empty())
        return std::ifstream(tempFileName + ".o");
    else
        return std::ifstream();
}

void compileEditorCode(const std::string& code)
{
    log("Compiling...");

    if (tempFileName.empty()) {
        tempFileName = newTempFileName();
    } else {
        std::filesystem::remove(tempFileName + ".o");
        std::filesystem::remove(tempFileName + ".orig.o");
    }

    const auto platform = m_device ? m_device->get_platform()
                                   : stmdsp::platform::L4;

    {
        std::ofstream file (tempFileName, std::ios::trunc | std::ios::binary);

        auto file_text =
            platform == stmdsp::platform::L4 ? stmdsp::file_header_l4
                                             : stmdsp::file_header_h7;
        const auto buffer_size = m_device ? m_device->get_buffer_size()
                                          : stmdsp::SAMPLES_MAX;

        stringReplaceAll(file_text, "$0", std::to_string(buffer_size));

        file << file_text << '\n' << code;
    }

    const auto scriptFile = tempFileName +
#ifndef STMDSP_WIN32
        ".sh";
#else
        ".bat";
#endif

    {
        std::ofstream makefile (scriptFile, std::ios::binary);
        auto make_text =
            platform == stmdsp::platform::L4 ? stmdsp::makefile_text_l4
                                             : stmdsp::makefile_text_h7;

        stringReplaceAll(make_text, "$0", tempFileName);
        stringReplaceAll(make_text, "$1",
                         std::filesystem::current_path().string());

        makefile << make_text;
    }

#ifndef STMDSP_WIN32
    system((std::string("chmod +x ") + scriptFile).c_str());
#endif

    const auto makeOutput = scriptFile + ".log";
    const auto makeCommand = scriptFile + " > " + makeOutput + " 2>&1";
    if (codeExecuteCommand(makeCommand, makeOutput))
        log("Compilation succeeded.");
    else
        log("Compilation failed.");

    std::filesystem::remove(tempFileName);
    std::filesystem::remove(scriptFile);
}

void disassembleCode()
{
    log("Disassembling...");

    //if (tempFileName.empty())
    //    compileEditorCode();

    const auto output = tempFileName + ".asm.log";
    const auto command =
        std::string("arm-none-eabi-objdump -d --no-show-raw-insn ") +
        tempFileName + ".orig.o > " + output + " 2>&1";

    if (codeExecuteCommand(command, output))
        log("Ready.");
    else
        log("Failed to load disassembly.");
}

std::string newTempFileName()
{
    const auto path = std::filesystem::temp_directory_path() / "stmdspgui_build";
    return path.string();
}

bool codeExecuteCommand(const std::string& command, const std::string& file)
{
    bool success = system(command.c_str()) == 0;

    if (std::ifstream output (file); output.good()) {
        std::ostringstream sstr;
        sstr << output.rdbuf();
        log(sstr.str().c_str());
    } else {
        log("Could not read command output!");
    }

    std::filesystem::remove(file);

    return success;
}

void stringReplaceAll(std::string& str, const std::string& what, const std::string& with)
{
    std::size_t i;
    while ((i = str.find(what)) != std::string::npos) {
        str.replace(i, what.size(), with);
        i += what.size();
    }
};

