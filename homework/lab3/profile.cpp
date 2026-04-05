#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifndef IMPL_PATH
#error IMPL_PATH is not defined
#endif

#ifndef IMPLEMENTATION_NAME
#define IMPLEMENTATION_NAME "unknown"
#endif

#define main rb_tree_solution_main
#include IMPL_PATH
#undef main

enum class ProfileMode {
    Insert,
    Find,
    Erase,
    Save,
    Load
};

struct Entry {
    std::string key;
    uint64_t value;
};

struct ProfileOptions {
    ProfileMode mode = ProfileMode::Insert;
    std::string dataset_path;
    std::string tmpfile_path = "/tmp/rb_tree_profile.bin";
    int runs = 5;
    int iterations = 1;
    int hold_ms = 0;
    std::optional<std::string> csv_path;
};

using profile_clock = std::chrono::steady_clock;
using duration_t = std::chrono::microseconds;

static uint64_t ParseUnsigned(const std::string& text, const std::string& flag_name) {
    std::size_t parsed_length = 0;
    uint64_t value = 0;

    try {
        value = std::stoull(text, &parsed_length);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid value for " + flag_name + ": " + text);
    }

    if (parsed_length != text.size())
        throw std::runtime_error("Invalid value for " + flag_name + ": " + text);

    return value;
}

static ProfileMode ParseMode(const std::string& text) {
    if (text == "insert")
        return ProfileMode::Insert;
    if (text == "find")
        return ProfileMode::Find;
    if (text == "erase")
        return ProfileMode::Erase;
    if (text == "save")
        return ProfileMode::Save;
    if (text == "load")
        return ProfileMode::Load;
    throw std::runtime_error("Unknown mode: " + text);
}

static const char* ModeName(ProfileMode mode) {
    if (mode == ProfileMode::Insert)
        return "insert";
    if (mode == ProfileMode::Find)
        return "find";
    if (mode == ProfileMode::Erase)
        return "erase";
    if (mode == ProfileMode::Save)
        return "save";
    return "load";
}

static ProfileOptions ParseArguments(int argc, char* argv[]) {
    ProfileOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string flag = argv[i];
        if (flag == "--mode") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --mode");
            options.mode = ParseMode(argv[++i]);
        } else if (flag == "--dataset") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --dataset");
            options.dataset_path = argv[++i];
        } else if (flag == "--tmpfile") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --tmpfile");
            options.tmpfile_path = argv[++i];
        } else if (flag == "--runs") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --runs");
            options.runs = static_cast<int>(ParseUnsigned(argv[++i], "--runs"));
        } else if (flag == "--iterations") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --iterations");
            options.iterations = static_cast<int>(ParseUnsigned(argv[++i], "--iterations"));
        } else if (flag == "--hold-ms") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --hold-ms");
            options.hold_ms = static_cast<int>(ParseUnsigned(argv[++i], "--hold-ms"));
        } else if (flag == "--csv") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --csv");
            options.csv_path = argv[++i];
        } else {
            throw std::runtime_error("Unknown argument: " + flag);
        }
    }

    if (options.dataset_path.empty())
        throw std::runtime_error("--dataset is required");
    if (options.runs <= 0)
        throw std::runtime_error("--runs must be positive");
    if (options.iterations <= 0)
        throw std::runtime_error("--iterations must be positive");

    return options;
}

static std::vector<Entry> LoadDataset(const std::string& path) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("Cannot open dataset: " + path);

    std::vector<Entry> entries;
    std::string key;
    uint64_t value = 0;

    while (input >> key >> value)
        entries.push_back({NormalizeWord(key), value});

    if (entries.empty())
        throw std::runtime_error("Dataset is empty: " + path);

    return entries;
}

static double Median(std::vector<uint64_t> values) {
    std::sort(values.begin(), values.end());

    const std::size_t middle = values.size() / 2;
    if (values.size() % 2 == 1)
        return static_cast<double>(values[middle]);

    return (static_cast<double>(values[middle - 1]) + static_cast<double>(values[middle])) / 2.0;
}

static std::string JoinTimes(const std::vector<uint64_t>& values) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(3);

    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0)
            output << ';';
        output << static_cast<double>(values[i]);
    }

    return output.str();
}

static bool FileIsEmpty(const std::string& path) {
    std::ifstream input(path);
    return !input.good() || input.peek() == std::ifstream::traits_type::eof();
}

static void MaybeWriteCsv(
    const ProfileOptions& options,
    std::size_t entry_count,
    const std::vector<uint64_t>& run_times_us,
    double median_total_us,
    double median_iteration_us
) {
    if (!options.csv_path.has_value())
        return;

    const bool file_is_empty = FileIsEmpty(*options.csv_path);
    std::ofstream output(*options.csv_path, std::ios::app);
    if (!output)
        throw std::runtime_error("Cannot open CSV file: " + *options.csv_path);

    if (file_is_empty)
        output << "implementation,mode,entries,runs,iterations,raw_total_us,median_total_us,median_us_per_iteration\n";

    output << std::fixed << std::setprecision(3);
    output
        << IMPLEMENTATION_NAME << ','
        << ModeName(options.mode) << ','
        << entry_count << ','
        << options.runs << ','
        << options.iterations << ','
        << '"' << JoinTimes(run_times_us) << '"' << ','
        << median_total_us << ','
        << median_iteration_us << '\n';
}

static uint64_t TimeCall(const std::function<void()>& action) {
    const auto start = profile_clock::now();
    action();
    const auto finish = profile_clock::now();
    return static_cast<uint64_t>(std::chrono::duration_cast<duration_t>(finish - start).count());
}

static void CheckOperation(bool success, const std::string& message) {
    if (!success)
        throw std::runtime_error(message);
}

static void CheckIo(bool success, const std::string& error) {
    if (!success)
        throw std::runtime_error(error);
}

static uint64_t RunInsert(const std::vector<Entry>& entries, int iterations, bool keep_alive, int hold_ms) {
    std::unique_ptr<RedBlackTreeDictionary> held_dictionary;
    uint64_t total_us = 0;

    for (int iteration = 0; iteration < iterations; ++iteration) {
        auto dictionary = std::make_unique<RedBlackTreeDictionary>();
        total_us += TimeCall([&]() {
            for (const Entry& entry : entries)
                CheckOperation(dictionary->Insert(entry.key, entry.value), "Insert failed in insert profile");
        });

        if (keep_alive && iteration + 1 == iterations)
            held_dictionary = std::move(dictionary);
    }

    if (hold_ms > 0 && held_dictionary)
        std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));

    return total_us;
}

static uint64_t RunFind(const std::vector<Entry>& entries, int iterations, bool keep_alive, int hold_ms) {
    auto dictionary = std::make_unique<RedBlackTreeDictionary>();
    for (const Entry& entry : entries)
        CheckOperation(dictionary->Insert(entry.key, entry.value), "Insert failed while preparing find profile");

    uint64_t total_us = TimeCall([&]() {
        for (int iteration = 0; iteration < iterations; ++iteration) {
            for (const Entry& entry : entries) {
                uint64_t value = 0;
                CheckOperation(dictionary->Find(entry.key, value), "Find failed in find profile");
            }
        }
    });

    if (keep_alive && hold_ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));

    return total_us;
}

static uint64_t RunErase(const std::vector<Entry>& entries, int iterations, bool keep_alive, int hold_ms) {
    std::unique_ptr<RedBlackTreeDictionary> held_dictionary;
    uint64_t total_us = 0;

    for (int iteration = 0; iteration < iterations; ++iteration) {
        auto dictionary = std::make_unique<RedBlackTreeDictionary>();
        for (const Entry& entry : entries)
            CheckOperation(dictionary->Insert(entry.key, entry.value), "Insert failed while preparing erase profile");

        total_us += TimeCall([&]() {
            for (const Entry& entry : entries)
                CheckOperation(dictionary->Erase(entry.key), "Erase failed in erase profile");
        });

        if (keep_alive && iteration + 1 == iterations)
            held_dictionary = std::move(dictionary);
    }

    if (hold_ms > 0 && held_dictionary)
        std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));

    return total_us;
}

static uint64_t RunSave(const std::vector<Entry>& entries, const std::string& path, int iterations, bool keep_alive, int hold_ms) {
    auto dictionary = std::make_unique<RedBlackTreeDictionary>();
    for (const Entry& entry : entries)
        CheckOperation(dictionary->Insert(entry.key, entry.value), "Insert failed while preparing save profile");

    uint64_t total_us = TimeCall([&]() {
        for (int iteration = 0; iteration < iterations; ++iteration) {
            std::string error;
            CheckIo(dictionary->Save(path, error), error);
        }
    });

    if (keep_alive && hold_ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));

    return total_us;
}

static uint64_t RunLoad(const std::vector<Entry>& entries, const std::string& path, int iterations, bool keep_alive, int hold_ms) {
    {
        RedBlackTreeDictionary fixture_dictionary;
        for (const Entry& entry : entries)
            CheckOperation(fixture_dictionary.Insert(entry.key, entry.value), "Insert failed while preparing load fixture");

        std::string error;
        CheckIo(fixture_dictionary.Save(path, error), error);
    }

    std::unique_ptr<RedBlackTreeDictionary> held_dictionary;
    uint64_t total_us = 0;

    for (int iteration = 0; iteration < iterations; ++iteration) {
        auto dictionary = std::make_unique<RedBlackTreeDictionary>();
        total_us += TimeCall([&]() {
            std::string error;
            CheckIo(dictionary->Load(path, error), error);
        });

        if (keep_alive && iteration + 1 == iterations)
            held_dictionary = std::move(dictionary);
    }

    if (hold_ms > 0 && held_dictionary)
        std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));

    return total_us;
}

static uint64_t RunOneMeasurement(const ProfileOptions& options, const std::vector<Entry>& entries, bool keep_alive) {
    if (options.mode == ProfileMode::Insert)
        return RunInsert(entries, options.iterations, keep_alive, options.hold_ms);
    if (options.mode == ProfileMode::Find)
        return RunFind(entries, options.iterations, keep_alive, options.hold_ms);
    if (options.mode == ProfileMode::Erase)
        return RunErase(entries, options.iterations, keep_alive, options.hold_ms);
    if (options.mode == ProfileMode::Save)
        return RunSave(entries, options.tmpfile_path, options.iterations, keep_alive, options.hold_ms);
    return RunLoad(entries, options.tmpfile_path, options.iterations, keep_alive, options.hold_ms);
}

int main(int argc, char* argv[]) {
    try {
        const ProfileOptions options = ParseArguments(argc, argv);
        const std::vector<Entry> entries = LoadDataset(options.dataset_path);

        std::vector<uint64_t> run_times_us;
        run_times_us.reserve(static_cast<std::size_t>(options.runs));

        for (int run = 0; run < options.runs; ++run) {
            const bool keep_alive = options.hold_ms > 0 && run + 1 == options.runs;
            run_times_us.push_back(RunOneMeasurement(options, entries, keep_alive));
        }

        const double median_total_us = Median(run_times_us);
        const double median_iteration_us = median_total_us / static_cast<double>(options.iterations);

        std::cout << "implementation=" << IMPLEMENTATION_NAME << "\n";
        std::cout << "mode=" << ModeName(options.mode) << "\n";
        std::cout << "entries=" << entries.size() << "\n";
        std::cout << "runs=" << options.runs << "\n";
        std::cout << "iterations=" << options.iterations << "\n";
        std::cout << "raw_total_us=[" << JoinTimes(run_times_us) << "]\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "median_total_us=" << median_total_us << "\n";
        std::cout << "median_us_per_iteration=" << median_iteration_us << "\n";

        MaybeWriteCsv(options, entries.size(), run_times_us, median_total_us, median_iteration_us);
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
