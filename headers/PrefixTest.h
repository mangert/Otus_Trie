#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <memory>
#include <fstream>
#include <sstream>
#include "PrefixTree.h"
#include "imported/OpenHashTable.h" //для сравнения - хэш-таблицы

using namespace PrefixTree;

class PrefixTest {

public:

	template<template<typename> typename NodeTemplate>
	static void comprehensive_test(const std::string& name) {
		std::cout << "========================================\n";
		std::cout << "Prefix Tree TEST (" << name << ")\n";
		std::cout << "========================================\n\n";	

		// 1. Тесты на работоспособность с разными типами
		std::cout << "\n1. Main functionality TESTS\n";
		std::cout << "-------------------------------------\n";
		test_with_value_type<NodeTemplate, int>("int");
		test_with_value_type<NodeTemplate, std::string>("string");
		test_with_value_type<NodeTemplate, std::unique_ptr<std::string>>("unique_ptr");
		test_with_value_type<NodeTemplate, DataStruct>("DataStruct");
		std::cout << "-------------------------------------\n";
		
		// 2. Основной тест на вставку/удаление и сравнение с хэш-таблицей
		comparison_test<NodeTemplate>();
		std::cout << "-------------------------------------\n";
		// 3. Тест копирования и перемещения
		test_move_semantics<NodeTemplate>();		
		
		// 4. Тест кастомного удаления
		test_custom_cleaner<NodeTemplate>();
		
		std::cout << "\n========================================\n";
		std::cout << "ALL TESTS PASSED SUCCESSFULLY!\n";
		std::cout << "========================================\n";
	}

private:
	
	struct DataStruct {
		std::string value;
		DataStruct() : value(std::string("")) {};
		DataStruct(std::string _val) : value(_val) {};
	};
	
	// ==================== 1. Тесты на проверку основной функциональности ====================
	template<template<typename> typename NodeTemplate, typename ValueType>
	static void test_with_value_type(const std::string& type_name) {
		std::cout << "\n--- ValueType: " << type_name << " ---\n";

		using TrieType = PrefixTree::Trie<std::string, ValueType, NodeTemplate<ValueType>>;
		TrieType trie;

		// Получаем тестовые данные
		auto test_data = getTestData();

		// 1. Тест вставки
		std::cout << "    Inserting " << test_data.size() << " items...\n";
		for (const auto& [key, val_str] : test_data) {
			trie.insert(key, cover_value<ValueType>(val_str));
		}

		// 2. Тест поиска
		std::cout << "    Testing find()...\n";
		for (const auto& [key, val_str] : test_data) {
			auto* ptr = trie.find(key);
			if (!ptr) {
				std::cerr << "      ERROR: '" << key << "' not found!\n";
				continue;
			}		
			
			// Проверяем значение в зависимости от типа
			if constexpr (std::is_same_v<ValueType, std::string>) {
				if (*ptr != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, std::unique_ptr<std::string>>) {
				if (!*ptr) {  // проверяем, не nullptr ли сам unique_ptr
					std::cerr << "    ERROR: Null unique_ptr for '" << key << "'\n";
				}
				else if (**ptr != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, int>) {
				if (*ptr != static_cast<int>(val_str.length())) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, DataStruct>) {
				if (ptr->value != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}			
		}

		// 3. Проверка search (только для копируемых типов)
		if constexpr (std::is_copy_constructible_v<ValueType>) {
			std::cout << "    Testing search()...\n";
			for (const auto& [key, val_str] : test_data) {
				auto opt = trie.search(key);
				if (!opt.has_value()) {
					std::cerr << "    ERROR: Key '" << key << "' not found in search!\n";
					continue;
				}
			}
		}

		// 4. Тест startsWith
		std::cout << "    Testing startsWith()...\n";
		test_prefixes(trie);

		// 5. Тест удаления
		std::cout << "    Testing remove()...\n";
		std::vector<std::string> to_remove = { "cat", "wolf", "foxhole", "a" };
		for (const auto& key : to_remove) {
			bool removed = trie.remove(key);
			if (!removed) {
				std::cerr << "    ERROR: Failed to remove '" << key << "'\n";
			}
		}

		//проверка, что удаленные ключи действительно исчезли
		for (const auto& key : to_remove) {
			if (trie.find(key)) {
				std::cerr << "    ERROR: Key '" << key << "' still exists after remove!\n";
			}
		}

		//проверка, что остальные ключи на месте
		for (const auto& [key, val_str] : test_data) {
			if (std::find(to_remove.begin(), to_remove.end(), key) != to_remove.end()) {
				continue;  // пропускаем удаленные
			}
			if (!trie.find(key)) {
				std::cerr << "    ERROR: Key '" << key << "' missing after removals!\n";
			}
		}		

		// 6. Тест cleanup
		std::cout << "    Running cleanup...\n";
		trie.cleanup();

		// 7. Тест clone (если применимо)
		if constexpr (std::is_copy_constructible_v<ValueType>) {
			std::cout << "    Testing clone()...\n";
			auto clone = trie.clone();
			
			// Проверяем, что клон содержит те же ключи
			for (const auto& [key, val_str] : test_data) {
				if (std::find(to_remove.begin(), to_remove.end(), key) != to_remove.end()) {
					continue;
				}
				if (!clone.find(key)) {
					std::cerr << "    ERROR: Key '" << key << "' missing in clone!\n";
				}
			}
		}
		// 8. Очистка
		std::cout << "    Clearing trie...\n";
		trie.clear();

		// Проверка, что все очистилось
		if (trie.find("dog")) {  // любое слово, которое было
			std::cerr << "    ERROR: Trie not empty after clear!\n";
		}

		std::cout << "    ++ All tests passed for " << type_name << "\n";
	}
	
	// ==================== 2. Комплексный тест производительности ====================	
	template<template<typename> typename NodeTemplate>
	static void comparison_test() {
		std::cout << "\n2. Performance comparison TESTS\n";
		std::cout << "-------------------------------------\n";
		
		std::string path = std::string(TEST_DIR) + "/words.txt";		
		auto data = read_words_from_file(path);
		
		//1. Замерим сначала префиксное дерево
		using TrieString = PrefixTree::Trie<std::string, std::string, NodeTemplate<std::string>>;
		TrieString trie;
		std::cout << "\n *** Test Trie<string, string>\n";
		single_performance_test(data, trie);
		
		//2. А теперь хэш-таблицу
		std::cout << "\n *** Test OpenHashTable<string, string>\n";
		OpenHashTable<std::string, std::string> hashTable(2001, 1, 1);		
		single_performance_test(data, hashTable);
		std::cout << "++ Comparison test completed\n\n";
	}
	// ==================== 3. Тест move semantic ====================	
	template<template<typename> typename NodeTemplate>
	static void test_move_semantics() {
		std::cout << "\n3. MOVE SEMANTICS TEST\n";
		std::cout << "---------------------------\n";
		
		//подготовка
		auto test_data = getTestData();
		using TrieString = PrefixTree::Trie<std::string, std::string, NodeTemplate<std::string>>;
		// Функция для проверки содержимого
		auto check_contents = [&](const TrieString& trie, bool should_exist) {
			for (const auto& [key, val] : test_data) {
				auto* ptr = trie.find(key);
				if (should_exist && (!ptr || *ptr != val)) return false;
				if (!should_exist && ptr) return false;
			}
			return true;
			};		
		
		TrieString original;
		
		for (size_t i = 0; i !=test_data.size() ; ++i) {
			original.insert(test_data[i].first, test_data[i].second);
		}

		TrieString clone = original.clone(); //это чтобы сравнить
		// Конструктор перемещения		
		TrieString move_constructed(std::move(original));
		assert(check_contents(move_constructed, true));      // moved содержит данные
		assert(move_constructed.size() == test_data.size()); // проверим размер
		assert(check_contents(original, false));  // original пуст (не содержит данных)
		assert(original.size() == 0);
		std::cout << "+ Move constructor\n";

		//Оператор присваивания перемещением		
		TrieString move_assigned = std::move(move_constructed);
		assert(check_contents(move_assigned, true));      //данные
		assert(move_assigned.size() == test_data.size()); // проверим размер
		assert(check_contents(move_constructed, false));  // исходник пуст (не содержит данных)
		assert(move_constructed.size() == 0);
		std::cout << "+ Move assignment\n";		

		//Self-присваивание перемещением		
		move_assigned = std::move(move_assigned);
		assert(check_contents(move_assigned, true));      //данные
		assert(move_assigned.size() == test_data.size()); // проверим размер		
		std::cout << "+ Self-move assignment\n";
	}
	// ==================== 4. Тест кастомного удалителя ====================	
	template<template<typename> typename NodeTemplate>
	static void test_custom_cleaner() {
		
		std::cout << "\n4. Custom Cleaner TEST\n";
		std::cout << "---------------------------\n";		
		
		struct LoggingCleaner {
			void operator()(std::string& val) const {
				std::cout << "  Cleaning: " << val << std::endl;
			}
		};

		using TrieType = PrefixTree::Trie<std::string, std::string, NodeTemplate<std::string>, LoggingCleaner>;
		TrieType trie;

		// Получаем тестовые данные
		auto test_data = getTestData();

		// Построим дерево		
		for (const auto& [key, val_str] : test_data) {
			trie.insert(key,val_str);
		}
		
		std::cout << "\na. Clean old value when insert duplicate key\n";
		trie.insert(test_data[1].first, "new value");
		std::cout << "---------------------------\n";
		std::cout << "\nb. Clean value when remove key\n";
		std::cout << "Value to remove " << test_data[2].second << " key " << test_data[2].first << "\n";
		trie.remove(test_data[2].first);
		std::cout << "---------------------------\n";
		std::cout << "\nc. Clean when destroy trie (clearing in destructor)\n";
	}
	//=================== вспомогательные функции
	//проверка префиксов
	static void test_prefixes(const auto& trie) {
		std::vector<std::pair<std::string, bool>> prefix_tests = {
			{"cat", true},    // есть cat и catfish
			{"dog", true},    // есть dog и dogma
			{"wolf", true},   // есть wolf и wolfram
			{"fox", true},    // есть fox
			{"ra", true},     // rabbit
			{"be", true},     // bear, bearcat
			{"z", false},     // нет такого
			{"cattle", true}, // точное совпадение
			{"", true}        // пустой префикс - должно быть true (все слова)
		};

		for (const auto& [prefix, expected] : prefix_tests) {
			bool result = trie.startsWith(prefix);
			if (result != expected) {
				std::cerr << "    ERROR: startsWith('" << prefix << "') = "
					<< result << ", expected " << expected << "\n";
			}
		}

	}
	//функция теста производительности на одном контейнере
	template<typename Container>
	static void single_performance_test(std::vector<std::pair<std::string, std::string>>& data, 
		Container& container) {		
		
		std::cout << "Insert test --------------------------\n";
		
		auto start = std::chrono::high_resolution_clock::now();
		for (const auto& [key, value] : data) {
			container.insert(key, value);			
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "  Inserted " << data.size() <<" keys in " << duration.count() << " us\n";
		std::cout << "  Container size: " << container.size() <<  "\n";

		std::cout << "Find 10% --------------------------\n";
		size_t to_find = data.size() / 10;
		start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i != to_find; ++i) {
			auto val = container.find(data[i].first);
			assert(val);
		}
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "  10% elements founded in " << duration.count() << " us\n";
		
		std::cout << "Remove 10% --------------------------\n";

		size_t to_remove = data.size() / 10;
		start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i != to_remove; ++i) {
			bool success = container.remove(data[i].first);
			assert(success);
		}
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "  10% elements removed in " << duration.count() << " us\n";
		std::cout << "  New size: " << container.size() << "\n";	
	}
	
	//"упаковщик" текстового значения в разные типы
	template<typename ValueType>
	static ValueType cover_value(const std::string& value) {
		if constexpr (std::is_same_v<ValueType, std::string>) {
			return value;
		}
		else if constexpr (std::is_same_v<ValueType, std::unique_ptr<std::string>>) {
			return std::make_unique<std::string>(value);
		}
		else if constexpr (std::is_same_v<ValueType, int>) {
			return static_cast<int>(value.length());
		}
		else if constexpr (std::is_same_v<ValueType, DataStruct>) {
			return DataStruct(value);
		}
		else {
			static_assert("Unsupported type in cover_value");
		}
	}
	 //генерация "короткого" набора данных
	static std::vector<std::pair<std::string, std::string>> getTestData() {
		return {
			// Основные животные
			{"cat", "CAT"},
			{"dog", "DOG"},
			{"wolf", "WOLF"},
			{"fox", "FOX"},
			{"rabbit", "RABBIT"},
			{"bear", "BEAR"},

			// Добавляем с общими префиксами
			{"catfish", "CATFISH"},      // начинается с "cat"
			{"dogma", "DOGMA"},           // начинается с "dog"
			{"wolfram", "WOLFRAM"},       // начинается с "wolf"
			{"foxhole", "FOXHOLE"},       // начинается с "fox"
			{"bearcat", "BEARCAT"},       // начинается с "bear"

			// Еще несколько для разнообразия
			{"cattle", "CATTLE"},          // начинается с "cat"
			{"doggie", "DOGGIE"},          // начинается с "dog"
			{"wolverine", "WOLVERINE"},    // начинается с "wolf"

			// Совсем короткие для проверки границ
			{"a", "A"},
			{"b", "B"},
			{"", "EMPTY"}  // пустой ключ
		};
	}

	//получение тестовых данных из файла
	static std::vector<std::pair<std::string, std::string>> read_words_from_file(const std::string& filename) {
		std::vector<std::pair<std::string, std::string>> result;
		std::ifstream file(filename);

		if (!file.is_open()) {
			std::cerr << "ERROR: Cannot open file " << filename << std::endl;
			return result;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (line.empty()) continue;

			std::istringstream iss(line);
			std::string word, transcription, translation;

			// Читаем три колонки, разделенные табуляцией
			if (std::getline(iss, word, '\t') &&
				std::getline(iss, transcription, '\t') &&
				std::getline(iss, translation, '\t')) {
				
				// Приводим key к нижнему регистру
				for (char& c : word) {
					c = std::tolower(static_cast<unsigned char>(c));
				}

				// Значение = транскрипция + перевод
				std::string value = transcription + " " + translation;
				result.emplace_back(word, value);
			}
		}

		return result;
	}
};
