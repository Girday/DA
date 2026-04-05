#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
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

enum class OperationType {
    Insert,
    Erase,
    Find
};

struct Operation {
    OperationType type;
    std::string key;
    uint64_t value;
};

struct BenchmarkOptions {
    int repeats = 5;
    std::size_t prefill = 0;
    std::string scenario = "custom";
    std::optional<std::string> csv_path;
};

struct OperationResult {
    bool success;
    uint64_t value;
};

struct OperationCounts {
    std::size_t insert_count = 0;
    std::size_t erase_count = 0;
    std::size_t find_count = 0;
};

using OrderedMap = std::map<std::string, uint64_t>;
using benchmark_clock = std::chrono::steady_clock;
using duration_t = std::chrono::nanoseconds;

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

static BenchmarkOptions ParseArguments(int argc, char* argv[]) {
    BenchmarkOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string flag = argv[i];
        if (flag == "--repeats") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --repeats");
            options.repeats = static_cast<int>(ParseUnsigned(argv[++i], "--repeats"));
            if (options.repeats <= 0)
                throw std::runtime_error("--repeats must be positive");
        } else if (flag == "--prefill") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --prefill");
            options.prefill = static_cast<std::size_t>(ParseUnsigned(argv[++i], "--prefill"));
        } else if (flag == "--scenario") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --scenario");
            options.scenario = argv[++i];
        } else if (flag == "--csv") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --csv");
            options.csv_path = argv[++i];
        } else {
            throw std::runtime_error("Unknown argument: " + flag);
        }
    }

    return options;
}

static void EnsureNoExtraTokens(std::istringstream& input, const std::string& line) {
    std::string extra;
    if (input >> extra)
        throw std::runtime_error("Invalid benchmark input line: " + line);
}

static std::vector<Operation> ReadOperations(std::istream& input) {
    std::vector<Operation> operations;
    std::string line;

    while (std::getline(input, line)) {
        const std::string trimmed_line = Trim(line);
        if (trimmed_line.empty())
            continue;

        if (trimmed_line[0] == '!')
            throw std::runtime_error("Save/Load commands are not supported in benchmark input");

        std::istringstream line_input(trimmed_line);

        if (trimmed_line[0] == '+') {
            char command = '\0';
            std::string word;
            uint64_t value = 0;
            if (!(line_input >> command >> word >> value) || command != '+')
                throw std::runtime_error("Invalid insert line: " + trimmed_line);
            EnsureNoExtraTokens(line_input, trimmed_line);
            operations.push_back({OperationType::Insert, NormalizeWord(word), value});
        } else if (trimmed_line[0] == '-') {
            char command = '\0';
            std::string word;
            if (!(line_input >> command >> word) || command != '-')
                throw std::runtime_error("Invalid erase line: " + trimmed_line);
            EnsureNoExtraTokens(line_input, trimmed_line);
            operations.push_back({OperationType::Erase, NormalizeWord(word), 0});
        } else {
            std::string word;
            if (!(line_input >> word))
                throw std::runtime_error("Invalid find line: " + trimmed_line);
            EnsureNoExtraTokens(line_input, trimmed_line);
            operations.push_back({OperationType::Find, NormalizeWord(word), 0});
        }
    }

    return operations;
}

static OperationResult ApplyOperation(RedBlackTreeDictionary& dictionary, const Operation& operation) {
    if (operation.type == OperationType::Insert)
        return {dictionary.Insert(operation.key, operation.value), 0};

    if (operation.type == OperationType::Erase)
        return {dictionary.Erase(operation.key), 0};

    uint64_t value = 0;
    const bool found = dictionary.Find(operation.key, value);
    return {found, value};
}

static OperationResult ApplyOperation(OrderedMap& dictionary, const Operation& operation) {
    if (operation.type == OperationType::Insert)
        return {dictionary.insert({operation.key, operation.value}).second, 0};

    if (operation.type == OperationType::Erase)
        return {dictionary.erase(operation.key) != 0, 0};

    const auto iterator = dictionary.find(operation.key);
    if (iterator == dictionary.end())
        return {false, 0};

    return {true, iterator->second};
}

static OperationResult GetKeyState(RedBlackTreeDictionary& dictionary, const std::string& key) {
    uint64_t value = 0;
    const bool found = dictionary.Find(key, value);
    return {found, value};
}

static OperationResult GetKeyState(OrderedMap& dictionary, const std::string& key) {
    const auto iterator = dictionary.find(key);
    if (iterator == dictionary.end())
        return {false, 0};

    return {true, iterator->second};
}

static bool ResultsMatch(const OperationResult& left, const OperationResult& right) {
    if (left.success != right.success)
        return false;

    if (!left.success)
        return true;

    return left.value == right.value;
}

static void RunDifferentialCheck(const std::vector<Operation>& operations) {
    RedBlackTreeDictionary rb_tree;
    OrderedMap std_map;

    for (std::size_t i = 0; i < operations.size(); ++i) {
        const OperationResult rb_result = ApplyOperation(rb_tree, operations[i]);
        const OperationResult map_result = ApplyOperation(std_map, operations[i]);

        if (!ResultsMatch(rb_result, map_result))
            throw std::runtime_error("Differential check failed on operation " + std::to_string(i + 1));

        const OperationResult rb_state = GetKeyState(rb_tree, operations[i].key);
        const OperationResult map_state = GetKeyState(std_map, operations[i].key);
        if (!ResultsMatch(rb_state, map_state))
            throw std::runtime_error("State mismatch after operation " + std::to_string(i + 1));
    }
}

template <typename Dictionary>
static void ApplyPrefill(Dictionary& dictionary, const std::vector<Operation>& operations, std::size_t prefill) {
    for (std::size_t i = 0; i < prefill; ++i)
        ApplyOperation(dictionary, operations[i]);
}

static uint64_t MeasureDictionary(RedBlackTreeDictionary& dictionary, const std::vector<Operation>& operations, std::size_t start_index) {
    const auto start_time = benchmark_clock::now();
    for (std::size_t i = start_index; i < operations.size(); ++i)
        ApplyOperation(dictionary, operations[i]);
    const auto end_time = benchmark_clock::now();

    return std::chrono::duration_cast<duration_t>(end_time - start_time).count();
}

static uint64_t MeasureDictionary(OrderedMap& dictionary, const std::vector<Operation>& operations, std::size_t start_index) {
    const auto start_time = benchmark_clock::now();
    for (std::size_t i = start_index; i < operations.size(); ++i)
        ApplyOperation(dictionary, operations[i]);
    const auto end_time = benchmark_clock::now();

    return std::chrono::duration_cast<duration_t>(end_time - start_time).count();
}

static double Median(const std::vector<uint64_t>& values) {
    std::vector<uint64_t> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());

    const std::size_t middle = sorted_values.size() / 2;
    if (sorted_values.size() % 2 == 1)
        return static_cast<double>(sorted_values[middle]);

    return (static_cast<double>(sorted_values[middle - 1]) + static_cast<double>(sorted_values[middle])) / 2.0;
}

static OperationCounts CountMeasuredOperations(const std::vector<Operation>& operations, std::size_t start_index) {
    OperationCounts counts;

    for (std::size_t i = start_index; i < operations.size(); ++i) {
        if (operations[i].type == OperationType::Insert)
            ++counts.insert_count;
        else if (operations[i].type == OperationType::Erase)
            ++counts.erase_count;
        else
            ++counts.find_count;
    }

    return counts;
}

static void PrintTimes(const std::string& label, const std::vector<uint64_t>& times_ns) {
    std::cout << label << "=[";
    for (std::size_t i = 0; i < times_ns.size(); ++i) {
        if (i != 0)
            std::cout << ", ";
        std::cout << std::fixed << std::setprecision(3) << static_cast<double>(times_ns[i]) / 1000.0;
    }
    std::cout << "]\n";
}

static std::string JoinTimesForCsv(const std::vector<uint64_t>& times_ns) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(3);

    for (std::size_t i = 0; i < times_ns.size(); ++i) {
        if (i != 0)
            output << ';';
        output << static_cast<double>(times_ns[i]) / 1000.0;
    }

    return output.str();
}

static bool FileIsEmpty(const std::string& path) {
    std::ifstream input(path);
    return !input.good() || input.peek() == std::ifstream::traits_type::eof();
}

static void WriteCsvRow(
    const BenchmarkOptions& options,
    const OperationCounts& counts,
    std::size_t measured_operations,
    const std::vector<uint64_t>& rb_times_ns,
    const std::vector<uint64_t>& map_times_ns,
    double rb_median_us,
    double map_median_us,
    double rb_ns_per_operation,
    double map_ns_per_operation,
    double ratio
) {
    if (!options.csv_path.has_value())
        return;

    const bool file_is_empty = FileIsEmpty(*options.csv_path);
    std::ofstream output(*options.csv_path, std::ios::app);
    if (!output)
        throw std::runtime_error("Cannot open CSV file: " + *options.csv_path);

    if (file_is_empty) {
        output
            << "implementation,scenario,repeats,prefill,measured_ops,insert_ops,find_ops,erase_ops,"
            << "rb_tree_raw_us,std_map_raw_us,rb_tree_median_us,std_map_median_us,"
            << "rb_tree_ns_per_op,std_map_ns_per_op,std_map_over_rb_tree\n";
    }

    output << std::fixed << std::setprecision(3);
    output
        << IMPLEMENTATION_NAME << ','
        << options.scenario << ','
        << options.repeats << ','
        << options.prefill << ','
        << measured_operations << ','
        << counts.insert_count << ','
        << counts.find_count << ','
        << counts.erase_count << ','
        << '"' << JoinTimesForCsv(rb_times_ns) << '"' << ','
        << '"' << JoinTimesForCsv(map_times_ns) << '"' << ','
        << rb_median_us << ','
        << map_median_us << ','
        << rb_ns_per_operation << ','
        << map_ns_per_operation << ','
        << ratio << '\n';
}

int main(int argc, char* argv[]) {
    try {
        const BenchmarkOptions options = ParseArguments(argc, argv);
        const std::vector<Operation> operations = ReadOperations(std::cin);

        if (operations.empty())
            throw std::runtime_error("Benchmark input is empty");

        if (options.prefill > operations.size())
            throw std::runtime_error("--prefill is greater than the number of input operations");

        const std::size_t measured_operations = operations.size() - options.prefill;
        if (measured_operations == 0)
            throw std::runtime_error("There are no measured operations after prefill");

        RunDifferentialCheck(operations);

        std::vector<uint64_t> rb_times_ns;
        std::vector<uint64_t> map_times_ns;
        rb_times_ns.reserve(static_cast<std::size_t>(options.repeats));
        map_times_ns.reserve(static_cast<std::size_t>(options.repeats));

        for (int repeat = 0; repeat < options.repeats; ++repeat) {
            RedBlackTreeDictionary rb_tree;
            OrderedMap std_map;

            ApplyPrefill(rb_tree, operations, options.prefill);
            ApplyPrefill(std_map, operations, options.prefill);

            if (repeat % 2 == 0) {
                rb_times_ns.push_back(MeasureDictionary(rb_tree, operations, options.prefill));
                map_times_ns.push_back(MeasureDictionary(std_map, operations, options.prefill));
            } else {
                map_times_ns.push_back(MeasureDictionary(std_map, operations, options.prefill));
                rb_times_ns.push_back(MeasureDictionary(rb_tree, operations, options.prefill));
            }
        }

        const OperationCounts counts = CountMeasuredOperations(operations, options.prefill);
        const double rb_median_ns = Median(rb_times_ns);
        const double map_median_ns = Median(map_times_ns);
        const double rb_median_us = rb_median_ns / 1000.0;
        const double map_median_us = map_median_ns / 1000.0;
        const double rb_ns_per_operation = rb_median_ns / static_cast<double>(measured_operations);
        const double map_ns_per_operation = map_median_ns / static_cast<double>(measured_operations);
        const double ratio = (rb_median_ns == 0.0)
            ? std::numeric_limits<double>::infinity()
            : map_median_ns / rb_median_ns;

        std::cout << "implementation=" << IMPLEMENTATION_NAME << "\n";
        std::cout << "scenario=" << options.scenario << "\n";
        std::cout << "prefill=" << options.prefill << "\n";
        std::cout << "measured_ops=" << measured_operations << "\n";
        std::cout << "insert_ops=" << counts.insert_count << "\n";
        std::cout << "find_ops=" << counts.find_count << "\n";
        std::cout << "erase_ops=" << counts.erase_count << "\n";
        PrintTimes("rb_tree_raw_us", rb_times_ns);
        PrintTimes("std_map_raw_us", map_times_ns);
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "rb_tree_median_us=" << rb_median_us << "\n";
        std::cout << "std_map_median_us=" << map_median_us << "\n";
        std::cout << "rb_tree_ns_per_op=" << rb_ns_per_operation << "\n";
        std::cout << "std_map_ns_per_op=" << map_ns_per_operation << "\n";
        std::cout << "std_map_over_rb_tree=" << ratio << "\n";

        WriteCsvRow(
            options,
            counts,
            measured_operations,
            rb_times_ns,
            map_times_ns,
            rb_median_us,
            map_median_us,
            rb_ns_per_operation,
            map_ns_per_operation,
            ratio
        );
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
