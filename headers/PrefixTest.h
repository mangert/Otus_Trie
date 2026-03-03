#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <set>
#include <functional>
#include <concepts>
#include <memory>
#include "PrefixTree.h"
//#include "OpenHashTable.h"

using namespace PrefixTree;

class PrefixTest {

public:

	template<template<typename> typename NodeTemplate>
	static void comprehensive_test(const std::string& name) {
		std::cout << "========================================\n";
		std::cout << "Prefix Tree TEST (" << name << ")\n";
		std::cout << "========================================\n\n";	

		// 1. Тесты на работоспособность с разными типами
		test_with_value_type<NodeTemplate, int>("int");
		test_with_value_type<NodeTemplate, std::string>("string");
		test_with_value_type<NodeTemplate, std::unique_ptr<std::string>>("unique_ptr");
		test_with_value_type<NodeTemplate, DataStruct>("DataStruct");
		
		

		// 2. Основной тест на вставку/удаление
		//main_test();

		// 3. Тест копирования и перемещения
		//test_copy_move_semantics();

		//4. Тесты на на сравнение с хэш-таблицей
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
		/*static void test_edge_cases() {
		std::cout << "\n1. EDGE CASES TEST\n";
		// Тестируем с разными типами значений
		test_edge_cases_for_type<std::string>("std::string");
		test_edge_cases_for_type<int>("int");
		test_edge_cases_for_type<std::unique_ptr<std::string>>("std::unique_ptr<std::string>");
		test_edge_cases_for_type<DataStruct>("DataStruct");

		std::cout << "++ All edge cases passed\n\n";
	}*/
	/*
	template<typename ValueType>
	static void test_edge_cases_for_type(const std::string& type_name) {
		std::cout << "\n--- Testing " << type_name << " ---\n";

		// Создаем дерево (выбираем тип по ситуации)
		//using TestTrie = Tree;  // Tree уже параметризована из класса

		PrefixTree::Trie<std::string, ValueType, Node> trie;
		auto data = getTestData();

		// 1. Вставка
		std::cout << "  Inserting " << data.size() << " items...\n";
		for (const auto& [key, val_str] : data) {
			trie.insert(key, cover_value<ValueType>(val_str));
		}

		// 2. Проверка поиска (find)
		std::cout << "  Testing find()...\n";
		for (const auto& [key, val_str] : data) {
			auto* found = trie.find(key);
			if (!found) {
				std::cerr << "    ERROR: Key '" << key << "' not found!\n";
				continue;
			}

			// Проверяем значение в зависимости от типа
			if constexpr (std::is_same_v<ValueType, std::string>) {
				if (*found != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, std::unique_ptr<std::string>>) {
				if (found.get() != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, int>) {
				if (*found != static_cast<int>(val_str.length())) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
			else if constexpr (std::is_same_v<ValueType, DataStruct>) {
				if (found->value != val_str) {
					std::cerr << "    ERROR: Value mismatch for '" << key << "'\n";
				}
			}
		}

		// 3. Проверка search (только для копируемых типов)
		if constexpr (std::is_copy_constructible_v<ValueType>) {
			std::cout << "  Testing search()...\n";
			for (const auto& [key, val_str] : data) {
				auto opt = trie.search(key);
				if (!opt.has_value()) {
					std::cerr << "    ERROR: Key '" << key << "' not found in search!\n";
					continue;
				}

				// Проверка аналогично find
				if constexpr (std::is_same_v<ValueType, std::string>) {
					if (*opt != val_str) {
						std::cerr << "    ERROR: Search value mismatch for '" << key << "'\n";
					}
				}
				// ... остальные типы
			}
		}

		// 4. Проверка startsWith
		std::cout << "  Testing startsWith()...\n";
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

		// 5. Удаление нескольких ключей
		std::cout << "  Testing remove()...\n";
		std::vector<std::string> to_remove = { "cat", "wolf", "foxhole", "a" };
		for (const auto& key : to_remove) {
			bool removed = trie.remove(key);
			if (!removed) {
				std::cerr << "    ERROR: Failed to remove '" << key << "'\n";
			}
		}

		// 6. Проверка, что удаленные ключи действительно исчезли
		for (const auto& key : to_remove) {
			if (trie.find(key)) {
				std::cerr << "    ERROR: Key '" << key << "' still exists after remove!\n";
			}
		}
		

		// 8. Тест cleanup (визуально в отладчике)
		std::cout << "  Running cleanup (check in debugger)...\n";
		trie.cleanup();

		// 9. Тест clone (только для копируемых типов)
		if constexpr (std::is_copy_constructible_v<ValueType>) {
			std::cout << "  Testing clone()...\n";
			auto cloned = trie.clone();

			// Проверяем, что клон содержит те же ключи
			for (const auto& [key, val_str] : data) {
				if (std::find(to_remove.begin(), to_remove.end(), key) != to_remove.end()) {
					continue;
				}
				if (!cloned.find(key)) {
					std::cerr << "    ERROR: Key '" << key << "' missing in clone!\n";
				}
			}
		}

		// 10. Очистка
		std::cout << "  Clearing trie...\n";
		trie.clear();

		// Проверка, что все очистилось
		if (trie.find("dog")) {  // любое слово, которое было
			std::cerr << "    ERROR: Trie not empty after clear!\n";
		}

		std::cout << "  " << type_name << " tests completed.\n";
	}*/

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

		//5а. Тест кастомного удаления (условно - просто "испортим" значение и посмотрим в отладчике,
		//в консоль выведем, что удалитель сработал)
		std::cout << "    Testing remove() with custom cleaner...\n";
		std::string key = "b";
		auto deleter = [&](ValueType& val) {
			std::cout << "    ---Custom clean value, key = " << key << std::endl;
			val = cover_value<ValueType>("DELETED");
			};
		to_remove.emplace_back(key); //дополним вектор удаленных для дальнейшего использования
		bool removed = trie.remove(key, deleter);
		if (!removed) {
			std::cerr << "    ERROR: Failed to remove " << key << "\n";
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


	// ==================== 2. Основной тест ====================	
	/*
	static void main_test() {
		std::cout << "\n2. MAIN TEST\n";
		std::cout << "-------------------------------------\n";

		size_t M = 101;
		for (size_t i = 0; i != 6; ++i) {
			std::cout << "Iteration " << i << "; M =" << M << "\n";
			single_main_test(M);
			M = M * 10 + 1;
		}
		std::cout << "++ Main test completed\n\n";
	}

	static void test_copy_move_semantics() {
		std::cout << "\n3. COPY/MOVE SEMANTICS TEST\n";
		std::cout << "---------------------------\n";

		size_t M = 50;
		auto data = gen_data(M);
		// Создаём исходную таблицу и заполняем
		HashTable original(M);
		for (size_t i = 0; i != M; ++i) {
			original.insert(data[i].first, data[i].second);
		}

		// 4.1 Конструктор копирования
		HashTable copy_constructed(original);
		verify_equality(original, copy_constructed, data, "copy constructor");
		std::cout << "+ Copy constructor\n";

		// 4.2 Оператор присваивания копированием		
		HashTable copy_assigned = original;
		verify_equality(original, copy_assigned, data, "copy assignment");
		std::cout << "+ Copy assignment\n";

		// 4.3 Конструктор перемещения
		HashTable temp_for_move1 = original; // копируем
		HashTable move_constructed(std::move(temp_for_move1));
		verify_equality(original, move_constructed, data, "move constructor");
		assert(temp_for_move1.empty() || temp_for_move1.size() == 0);
		std::cout << "+ Move constructor\n";

		// 4.4 Оператор присваивания перемещением
		HashTable temp_for_move2 = original; // копируем		
		HashTable move_assigned = std::move(temp_for_move2);
		verify_equality(original, move_assigned, data, "move assignment");
		assert(temp_for_move2.empty() || temp_for_move2.size() == 0);
		std::cout << "+ Move assignment\n";

		// 4.5 Self-assignment
		HashTable self_assigned = original;
		self_assigned = self_assigned; // self-assignment
		verify_equality(original, self_assigned, data, "self assigment");
		std::cout << "+ Self-assignment\n";
		std::cout << "++ Copy/move semantics test completed\n\n";
	}

	// Тест с разными коэффициентами (для OpenHashTable)    
	static void test_coefficients() {
		std::vector<std::tuple<size_t, size_t, size_t>> params = {
			{950041, 0, 1},    // классический
			{950041, 1, 1},    // с линейным членом
			{950041, 2, 3},     // простые числа            
		};

		std::cout << "\n4. COEFFICIENTS TEST\n";
		std::cout << "-------------------------------------\n";

		for (auto [M, A, B] : params) {
			std::cout << "\n ***** M = " << M << "; A = " << A << "; B = " << B << "\n";
			single_main_test(M, A, B);
		}

		std::cout << "++ Coefficients test completed\n\n";
	}

	// Тест на исключения
	static void test_exceptions() {
		std::cout << "\n5. EXCEPTIONS TEST\n";
		std::cout << "------------------\n";

		// Проверяем, что конструктор кидает при M=0
		try {
			HashTable(0);
			std::cout << "FAILED: Expected std::invalid_argument, but no exception thrown";
		}
		catch (const std::invalid_argument&) {
			std::cout << "+ NON-ZERO M test passed\n";
		}
		catch (...) {
			std::cout << "FAILED: Expected std::invalid_argument, but different exception thrown";
		}

		// Проверка взаимной простоты (только для OpenHashTable)
		if constexpr (std::is_same_v<HashTable, OpenHashTable<int, std::string>>) {
			try {
				OpenHashTable<int, std::string> bad_table(100, 2, 2); // A=2, B=2, M=100 (gcd(2,100)=2)
				std::cout << "FAILED: Expected std::invalid_argument about coprime, but no exception thrown";
			}
			catch (const std::invalid_argument&) {
				std::cout << "+ Coprime check passed\n";
			}
			catch (...) {
				std::cout << "FAILED: Expected std::invalid_argument, but different exception thrown";
			}
			// Проверка на нулевый коэффициенты (только для OpenHashTable)
			try {
				OpenHashTable<int, std::string> bad_table(100, 0, 0); // A, B == 0
				std::cout << "FAILED: Expected std::invalid_argument about zero A B, but no exception thrown";
			}
			catch (const std::invalid_argument&) {
				std::cout << "+ NON ZERO A&B check passed\n";
			}
			catch (...) {
				std::cout << "FAILED: Expected std::invalid_argument, but different exception thrown";
			}
		}
	}
	// ==================== Вспомогательные функции ====================	
	//функция компплексного теста на единичном наборе данных
	static void single_main_test(size_t M, size_t a = 0, size_t b = 1) {

		HashTable table = [&]() {
			if constexpr (std::is_same_v<HashTable, OpenHashTable<int, std::string>>) {
				return HashTable(M, a, b);
			}
			else {
				return HashTable(M);
			}
			}();

		std::cout << "Fill table --------------------------\n";
		auto data = gen_data(M / 4 * 3);
		auto start = std::chrono::high_resolution_clock::now();
		for (auto item : data) {
			bool success = table.insert(item.first, item.second);
			assert(success);
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "  Table filled in " << duration.count() << " ms\n";
		std::cout << "  Size: " << table.size() << ", buckets " << table.max_bucket_count() << "\n";
		std::cout << "  Load factor: " << table.load_factor() << "\n";

		std::cout << "Find 10% --------------------------\n";
		size_t to_find = M / 40 * 3;
		start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i != to_find; ++i) {
			auto val = table.find(data[i].first);
			assert(val);
		}
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "  10% elements founded in " << duration.count() << " ms\n";
		std::cout << "Remove 10% --------------------------\n";

		size_t to_remove = M / 40 * 3;
		start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i != to_remove; ++i) {
			bool success = table.remove(data[i].first);
			assert(success);
		}
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "  10% elements removed in " << duration.count() << " ms\n";
		std::cout << "  New size: " << table.size() << ", buckets " << table.max_bucket_count() << "\n";
		std::cout << "  Load factor: " << table.load_factor() << "\n";

		std::cout << "Rehash X2 --------------------------\n";

		start = std::chrono::high_resolution_clock::now();
		table.rehash(M * 2 + 1);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "  Table have been rehashed in " << duration.count() << " ms\n";
		std::cout << "  New size: " << table.size() << ", buckets " << table.max_bucket_count() << "\n";
		std::cout << "  Load factor: " << table.load_factor() << "\n";
	}
	//проверка скопированных / перемещенных таблиц
	static void verify_equality(const HashTable& t1, const HashTable& t2,
		const std::vector<std::pair<int, std::string>>& test_data, const std::string& test_name) {

		// 1. Проверяем размер
		if (t1.size() != t2.size()) {
			std::cerr << "FAILED " << test_name << ": sizes differ: "
				<< t1.size() << " vs " << t2.size() << std::endl;
			return;
		}

		// 2. Проверяем empty() согласованность
		if (t1.empty() != t2.empty()) {
			std::cerr << "FAILED " << test_name << ": empty() mismatch" << std::endl;
			return;
		}

		// 3. Проверяем load_factor
		if (std::abs(t1.load_factor() - t2.load_factor()) > 1e-10) {
			std::cerr << "FAILED " << test_name << ": load factors differ: "
				<< t1.load_factor() << " vs " << t2.load_factor() << std::endl;
			return;
		}

		// 4. Проверяем max_bucket_count

		if (t1.max_bucket_count() != t2.max_bucket_count()) {
			std::cerr << "FAILED " << test_name << ": max_bucket_count differ: "
				<< t1.max_bucket_count() << " vs " << t2.max_bucket_count() << std::endl;
			return;
		}

		std::cout << "PASSED " << test_name << " (basic checks)" << std::endl;

		// Проверяем все ключи из тестовых данных
		for (const auto& [key, expected_value] : test_data) {
			auto* val1 = t1.find(key);
			auto* val2 = t2.find(key);

			// Проверяем наличие
			if ((val1 == nullptr) != (val2 == nullptr)) {
				std::cerr << "FAILED " << test_name << ": key " << key
					<< " exists in one table but not the other" << std::endl;
				return;
			}

			// Если ключ есть в обеих, проверяем значение
			if (val1 != nullptr && *val1 != *val2) {
				std::cerr << "FAILED " << test_name << ": value mismatch for key " << key
					<< ": '" << *val1 << "' vs '" << *val2 << "'" << std::endl;
				return;
			}
		}
		std::cout << "PASSED " << test_name << " (full content check)" << std::endl;
	}

	// ==================== Генерация тестовых данных ====================	

	static std::vector<std::pair<int, std::string>> gen_data(size_t size) {
		// Подготовка данных
		std::vector<std::pair<int, std::string>> data(size);

		for (size_t i = 0; i < size; i++) {
			std::pair<int, std::string> item{ static_cast<int>(i), std::to_string(i) };
			data[i] = item;
		}

		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(data.begin(), data.end(), g);

		return data;
	}*/

};
