#include "check_reducibility.hpp"
#include "generate_colorings.hpp"
#include "generate_kempes.hpp"
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <exception>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

int main(const int ac, const char *const *const av) {
    using namespace boost::program_options;
    options_description description("Options");
    // clang-format off
    description.add_options()
        ("kempe,k", value<int>()->default_value(0), "Number of kempe files to generate")
        ("color,c", value<int>()->default_value(0), "Number of color files to generate")
        ("input,i", value<string>(), "The file to evaluate")
        ("help,H", "Display options")
        ("verbosity,v", value<int>()->default_value(0), "1 for debug, 2 for trace");
    // clang-format on

    variables_map vm;
    store(parse_command_line(ac, av, description), vm);
    notify(vm);

    if (vm.count("help")) {
        description.print(cout);
        return 0;
    }
    if (vm.count("verbosity")) {
        auto v = vm["verbosity"].as<int>();
        if (v == 1) {
            spdlog::set_level(spdlog::level::debug);
        }
        if (v == 2) {
            spdlog::set_level(spdlog::level::trace);
        }
    }
    if (vm.count("kempe")) {
        auto k = vm["kempe"].as<int>();
        if (k > 0) {
            spdlog::info("Generating {} kempe files...", k);
            GenerateKempes(k);
        }
    }
    if (vm.count("color")) {
        auto c = vm["color"].as<int>();
        if (c > 0) {
            spdlog::info("Generating {} color files...", c);
            GenerateColorings(c);
        }
    }
    if (vm.count("input")) {
        auto fileName   = vm["input"].as<string>();
        KempeType kType = KempeType::HalfChain;
        ColorType cType = ColorType::All;
        try {
            EvaluateConf(fileName, kType, cType);
        } catch (const std::exception &e) {
            spdlog::critical("The program threw an error: {}", e.what());
            spdlog::critical("Terminating.");
        }
    }
    return 0;
}
