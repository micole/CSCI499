#include <iostream>
#include <deque>
#include <vector>
#include <string>

#include "strfuncs.hpp"
#include "digraph.hpp"

using std::ostream;
using std::vector;
using std::deque;
using std::string;

template <class T>
void digraph<T>::bfs(int src, vector<int>& parent) const
{
  parent.clear();
  parent.resize(numVerts(), -1);   // size = num verts and init to -1

  deque<int> q;
  vector<bool> visited(numVerts(), false);  // initially all not visited

  q.push_back(src);   // start off with src vertex
  visited.AT(src) = true;
  while (!q.empty()) {
    int v = q.front();
    q.pop_front();

    // for each vertex, w, adjacent to vertex v do...
    for (typename digraph<T>::umapEdge::const_iterator it = adj(v).begin();
	 it != adj(v).end(); ++it) {
      int w = it->first;
      if (!visited.AT(w)) {
	visited.AT(w) = true;
	parent.AT(w) = v;
	q.push_back(w);
      }
    }
  }
}

template <class T>
string digraph<T>::toXML() const
{
  string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n\
<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" \n\
   xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \n\
   xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns \n\
   http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";

  res += "  <graph id=\"G\" edgedefault=\"undirected\">\n";

  for (int v = 0; v < numVerts(); v++) {
    res += "    <node id=\"n" + num2str<int>(v) + "\"/>\n";
  }

  for (int v = 0; v < numVerts(); v++) {
    for (typename digraph<T>::umapEdge::const_iterator it = adj(v).begin();
	 it != adj(v).end(); ++it) {
      int id = it->first;
      res += "    <edge source=\"n" + num2str<int>(v)
	+ "\" target=\"n" + num2str<int>(id) + "\"/>\n";
    }
  }

  res += "  </graph>\n";
  res += "</graphml>\n";

  return res;
}

template <class T>
string digraph<T>::toDIMACS(int src, int dst) const
{
  // use:  dot -Tsvg dog.dot >dog.svg
  string res = "c toDIMACS()\n";
  res += "p max     " + num2str<int>(numVerts()) + "    "
    + num2str<int>(numEdges()) + "\n";
  res += "n         " + num2str<int>(src+1) + "  s\n";
  res += "n         " + num2str<int>(dst+1) + "  t\n";

  for (int v = 0; v < numVerts(); v++) {
    for (typename digraph<T>::umapEdge::const_iterator it = adj(v).begin();
	 it != adj(v).end(); ++it) {
      int id = it->first;
      T e(it->second);   // weight

      res += "a       " + num2str<int>(v+1) + "       " + num2str<int>(id+1)
	+ "       " + num2str<T>(e) + "\n";
    }
  }

  return res;
}

template <class T>
string digraph<T>::toAdjMat() const
{
  string res;

  if (numVerts() < 1)   // after this point, we have at least a 1 x 1 matrix
    return res;

  res = "[";
  for (int v = 0; v < numVerts(); v++) {
    string prefix = (v == 0) ? "[" : ",\n[";
    typename digraph<T>::umapEdge::const_iterator it = findEdge(v, 0);
    T val = (it != adj(v).end()) ? it->second : T(-1);
    res += (prefix + num2str<T>(val));

    for (int w = 1; w < numVerts(); w++) {
      typename digraph<T>::umapEdge::const_iterator it = findEdge(v, w);
      T val = (it != adj(v).end()) ? it->second : T(-1);
      res += ("," + num2str<T>(val));
    }
    res += ']';
  }
  res += "]\n";

  return res;
}

// explicit member function template instantiation for ints
template void digraph<int>::bfs(int src, vector<int>& parent) const;
template string digraph<int>::toDIMACS(int src, int dst) const;
template string digraph<int>::toAdjMat() const;
