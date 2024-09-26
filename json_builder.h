#pragma once

#include "json.h"



namespace json {

	class Builder {
	private:

		class DictItemContext;
		class ArrayItemContext;
		class DictKeyContext;

		Node root_;
		std::vector<Node*> nodes_stack_;
		Node* value_ = nullptr;

	public:

		Builder();

		DictKeyContext Key(std::string key);
		Builder& Value(Node::Value value);

		DictItemContext StartDict();
		Builder& EndDict();

		ArrayItemContext StartArray();
		Builder& EndArray();

		Node Build();
		Node GetNode(Node::Value value);

	};


	class Builder::DictItemContext {
	public:
		DictItemContext(Builder& builder);

		DictKeyContext Key(std::string key);
		Builder& EndDict();

	private:
		Builder& builder_;
	};
	class Builder::ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder);

		ArrayItemContext Value(Node::Value value);

		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
	private:
		Builder& builder_;
	};
	class Builder::DictKeyContext {
	public:
		DictKeyContext(Builder& builder);

		DictItemContext Value(Node::Value value);

		ArrayItemContext StartArray();
		DictItemContext StartDict();

	private:
		Builder& builder_;
	};

}  // namespace json
