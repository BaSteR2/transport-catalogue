#pragma once
#include "json.h"

#include <vector>
#include <string>
#include <iostream>
#include <optional>

namespace json {

    class Builder {

    private:
        class BaseContext;
        class KeyItemContext;
        class DictItemContext;
        class ArrayItemContext;
    public:

        KeyItemContext Key(std::string key);
        BaseContext Value(Node::Value node);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

        BaseContext EndDict();
        BaseContext EndArray();
        Node Build();

    private:
        class BaseContext {
        public:
            BaseContext(Builder& builder) : builder_(builder) {}
            
            KeyItemContext Key(std::string key) {
                return builder_.Key(key);
            }
            BaseContext Value(Node::Value node) {
                return builder_.Value(node);
            }
            DictItemContext StartDict() {
                return builder_.StartDict();
            }
            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }
            BaseContext EndDict() {
                return builder_.EndDict();
            }
            BaseContext EndArray() {
                return builder_.EndArray();
            }
            Node Build() {
                return builder_.Build();
            }          
        protected:
            Builder& builder_;
        };

        class KeyItemContext 
            : public  BaseContext {

        public:
            KeyItemContext(BaseContext base) : BaseContext(base) {};
            DictItemContext Value(Node::Value node) {
                return builder_.Value(node);
            }
            KeyItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
            Node Build() = delete;
        };

        class DictItemContext
            : public  BaseContext { 

        public:
            DictItemContext(BaseContext base) : BaseContext(base) {};
            BaseContext Value(Node::Value node) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            BaseContext EndArray() = delete;
            Node Build() = delete;
        };
       
        class ArrayItemContext 
            : public  BaseContext { 

        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {};
            ArrayItemContext Value(Node::Value node) {
                return builder_.Value(node);
            }
            KeyItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            Node Build() = delete;
        };

    private:
        std::optional<Node> root_;
        bool key_on_ = true;
        std::vector<std::string> keys_;
        std::vector<Node*> stack_;
    };
}