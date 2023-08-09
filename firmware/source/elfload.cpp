/**
 * @file elfload.cpp
 * @brief Loads ELF binary data into memory for execution.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "elfload.hpp"
#include "elf.h"

#include <algorithm>
#include <cstring>

__attribute__((section(".convdata")))
ELFManager::EntryFunc ELFManager::m_entry = nullptr;
std::array<unsigned char, MAX_ELF_FILE_SIZE> ELFManager::m_file_buffer = {};

static const unsigned char elf_header[] = { '\177', 'E', 'L', 'F' };

__attribute__((section(".convcode")))
ELFManager::EntryFunc ELFManager::loadedElf()
{
    return m_entry;
}

unsigned char *ELFManager::fileBuffer()
{
    return m_file_buffer.data();
}

void ELFManager::unload()
{
    m_entry = nullptr;
}

template<typename T>
constexpr static auto ptr_from_offset(void *base, uint32_t offset)
{
    return reinterpret_cast<T>(reinterpret_cast<uint8_t *>(base) + offset);
}

bool ELFManager::loadFromInternalBuffer()
{
    m_entry = nullptr;

    auto elf_data = m_file_buffer.data();

    // Check the ELF's header signature
    auto ehdr = reinterpret_cast<Elf32_Ehdr *>(elf_data);
    if (!std::equal(ehdr->e_ident, ehdr->e_ident + 4, elf_header))
        return false;

    // Iterate through program header LOAD sections
    bool loaded = false;
    auto phdr = ptr_from_offset<Elf32_Phdr *>(elf_data, ehdr->e_phoff);
    for (Elf32_Half i = 0; i < ehdr->e_phnum; i++) {
        if (phdr->p_type == PT_LOAD) {
            if (phdr->p_filesz == 0) {
                std::memset(reinterpret_cast<void *>(phdr->p_vaddr),
                            0,
                            phdr->p_memsz);
            } else {
                std::memcpy(reinterpret_cast<void *>(phdr->p_vaddr),
                            ptr_from_offset<void *>(elf_data, phdr->p_offset),
                            phdr->p_filesz);
                if (!loaded)
                    loaded = true;
            }
        }

        phdr = ptr_from_offset<Elf32_Phdr *>(phdr, ehdr->e_phentsize);
    }


    if (loaded)
        m_entry = reinterpret_cast<ELFManager::EntryFunc>(ehdr->e_entry);

    return loaded;
}

