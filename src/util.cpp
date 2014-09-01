#include <list>

/* Redirect list's _M_hook to the old deprecated one that wasn't in a __detail
   namespace, so that GLIBCXX_2.14 isn't required */

namespace std {
	struct _List_node_base {
		void hook(_List_node_base* ptr);
	};
	namespace __detail {
		void _List_node_base::_M_hook(_List_node_base* const ptr) noexcept {
			reinterpret_cast<std::_List_node_base*>(this)->hook(reinterpret_cast<std::_List_node_base*>(ptr));
		}
	}
}

