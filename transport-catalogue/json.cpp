#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        std::string LoadLiteral(std::istream& input) {
            std::string s;
            while (std::isalpha(input.peek())) {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node LoadNumber(istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input]() {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            auto read_digits = [&input, &read_char]() {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }

            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;

            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (input.peek() == 'e' || input.peek() == 'E') {
                read_char();
                if (input.peek() == '+' || input.peek() == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    return Node(stoi(parsed_num));
                }
                else {
                    return Node(stod(parsed_num));
                }
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }


        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(move(s));
        }


        Node LoadBool(std::istream& input) {
            const auto s = LoadLiteral(input);
            if (s == "true"sv) {
                return Node{ true };
            }
            else if (s == "false"sv) {
                return Node{ false };
            }
            else {
                throw ParsingError("Failed to parse '"s + s + "' as bool"s);
            }
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadArray(istream& input) {
            Array result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error"s);
            }
            return Node(std::move(result));
        }

        Node LoadDict(istream& input) {
            Dict dict;

            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                }
                else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }
            return Node(std::move(dict));
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Unexpected EOF"s);
            }
            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
                // подсказкой компилятору и человеку, что здесь программист явно задумывал
                // разрешить переход к инструкции следующей ветки case, а не случайно забыл
                // написать break, return или throw.
                // В данном случае, встретив t или f, переходим к попытке парсинга
                // литералов true либо false
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node() : value_(nullptr) {}
    Node::Node(Array array) : value_(move(array)) {}
    Node::Node(Dict map) : value_(move(map)) {}
    Node::Node(int value) : value_(value) {}
    Node::Node(double value) : value_(value) {}
    Node::Node(std::string value) : value_(move(value)) {}
    Node::Node(bool value) : value_(value) {}
    Node::Node(std::nullptr_t value) : value_(value) {}

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<int>(value_) || holds_alternative<double>(value_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(value_);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(value_);
        }
        throw logic_error("Value is not an int");
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(value_);
        }
        throw logic_error("Value is not a bool");
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(AsInt());
        }
        else if (IsPureDouble()) {
            return get<double>(value_);
        }
        throw logic_error("Value is not a double");
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return get<string>(value_);
        }
        throw logic_error("Value is not a string");
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(value_);
        }
        throw logic_error("Value is not an array");
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(value_);
        }
        throw logic_error("Value is not a map");
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool Node::operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, std::ostream& output);

    void PrintValue(std::nullptr_t, std::ostream& output) {
        output << "null";
    }

    void PrintValue(bool value, std::ostream& output) {
        output << (value ? "true" : "false");
    }

    void PrintValue(const std::string& value, std::ostream& output) {
        output << '"';
        for (char c : value) {
            switch (c) {
            case '\n': output << "\\n"; break;
            case '\t': output << "\\t"; break;
            case '\r': output << "\\r"; break;
            case '\\': output << "\\\\"; break;
            case '"': output << "\\\""; break;
            default: output << c; break;
            }
        }
        output << '"';
    }


    void PrintValue(double value, std::ostream& output) {
        output << value;
    }

    void PrintValue(int value, std::ostream& output) {
        output << value;
    }

    void PrintValue(const Array& value, std::ostream& output) {
        output << '[';
        bool first = true;
        for (const auto& item : value) {
            if (!first) {
                output << ", ";
            }
            first = false;
            PrintNode(item, output);
        }
        output << ']';
    }

    void PrintValue(const Dict& value, std::ostream& output) {
        output << '{';
        bool first = true;
        for (const auto& [key, node] : value) {
            if (!first) {
                output << ", ";
            }
            first = false;
            PrintValue(key, output);
            output << ": ";
            PrintNode(node, output);
        }
        output << '}';
    }

    void PrintNode(const Node& node, std::ostream& output) {
        std::visit([&output](const auto& value) {
            PrintValue(value, output);
            },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

}  // namespace json