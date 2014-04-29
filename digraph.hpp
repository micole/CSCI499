#ifndef digraph_hpp
#define digraph_hpp

//#define HASH_STATS

//#define USE_MY_HASH

#ifdef HASH_STATS
#include <iostream>
#else
#include <iosfwd>
#endif

#include <vector>
#include <unordered_map>

// AT() used for bounds checking when debugging
#ifdef _DEBUG
#include <stdexcept>   // for throwing exception in addEdge() below
#define AT(x) at(x)
#else
#define AT(x) operator[](x)
#endif

#ifdef USE_INT_HASH
class intHash {
public:
  size_t operator()(const int& k) const {
    return k;
  }
};
#endif // USE_INT_HASH

template <class T>
class edgeContainer {
public:
#ifdef USE_INT_HASH
  typedef std::unordered_map<int, T, intHash> intTmap_t;
#else
  typedef std::unordered_map<int, T> intTmap_t;
#endif // USE_INT_HASH
  typedef typename intTmap_t::iterator iterator;
  typedef typename intTmap_t::const_iterator const_iterator;
  typedef typename std::pair<int, T> value_type;

  edgeContainer();
  ~edgeContainer();

  iterator       begin()       { return m_ec.begin(); }
  const_iterator begin() const { return m_ec.begin(); }
  iterator       end()         { return m_ec.end(); }
  const_iterator end()   const { return m_ec.end(); }

  size_t size() const                { return m_ec.size(); }
  void   erase (const_iterator it)   { m_ec.erase(it); }
  bool   erase (int key)             { return m_ec.erase(key) != 0; }
  std::pair<iterator, bool> insert(value_type const& v)
       { return m_ec.insert(v); }
  T& operator[] (int key)            { return m_ec[key]; }

  const_iterator find(int key) const { return m_ec.find(key); }
  iterator find(int key) { return m_ec.find(key); }

  void clear () { m_ec.clear(); }

  size_t bucket_count() const { return m_ec.bucket_count(); }
  void rehash(size_t nbuckets) { m_ec.rehash(nbuckets); }
  void reserve(size_t nbuckets) { m_ec.reserve(nbuckets); }
  float load_factor() const { return m_ec.load_factor(); }
private:
  // NOTE:  intHash didn't seem to be any more efficient than std::Hash<int>
  intTmap_t m_ec;
  // NOTE:  pair <int, T> consists of <adjacent vertex index, edge information>
  //        the edge information, in simplest case (int), is a WEIGHT
};

template <class T>
inline edgeContainer<T>::edgeContainer()
/*
  : m_ec(7, std::hash<int>(), std::equal_to<int>(),
	 std::allocator<std::pair<int, T> >())
*/
//  : m_ec(31) // number of buckets (I think 7 or 11 is the default)
  : m_ec()
{
}

template <class T>
inline edgeContainer<T>::~edgeContainer()
{
#ifdef HASH_STATS
  static int count;

  std::cout << "number of buckets[" << count << "] = "
	    << m_ec.bucket_count() << "\n";
  std::cout << "load factor[" << count << "] = "
	    << m_ec.load_factor() << "\n";
  for (unsigned int i = 0; i < m_ec.bucket_count(); i++) {
    std::cout << "\t" << m_ec.bucket_size(i) << "\n";
  }
  count++;
#endif // HASH_STATS
}

// T is our edge data (possibly a class)
template <class T>
class digraph {
public:
  // emapEdge is a map of (destination, T)
  typedef edgeContainer<T> umapEdge;
  // next two for notational convenience in code that follows
  typedef typename umapEdge::iterator iterator;
  typedef typename umapEdge::const_iterator const_iterator;
  
  explicit digraph(int nverts);     // construct a digraph with nverts
  digraph(digraph const& g);        // copy constructor
  digraph(std::vector<int> const& parent);  // create from parent tree array

  void clear();               // clear out the digraph (vertices)
  int  resize(int nverts);    // resize the digraph (only larger)
  int  numVerts() const;      // return number of vertices
  int  numEdges() const;      // return number of edges

  // return index of minmimum outdegree node excluding, exclude_list[] nodes
  int  minOutDegreeIndex(std::vector<bool> const& include_set) const;

  // add, del, or find directed edge from src to dst (with weight w)
  std::pair<iterator, bool> addEdge(int src, int dst, T const& e);
  bool delEdge(int src, int dst);

  // returns iterator to edge or end()
  // NOTE: C++ requires one to indicate which dependent names denote
  //   types by using the typename keyword
  const_iterator findEdge(int src, int dst) const;

  umapEdge const& adj(int src) const; // const ref to edge list for src
  umapEdge const& operator[] (int src) const;  // const variant
  umapEdge&       operator[] (int src);        // non-const variant

  // maybe good to have (u, v) operator instead/in addition to above?

  void delInEdges(int src);   // delete ALL IN directed edges to src
  void delOutEdges(int src);  // delete ALL OUT emanating edges from src
  //void zeroOutEdges(int src); // zeros ALL OUT emanating edges from src

  int addVertex(int v);

  // simple graph algorithms
  void bfs(int src, std::vector<int>& parent) const; // breadth-first search
  void appendGraph(digraph<T> const& g);      // append g to this graph

  // object-to-string conversion routines  (useful for output)
  std::string toXML() const;            // convert to XML string

  int  getNumBuckets(int src) const { return adj(src).bucket_count(); }
  void setNumBuckets(int src, int nbuckets) { (*this)[src].rehash(nbuckets); }
  float load_factor(int src) const { return adj(src).load_factor(); }

  std::string toDIMACS(int src, int dst) const; // convert to DIMACS string
  std::string toDOT(bool label = false) const;  // convert to DOT string
  std::string toAdjMat() const;         // convert to Adjacency Matrix string
protected:
  void delEdge(int src, const_iterator ditr);
  /*
   NOTE: may want to use a vector of umapEdge POINTERS !!  This way we
   could move around without actually copying.  Would be dynamic, now,
   however, and will need constructors/destructors.
  */
  std::vector<umapEdge> m_adj;    // adjacency unordered map of edges
};

template <class T>
inline digraph<T>::digraph(int nverts)
  : m_adj(nverts)
{
}

template <class T>
inline digraph<T>::digraph(digraph const& g)
  : m_adj(g.m_adj)
{
  // following doesn't seem necessary:
  //for (int v = 0; v < numVerts(); v++)
  //  m_adj[v].reserve(g.adj(v).size());
}

// build a graph from a tree (represented with parent array)
template <class T>
inline digraph<T>::digraph(std::vector<int> const& parent)
  : m_adj(parent.size())
{
  for (unsigned int dst = 0; dst < parent.size(); dst++) {
    int src = parent[dst];
    if (src != -1 && src < (int) parent.size()) {
      // insert new directed edge:  src -> dst
      (*this).addEdge(src, dst, T());
    }
  }
}

template <class T>
inline void digraph<T>::clear()
{
  m_adj.clear();
}

template <class T>
inline int digraph<T>::resize(int nverts)
{
  if (nverts < numVerts())
    return 0;    // smaller digraph invalidates existing adjacency list

  m_adj.resize(nverts);
  return 1;
}

template <class T>
inline int digraph<T>::numVerts() const
{
  return m_adj.size();
}

template <class T>
inline int digraph<T>::numEdges() const
{
  int m = 0;
  for (int v = 0; v < numVerts(); v++) {
    m += adj(v).size();
  }
  return m;
}

// include_set is (should be) the same size as m_adj[]
template <class T>
inline int digraph<T>::minOutDegreeIndex(std::vector<bool> const& include_set)
  const
{
  unsigned int m = numVerts();      // minimum out degree
  int ndx = -1;                     // minimum out degree index

  for (unsigned int v = 0; v < include_set.size(); v++) {
    if (include_set[v] && adj(v).size() < m) {
      m = adj(v).size();
      ndx = v;
    }
  }
  return ndx;
}

// return iterator to edge if successful; otherwise adj(src).end()
template <class T>
inline typename digraph<T>::const_iterator
digraph<T>::findEdge(int src, int dst) const
{
  return adj(src).find(dst);
}

template <class T>
inline typename digraph<T>::umapEdge const& digraph<T>::adj(int src) const
{
  return m_adj.AT(src);
}

template <class T>
inline typename digraph<T>::umapEdge const& digraph<T>::operator[] (int src)
  const
{
  return m_adj.AT(src);
}

// non-const variant
template <class T>
inline typename digraph<T>::umapEdge& digraph<T>::operator[] (int src)
{
  return m_adj.AT(src);
}

// add weighted directed edge from src -> dst:
//   return true if successful; otherwise false
template <class T>
inline std::pair<typename digraph<T>::iterator, bool>
digraph<T>::addEdge(int src, int dst, T const& e)
{
#ifdef _DEBUG
  // AT() doesn't operate on dst for maps.  So, explicitly check for
  // overflow and and throw if necessary
  if (dst >= numVerts())
    throw std::out_of_range("addEdge() dst");
#endif

  // insert new weighted directed edge:  src -> dst
  return (*this)[src].insert(typename digraph<T>::umapEdge::value_type(dst, e));
}

// add a vertex with adjacent edges
template <class T>
inline int digraph<T>::addVertex(int v)
{
  /*
  int pos = numVerts();
  umapEdge pig;
  pig.reserve(adj(v).bucket_count());
  std::cout << "pos = " << pos << "\n";
  std::cout << "before vector capacity = " << m_adj.capacity() << "\n";
  m_adj.push_back(pig);
  std::cout << "after vector capacity = " << m_adj.capacity() << "\n";
  for (typename digraph<T>::const_iterator it = adj(v).begin();
       it != adj(v).end(); ++it) {
    std::cout << "inserting edge (" << pos << ", " << it->first
	      << ") with " << it->second << "\n";
    addEdge(pos, it->first, it->second);
  }
  */
  m_adj.push_back(adj(v));
  return numVerts() - 1;   // return position where added
}

// delete directed edge from src -> dst:
//   return true if successful; otherwise false
template <class T>
inline bool digraph<T>::delEdge(int src, int dst)
{
  return (*this)[src].erase(dst) != 0;  // equal_range one element at most
}

template <class T>
inline void
digraph<T>::delEdge(int src, typename digraph<T>::const_iterator ditr)
{
  (*this)[src].erase(ditr);  // equal_range one element at most
}

template <class T>
inline void digraph<T>::delInEdges(int src)   // delete ALL IN directed edges
{
  for (typename digraph<T>::const_iterator it = adj(src).begin();
       it != adj(src).end(); ++it) {
    delEdge(it->first, src);
  }
}

template <class T>
inline void digraph<T>::delOutEdges(int src)   // delete ALL OUT emanating edges
{
  (*this)[src].clear();
}

// CURRENTLY THIS NEXT ROUTINE NOT USED ANYWHERE
/*
template <class T>
inline void digraph<T>::zeroOutEdges(int src)  // zeroes ALL OUT emanating edges
{
  for (typename digraph<T>::iterator it = (*this)[src].begin();
       it != (*this)[src].end(); ++it) {
    it->second = T(0);
  }
}
*/

template <class T>
inline void digraph<T>::appendGraph(digraph<T> const& g)
{
  const int n = g.numVerts();   // can't use size in loop because growing
  for (int v = 0; v < n; v++) {
    m_adj.push_back(g.adj(v));
  }
}

#endif // digraph_hpp
