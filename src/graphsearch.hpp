/* License and copyright go here*/

// utility algorithms to search a graph
#ifndef GRAPH_SEARCH_HPP
#define GRAPH_SEARCH_HPP

#include <map>
#include <vector>

template <typename k, typename v,
	  typename map=std::map<k,v>, 
	  typename mapi = typename std::map<k,v>::iterator, 
	  typename i=typename std::vector<k>>
class graphSearch {
public:

  virtual i deeper(const k&) = 0;

  bool isCovered(const map & m, const k& s) {
    const auto mEnd = m.end();
    for (auto slot : deeper(s)) {
      auto p = m.find(slot);
      if (p != mEnd // slot exists
	  && p->second // slot is filled
	  ) return true; // something is covering n.
      if (isCovered(m, p->first))
	return true; // nothing in this slot, but may be something in a more outer one recursively.
    }
    return false;
  }
};

#endif // ndef GRAPH_SEARCH_HPP
