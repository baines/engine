#ifndef TRIE_H_
#define TRIE_H_
#include <vector>
#include <cstdint>
#include <cstdio>
#include "util.h"

/* I implemented this pretty weirdly. Instead of an array of 64 pointers, each
   node has a bitset with a 0 if the pointer would have been null along with
   a linked list that only contains the non-null pointers.
   
   This saves ~250 / ~500 bytes per node for 32bit / 64bit respectively.
   
   Also the whole thing is backed by a std::vector because it wasn't complicated
   enough already... (and a contiguous block of memory for nodes is nicer.)
   
   Oh and it only accepts numbers, lowercase letters and '_' because that's all
   I needed. There's room for 27 more characters though if you change the index
   function.
   
   -- Alex
*/
template<class T>
struct Trie {

	Trie(){
		storage.emplace_back();
	}
	
	static constexpr unsigned index(unsigned char input){
		return ((input & 0x70) == 0x30) ? input - 0x30 : input == '_' ? 10 : input - 0x56;
	}
	
	static constexpr unsigned char inv_index(unsigned num){
		return num < 10 ? num + 0x30 : num == 10 ? '_' : num + 0x56;
	}

	void add(const char* name, T val){
		unsigned node_idx = 0;
		
		for(const char* p = name; *p; ++p){
			const unsigned shift = index(*p);
			const unsigned offset = __builtin_popcountll(storage[node_idx].bitset & ((1ULL << shift)-1ULL));
			
			//printf("adding %s. [%c] node_idx: %d, shift: %d, offset: %d. bitset: %llx\n", name, *p, node_idx, shift, offset, storage[node_idx].bitset);
			
			unsigned child_idx = storage[node_idx].children_idx;
			for(unsigned i = 1; i < offset; ++i){
				child_idx = storage[child_idx].next_idx;
			}
			
			const bool node_already_present = (storage[node_idx].bitset & (1ULL << shift));
			if(!node_already_present){
			
				storage.emplace_back();
				const unsigned newnode_idx = storage.size() - 1;
				
				if(storage[node_idx].children_idx == 0){
					storage[node_idx].children_idx = newnode_idx;
				} else {
					if(offset == 0){
						storage[newnode_idx].next_idx = storage[node_idx].children_idx;
						storage[node_idx].children_idx = newnode_idx;
					} else {
						storage[newnode_idx].next_idx = storage[child_idx].next_idx;
						storage[child_idx].next_idx = newnode_idx;
					}
				}
				
				storage[node_idx].bitset |= (1ULL << shift);
				
				node_idx = newnode_idx;
			} else {
				node_idx = offset ? storage[child_idx].next_idx : child_idx;
			}
		}
		
		storage[node_idx].val = val;
	}
	
	T find(const char* name){
		unsigned node_idx = find_idx(name);
		if(!node_idx) return T();
		
		return storage[node_idx].val;
	}
	
	bool prefixExtend(std::string& prefix){
		bool modified = false;
		if(unsigned node_idx = find_idx(prefix.c_str())){			
			for(Node* n = &storage[node_idx]; __builtin_popcountll(n->bitset) == 1; n = &storage[n->children_idx]){
				prefix += inv_index(log2ll(n->bitset));
				modified = true;
			} 
		}
		return modified;
	}
	
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

	unsigned find_idx(const char* name){
		unsigned node_idx = 0;
		
		for(const char* p = name; *p; ++p){
			unsigned shift = index(*p);
			unsigned offset = __builtin_popcountll(storage[node_idx].bitset & ((1ULL << shift)-1ULL));
			bool has_child = (storage[node_idx].bitset & (1ULL << shift));
						
			if(!has_child){
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
