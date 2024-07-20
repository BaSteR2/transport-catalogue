#include "json_builder.h"

namespace json {  
    Builder::KeyItemContext Builder::Key(std::string key) {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        if (!std::holds_alternative<Dict>(stack_.back()->GetValue()) || !key_on_) {
            throw std::logic_error("Key() cannot call after non-Dict() or other Key()");
        }
        key_on_ = false;
        keys_.push_back(key);
        return BaseContext(*this);
    }

    Builder::BaseContext Builder::Value(Node::Value node) {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        else if (stack_.empty()) {
            root_ = node;
        }
        else if (std::holds_alternative<Array>(stack_.back()->GetValue())) {
            json::Array& array_stack = std::get<json::Array>(stack_.back()->GetValue());
            array_stack.emplace_back(move(node));
        }
        else {
            if (key_on_) {
                throw std::logic_error("Value() in Dict() cannot call without Key()");
            }
            json::Dict& dict_stack = std::get<json::Dict>(stack_.back()->GetValue());
            dict_stack[keys_.back()] = node;
            key_on_ = true;
            keys_.pop_back();
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        else {            
            if (!stack_.empty() && (std::holds_alternative<Dict>(stack_.back()->GetValue()) && key_on_)) {
                throw std::logic_error("Start new Dict in already existing Dict without Key() unacceptable");
            }
            key_on_ = true;

            Node* dict_stack = new Node(Dict());
            stack_.push_back(std::move(dict_stack));
            return BaseContext(*this);
        }
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        else {            
            if (!stack_.empty() && (std::holds_alternative<Dict>(stack_.back()->GetValue()) && key_on_)) {
                throw std::logic_error("Start new Array in already existing Dict without Key() unacceptable");
            }
            key_on_ = true;

            Node* array_stack = new Node(Array());
            stack_.push_back(std::move(array_stack));
            return BaseContext(*this);
        }
    }

    Builder::BaseContext Builder::EndDict() {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        else if (stack_.size() != 1) {
            
            if (!std::holds_alternative<Dict>(stack_.back()->GetValue())) {
                throw std::logic_error("Current container is non-dict");
            }
            
            json::Dict dict_last = move(std::get<json::Dict>(stack_.back()->GetValue()));
            delete stack_.back();
            stack_.pop_back();

            if (std::holds_alternative<Array>(stack_.back()->GetValue())) {
                json::Array& array_stack = std::get<json::Array>(stack_.back()->GetValue());
                array_stack.push_back(move(dict_last));
            }
            else {
                json::Dict& dict_stack = std::get<json::Dict>(stack_.back()->GetValue());
                dict_stack[keys_.back()] = move(dict_last);
                keys_.pop_back();
            }
        }
        else {
            if (!std::holds_alternative<Dict>(stack_.back()->GetValue())) {
                throw std::logic_error("Current container is non-dict");
            }
            root_ = *stack_.back();
            delete stack_.back();
            stack_.pop_back();
        }
        return *this;
    }

    Builder::BaseContext Builder::EndArray() {
        if (root_.has_value()) {
            throw std::logic_error("Json-node already build");
        }
        else if (stack_.size() != 1) {
            if (!std::holds_alternative<Array>(stack_.back()->GetValue())) {
                throw std::logic_error("Current container is non-array");
            }
            json::Array array_last = std::get<json::Array>(stack_.back()->GetValue());
            delete stack_.back();
            stack_.pop_back();
            if (std::holds_alternative<Array>(stack_.back()->GetValue())) {
                json::Array& array_stack = std::get<json::Array>(stack_.back()->GetValue());
                array_stack.push_back(move(array_last));
            }
            else {
                json::Dict& dict_stack = std::get<json::Dict>(stack_.back()->GetValue());
                dict_stack[keys_.back()] = move(array_last);
                keys_.pop_back();
            }
        }
        else {
            if (!std::holds_alternative<Array>(stack_.back()->GetValue())) {
                throw std::logic_error("Current container is non-array");
            }
            root_ = *stack_.back();
            delete stack_.back();
            stack_.pop_back();
        }
        return *this;
    }

    Node Builder::Build() {
        if (stack_.size() != 0 || !root_.has_value()) {
            throw std::logic_error("Json-node was not build");
        }
        return *root_;
    }
}