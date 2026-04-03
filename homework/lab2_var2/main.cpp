#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <utility>

class RedBlackTreeDictionary {
private:
    enum Color {
        RED,
        BLACK
    };

    struct Node {
        std::string key;
        uint64_t value;
        Color color;
        Node* parent;
        Node* left;
        Node* right;

        Node()
            : value(0), color(BLACK), parent(nullptr), left(nullptr), right(nullptr) {
        }

        Node(std::string key_value, uint64_t stored_value, Color node_color, Node* nil)
            : key(std::move(key_value)),
              value(stored_value),
              color(node_color),
              parent(nil),
              left(nil),
              right(nil) {
        }
    };

    class FileHandle {
    public:
        explicit FileHandle(std::FILE* file = nullptr) : file_(file) {
        }

        ~FileHandle() {
            if (file_ != nullptr)
                std::fclose(file_);
        }

        std::FILE* get() const {
            return file_;
        }

        int Close() {
            if (file_ == nullptr)
                return 0;
            std::FILE* raw = file_;
            file_ = nullptr;
            return std::fclose(raw);
        }

    private:
        std::FILE* file_;
    };

    enum class ReadStatus {
        OK,
        END_OF_FILE,
        SYSTEM_ERROR
    };

    static constexpr unsigned char kMagic[4] = {'R', 'B', 'T', 'D'};
    static constexpr unsigned char kVersion = 1;

public:
    RedBlackTreeDictionary()
        : root_(nullptr), nil_(new Node()), size_(0) {
        nil_->color = BLACK;
        nil_->parent = nil_;
        nil_->left = nil_;
        nil_->right = nil_;
        root_ = nil_;
    }

    RedBlackTreeDictionary(const RedBlackTreeDictionary&) = delete;
    RedBlackTreeDictionary& operator=(const RedBlackTreeDictionary&) = delete;

    ~RedBlackTreeDictionary() {
        Clear();
        delete nil_;
    }

    void Swap(RedBlackTreeDictionary& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(nil_, other.nil_);
        std::swap(size_, other.size_);
    }

    bool Insert(std::string key, uint64_t value) {
        Node* parent = nil_;
        Node* current = root_;

        while (current != nil_) {
            parent = current;
            if (key == current->key)
                return false;
            current = (key < current->key) ? current->left : current->right;
        }

        Node* inserted = new Node(std::move(key), value, RED, nil_);
        inserted->parent = parent;

        if (parent == nil_)
            root_ = inserted;
        else if (inserted->key < parent->key)
            parent->left = inserted;
        else
            parent->right = inserted;

        InsertFixup(inserted);
        ++size_;
        return true;
    }

    bool Erase(const std::string& key) {
        Node* target = FindNode(key);
        if (target == nil_)
            return false;

        Node* replacement_parent_candidate = target;
        Color original_color = replacement_parent_candidate->color;
        Node* fixup_node = nil_;

        if (target->left == nil_) {
            fixup_node = target->right;
            Transplant(target, target->right);
        } else if (target->right == nil_) {
            fixup_node = target->left;
            Transplant(target, target->left);
        } else {
            replacement_parent_candidate = Minimum(target->right);
            original_color = replacement_parent_candidate->color;
            fixup_node = replacement_parent_candidate->right;

            if (replacement_parent_candidate->parent == target) {
                fixup_node->parent = replacement_parent_candidate;
            } else {
                Transplant(replacement_parent_candidate, replacement_parent_candidate->right);
                replacement_parent_candidate->right = target->right;
                replacement_parent_candidate->right->parent = replacement_parent_candidate;
            }

            Transplant(target, replacement_parent_candidate);
            replacement_parent_candidate->left = target->left;
            replacement_parent_candidate->left->parent = replacement_parent_candidate;
            replacement_parent_candidate->color = target->color;
        }

        delete target;
        --size_;

        if (original_color == BLACK)
            DeleteFixup(fixup_node);

        return true;
    }

    bool Find(const std::string& key, uint64_t& value) const {
        Node* found = FindNode(key);
        if (found == nil_)
            return false;
        value = found->value;
        return true;
    }

    bool Save(const std::string& path, std::string& error) const {
        error.clear();

        FileHandle file(std::fopen(path.c_str(), "wb"));
        if (file.get() == nullptr) {
            error = MakeSystemErrorMessage(errno);
            return false;
        }

        if (!WriteBytes(file.get(), kMagic, sizeof(kMagic), error) ||
            !WriteByte(file.get(), kVersion, error) ||
            !WriteUint64(file.get(), size_, error) ||
            !WriteInOrder(file.get(), root_, error))
            return false;

        if (file.Close() != 0) {
            error = MakeSystemErrorMessage(errno);
            return false;
        }

        return true;
    }

    bool Load(const std::string& path, std::string& error) {
        error.clear();

        FileHandle file(std::fopen(path.c_str(), "rb"));
        if (file.get() == nullptr) {
            const int open_error = errno;
            if (open_error == ENOENT) {
                RedBlackTreeDictionary empty_dictionary;
                Swap(empty_dictionary);
                return true;
            }
            error = MakeSystemErrorMessage(open_error);
            return false;
        }

        RedBlackTreeDictionary loaded_dictionary;
        if (!ReadFromFile(file.get(), loaded_dictionary, error))
            return false;

        if (file.Close() != 0) {
            error = MakeSystemErrorMessage(errno);
            return false;
        }

        Swap(loaded_dictionary);
        return true;
    }

private:
    Node* root_;
    Node* nil_;
    uint64_t size_;

    static char ToLowerAscii(char symbol) {
        if (symbol >= 'A' && symbol <= 'Z')
            return static_cast<char>(symbol - 'A' + 'a');
        return symbol;
    }

    static bool IsEnglishLetter(char symbol) {
        return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
    }

    static std::string MakeSystemErrorMessage(int error_code) {
        if (error_code == 0)
            return "ERROR: I/O error";
        return std::string("ERROR: ") + std::strerror(error_code);
    }

    static std::string MakeInvalidFormatMessage() {
        return "ERROR: invalid file format";
    }

    static ReadStatus ReadBytes(std::FILE* file, void* buffer, std::size_t size, std::string& error) {
        if (size == 0)
            return ReadStatus::OK;

        const std::size_t bytes_read = std::fread(buffer, 1, size, file);
        if (bytes_read == size)
            return ReadStatus::OK;

        if (std::ferror(file) != 0) {
            error = MakeSystemErrorMessage(errno);
            return ReadStatus::SYSTEM_ERROR;
        }

        return ReadStatus::END_OF_FILE;
    }

    static bool WriteBytes(std::FILE* file, const void* buffer, std::size_t size, std::string& error) {
        if (size == 0)
            return true;

        if (std::fwrite(buffer, 1, size, file) != size) {
            error = MakeSystemErrorMessage(errno);
            return false;
        }

        return true;
    }

    static bool WriteByte(std::FILE* file, unsigned char value, std::string& error) {
        return WriteBytes(file, &value, sizeof(value), error);
    }

    static bool WriteUint16(std::FILE* file, uint16_t value, std::string& error) {
        unsigned char buffer[2] = {
            static_cast<unsigned char>(value & 0xFFu),
            static_cast<unsigned char>((value >> 8) & 0xFFu)
        };
        return WriteBytes(file, buffer, sizeof(buffer), error);
    }

    static bool WriteUint64(std::FILE* file, uint64_t value, std::string& error) {
        unsigned char buffer[8];
        for (int i = 0; i < 8; ++i)
            buffer[i] = static_cast<unsigned char>((value >> (i * 8)) & 0xFFu);
        return WriteBytes(file, buffer, sizeof(buffer), error);
    }

    static ReadStatus ReadUint16(std::FILE* file, uint16_t& value, std::string& error) {
        unsigned char buffer[2];
        const ReadStatus status = ReadBytes(file, buffer, sizeof(buffer), error);
        if (status != ReadStatus::OK)
            return status;
        value = static_cast<uint16_t>(buffer[0]) |
                (static_cast<uint16_t>(buffer[1]) << 8);
        return ReadStatus::OK;
    }

    static ReadStatus ReadUint64(std::FILE* file, uint64_t& value, std::string& error) {
        unsigned char buffer[8];
        const ReadStatus status = ReadBytes(file, buffer, sizeof(buffer), error);
        if (status != ReadStatus::OK)
            return status;

        value = 0;
        for (int i = 0; i < 8; ++i)
            value |= static_cast<uint64_t>(buffer[i]) << (i * 8);
        return ReadStatus::OK;
    }

    static ReadStatus ReadExpected(std::FILE* file, void* buffer, std::size_t size, std::string& error) {
        const ReadStatus status = ReadBytes(file, buffer, size, error);
        if (status == ReadStatus::END_OF_FILE)
            error = MakeInvalidFormatMessage();
        return status;
    }

    bool ReadFromFile(std::FILE* file, RedBlackTreeDictionary& loaded_dictionary, std::string& error) {
        int first_byte = std::fgetc(file);
        if (first_byte == EOF) {
            if (std::ferror(file) != 0) {
                error = MakeSystemErrorMessage(errno);
                return false;
            }

            RedBlackTreeDictionary empty_dictionary;
            loaded_dictionary.Swap(empty_dictionary);
            return true;
        }

        unsigned char magic[4];
        magic[0] = static_cast<unsigned char>(first_byte);
        ReadStatus status = ReadExpected(file, magic + 1, sizeof(magic) - 1, error);
        if (status != ReadStatus::OK)
            return false;

        for (std::size_t i = 0; i < sizeof(magic); ++i) {
            if (magic[i] != kMagic[i]) {
                error = MakeInvalidFormatMessage();
                return false;
            }
        }

        unsigned char version = 0;
        status = ReadExpected(file, &version, sizeof(version), error);
        if (status != ReadStatus::OK)
            return false;
        if (version != kVersion) {
            error = MakeInvalidFormatMessage();
            return false;
        }

        uint64_t count = 0;
        status = ReadUint64(file, count, error);
        if (status == ReadStatus::END_OF_FILE) {
            error = MakeInvalidFormatMessage();
            return false;
        }
        if (status == ReadStatus::SYSTEM_ERROR)
            return false;

        for (uint64_t i = 0; i < count; ++i) {
            uint16_t length = 0;
            status = ReadUint16(file, length, error);
            if (status == ReadStatus::END_OF_FILE) {
                error = MakeInvalidFormatMessage();
                return false;
            }
            if (status == ReadStatus::SYSTEM_ERROR)
                return false;
            if (length == 0 || length > 256) {
                error = MakeInvalidFormatMessage();
                return false;
            }

            std::string key(length, '\0');
            status = ReadExpected(file, key.data(), key.size(), error);
            if (status != ReadStatus::OK)
                return false;

            for (char& symbol : key) {
                if (!IsEnglishLetter(symbol)) {
                    error = MakeInvalidFormatMessage();
                    return false;
                }
                symbol = ToLowerAscii(symbol);
            }

            uint64_t value = 0;
            status = ReadUint64(file, value, error);
            if (status == ReadStatus::END_OF_FILE) {
                error = MakeInvalidFormatMessage();
                return false;
            }
            if (status == ReadStatus::SYSTEM_ERROR)
                return false;

            if (!loaded_dictionary.Insert(std::move(key), value)) {
                error = MakeInvalidFormatMessage();
                return false;
            }
        }

        const int extra_byte = std::fgetc(file);
        if (extra_byte != EOF) {
            error = MakeInvalidFormatMessage();
            return false;
        }
        if (std::ferror(file) != 0) {
            error = MakeSystemErrorMessage(errno);
            return false;
        }

        return true;
    }

    void Clear() noexcept {
        DestroySubtree(root_);
        root_ = nil_;
        size_ = 0;
        nil_->parent = nil_;
        nil_->left = nil_;
        nil_->right = nil_;
    }

    void DestroySubtree(Node* node) noexcept {
        if (node == nil_)
            return;
        DestroySubtree(node->left);
        DestroySubtree(node->right);
        delete node;
    }

    Node* FindNode(const std::string& key) const {
        Node* current = root_;
        while (current != nil_) {
            if (key == current->key)
                return current;
            current = (key < current->key) ? current->left : current->right;
        }
        return nil_;
    }

    Node* Minimum(Node* node) const {
        while (node->left != nil_)
            node = node->left;
        return node;
    }

    void LeftRotate(Node* node) {
        Node* pivot = node->right;
        node->right = pivot->left;
        if (pivot->left != nil_)
            pivot->left->parent = node;

        pivot->parent = node->parent;
        if (node->parent == nil_)
            root_ = pivot;
        else if (node == node->parent->left)
            node->parent->left = pivot;
        else
            node->parent->right = pivot;

        pivot->left = node;
        node->parent = pivot;
    }

    void RightRotate(Node* node) {
        Node* pivot = node->left;
        node->left = pivot->right;
        if (pivot->right != nil_)
            pivot->right->parent = node;

        pivot->parent = node->parent;
        if (node->parent == nil_)
            root_ = pivot;
        else if (node == node->parent->right)
            node->parent->right = pivot;
        else
            node->parent->left = pivot;

        pivot->right = node;
        node->parent = pivot;
    }

    void InsertFixup(Node* node) {
        while (node->parent->color == RED) {
            if (node->parent == node->parent->parent->left) {
                Node* uncle = node->parent->parent->right;
                if (uncle->color == RED) {
                    node->parent->color = BLACK;
                    uncle->color = BLACK;
                    node->parent->parent->color = RED;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        LeftRotate(node);
                    }
                    node->parent->color = BLACK;
                    node->parent->parent->color = RED;
                    RightRotate(node->parent->parent);
                }
            } else {
                Node* uncle = node->parent->parent->left;
                if (uncle->color == RED) {
                    node->parent->color = BLACK;
                    uncle->color = BLACK;
                    node->parent->parent->color = RED;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        RightRotate(node);
                    }
                    node->parent->color = BLACK;
                    node->parent->parent->color = RED;
                    LeftRotate(node->parent->parent);
                }
            }
        }

        root_->color = BLACK;
    }

    void Transplant(Node* source, Node* destination) {
        if (source->parent == nil_)
            root_ = destination;
        else if (source == source->parent->left)
            source->parent->left = destination;
        else
            source->parent->right = destination;
        destination->parent = source->parent;
    }

    void DeleteFixup(Node* node) {
        while (node != root_ && node->color == BLACK) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    node->parent->color = RED;
                    LeftRotate(node->parent);
                    sibling = node->parent->right;
                }

                if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
                    sibling->color = RED;
                    node = node->parent;
                } else {
                    if (sibling->right->color == BLACK) {
                        sibling->left->color = BLACK;
                        sibling->color = RED;
                        RightRotate(sibling);
                        sibling = node->parent->right;
                    }

                    sibling->color = node->parent->color;
                    node->parent->color = BLACK;
                    sibling->right->color = BLACK;
                    LeftRotate(node->parent);
                    node = root_;
                }
            } else {
                Node* sibling = node->parent->left;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    node->parent->color = RED;
                    RightRotate(node->parent);
                    sibling = node->parent->left;
                }

                if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
                    sibling->color = RED;
                    node = node->parent;
                } else {
                    if (sibling->left->color == BLACK) {
                        sibling->right->color = BLACK;
                        sibling->color = RED;
                        LeftRotate(sibling);
                        sibling = node->parent->left;
                    }

                    sibling->color = node->parent->color;
                    node->parent->color = BLACK;
                    sibling->left->color = BLACK;
                    RightRotate(node->parent);
                    node = root_;
                }
            }
        }

        node->color = BLACK;
    }

    bool WriteInOrder(std::FILE* file, Node* node, std::string& error) const {
        if (node == nil_)
            return true;

        if (!WriteInOrder(file, node->left, error))
            return false;

        if (!WriteUint16(file, static_cast<uint16_t>(node->key.size()), error) ||
            !WriteBytes(file, node->key.data(), node->key.size(), error) ||
            !WriteUint64(file, node->value, error))
            return false;

        return WriteInOrder(file, node->right, error);
    }
};

static std::string Trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() &&
           (text[start] == ' ' || text[start] == '\t' || text[start] == '\r' || text[start] == '\n'))
        ++start;

    std::size_t end = text.size();
    while (end > start &&
           (text[end - 1] == ' ' || text[end - 1] == '\t' || text[end - 1] == '\r' || text[end - 1] == '\n'))
        --end;

    return text.substr(start, end - start);
}

static std::string NormalizeWord(const std::string& word) {
    std::string normalized = word;
    for (char& symbol : normalized)
        if (symbol >= 'A' && symbol <= 'Z')
            symbol = static_cast<char>(symbol - 'A' + 'a');
    return normalized;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    RedBlackTreeDictionary dictionary;
    std::string line;

    while (std::getline(std::cin, line)) {
        const std::string trimmed_line = Trim(line);
        if (trimmed_line.empty())
            continue;

        try {
            if (trimmed_line[0] == '+') {
                std::istringstream input(trimmed_line);
                char command = '\0';
                std::string word;
                uint64_t value = 0;
                input >> command >> word >> value;

                if (dictionary.Insert(NormalizeWord(word), value))
                    std::cout << "OK\n";
                else
                    std::cout << "Exist\n";
            } else if (trimmed_line[0] == '-') {
                std::istringstream input(trimmed_line);
                char command = '\0';
                std::string word;
                input >> command >> word;

                if (dictionary.Erase(NormalizeWord(word)))
                    std::cout << "OK\n";
                else
                    std::cout << "NoSuchWord\n";
            } else if (trimmed_line[0] == '!') {
                std::istringstream input(trimmed_line);
                char command = '\0';
                std::string action;
                std::string path;
                input >> command >> action;
                std::getline(input, path);
                path = Trim(path);

                std::string error;
                bool success = false;

                if (action == "Save")
                    success = dictionary.Save(path, error);
                else
                    success = dictionary.Load(path, error);

                if (success)
                    std::cout << "OK\n";
                else
                    std::cout << error << '\n';
            } else {
                uint64_t value = 0;
                if (dictionary.Find(NormalizeWord(trimmed_line), value))
                    std::cout << "OK: " << value << '\n';
                else
                    std::cout << "NoSuchWord\n";
            }
        } catch (const std::bad_alloc&) {
            std::cout << "ERROR: not enough memory\n";
        }
    }

    return 0;
}
