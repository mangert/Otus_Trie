#include <iostream>
#include "Trie.cpp"

int main() {
	
	Trie trie;
	trie.insert("apple");
	std::cout << trie.search("apple") << std::endl;   // return True
	std::cout << trie.search("app") << std::endl;     // return False
	std::cout << trie.startsWith("app") << std::endl; // return True
	trie.insert("app");
	std::cout << trie.search("app") << std::endl;
	
	return 0;
}