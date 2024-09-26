#include "json_builder.h"
#include <vector>
#include <stdexcept>

namespace json {

	using namespace std::literals;

	Builder::Builder() {
		nodes_stack_.emplace_back(&root_);
	}

	Builder::DictKeyContext Builder::Key(std::string key) {
		auto* top_node = nodes_stack_.back();

		if (top_node->IsDict() and value_ == nullptr) {
			auto& dict = const_cast<Dict&>(top_node->AsDict());
			value_ = &dict.insert({ std::move(key), Node() }).first->second;
		}
		else {
			throw std::logic_error("Key can't be called now."s);
		}

		return *this;
	}

	Builder& Builder::Value(Node::Value value) {
		if (nodes_stack_.empty()) {
			throw std::logic_error("No container to place the value into."s);
		}

		auto* top_node = nodes_stack_.back();

		if (top_node->IsDict() and value_ != nullptr) {
			*value_ = GetNode(std::move(value));
			value_ = nullptr;
		}
		else if (top_node->IsArray()) {
			auto& array = const_cast<Array&>(top_node->AsArray());
			array.emplace_back(GetNode(std::move(value)));
		}
		else if (root_.IsNull()) {
			root_ = GetNode(std::move(value));
		}
		else {
			throw std::logic_error("Value can't be called now."s);
		}
		return *this;
	}

	Node Builder::Build() {
		if (root_.IsNull() or nodes_stack_.size() > 1) {
			throw std::logic_error("JSON document wasn't fully built."s);
		}

		return root_;
	}

	Node Builder::GetNode(Node::Value value) {
		if (std::holds_alternative<int>(value)) return Node(std::get<int>(value));
		if (std::holds_alternative<double>(value)) return Node(std::get<double>(value));
		if (std::holds_alternative<std::string>(value)) return Node(std::get<std::string>(value));
		if (std::holds_alternative<std::nullptr_t>(value)) return Node(std::get<std::nullptr_t>(value));
		if (std::holds_alternative<bool>(value)) return Node(std::get<bool>(value));
		if (std::holds_alternative<Dict>(value)) return Node(std::get<Dict>(value));
		if (std::holds_alternative<Array>(value)) return Node(std::get<Array>(value));
		return {};
	}

	Builder::DictItemContext Builder::StartDict() {
		auto* top_node = nodes_stack_.back();

		if (top_node->IsArray()) {
			auto& array = const_cast<Array&>(top_node->AsArray());
			array.emplace_back(Node(Dict{}));
			nodes_stack_.push_back(&array.back());
		}
		else if (top_node->IsDict() and value_ != nullptr) {
			*value_ = Node(Dict{});
			nodes_stack_.push_back(value_);
			value_ = nullptr;
		}
		else if (top_node->IsNull()) {
			*top_node = Node(Dict{});
		}
		else {
			throw std::logic_error("StartDict can't be called now."s);
		}

		return *this;
	}

	Builder& Builder::EndDict() {
		auto* top_node = nodes_stack_.back();

		if (!top_node->IsDict()) {
			throw std::logic_error("EndDict can't be called now."s);
		}

		nodes_stack_.pop_back();
		return *this;
	}

	Builder::ArrayItemContext Builder::StartArray() {
		auto* top_node = nodes_stack_.back();

		if (top_node->IsArray()) {
			auto& array = const_cast<Array&>(top_node->AsArray());
			array.emplace_back(Node(Array{}));
			nodes_stack_.push_back(&array.back());
		}
		else if (top_node->IsDict() and value_ != nullptr) {
			*value_ = Node(Array{});
			nodes_stack_.push_back(value_);
			value_ = nullptr;
		}
		else if (top_node->IsNull()) {
			*top_node = Node(Array{});
		}
		else {
			throw std::logic_error("StartArray can't be called now."s);
		}

		return *this;
	}

	Builder& Builder::EndArray() {
		auto* top_node = nodes_stack_.back();

		if (!top_node->IsArray()) {
			throw std::logic_error("EndArray can't be called now."s);
		}

		nodes_stack_.pop_back();
		return *this;
	}

	//----------------------------------------ItemDictContext---------------------------------

	Builder::DictItemContext::DictItemContext(Builder& builder)
		: builder_(builder) {
	}

	Builder::DictKeyContext Builder::DictItemContext::Key(std::string key) {
		return builder_.Key(key);
	}

	Builder& Builder::DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	//----------------------------------------ItemArrayContext---------------------------------

	Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
		: builder_(builder) {
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
		return ArrayItemContext(builder_.Value(value));
	}

	Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	//----------------------------------------KeyDictContext---------------------------------

	Builder::DictKeyContext::DictKeyContext(Builder& builder)
		: builder_(builder) {
	}

	Builder::DictItemContext Builder::DictKeyContext::Value(Node::Value value) {
		return DictItemContext(builder_.Value(value));
	}

	Builder::ArrayItemContext Builder::DictKeyContext::StartArray() {
		return builder_.StartArray();
	}

	Builder::DictItemContext Builder::DictKeyContext::StartDict() {
		return builder_.StartDict();
	}

}
