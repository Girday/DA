#include <cctype>
#include <cstdint>
#include <deque>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Position {
    std::size_t line;
    std::size_t word;
};

struct PatternInfo {
    std::size_t id;
    std::size_t length;
};

class AhoCorasick {
private:
    struct Node {
        std::unordered_map<std::uint32_t, std::size_t> next;
        std::size_t failure_link = 0;
        int output_link = -1;
        std::vector<PatternInfo> patterns;
    };

public:
    void AddPattern(const std::vector<std::uint32_t>& pattern, std::size_t pattern_id) {
        std::size_t current = 0;

        for (std::uint32_t value : pattern) {
            auto found = nodes_[current].next.find(value);
            if (found == nodes_[current].next.end()) {
                const std::size_t new_node = nodes_.size();
                nodes_.push_back(Node());
                nodes_[current].next[value] = new_node;
                current = new_node;
            } else
                current = found->second;
        }

        nodes_[current].patterns.push_back(PatternInfo{pattern_id, pattern.size()});
        if (pattern.size() > max_pattern_length_)
            max_pattern_length_ = pattern.size();
    }

    void BuildLinks() {
        std::queue<std::size_t> queue;

        for (const auto& edge : nodes_[0].next) {
            const std::size_t child = edge.second;
            nodes_[child].failure_link = 0;
            nodes_[child].output_link = -1;
            queue.push(child);
        }

        while (!queue.empty()) {
            const std::size_t vertex = queue.front();
            queue.pop();

            for (const auto& edge : nodes_[vertex].next) {
                const std::uint32_t value = edge.first;
                const std::size_t child = edge.second;
                std::size_t fallback = nodes_[vertex].failure_link;

                while (fallback != 0 && nodes_[fallback].next.find(value) == nodes_[fallback].next.end())
                    fallback = nodes_[fallback].failure_link;

                auto transition = nodes_[fallback].next.find(value);
                if (transition != nodes_[fallback].next.end() && transition->second != child)
                    nodes_[child].failure_link = transition->second;
                else
                    nodes_[child].failure_link = 0;

                const std::size_t suffix = nodes_[child].failure_link;
                if (!nodes_[suffix].patterns.empty())
                    nodes_[child].output_link = static_cast<int>(suffix);
                else
                    nodes_[child].output_link = nodes_[suffix].output_link;

                queue.push(child);
            }
        }
    }

    std::size_t MaxPatternLength() const {
        return max_pattern_length_;
    }

    std::size_t NextState(std::size_t state, std::uint32_t value) const {
        while (state != 0 && nodes_[state].next.find(value) == nodes_[state].next.end())
            state = nodes_[state].failure_link;

        auto transition = nodes_[state].next.find(value);
        if (transition != nodes_[state].next.end())
            return transition->second;

        return 0;
    }

    template <typename Callback>
    void ForEachMatch(std::size_t state, Callback on_match) const {
        int current = static_cast<int>(state);

        while (current != -1) {
            for (const PatternInfo& pattern : nodes_[current].patterns)
                on_match(pattern);

            current = nodes_[current].output_link;
        }
    }

private:
    std::vector<Node> nodes_ = std::vector<Node>(1);
    std::size_t max_pattern_length_ = 0;
};

template <typename Callback>
void ParseNumbers(const std::string& line, Callback on_number) {
    std::size_t index = 0;

    while (index < line.size()) {
        while (index < line.size() &&
               std::isspace(static_cast<unsigned char>(line[index])))
            ++index;

        if (index == line.size())
            break;

        std::uint64_t value = 0;
        while (index < line.size() &&
               !std::isspace(static_cast<unsigned char>(line[index]))) {
            value = value * 10 + static_cast<std::uint64_t>(line[index] - '0');
            ++index;
        }

        on_number(static_cast<std::uint32_t>(value));
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    AhoCorasick automaton;
    std::string line;
    std::size_t pattern_id = 1;

    while (std::getline(std::cin, line)) {
        std::vector<std::uint32_t> pattern;
        ParseNumbers(line, [&pattern](std::uint32_t value) {
            pattern.push_back(value);
        });

        if (pattern.empty())
            break;

        automaton.AddPattern(pattern, pattern_id);
        ++pattern_id;
    }

    if (pattern_id == 1)
        return 0;

    automaton.BuildLinks();

    std::deque<Position> recent_positions;
    std::size_t state = 0;
    std::size_t line_number = 0;
    const std::size_t max_pattern_length = automaton.MaxPatternLength();

    while (std::getline(std::cin, line)) {
        ++line_number;
        std::size_t word_number = 0;

        ParseNumbers(line, [&](std::uint32_t value) {
            ++word_number;

            recent_positions.push_back(Position{line_number, word_number});
            if (recent_positions.size() > max_pattern_length)
                recent_positions.pop_front();

            state = automaton.NextState(state, value);

            automaton.ForEachMatch(state, [&](const PatternInfo& pattern) {
                const std::size_t start_index = recent_positions.size() - pattern.length;
                const Position& start = recent_positions[start_index];
                std::cout << start.line << ", " << start.word << ", " << pattern.id << '\n';
            });
        });
    }

    return 0;
}
