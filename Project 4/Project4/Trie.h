#ifndef TRIE_INCLUDED
#define TRIE_INCLUDED

#include <string>
#include <vector>

template<typename ValueType>
class Trie
{
public:
	Trie();
	~Trie();
	void reset();
	void insert(const std::string &key, const ValueType &value);
	std::vector<ValueType> find(const std::string &key, bool exactMatchOnly) const;

	  // C++11 syntax for preventing copying and assignment
	Trie(const Trie&) = delete;
	Trie& operator=(const Trie&) = delete;
private:
	struct Node;
	Node *m_root;

		// called by destructor and reset
	void deleteNode(Node *root);

		// called by insert
	bool isChild(const Node *root, const char id, Node *&child) const;
	Node* createNode(Node *root, const char id);

		// called by find
	void findNode(const Node *root, const std::string &key, bool exactMatchOnly, std::vector<ValueType> &vals) const;
	void fillVector(const std::vector<ValueType> &filler, std::vector<ValueType> &fillMe) const;
};

//=================================================================================================
//	PUBLIC MEMBERS
//=================================================================================================

//=================================================================================================
//	constructor
//	dynamically allocates the root node with m_root
//=================================================================================================
template<typename ValueType>
Trie<ValueType>::Trie() {
	m_root = new Node;
}

//=================================================================================================
//	destructor
//	deletes each node in the tree using deleteNode
//=================================================================================================
template<typename ValueType>
Trie<ValueType>::~Trie() {
	deleteNode(m_root);
}

//=================================================================================================
//	void reset
//	deletes the entire tree and creates a new root node
//=================================================================================================
template<typename ValueType>
void Trie<ValueType>::reset() {
	deleteNode(m_root);
	m_root = new Node;
}

//=================================================================================================
//	void insert
//	maps value to key using the tree structure
//=================================================================================================
template<typename ValueType>
void Trie<ValueType>::insert(const std::string &key, const ValueType &value) {
	Node *cur = m_root;	// the current node being analyzed

	for (size_t i = 0; i < key.size(); i++) {
		Node *temp;	//temporary holder for cur's child

		if (!isChild(cur, key[i], temp))	// checks if cur already has a child with the given id
											// and sets temp to its child if true
			temp = createNode(cur, key[i]);	// creates a new child of cur with the given id and
											// sets temp to it

		cur = temp;	// sets cur to its child held in temp for next iteration
	}

	cur->vals.push_back(value);	// adds value to cur, which is the leaf node
}

//=================================================================================================
//	std::vector<ValueType> find
//	find the values mapped to key as well as those mapped to a key with one char difference
//	(excluding the first char) unless exactMatchOnly is true
//=================================================================================================
template<typename ValueType>
std::vector<ValueType> Trie<ValueType>::find(const std::string &key, bool exactMatchOnly) const {
	std::vector<ValueType> values;
	for (size_t i = 0; i < m_root->childs.size(); i++) {
		if (key[0] == m_root->childs[i]->id)
			findNode(m_root->childs[i], key.substr(1), exactMatchOnly, values);
	}
	return values;
}

//=================================================================================================
//	PRIVATE MEMBERS
//=================================================================================================

//=================================================================================================
//	struct Node
//	contains a character id, a vector of values, and a vector of child pointers
//=================================================================================================
template<typename ValueType>
struct Trie<ValueType>::Node {
	char id;
	std::vector<ValueType> vals;
	std::vector<Node*> childs;
};

//=================================================================================================
//	void deleteNode
//	deletes the given root node as well as all children branching from the root
//=================================================================================================
template<typename ValueType>
void Trie<ValueType>::deleteNode(Node *root) {
	for (size_t i = 0; i < root->childs.size(); i++)
		deleteNode(root->childs[i]);
	delete root;
}

//=================================================================================================
//	bool isChild
//	returns true if root has a child with the given id and sets child to that particular child
//	otherwise, returns false and leaves child unchanged
//=================================================================================================
template<typename ValueType>
bool Trie<ValueType>::isChild(const Node *root, const char id, Node *&child) const {
	for (int i = 0; i < root->childs.size(); i++) {
		if (id == root->childs[i]->id) {
			child = root->childs[i];
			return true;
		}
	}
	return false;
}

//=================================================================================================
//	Node* createNode
//	creates a new child of root with specified id and returns a pointer to this child
//=================================================================================================
template<typename ValueType>
typename Trie<ValueType>::Node* Trie<ValueType>::createNode(Node *root, char id) {
	Node *child = new Node;
	child->id = id;
	root->childs.push_back(child);
	return child;
}

//=================================================================================================
//	void findNode
//	recursively find the node with the given key and add its values to vals
//=================================================================================================
template<typename ValueType>
void Trie<ValueType>::findNode(const Node *root, const std::string &key, bool exactMatchOnly, std::vector<ValueType> &vals) const {
	if (key.empty()) {	// base case: reached end of key on leaf node so add its vals
		fillVector(root->vals, vals);
		return;
	}

	for (size_t i = 0; i < root->childs.size(); i++) {
		if (key[0] == root->childs[i]->id)	// current char of key matches child's id so call
											// recursively on child
			findNode(root->childs[i], key.substr(1), exactMatchOnly, vals);
		else if (!exactMatchOnly)	// current char of key doesn't match child's id but
									// exactMatchOnly is false so call recursively with true
			findNode(root->childs[i], key.substr(1), true, vals);
	}
}

//=================================================================================================
//	void fillVector
//	adds all values in filler to fillMe
//=================================================================================================
template<typename ValueType>
void Trie<ValueType>::fillVector(const std::vector<ValueType> &filler, std::vector<ValueType> &fillMe) const {
	for (size_t i = 0; i < filler.size(); i++)
		fillMe.push_back(filler[i]);
}

#endif // TRIE_INCLUDED
