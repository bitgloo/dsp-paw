/**
 * @file samples.cpp
 * @brief Provides sample buffers for inputs and outputs.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "samples.hpp"

#include "ch.h"
#include "hal.h"

#include <cstdint>

static_assert(sizeof(adcsample_t) == sizeof(uint16_t));
static_assert(sizeof(dacsample_t) == sizeof(uint16_t));

#if defined(TARGET_PLATFORM_H7)
__attribute__((section(".convdata")))
SampleBuffer Samples::In (reinterpret_cast<Sample *>(0x38000000)); // 16k
__attribute__((section(".convdata")))
SampleBuffer Samples::Out (reinterpret_cast<Sample *>(0x30004000)); // 16k
SampleBuffer Samples::SigGen (reinterpret_cast<Sample *>(0x30000000)); // 16k
#else
__attribute__((section(".convdata")))
SampleBuffer Samples::In (reinterpret_cast<Sample *>(0x20008000)); // 16k
__attribute__((section(".convdata")))
SampleBuffer Samples::Out (reinterpret_cast<Sample *>(0x2000C000)); // 16k
SampleBuffer Samples::Generator (reinterpret_cast<Sample *>(0x20010000)); // 16k
#endif

