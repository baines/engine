#ifndef TRIE_H_
#define TRIE_H_
#include <vector>
#include <cstdint>
#include "util.h"

/* I implemented this pretty weirdly. Instead of an array of 64 pointers, each
   node has a bitset with a 0 if the pointer would have been null along with
   a linked list that only contains the non-null pointers.
   
   This saves ~250 / ~500 bytes per node for 32bit / 64bit respectively.
   
   Also the whole thing is backed by a std::vector because it wasn't complicated
   enough already... (and a contiguous block of memory for nodes is nicer.)
   
   Oh and it only accepts numbers, lowercase letters and '_' because that's all
   I needed. There's room for 27 more characters though if you change the 
   char<->bitset position functions.
   
   -- Alex
*/
template<class T>
struct Trie {

	Trie(){
		// add the root node of the trie at index 0 in the storage vector.
		storage.emplace_back();
	}
	
	static constexpr unsigned char_to_bitset_pos(unsigned char input){
		return (input >= '0' && input <= '9') ? input - '0'
		     : (input == '_')                 ? 10
			 : (input >= 'a' && input <= 'z') ? input - ('a' - 11)
			 : 63 // map invalid chars to highest bit.
			 ;
	}
	
	static constexpr unsigned char bitset_pos_to_char(unsigned num){
		return (num <  10) ? num + '0' 
		     : (num == 10) ? '_' 
		     : (num <= 36) ? num + ('a' - 11)
			 : '?'
			 ;
	}

	void add(const char* name, T val){
		// start traversing from the root node (index 0).
		unsigned node_idx = 0;
		
		for(const char* p = name; *p; ++p){
			// get the bitset position of the current character.
			const unsigned shift = char_to_bitset_pos(*p);
			
			// get how many child nodes precede the one that represents the current char.
			const unsigned offset = popcount64(
				storage[node_idx].bitset & ((1ULL << shift)-1ULL)
			);

			// get the child node that should directly precede the current char's node.
			unsigned child_idx = storage[node_idx].children_idx;
			for(unsigned i = 1; i < offset; ++i){
				child_idx = storage[child_idx].next_idx;
			}
			
			// if the child node for this char exists, continue traversing from there.
			const bool child_node_exists = (storage[node_idx].bitset & (1ULL << shift));
			if(child_node_exists){
				node_idx = offset ? storage[child_idx].next_idx : child_idx;

			// otherwise we need to insert it.
			} else {
				storage.emplace_back();
				const unsigned newnode_idx = storage.size() - 1;
				
				// if the new node is the first child node, just link it with the parent node.
				if(storage[node_idx].children_idx == 0){
					storage[node_idx].children_idx = newnode_idx;
				// otherwise, we have to link it with the other child nodes too.
				} else {
					// if the new node has the lowest bitset position, change the parent node
					// to point to it, and link in the old lowest to the new node's next.
					if(offset == 0){
						storage[newnode_idx].next_idx = storage[node_idx].children_idx;
						storage[node_idx].children_idx = newnode_idx;
					// otherwise, link the preceding child to point to the new one, and the 
					// new one to point at what the preceding one used to point to.
					} else {
						storage[newnode_idx].next_idx = storage[child_idx].next_idx;
						storage[child_idx].next_idx = newnode_idx;
					}
				}
				
				// update the parent's bitset to include the new child node.
				storage[node_idx].bitset |= (1ULL << shift);
				
				// continue traversing from the new node.
				node_idx = newnode_idx;
			} 
		}
		
		// finally after the traversal is complete, set the node's value.
		storage[node_idx].val = val;
	}
	
	T find(const char* name){
		unsigned node_idx = find_idx(name);
		if(!node_idx) return T();
		
		return storage[node_idx].val;
	}
	
	// auto-completes the given string as much as possible.
	bool prefixExtend(alt::StrMut& prefix, size_t offset){
		bool modified = false;

		if(unsigned node_idx = find_idx(prefix.c_str()+offset)){
			Node* n = &storage[node_idx];

			while(!n->val && popcount64(n->bitset) == 1){
				prefix.append(bitset_pos_to_char(log2ll(n->bitset)));
				n = &storage[n->children_idx];
				modified = true;
			} 
		}

		return modified;
	}
	
	// puts all the nodes with the given prefix into the given container.
	template<class Output>
	void prefixSearch(const char* prefix, Output& matches){
		unsigned node_idx = find_idx(prefix);
		
		if(!node_idx) return;
		
		if(storage[node_idx].val){
			matches.push_back(storage[node_idx].val);
		}
		
		if(storage[node_idx].children_idx){
			search_ids.push_back(storage[node_idx].children_idx);
		}
		
		while(!search_ids.empty()){
			Node& n = storage[search_ids.back()];
			search_ids.pop_back();
			
			if(n.children_idx){
				search_ids.push_back(n.children_idx);
			}
			
			if(n.next_idx){
				search_ids.push_back(n.next_idx);
			}
			
			if(n.val){
				matches.push_back(n.val);
			}
		}
	}
private:

	// finds the storage index of the node that represents the chain of characters forming name.
	unsigned find_idx(const char* name){
		unsigned node_idx = 0;
		
		for(const char* p = name; *p; ++p){
			const unsigned shift = char_to_bitset_pos(*p);
			const unsigned offset = popcount64(
				storage[node_idx].bitset & ((1ULL << shift)-1ULL)
			);

			bool child_node_exists = (storage[node_idx].bitset & (1ULL << shift));
			if(!child_node_exists){
				return 0;
			}
			
			unsigned child_idx = storage[node_idx].children_idx;
			for(unsigned i = 1; i < offset; ++i){
				if(!storage[child_idx].next_idx){
					return 0;
				} else {
					child_idx = storage[child_idx].next_idx;
				}
			}
			node_idx = offset ? storage[child_idx].next_idx : child_idx;
		}
		return node_idx;
	}

	struct Node {
		Node() : bitset(0), val(), next_idx(0), children_idx(0){}
		uint64_t bitset;
		T val;
		unsigned next_idx;
		unsigned children_idx;
	};
	
	std::vector<Node> storage;
	std::vector<unsigned> search_ids;
};

#endif
