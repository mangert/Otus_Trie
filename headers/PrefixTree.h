#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <concepts>
#include <optional>

namespace PrefixTree {

    // Базовый маркер
    struct NodeBase {};

    // Концепт для проверки типа узла
    template<typename T>
    concept NodeType = std::derived_from<T, NodeBase>;

    // Вариант 1: Классический массив
    template<typename V>
    struct ArrayNode : NodeBase {
        static constexpr size_t ALPHABET_SIZE = 26;
        std::array<std::unique_ptr<ArrayNode>, ALPHABET_SIZE> children;
        V value;
        bool has_value = false;

        ArrayNode* getChild(char c) const {
            return children[c - 'a'].get();
        }

        void setChild(char c, std::unique_ptr<ArrayNode> node) {
            children[c - 'a'] = std::move(node);
        }
    };

    // Вариант 2: Хеш-таблица (для больших алфавитов)
    template<typename V>
    struct HashMapNode : NodeBase {
        std::unordered_map<char, std::unique_ptr<HashMapNode>> children;
        V value;
        bool has_value = false;

        HashMapNode* getChild(char c) const {
            auto it = children.find(c);
            return it != children.end() ? it->second.get() : nullptr;
        }
    };

    // Вариант 3: Гибрид (умный по умолчанию)
    template<typename V>
    struct HybridNode : NodeBase {
        static constexpr size_t FAST_LETTERS = 8; // a-h
        std::array<std::unique_ptr<HybridNode>, FAST_LETTERS> fast_children;
        std::unordered_map<char, std::unique_ptr<HybridNode>> slow_children;
        V value;
        bool has_value = false;

        HybridNode* getChild(char c) const {
            if (c < 'a' + FAST_LETTERS) {
                return fast_children[c - 'a'].get();
            }
            auto it = slow_children.find(c);
            return it != slow_children.end() ? it->second.get() : nullptr;
        }

        void setChild(char c, std::unique_ptr<HybridNode> node) {
            if (c < 'a' + FAST_LETTERS) {
                fast_children[c - 'a'] = std::move(node);
            }
            else {
                slow_children[c] = std::move(node);
            }
        }
    };

    // Основной класс Trie
    template<typename Key, typename Value, NodeType NodeT>
        requires std::convertible_to<Key, std::string_view>
    class Trie {
    private:
        struct RootHolder {
            std::unique_ptr<NodeT> root;
            RootHolder() : root(std::make_unique<NodeT>()) {}
        };

        RootHolder root_holder;

        // Вспомогательная функция для обхода
        template<typename F>
        bool traverse(const Key& key, F&& func) const {
            NodeT* current = root_holder.root.get();
            for (char c : key) {
                current = current->getChild(c);
                if (!current) return false;
            }
            return std::forward<F>(func)(current);
        }

    public:
        Trie() = default;

        void insert(const Key& key, const Value& val) {
            NodeT* current = root_holder.root.get();
            for (char c : key) {
                NodeT* next = current->getChild(c);
                if (!next) {
                    auto new_node = std::make_unique<NodeT>();
                    next = new_node.get();
                    current->setChild(c, std::move(new_node));
                }
                current = next;
            }
            current->has_value = true;
            current->value = val;
        }

        std::optional<Value> search(const Key& key) const {
            std::optional<Value> result;
            traverse(key, [&](NodeT* node) {
                if (node->has_value) {
                    result = node->value;
                }
                return true;
                });
            return result;
        }

        bool startsWith(const Key& prefix) const {
            return traverse(prefix, [](NodeT*) { return true; });
        }

        // Статистика для отчета
        struct Stats {
            size_t node_count;
            size_t total_children_capacity;
            size_t used_children;
            double memory_estimate_mb;
        };

        Stats getStats() const {
            Stats stats{ 0, 0, 0, 0.0 };
            traverseAll(root_holder.root.get(), [&](NodeT* node) {
                stats.node_count++;
                // Подсчет детей зависит от типа узла
                // Можно добавить виртуальные функции или type traits
                });
            return stats;
        }
    };

    // Удобные алиасы для пользователей
    template<typename V>
    using ArrayTrie = Trie<std::string, V, ArrayNode<V>>;

    template<typename V>
    using HashMapTrie = Trie<std::string, V, HashMapNode<V>>;

    template<typename V>
    using HybridTrie = Trie<std::string, V, HybridNode<V>>;

} // namespace PrefixTree