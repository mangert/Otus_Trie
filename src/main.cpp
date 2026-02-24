#include <iostream>
#include "Trie.cpp"
#include "PrefixTree.h"

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

	
	return 0;
}