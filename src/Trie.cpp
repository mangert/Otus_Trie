#include <iostream>
#include <string>
#include <array>

/// <summary>
/// Leatcode problem #208
/// https://leetcode.com/problems/implement-trie-prefix-tree/
/// https://leetcode.com/problems/implement-trie-prefix-tree/solutions/7603694/otus-homework-prefix-tree-by-mangert-6am7
/// Constraints:
/// 1 <= word.length, prefix.length <= 2000
/// word and prefix consist only of lowercase English letters.
/// At most 3 * 104 calls in total will be made to insert, search, and startsWith.
/// </summary>

class Trie {
public:    
    
    Trie() : root(new Node) {}
    
    ~Trie() {        
        delete root;        
    }

    void insert(const std::string& word) {        
        Node* node_ptr = root;
        for (auto& letter : word) {            
            node_ptr = node_ptr->setLetter(letter);
        }
        node_ptr->is_end = true;        
    }

    bool search(const std::string& word) {
        
        Node* node_ptr = root;

        for (auto& letter : word) {
            node_ptr = node_ptr->getLetter(letter);
            if (!node_ptr) return false;
        }       
        
        return (node_ptr->is_end);
    }

    bool startsWith(const std::string& prefix) {
        
        Node* node_ptr = root;

        for (auto& letter : prefix) {
            node_ptr = node_ptr->getLetter(letter);
            if (!node_ptr) return false;
        }
        return true;
    }

private:
    struct Node {
        std::array<Node*, 26> letters = { { nullptr } };
        bool is_end = false;

        Node() = default;
        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;

        ~Node() {
            for (auto& node_ptr : letters) {
                if (node_ptr) delete node_ptr;
            }
        }

        Node* setLetter(char letter) {
            
            auto& node_ptr = getLetter(letter);
            if (!node_ptr) {
                node_ptr = new Node;
            }
            return node_ptr;
        }

        Node*& getLetter(char letter) {                        
            constexpr char startLetter = 'a';      
            if (letter < 'a' || letter > 'z') {
                throw std::out_of_range("Invalid character");
            }
            return letters[letter - startLetter];
        }
    };
private:
    Node* root;
};

/**
 * Your Trie object will be instantiated and called as such:
 * Trie* obj = new Trie();
 * obj->insert(word);
 * bool param_2 = obj->search(word);
 * bool param_3 = obj->startsWith(prefix);
 */