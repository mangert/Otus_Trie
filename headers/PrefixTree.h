#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <stack>
#include <memory>
#include <unordered_map>
#include <concepts>
#include <optional>


namespace PrefixTree {    
    
    //-------------Описание структруры узлов -------------------//        
    
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
            
            if (c < 'a' || c > 'z') {
                throw std::out_of_range("Invalid character");
            }
            return children[c - 'a'].get();
        }

        void setChild(char c, std::unique_ptr<ArrayNode> node) {
            
            if (c < 'a' || c > 'z') {
                throw std::out_of_range("Invalid character");
            }
            children[c - 'a'] = std::move(node);
        }
        //======== Обходы детей ============//
        // const версия
        template<typename F>
        void forEachChild(F&& func) const {
            for (size_t i = 0; i < children.size(); ++i) {
                if (children[i]) {
                    char c = 'a' + static_cast<char>(i);
                    std::forward<F>(func)(c, children[i].get());
                }
            }
        }
        // non-const версия (для модификации)
        template<typename F>
        void forEachChild(F&& func) {
            for (size_t i = 0; i < children.size(); ++i) {
                if (children[i]) {
                    char c = 'a' + static_cast<char>(i);
                    std::forward<F>(func)(c, children[i]);
                }
            }
        }
        // Версия для перемещения unique_ptr (для clear)
        template<typename F>
        void forEachChildUnique(F&& func) {
            for (auto& child : children) {
                if (child) {
                    func(std::move(child));
                }
            }
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

        void setChild(char c, std::unique_ptr<HashMapNode> node) {
            children[c] = std::move(node);
        }        

        //======== Обходы детей ============//
        // const версия
        template<typename F>
        void forEachChild(F&& func) const {
            for (const auto& [c, child_ptr] : children) {
                std::forward<F>(func)(c, child_ptr.get());
            }
        }
        // non-const версия (для модификации)
        template<typename F>
        void forEachChild(F&& func) {
            for (auto& [c, child_ptr] : children) {
                std::forward<F>(func)(c, child_ptr);
            }
        }
        // Версия для перемещения unique_ptr (для clear)
        template<typename F>
        void forEachChildUnique(F&& func) {
            for (auto& [c, child_ptr] : children) {
                if (child_ptr) {
                    func(std::move(child_ptr));
                }
            }
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
            if (c >= 'a' && c < 'a' + FAST_LETTERS) {
                return fast_children[c - 'a'].get();
            }
            auto it = slow_children.find(c);
            return it != slow_children.end() ? it->second.get() : nullptr;
        }

        void setChild(char c, std::unique_ptr<HybridNode> node) {
            if (c >= 'a' && c < 'a' + FAST_LETTERS) {
                fast_children[c - 'a'] = std::move(node);
            }
            else {
                slow_children[c] = std::move(node);
            }
        }
        
        //======== Обходы детей ============//
        // const версия
        template<typename F>
        void forEachChild(F&& func) const {
            // Сначала быстрые дети
            for (size_t i = 0; i < fast_children.size(); ++i) {
                if (fast_children[i]) {
                    char c = 'a' + static_cast<char>(i);
                    std::forward<F>(func)(c, fast_children[i].get());
                }
            }
            // Потом медленные
            for (const auto& [c, child_ptr] : slow_children) {
                std::forward<F>(func)(c, child_ptr.get());
            }
        }
        // non-const версия (для модификации)
        template<typename F>
        void forEachChild(F&& func) {
            for (size_t i = 0; i < fast_children.size(); ++i) {
                if (fast_children[i]) {
                    char c = 'a' + static_cast<char>(i);
                    std::forward<F>(func)(c, fast_children[i]);
                }
            }
            for (auto& [c, child_ptr] : slow_children) {
                std::forward<F>(func)(c, child_ptr);
            }
        }
        // Версия для перемещения unique_ptr (для clear)
        template<typename F>
        void forEachChildUnique(F&& func) {
            for (auto& child : fast_children) {
                if (child) {
                    func(std::move(child));
                }
            }
            for (auto& [c, child_ptr] : slow_children) {
                if (child_ptr) {
                    func(std::move(child_ptr));
                }
            }
        }
    };

    //-------------Основной класс Trie -------------------//    

    template<typename Key, typename Value, NodeType NodeT>
        requires std::convertible_to<Key, std::string_view>    
    class Trie {
    public:
        //=========== Конструкторы ===============//
        // Конструктор по умолчанию
        Trie() = default;

        // Запрещаем копирование
        Trie(const Trie&) = delete;
        Trie& operator=(const Trie&) = delete;

        // Перемещение
        Trie(Trie&&) noexcept = default;
        Trie& operator=(Trie&&) noexcept = default;

        // Деструктор - предупреждаем о возможной рекурсии
        ~Trie() {
            clear();
        }

        //=========== Основные операции ===============//
        
        // Вставка с perfect forwarding для ключа и значения
        template<typename K, typename V>
        void insert(K&& key, V&& value) {
            traverse_or_create(std::forward<K>(key), [&](NodeT* node) {
                node->value = std::forward<V>(value);
                node->has_value = true;
                });
        }

        //удваление
        template<typename K>
        bool remove(K&& key) {
            return traverse(std::forward<K>(key), [](const NodeT* node) -> bool {
                if (node && node->has_value) {
                    node->has_value = false;
                    ValueCleaner<Value>::clear(node->value);
                    return true;
                }
                return false;
                });
        }

        //=========== Операции поиска и доступа ===============//
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
        //=========== Другие методы ===============//
        //полная очистка дерева
        void clear() noexcept {
            if (!root_holder.root) return;

            std::stack<std::unique_ptr<NodeT>> stack;
            stack.push(std::move(root_holder.root));

            while (!stack.empty()) {
                auto node = std::move(stack.top());
                stack.pop();

                // Собираем всех детей в стек
                node->forEachChildUnique([&stack](std::unique_ptr<NodeT> child) {
                    if (child) {
                        stack.push(std::move(child));
                    }
                    });               
            }
        }                
        //клонирование дерева (вместо конструктора копирования, может кинуть исключение)
        Trie clone() const {
            Trie result;
            if (root_holder.root) {
                result.root_holder.root = cloneNode(root_holder.root.get());
            }
            return result;
        }
    
private:
        //=========== Удалители значения для узлов ===============//
        template<typename T>
        struct ValueCleaner {
            // Общий случай - вызываем деструктор через присваивание
            static void clear(T& value) {
                value = T{};  // Требует default constructible
            }
        };

        // Специализация для указателей
        template<typename U>
        struct ValueCleaner<U*> {
            static void clear(U*& ptr) {
                ptr = nullptr;  // Указатели зануляем
            }
        };

        // Специализация для арифметических типов
        template<typename T>
        struct ValueCleaner<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
            static void clear(T&) {
                // Ничего не делаем - число можно оставить
                // Оно всё равно занимает те же 4/8 байт
            }
        };

        // Специализация для типов с дешевым default constructor'ом
        template<typename T>
        struct ValueCleaner<T, std::enable_if_t<is_cheap_to_default_construct_v<T>>> {
            static void clear(T& value) {
                value = T{};  // Например, пустая строка
            }
        };

    private:
        //=========== Вспомогательные методы ===============//
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
        //клонирование узлов
        std::unique_ptr<NodeT> cloneNode(const NodeT* node, size_t depth = 0) {
            if (!node) return nullptr;

            if (depth > MAX_DEPTH) {
                throw std::runtime_error("Trie too deep for cloning");
            }

            auto new_node = std::make_unique<NodeT>();
            new_node->has_value = node->has_value;
            new_node->value = node->value;  // Копируем значение

            // Используем forEachChild для копирования детей
            node->forEachChild([&new_node](char c, const NodeT* child) {
                auto cloned_child = cloneNode(child, depth + 1);
                if (cloned_child) {
                    new_node->setChild(c, std::move(cloned_child));
                }
                });

            return new_node;
        }

    private:
        struct RootHolder {
            std::unique_ptr<NodeT> root;
            RootHolder() : root(std::make_unique<NodeT>()) {}
        };

        RootHolder root_holder;

        //ограничитель глубины рекурсии (для клонирования)
        static constexpr size_t MAX_DEPTH = 10000;
    };

    //-------------Aлиасы для разных вариантов типа узла с ключом типа string---//        
    template<typename V>
    using ArrayTrie = Trie<std::string, V, ArrayNode<V>>;

    template<typename V>
    using HashMapTrie = Trie<std::string, V, HashMapNode<V>>;

    template<typename V>
    using HybridTrie = Trie<std::string, V, HybridNode<V>>;

} // namespace PrefixTree