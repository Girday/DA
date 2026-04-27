#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#define main solution_main
#include "main.cpp"
#undef main

struct InputData {
    std::vector<std::vector<std::uint32_t>> patterns;
    std::vector<std::uint32_t> text;
    std::size_t total_pattern_length = 0;
    std::size_t max_pattern_length = 0;
};

struct Options {
    std::string case_name;
    std::string scenario;
    std::size_t inserted_matches = 0;
    std::size_t seed = 0;
    std::size_t repeats = 5;
    std::string naive_mode = "auto";
};

struct AhoRun {
    std::uint64_t build_us = 0;
    std::uint64_t search_us = 0;
    std::uint64_t total_us = 0;
    std::uint64_t matches = 0;
};

const std::uint64_t NAIVE_AUTO_LIMIT = 100000000;

std::uint64_t ToUint64(const std::string& value, const std::string& name) {
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(value.c_str(), &end, 10);

    if (end == value.c_str() || *end != '\0')
        throw std::runtime_error("invalid integer for " + name + ": " + value);

    return static_cast<std::uint64_t>(parsed);
}

Options ParseOptions(int argc, char** argv) {
    Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--case-name" && i + 1 < argc)
            options.case_name = argv[++i];
        else if (arg == "--scenario" && i + 1 < argc)
            options.scenario = argv[++i];
        else if (arg == "--inserted" && i + 1 < argc)
            options.inserted_matches = ToUint64(argv[++i], "--inserted");
        else if (arg == "--seed" && i + 1 < argc)
            options.seed = ToUint64(argv[++i], "--seed");
        else if (arg == "--repeats" && i + 1 < argc)
            options.repeats = ToUint64(argv[++i], "--repeats");
        else if (arg == "--naive" && i + 1 < argc)
            options.naive_mode = argv[++i];
        else
            throw std::runtime_error("unknown or incomplete argument: " + arg);
    }

    if (options.case_name.empty())
        throw std::runtime_error("--case-name is required");
    if (options.scenario.empty())
        throw std::runtime_error("--scenario is required");
    if (options.repeats == 0)
        throw std::runtime_error("--repeats must be positive");
    if (options.naive_mode != "auto" && options.naive_mode != "on" && options.naive_mode != "off")
        throw std::runtime_error("--naive must be one of: auto, on, off");

    return options;
}

InputData ReadInput() {
    InputData input;
    std::string line;

    while (std::getline(std::cin, line)) {
        std::vector<std::uint32_t> pattern;
        ParseNumbers(line, [&pattern](std::uint32_t value) {
            pattern.push_back(value);
        });

        if (pattern.empty())
            break;

        input.total_pattern_length += pattern.size();
        if (pattern.size() > input.max_pattern_length)
            input.max_pattern_length = pattern.size();

        input.patterns.push_back(pattern);
    }

    while (std::getline(std::cin, line))
        ParseNumbers(line, [&input](std::uint32_t value) {
            input.text.push_back(value);
        });

    if (input.patterns.empty())
        throw std::runtime_error("input has no patterns");

    return input;
}

std::uint64_t NowUs() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
}

AhoRun RunAho(const InputData& input) {
    AhoRun run;

    const std::uint64_t build_start = NowUs();
    AhoCorasick automaton;
    for (std::size_t i = 0; i < input.patterns.size(); ++i)
        automaton.AddPattern(input.patterns[i], i + 1);
    automaton.BuildLinks();
    const std::uint64_t build_end = NowUs();

    const std::uint64_t search_start = NowUs();
    std::size_t state = 0;
    std::uint64_t matches = 0;

    for (std::uint32_t value : input.text) {
        state = automaton.NextState(state, value);
        automaton.ForEachMatch(state, [&matches](const PatternInfo& pattern) {
            (void)pattern;
            ++matches;
        });
    }

    const std::uint64_t search_end = NowUs();

    run.build_us = build_end - build_start;
    run.search_us = search_end - search_start;
    run.total_us = run.build_us + run.search_us;
    run.matches = matches;
    return run;
}

std::uint64_t RunNaive(const InputData& input) {
    std::uint64_t matches = 0;

    for (std::size_t start = 0; start < input.text.size(); ++start) {
        for (const auto& pattern : input.patterns) {
            if (start + pattern.size() > input.text.size())
                continue;

            bool equal = true;
            for (std::size_t offset = 0; offset < pattern.size(); ++offset) {
                if (input.text[start + offset] != pattern[offset]) {
                    equal = false;
                    break;
                }
            }

            if (equal)
                ++matches;
        }
    }

    return matches;
}

std::uint64_t Median(std::vector<std::uint64_t> values) {
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

std::string JoinRaw(const std::vector<std::uint64_t>& values) {
    std::ostringstream out;

    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0)
            out << ';';
        out << values[i];
    }

    return out.str();
}

bool ShouldRunNaive(const Options& options, const InputData& input) {
    if (options.naive_mode == "on")
        return true;
    if (options.naive_mode == "off")
        return false;

    const std::uint64_t estimated =
        static_cast<std::uint64_t>(input.text.size()) *
        static_cast<std::uint64_t>(input.total_pattern_length);

    return estimated <= NAIVE_AUTO_LIMIT;
}

void PrintCsvRow(const Options& options, const InputData& input, bool naive_enabled,
                 const std::vector<AhoRun>& aho_runs,
                 const std::vector<std::uint64_t>& naive_times,
                 std::uint64_t naive_matches) {
    std::vector<std::uint64_t> build_times;
    std::vector<std::uint64_t> search_times;
    std::vector<std::uint64_t> total_times;

    for (const AhoRun& run : aho_runs) {
        build_times.push_back(run.build_us);
        search_times.push_back(run.search_us);
        total_times.push_back(run.total_us);
    }

    const std::uint64_t build_median = Median(build_times);
    const std::uint64_t search_median = Median(search_times);
    const std::uint64_t total_median = Median(total_times);
    const std::uint64_t aho_matches = aho_runs.front().matches;

    std::cout << options.case_name << ','
              << options.scenario << ','
              << input.patterns.size() << ','
              << input.total_pattern_length << ','
              << input.max_pattern_length << ','
              << input.text.size() << ','
              << options.inserted_matches << ','
              << options.seed << ','
              << options.repeats << ','
              << '"' << JoinRaw(build_times) << '"' << ','
              << '"' << JoinRaw(search_times) << '"' << ','
              << '"' << JoinRaw(total_times) << '"' << ','
              << build_median << ','
              << search_median << ','
              << total_median << ','
              << aho_matches << ','
              << (naive_enabled ? 1 : 0) << ',';

    if (naive_enabled) {
        const std::uint64_t naive_median = Median(naive_times);
        const double ratio = total_median == 0 ? 0.0 : static_cast<double>(naive_median) / total_median;

        std::cout << '"' << JoinRaw(naive_times) << '"' << ','
                  << naive_median << ','
                  << naive_matches << ','
                  << std::fixed << std::setprecision(3) << ratio;
    } else {
        std::cout << ",,,";
    }

    std::cout << '\n';
}

int main(int argc, char** argv) {
    try {
        const Options options = ParseOptions(argc, argv);
        const InputData input = ReadInput();
        const bool naive_enabled = ShouldRunNaive(options, input);

        std::vector<AhoRun> aho_runs;
        std::vector<std::uint64_t> naive_times;
        std::uint64_t naive_matches = 0;

        for (std::size_t run = 0; run < options.repeats; ++run) {
            aho_runs.push_back(RunAho(input));

            if (run > 0 && aho_runs[run].matches != aho_runs[0].matches)
                throw std::runtime_error("Aho-Corasick match count changed between runs");
        }

        if (naive_enabled) {
            for (std::size_t run = 0; run < options.repeats; ++run) {
                const std::uint64_t start = NowUs();
                const std::uint64_t matches = RunNaive(input);
                const std::uint64_t end = NowUs();

                if (run == 0)
                    naive_matches = matches;
                else if (matches != naive_matches)
                    throw std::runtime_error("naive match count changed between runs");

                naive_times.push_back(end - start);
            }

            if (naive_matches != aho_runs.front().matches)
                throw std::runtime_error("Aho-Corasick and naive match counts differ");
        }

        PrintCsvRow(options, input, naive_enabled, aho_runs, naive_times, naive_matches);
    } catch (const std::exception& error) {
        std::cerr << "benchmark error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
