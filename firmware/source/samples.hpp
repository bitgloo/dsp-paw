/**
 * @file samples.hpp
 * @brief Provides sample buffers for inputs and outputs.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_SAMPLES_HPP
#define STMDSP_SAMPLES_HPP

#include "samplebuffer.hpp"

// Define sample buffers for the input and output signals and the signal
// generator.
class Samples
{
public:
    static SampleBuffer In;
    static SampleBuffer Out;
    static SampleBuffer Generator;
};

#endif // STMDSP_SAMPLES_HPP

