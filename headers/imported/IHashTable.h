#pragma once
#include <concepts>

// NOTE: This is a copy from HashTable project
// Last synced: 2024-03-04

template<typename T>
concept HashableKey = requires(T a, T b) {
	{ std::hash<T>{}(a) } -> std::same_as<size_t>;
	{ a == b } -> std::same_as<bool>;
};

template <typename K, typename V> requires HashableKey<K>
class IHashTable {

public:	
	
	//---------- Основные операции-------------------//
	//Операции вставки
	virtual bool insert(K key, const V& value) = 0;
	virtual bool insert(K key, V&& value) = 0;

	//операции удаления
	virtual bool remove(const K& key) = 0;
	
	//операции доступа и поиска
	virtual bool contains(const K& key) const = 0;
	
	virtual V* find(const K& key) = 0;
	virtual const V* find(const K& key) const = 0;

	virtual V& at(const K& key) = 0;
	virtual const V& at(const K& key) const = 0;
	
	// Только для неконстантных объектов
	virtual V& operator[](const K& key) = 0;

	//очистка
	virtual void clear() = 0;
	
	//---------- Рехэширование -------------------//
	virtual void rehash(size_t new_size) = 0;
	
	//---------- Характeристики-------------------//

	//максимальное число бакетов
	virtual [[nodiscard]] size_t max_bucket_count() const noexcept = 0;
	
	//фактический размер
	virtual size_t size() const noexcept = 0;
	//проверка на пустоту
	virtual bool empty() const noexcept = 0;

	// Коэффициент заполнения
	virtual double load_factor() const = 0;
};