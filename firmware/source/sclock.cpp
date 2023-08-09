/**
 * @file sclock.cpp
 * @brief Manages sampling rate clock speeds.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "sclock.hpp"

GPTDriver *SClock::m_timer = &GPTD6;
unsigned int SClock::m_div = 1;
unsigned int SClock::m_runcount = 0;

const GPTConfig SClock::m_timer_config = {
#if defined(TARGET_PLATFORM_H7)
    .frequency = 4800000,
#else
    .frequency = 36000000,
#endif
    .callback = nullptr,
    .cr2 = TIM_CR2_MMS_1, /* TRGO */
    .dier = 0
};

const std::array<unsigned int, 6> SClock::m_rate_divs = {{
#if defined(TARGET_PLATFORM_H7)
    /* 8k  */ 600,
    /* 16k */ 300,
    /* 20k */ 240,
    /* 32k */ 150,
    /* 48k */ 100,
    /* 96k */ 50
#else
    /* 8k  */ 4500,
    /* 16k */ 2250,
    /* 20k */ 1800,
    /* 32k */ 1125,
    /* 48k */ 750,
    /* 96k */ 375
#endif
}};

void SClock::begin()
{
    gptStart(m_timer, &m_timer_config);
}

void SClock::start()
{
    if (m_runcount++ == 0)
        gptStartContinuous(m_timer, m_div);
}

void SClock::stop()
{
    if (--m_runcount == 0)
        gptStopTimer(m_timer);
}

void SClock::setRate(SClock::Rate rate)
{
    m_div = m_rate_divs[static_cast<unsigned int>(rate)];
}

unsigned int SClock::getRate()
{
    for (unsigned int i = 0; i < m_rate_divs.size(); ++i) {
        if (m_rate_divs[i] == m_div)
            return i;
    }

    return static_cast<unsigned int>(-1);
}

