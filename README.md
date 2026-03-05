# Префиксное дерево (Trie)

Учебный проект по реализации ассоциативного массива на базе префиксного дерева (Trie).

## Возможности

- Вставка ключ-значение (`insert`)
- Поиск по ключу (`find`, `search`)
- Проверка наличия префикса (`startsWith`)
- Удаление с кастомным очистителем (`remove`)
- Клонирование (`clone`)
- Очистка (`clear`)
- Семантика перемещения

## Классы

### `Trie<Key, Value, NodeT, Cleaner>`

Основной класс.

**Параметры шаблона:**
- `Key` — тип ключа (должен конвертироваться в `std::string_view`)
- `Value` — тип значения (должен быть default constructible)
- `NodeT` — тип узла (одна из реализаций ниже)
- `Cleaner` — функтор для очистки значений (по умолчанию `NoopCleaner`)

### Типы узлов

| Класс узла | Описание |
|------------|----------|
| `ArrayNode<Value>` | Фиксированный массив на 26 букв (a-z). Максимальная скорость. |
| `HashMapNode<Value>` | `std::unordered_map` для произвольных символов. |
| `HybridNode<Value>` | Комбинация: массив для a-h + хеш-таблица для остальных. |

### Aлиасы

```cpp
using ArrayTrie<V>    = Trie<std::string, V, ArrayNode<V>>;
using HashMapTrie<V>  = Trie<std::string, V, HashMapNode<V>>;
using HybridTrie<V>   = Trie<std::string, V, HybridNode<V>>;
```

## Пример использования

```cpp
// Создание дерева с int-значениями
ArrayTrie<int> trie;

// Вставка
trie.insert("cat", 5);
trie.insert("dog", 7);

// Поиск
if (auto* val = trie.find("cat")) {
    std::cout << *val;  // 5
}

// Проверка префикса
bool hasCat = trie.startsWith("ca");  // true

// Удаление
trie.remove("dog");
```

## Типы значений

- ✅ Копируемые типы (`int`, `std::string`, ...) — доступны все методы
- ✅ Некопируемые типы (`std::unique_ptr`, ...) — доступен `find` (возвращает указатель)
- ❌ Типы без конструктора по умолчанию — требуется `std::optional<Value>`

## Кастомные очистители

```cpp
struct Logger {
    void operator()(std::string& val) {
        std::cout << "Cleaning: " << val << '\n';
    }
};

Trie<std::string, std::string, ArrayNode, Logger> trie;
```

## Сборка

Проект использует CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./Trie
```

## Тестирование

Запуск исполняемого файла выполняет все тесты и выводит результаты в консоль.

## Укрупненная структура проекта

```
├── headers/
│   ├── PrefixTree.h      // Основная реализация Trie
│   └── PrefixTest.h       // Тестовый фреймворк
├── src/
│   └── main.cpp          // Точка входа
├── report/
│   └── output.txt         // Результаты тестов
└── CMakeLists.txt
```
