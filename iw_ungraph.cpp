// iw_ungraph stands for integer weighted undirected graph

#include "strfuncs.hpp"
#include "iw_ungraph.hpp"

using std::string;

// undefine next to experiment with producing better looking graphs:
//#define EXPDOT

string iw_ungraph::toDOT(bool label) const
{
  // use:  sfdp -Tsvg pig.dot >pig.svg  (for large graphs)
  // or
  // use:  neato -Tsvg pig.dot >pig.svg
  string res = "graph graphname {\n";

  res += "   overlap=\"false\";\n";   // don't overlap nodes
#ifdef EXPDOT
  res += "   mode=\"major\";\n";      // use stress majorization
  res += "   splines=\"true\";\n";    // allow curved edges (arcs)
  res += "   start=\"3\";\n";         // different random seed (init placement)
  res += "   epsilon=\"0.0001\";\n";  // cutoff for the solver (def:  0.1)
#endif

  for (int v = 0; v < numVerts(); v++) {
    iw_ungraph::umapEdge::const_iterator it;
    for (it = adj(v).begin(); it != adj(v).end(); ++it) {
      int w = it->first;
      int w_weight = it->second;
      if (w > v) {
	res += "   " + num2str<int>(v) + " -- " + num2str<int>(w)
	  + " [weight=" + num2str<int>(w_weight) + "];\n";
      }
    }
  }
  res += "}\n";

  return res;
}
