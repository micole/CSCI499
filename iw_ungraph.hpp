#ifndef iw_ungraph_hpp
#define iw_ungraph_hpp

// iw_ungraph stands for integer weighted undirected graph.

#include "ungraph.hpp"

class iw_ungraph : public ungraph<int> {
public:
  explicit iw_ungraph(int nverts);

  std::string toDOT(bool label = false) const;  // convert to DOT string
};

inline iw_ungraph::iw_ungraph(int nverts)
  : ungraph<int>(nverts)
{
}

void delDeg12Nodes(ungraph<int> const& g, int src, int dst);

#endif // iw_ungraph_hpp
