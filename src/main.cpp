#include <iostream>
#include "Trie.cpp"
#include "PrefixTree.h"
#include <string>
#include <memory>

struct Bad {
	Bad() = delete;
	Bad(bool is_valid) : val("new value") {}

	std::string val;
};

int main() {
	
	/*Trie trie;
	trie.insert("apple");
	std::cout << trie.search("apple") << std::endl;   // return True
	std::cout << trie.search("app") << std::endl;     // return False
	std::cout << trie.startsWith("app") << std::endl; // return True
	trie.insert("app");
	std::cout << trie.search("app") << std::endl;*/
	using namespace PrefixTree;
	ArrayTrie<int> trie;
	trie.insert("dog", 10);
	std::cout << trie.search("dog").value() << std::endl;
	if(!trie.search("cat").has_value())
		std::cout << "not found" << std::endl;
	trie.insert("wolf", 18);
	std::cout << trie.search("wolf").value() << std::endl;
	trie.remove("wolf");
	if (!trie.search("wolf").has_value())
		std::cout << "not found" << std::endl;
	//auto trie_clone = trie.clone();
	//std::cout << trie_clone.search("dog").value() << std::endl;

	ArrayTrie<std::unique_ptr<std::string>> unique_trie;
	unique_trie.insert("wolf", std::make_unique<std::string>("WOLF"));
	unique_trie.remove("wolf", [](std::unique_ptr<std::string>& p) {
		std::cout << "wolf cleaner" << std::endl;
		p.reset();
		});
	unique_trie.insert("elf", std::make_unique<std::string>("elf"));
	auto x = unique_trie.search("elf");
	//auto const y = unique_trie.find("elf");
	
	//auto clone = unique_trie.clone();
	
	//std::cout << unique_trie.search("wolf").value() << std::endl;
	
	ArrayTrie<int*> ptr_trie;
	ptr_trie.insert("fox", new int{3});
	std::cout << *ptr_trie.search("fox").value() << std::endl;
	ptr_trie.remove("fox");

	ArrayTrie<std::string> str_trie;
	str_trie.insert("bear", "BEAR");
	std::cout << str_trie.search("bear").value() << std::endl;
	str_trie.remove("bear");

	

	//Bad badVal(true);
	//bad_trie.insert("rabbit", badVal);
	//std::cout << str_trie.search("rabbit").value() << std::endl;


	return 0;
}