/**
 * @file device_formula.cpp
 * @brief Function for filling generator buffer using a mathematical formula.
 * This is kept in its own file as exprtk.hpp takes forever to compile.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "stmdsp.hpp"

#include <algorithm>
#include <random>
#include <string_view>
#include <vector>

#ifndef STMDSP_DISABLE_FORMULAS

#define exprtk_disable_comments
#define exprtk_disable_break_continue
#define exprtk_disable_sc_andor
#define exprtk_disable_return_statement
#define exprtk_disable_enhanced_features
//#define exprtk_disable_string_capabilities
#define exprtk_disable_superscalar_unroll
#define exprtk_disable_rtl_io_file
#define exprtk_disable_rtl_vecops
//#define exprtk_disable_caseinsensitivity
#include "exprtk.hpp"

static std::random_device randomDevice;

std::vector<stmdsp::dacsample_t> deviceGenLoadFormulaEval(const std::string& formulaString)
{
    double x = 0;

    exprtk::symbol_table<double> symbol_table;
    exprtk::function_compositor<double> compositor (symbol_table);
    exprtk::expression<double> expression;
    exprtk::parser<double> parser;

    symbol_table.add_constants();
    symbol_table.add_variable("x", x);
    symbol_table.add_function("random",
        [](double l, double h) -> double {
            return std::uniform_real_distribution<double>(l, h)(randomDevice);
        });
    compositor.add(exprtk::function_compositor<double>::function()
                       .name("square")
                       .var("X")
                       .expression("ceil(sin(pi*X))"));
    compositor.add(exprtk::function_compositor<double>::function()
                       .name("triangle")
                       .var("X")
                       .expression("ceil(sin(pi*X))*(X-floor(X))+ceil(-sin(pi*X))*(-X-floor(-X))"));
    compositor.add(exprtk::function_compositor<double>::function()
                       .name("pulse")
                       .var("L")
                       .var("X")
                       .expression("if(X<=L,1,0)"));
    expression.register_symbol_table(symbol_table);
    parser.compile(formulaString, expression);

    const auto genFun = [&x, &expression] {
        const auto s = std::clamp(expression.value(), -1., 1.) * 2048. + 2048.;
        ++x;
        return static_cast<stmdsp::dacsample_t>(std::min(s, 4095.));
    };

    std::vector<stmdsp::dacsample_t> samples (stmdsp::SAMPLES_MAX);
    std::generate(samples.begin(), samples.end(), genFun);
    return samples;
}

#else // no formula support

std::vector<stmdsp::dacsample_t> deviceGenLoadFormulaEval(const std::string&)
{
    return {};
}

#endif // STMDSP_DISABLE_FORMULAS

