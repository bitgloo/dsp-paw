/**
 * @file elfload.hpp
 * @brief Loads ELF binary data into memory for execution.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ELF_LOAD_HPP_
#define ELF_LOAD_HPP_

#include "samplebuffer.hpp"

#include <array>
#include <cstddef>

constexpr unsigned int MAX_ELF_FILE_SIZE = 16 * 1024;

class ELFManager
{
public:
    using EntryFunc = Sample *(*)(Sample *, size_t);
    
    static bool loadFromInternalBuffer();
    static EntryFunc loadedElf();
    static unsigned char *fileBuffer();
    static void unload();

private:
    static EntryFunc m_entry;

    static std::array<unsigned char, MAX_ELF_FILE_SIZE> m_file_buffer;
};

#endif // ELF_LOAD_HPP_

