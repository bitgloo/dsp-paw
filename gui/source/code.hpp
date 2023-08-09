/**
 * @file code.hpp
 * @brief Functionality for compiling and disassembling source code.
 *
 * Copyright (C) 2022 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSPGUI_CODE_HPP
#define STMDSPGUI_CODE_HPP

#include <fstream>
#include <istream>
#include <string>

/**
 * Attempts to open the most recently created binary file.
 * @return An opened stream of the file if it exists, an empty stream otherwise.
 */
std::ifstream compileOpenBinaryFile();

/**
 * Attempts to compile the given C++ algorithm code into a binary.
 * Errors are reported to the log view.
 * @param code The C++ code for the algorithm (usually from the text editor).
 */
void compileEditorCode(const std::string& code);

/**
 * Disassembles the most recently compiled binary, outputting the results to
 * the log view.
 */
void disassembleCode();

#endif // STMDSPGUI_CODE_HPP

