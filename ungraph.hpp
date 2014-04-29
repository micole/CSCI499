#ifndef ungraph_hpp
#define ungraph_hpp

#include <string>
#include "digraph.hpp"

/*
 An undirected graph implementation based on a directed graph.  NOTE:
 an undirected graph is NO DIFFERENT STRUCTURALLY than an undirected
 graph.  The only difference is that the methods below ensure that a
 method done to edge, (i, j), is also done to edge (j, i).
*/

template <class T>
class ungraph : public digraph<T> {
public:
  // initialize a graph with nverts
  explicit ungraph(int nverts);

  bool addEdge(int src, int dst, T const& e);
  bool delEdge(int src, int dst);

  void delAllEdges(int src);  // delete ALL edges IN/OUT to/from src
  void delDeg12Nodes(int src, int dst);

  T& getEdge(int src, int dst); // **** NOT IMPLEMENTED  (NEEDED?) ****

  std::string toDOT(bool label = false) const;  // convert to DOT string
  void calc_node_degree_stats(int& min_degree, int& max_degree,
			      double& avg_degree) const;
protected:
  // NOTE: dangerous to have non-const variant as, for example,
  // something like this: g[i].clear() would ruin the integrity of our
  // undirected graph

  // make inherited non-const variant protected or private
  typename ungraph<T>::umapEdge& operator[] (int src);

  void delEdge(int src, typename digraph<T>::umapEdge::iterator ditr);
};

template <class T>
inline ungraph<T>::ungraph(int nverts)
  : digraph<T>(nverts)
{
}

// add undirected weighted edge between src <-> dst
template <class T>
inline bool ungraph<T>::addEdge(int src, int dst, T const& e)
{
  // NOTE:  using "this" didn't work (segmentation fault)
  //return this->addEdge(src, dst, e) && this->addEdge(dst, src, e);
  return digraph<T>::addEdge(src, dst, e).second
    && digraph<T>::addEdge(dst, src, e).second;
}

// delete undirected weighted edge between src <-> dst
template <class T>
inline bool ungraph<T>::delEdge(int src, int dst)
{
  return digraph<T>::delEdge(src, dst) && digraph<T>::delEdge(dst, src);
}

template <class T>
inline void
ungraph<T>::delEdge(int src, typename digraph<T>::umapEdge::iterator ditr)
{
  // must do this (dst, src) first, then (src, ditr)
  digraph<T>::delEdge(ditr->first, src);
  digraph<T>::delEdge(src, ditr);
}

template <class T>
inline void ungraph<T>::delAllEdges(int src)   // delete ALL IN and OUT edges
{
  digraph<T>::delInEdges(src);
  digraph<T>::delOutEdges(src);
}

template <class T>
inline typename ungraph<T>::umapEdge& ungraph<T>::operator[] (int src)
{
  return digraph<T>::operator[](src);
}

template <class T>
void ungraph<T>::calc_node_degree_stats(int& min_degree, int& max_degree,
					double& avg_degree) const
{
  min_degree = digraph<T>::numVerts();
  max_degree = 0;
  int total_degrees = 0;

  for (int i = 0; i < digraph<T>::numVerts(); i++) {
    int degree = digraph<T>::adj(i).size();
    total_degrees += degree;
    if (degree > max_degree)
      max_degree = degree;
    if (degree < min_degree)
      min_degree = degree;
  }

  avg_degree = total_degrees / (double) digraph<T>::numVerts();
}

#endif // ungraph_hpp
