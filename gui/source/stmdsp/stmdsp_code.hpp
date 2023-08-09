/**
 * @file stmdsp_code.hpp
 * @brief Source code and build scripts for stmdsp device algorithms.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_CODE_HPP
#define STMDSP_CODE_HPP

#ifdef STMDSP_WIN32
#define NEWLINE "\r\n"
#define COPY "copy"
#else
#define NEWLINE "\n"
#define COPY "cp"
#endif

namespace stmdsp {

// $0 = temp file name
// TODO try -ffunction-sections -fdata-sections -Wl,--gc-sections
static std::string makefile_text_h7 =
#ifdef STMDSP_WIN32
	"echo off" NEWLINE
#endif
    "arm-none-eabi-g++ -x c++ -Os -std=c++20 -fno-exceptions -fno-rtti "
        "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -mtune=cortex-m7 "
	"-nostartfiles "
        "-Wl,-Ttext-segment=0x00000000 -Wl,-zmax-page-size=512 -Wl,-eprocess_data_entry "
        "$0 -o $0.o" NEWLINE
	COPY " $0.o $0.orig.o" NEWLINE
	"arm-none-eabi-strip -s -S --strip-unneeded $0.o" NEWLINE
	"arm-none-eabi-objcopy --remove-section .ARM.attributes "
                          "--remove-section .comment "
                          "--remove-section .noinit "
                          "$0.o" NEWLINE
	"arm-none-eabi-size $0.o" NEWLINE;
static std::string makefile_text_l4 =
#ifdef STMDSP_WIN32
	"echo off" NEWLINE
#endif
    "arm-none-eabi-g++ -x c++ -Os -std=c++20 -fno-exceptions -fno-rtti "
        "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mtune=cortex-m4 "
        "-nostartfiles -I$1/cmsis "
        "-Wl,-Ttext-segment=0x10000000 -Wl,-zmax-page-size=512 -Wl,-eprocess_data_entry "
        "$0 -o $0.o" NEWLINE
    COPY " $0.o $0.orig.o" NEWLINE
    "arm-none-eabi-strip -s -S --strip-unneeded $0.o" NEWLINE
    "arm-none-eabi-objcopy --remove-section .ARM.attributes "
                          "--remove-section .comment "
                          "--remove-section .noinit "
                          "$0.o" NEWLINE
    "arm-none-eabi-size $0.o" NEWLINE;

// $0 = buffer size
static std::string file_header_h7 = R"cpp(
#include <cstdint>
#include <span>

using Sample = uint16_t;
using Samples = std::span<Sample, $0>;

Sample *process_data(Samples samples);
extern "C" void process_data_entry()
{
    Sample *samples;
    asm("mov %0, r0" : "=r" (samples));
    process_data(Samples(samples, $0));
}

static double PI = 3.14159265358979323846L;
__attribute__((naked))
auto sin(double x) {
asm("vmov.f64 r1, r2, d0;"
    "eor r0, r0;"
    "svc 1;"
    "vmov.f64 d0, r1, r2;"
    "bx lr");
return 0;
}
__attribute__((naked))
auto cos(double x) {
asm("vmov.f64 r1, r2, d0;"
	"mov r0, #1;"
	"svc 1;"
	"vmov.f64 d0, r1, r2;"
	"bx lr");
return 0;
}
__attribute__((naked))
auto tan(double x) {
asm("vmov.f64 r1, r2, d0;"
	"mov r0, #2;"
	"svc 1;"
	"vmov.f64 d0, r1, r2;"
	"bx lr");
return 0;
}
__attribute__((naked))
auto sqrt(double x) {
asm("vsqrt.f64 d0, d0; bx lr");
return 0;
}

auto readalt() {
Sample s;
asm("svc 3; mov %0, r0" : "=&r"(s));
return s;
}

// End stmdspgui header code

)cpp";
static std::string file_header_l4 = R"cpp(
#include <cstdint>

using Sample = uint16_t;
using Samples = Sample[$0];
constexpr unsigned int SIZE = $0;

Sample *process_data(Samples samples);
extern "C" void process_data_entry()
{
    Sample *samples;
    asm("mov %0, r0" : "=r" (samples));
    process_data(samples);
}

static inline float PI = 3.14159265358979L;
__attribute__((naked))
static inline auto sin(float x) {
    asm("vmov.f32 r1, s0;"
    "eor r0, r0;"
    "svc 1;"
    "vmov.f32 s0, r1;"
    "bx lr");
    return 0;
}
__attribute__((naked))
static inline auto cos(float x) {
    asm("vmov.f32 r1, s0;"
	"mov r0, #1;"
	"svc 1;"
	"vmov.f32 s0, r1;"
	"bx lr");
    return 0;
}
__attribute__((naked))
static inline auto tan(float x) {
    asm("vmov.f32 r1, s0;"
	"mov r0, #2;"
	"svc 1;"
	"vmov.f32 s0, r1;"
	"bx lr");
    return 0;
}
__attribute__((naked))
static inline auto sqrt(float) {
    asm("vsqrt.f32 s0, s0; bx lr");
    return 0;
}

static inline auto param1() {
    Sample s;
    asm("eor r0, r0; svc 3; mov %0, r0" : "=r" (s) :: "r0");
    return s;
}
static inline auto param2() {
    Sample s;
    asm("mov r0, #1; svc 3; mov %0, r0" : "=r" (s) :: "r0");
    return s;
}

//static inline void puts(const char *s) {
//    // 's' will already be in r0.
//    asm("push {r4-r6}; svc 4; pop {r4-r6}");
//}

// End stmdspgui header code

)cpp";


static std::string file_content = 
R"cpp(Sample* process_data(Samples samples)
{
    return samples;
}
)cpp";

} // namespace stmdsp

#endif // STMDSP_CODE_HPP

