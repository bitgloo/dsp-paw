/**
 * @file handlers.hpp
 * @brief Interrupt service routine handlers.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_HANDLERS_HPP
#define STMDSP_HANDLERS_HPP

#include "ch.h"

extern "C" {

__attribute__((naked))
void port_syscall(struct port_extctx *ctxp, uint32_t n);

__attribute__((naked))
void MemManage_Handler();

__attribute__((naked))
void HardFault_Handler();

}

#endif // STMDSP_HANDLERS_HPP

