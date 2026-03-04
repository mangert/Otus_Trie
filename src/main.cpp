#include <iostream>
#include "Trie.cpp"
#include "PrefixTree.h"
#include "PrefixTest.h"
#include <string>
#include <memory>

struct Bad {
	Bad() = delete;
	Bad(bool is_valid) : val("new value") {}

	std::string val;
};

int main() {
	std::cout << "************************************Prefix Trees Tests\n\n";
	PrefixTest::comprehensive_test<ArrayNode>("Testing ArrayNode");
	std::cout << "\n-------------------------------------------------\n";
	PrefixTest::comprehensive_test<HashMapNode>("Testing HashMapNode");
	std::cout << "\n-------------------------------------------------\n";
	PrefixTest::comprehensive_test<HybridNode>("Testing HybridNode");
	return 0;
}