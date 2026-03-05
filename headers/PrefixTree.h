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
                    std::forward<F>(func)(c, children[i].get());
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
                std::forward<F>(func)(c, child_ptr.get());
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

    // Вариант 3: Гибрид
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
                    std::forward<F>(func)(c, fast_children[i].get());
                }
            }
            for (auto& [c, child_ptr] : slow_children) {
                std::forward<F>(func)(c, child_ptr.get());
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
    //-------------Дефолтный очиститель значения -------------------//    
    struct NoopCleaner {
        template<typename T>
        void operator()(T&) const noexcept {}
    };

    // Концепт для проверки типа очистителя
    template<typename T>
    concept DefaultConstructible = std::is_default_constructible_v<T>;

    template<typename T, typename Value>
    concept InvocableWithValue = requires(T & cleaner, Value & val) {
        { cleaner(val) } -> std::same_as<void>;
    };

    // Итоговый концепт для Cleaner
    template<typename C, typename Value>
    concept CleanerType = DefaultConstructible<C>
        && InvocableWithValue<C, Value>;
    
    //-------------Основной класс Trie -------------------//    
    /**
    * @tparam Value Type of values stored in trie.
    *
    * Note: Value type must be default constructible because each node
    * contains a Value member even if it doesn't store a value.
    *
    * To store non-default-constructible types, use std::optional<Value>.
    *
    * Example:
    * Trie<std::optional<MyType>> trie;
    */
    template<typename Key, typename Value, NodeType NodeT, CleanerType<Value> Cleaner = NoopCleaner>
        requires std::convertible_to<Key, std::string_view>
    && std::is_default_constructible_v<Value>
    class Trie {                
    public:
        //=========== Конструкторы ===============//
        // Конструктор по умолчанию
        Trie(Cleaner c = Cleaner{}) : cleaner(std::move(c)) {}        

        // Запрещаем копирование
        Trie(const Trie&) = delete;
        Trie& operator=(const Trie&) = delete;

        // Конструктор перемещения
        Trie(Trie&& other) noexcept
            : root_holder(std::move(other.root_holder))
            , element_count(std::exchange(other.element_count, 0)) 
            , cleaner(std::move(other.cleaner)) {
            
            other.root_holder.root = std::make_unique<NodeT>();
        }

        // Оператор перемещения
        Trie& operator=(Trie&& other) noexcept {
            if (this != &other) {
                // Удаляем текущее содержимое
                clear();

                // Перемещаем данные
                root_holder = std::move(other.root_holder);
                element_count = std::exchange(other.element_count, 0);
                cleaner = std::move(other.cleaner);
                // Восстанавливаем other
                other.root_holder.root = std::make_unique<NodeT>();
            }
            return *this;
        }

        // Деструктор
        ~Trie() {
            clear();
        }

        //=========== Основные операции ===============//        
        // Вставка элемента
        template<typename K, typename V>
        void insert(K&& key, V&& value) {
            traverse_or_create(std::forward<K>(key), [&](NodeT* node) {
                if (node->has_value)
                    cleaner(node->value);
                else ++element_count;
                node->value = std::forward<V>(value);
                node->has_value = true;
                });
        }

        //удаление ключа                 
        template<typename K>
        bool remove(K&& key) {
            return traverse(std::forward<K>(key), [&](NodeT* node) -> bool {
                if (node && node->has_value) {
                    node->has_value = false;
                    --element_count;                    
                    cleaner(node->value);

                    return true;
                }
                return false;
                });
        }

        //=========== Операции поиска и доступа ===============//       
        // Проверка наличия ключа
        template<typename K>
        bool contains(K&& key) const {
            return traverse(std::forward<K>(key), [](const NodeT* node) {
                return node && node->has_value;
                });
        }
        // Поиск с возвратом по указателю (константная версия)
        template<typename K>
        const Value* find(K&& key) const {            
            return traverse(std::forward<K>(key), [](const NodeT* node) -> const Value* {                
                return (node && node->has_value) ? &node->value : nullptr;
                });
        }
        // Поиск с возвратом по указателю
        template<typename K>
        Value* find(K&& key) {
            return traverse(std::forward<K>(key), [](NodeT* node) -> Value* {
                return (node && node->has_value) ? &node->value : nullptr;
                });
        }       
        
        // Поиск с возвратом значения (только для копируемых типов)
        /**
         * @note The search(Key) method requires Value to be copy constructible.
         * For non-copyable types (like std::unique_ptr), clone is not available.
         */
        template<typename K>
            requires std::is_copy_constructible_v<Value>
        std::optional<Value> search(K&& key) const {
            return traverse(std::forward<K>(key), [](const NodeT* node) -> std::optional<Value> {
                if (node && node->has_value) {
                    return node->value;  // Возвращаем копию значения
                }
                return std::nullopt;
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
            
            if (!root_holder.root) {
                root_holder.root = std::make_unique<NodeT>();  // на всякий случай
                return;
            }            

            std::stack<std::unique_ptr<NodeT>> stack;
            stack.push(std::move(root_holder.root));

            while (!stack.empty()) {
                auto node = std::move(stack.top());
                if (node->has_value)
                    cleaner(node->value);
                stack.pop();

                // Собираем всех детей в стек
                node->forEachChildUnique([&stack](std::unique_ptr<NodeT> child) {
                    if (child) {
                        stack.push(std::move(child));
                    }
                    });               
            }
            // 2. Создаем новый пустой корень
            root_holder.root = std::make_unique<NodeT>();
        }
        //Очистка "мертвых" узлов
        void cleanup() {            
            cleanup_impl(root_holder.root.get());
        }
        
        //клонирование дерева
        /**
         * @note The clone() method requires Value to be copy constructible.
         * For non-copyable types (like std::unique_ptr), clone is not available.
         */
        Trie clone() const 
            requires std::is_copy_constructible_v<Value> {
            
            Trie result;
            if (root_holder.root) {
                result.root_holder.root = cloneNode(root_holder.root.get());
            }
            result.element_count = element_count;
            result.cleaner = cleaner;
            return result;
        }
        //=========== Метрики ====================//
        bool empty() const {
            return element_count == 0;
        }

        size_t size() const {
            return element_count;
        }

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
        //рекурсивная функция для очистки мертвых узлов
        bool cleanup_impl(NodeT* node, size_t depth = 0) {
            if (!node) return true;

            if (depth > MAX_DEPTH) {
                throw std::runtime_error("Trie too deep for clean up");
            }

            //сначала проходим по всем узлам
            node->forEachChild([&](char c, NodeT* child) {
                if (cleanup_impl(child, depth + 1)) {
                    node->setChild(c, nullptr);
                }
                });
            //считаем оставшихся детей
            size_t count_children = 0;
            node->forEachChild([&](char c, NodeT* child) {
                if (child) ++count_children;
                });

            return !node->has_value && !count_children;
        }
        
        //клонирование узлов
        std::unique_ptr<NodeT> cloneNode(const NodeT* node, size_t depth = 0) const {
            if (!node) return nullptr;

            if (depth > MAX_DEPTH) {
                throw std::runtime_error("Trie too deep for cloning");
            }

            auto new_node = std::make_unique<NodeT>();
            new_node->has_value = node->has_value;

            // Копируем значение, если оно есть
            if (node->has_value) {
                new_node->value = node->value;  // Копируем значение
            }

            // Копируем детей - теперь func принимает (char, const NodeT*)
            node->forEachChild([&](char c, const NodeT* child) {
                auto cloned_child = cloneNode(child, depth + 1);
                if (cloned_child) {
                    new_node->setChild(c, std::move(cloned_child));  // вставляем по букве
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

        size_t element_count = 0; //счетчик ключей
        
        [[no_unique_address]] Cleaner cleaner;  // очиститель значения - пустой для NoopCleaner

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