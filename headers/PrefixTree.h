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

    // Вариант 1: Классический массив (только строчные латинские буквы)
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

    // Вариант 2: Хеш-таблица
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
    public:
        // Вставка с perfect forwarding для ключа и значения
        template<typename K, typename V>
        void insert(K&& key, V&& value) {
            traverse_or_create(std::forward<K>(key), [&](NodeT* node) {
                node->value = std::forward<V>(value);
                node->has_value = true;
                });
        }

        // Поиск с perfect forwarding для ключа
        template<typename K>
        std::optional<Value> search(K&& key) const {
            return traverse(std::forward<K>(key), [](const NodeT* node) -> std::optional<Value> {
                if (node && node->has_value) {
                    return node->value;  // Возвращаем копию значения
                }
                return std::nullopt;
                });
        }

        // Поиск с возможностью получить значение по ссылке (для больших объектов)
        template<typename K>
        const Value* find(K&& key) const {
            return traverse(std::forward<K>(key), [](const NodeT* node) -> const Value* {
                return (node && node->has_value) ? &node->value : nullptr;
                });
        }

        // Поиск с возможностью получить значение по ссылке (для больших объектов)
        template<typename K>
        Value* find(K&& key) {
            return traverse(std::forward<K>(key), [](const NodeT* node) -> Value* {
                return (node && node->has_value) ? &node->value : nullptr;
                });
        }

        // Проверка наличия ключа
        template<typename K>
        bool contains(K&& key) const {
            return traverse(std::forward<K>(key), [](const NodeT* node) {
                return node && node->has_value;
                });
        }

        // Проверка префикса
        template<typename K>
        bool startsWith(K&& prefix) const {
            return traverse(std::forward<K>(prefix), [](const NodeT* node) {
                return node != nullptr;  // Просто проверяем, что путь существует
                });
        }

    private:
        // const traverse - только чтение
        template<typename K, typename F>
        decltype(auto) traverse(K&& key, F&& func) const {
            NodeT* current = root_holder.root.get();
            for (auto c : std::string_view(std::forward<K>(key))) {  // Преобразуем в последовательность char
                current = current->getChild(c);
                if (!current) return std::forward<F>(func)(nullptr);
            }
            return std::forward<F>(func)(current);
        }

        // non-const traverse - для модификации
        template<typename K, typename F>
        decltype(auto) traverse(K&& key, F&& func) {
            NodeT* current = root_holder.root.get();
            for (auto c : std::string_view(std::forward<K>(key))) {
                current = current->getChild(c);
                if (!current) return std::forward<F>(func)(nullptr);
            }
            return std::forward<F>(func)(current);
        }

        // traverse_or_create - создает путь при необходимости
        template<typename K, typename F>
        decltype(auto) traverse_or_create(K&& key, F&& func) {
            NodeT* current = root_holder.root.get();
            for (auto c : std::string_view(std::forward<K>(key))) {
                NodeT* next = current->getChild(c);
                if (!next) {
                    auto new_node = std::make_unique<NodeT>();
                    next = new_node.get();
                    current->setChild(c, std::move(new_node));
                }
                current = next;
            }
            return std::forward<F>(func)(current);
        }
    private:
        struct RootHolder {
            std::unique_ptr<NodeT> root;
            RootHolder() : root(std::make_unique<NodeT>()) {}
        };

        RootHolder root_holder;

    };

    //Aлиасы для разных вариантов типа узла с ключом типа string
    template<typename V>
    using ArrayTrie = Trie<std::string, V, ArrayNode<V>>;

    template<typename V>
    using HashMapTrie = Trie<std::string, V, HashMapNode<V>>;

    template<typename V>
    using HybridTrie = Trie<std::string, V, HybridNode<V>>;

} // namespace PrefixTree