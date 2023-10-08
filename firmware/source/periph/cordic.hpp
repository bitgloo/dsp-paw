/**
 * @file cordic.hpp
 * @brief Provides mathematical functions for algorithms.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CORDIC_HPP_
#define CORDIC_HPP_

/**
 * Named after the hardware CORDIC peripheral even though software
 * implementations may be used.
 */
namespace cordic {
    // Provides pi in case cordic functions require it.
    constexpr double PI = 3.1415926535L;

    /**
     * Prepares cordic functions for use.
     */
    void init();

    // mod - Calculates remainder for given fraction.
    // cos, sin, tan - The trig functions.

#if !defined(TARGET_PLATFORM_L4)
    double mod(double n, double d);

    double cos(double x);
    double sin(double x);
    double tan(double x);
#else
    float mod(float n, float d);

    float cos(float x);
    float sin(float x);
    float tan(float x);
#endif
}

#endif // CORDIC_HPP_

