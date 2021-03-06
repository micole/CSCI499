#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>

#include "strfuncs.hpp"
#include "iw_ungraph.hpp"

using namespace std;

// make screen width and height the same (ie square)
static int swidth  = 800;
static int sheight = 800;

// width x height number of cells
static int width, height;
static int depth = 0;  // keep track of recursion depth (not really using)

static int debug = 0;  // i.e. no debugging

static iw_ungraph *g;
vector<bool> gstate;

inline int mypow2(int k)
{
  int res = 1;
  for (int i = 0; i < k; i++) {
    res *= 2;
  }

  return res;
}

inline int midpoint(int a, int b)
{
  // NOTE: a/width is the row of a;  a%width is the col of a
  return (a/width+b/width)/2*width+(a%width+b%width)/2;
}

inline void tri_connect(int a, int b, int c)
{
  g->addEdge(a, b, depth);
  g->addEdge(b, c, depth);
  g->addEdge(c, a, depth);
}

void build_sierpinski_graph(int a, int b, int c)
{
  depth++;

  if (debug)
    cerr << "a = " << a << " b = " << b << " c = " << c << "\n";

  int ab = midpoint(a, b);
  int bc = midpoint(b, c);
  int ac = midpoint(a, c);

  if (ab > a && ac > ab) {
    if (debug)
      cerr << "  a = " << a << " ab = " << ab << " ac = " << ac << "\n";

    build_sierpinski_graph(a, ab, ac);
  }
  else
    tri_connect(a, b, c);

  if (ab > a && b > ab && bc > b) {
    if (debug)
      cerr << "  ab = " << ab << " b = " << b << " bc = " << bc << "\n";

    build_sierpinski_graph(ab, b, bc);
  }
  else
    tri_connect(a, b, c);

  if (ac > a && bc > ac && c > bc) {
    if (debug)
      cerr << "  ac = " << ac << " bc = " << bc << " c = " << c << "\n";

    build_sierpinski_graph(ac, bc, c);
  }
  else
    tri_connect(a, b, c);

  depth--;
}

/*
  Following are game of life rules (with 8 neighbors on a grid):

    Any live cell with fewer than two live neighbours dies, as if
    caused by under-population.

    Any live cell with two or three live neighbours lives on to the
    next generation.

    Any live cell with more than three live neighbours dies, as if by
    overcrowding.

    Any dead cell with exactly three live neighbours becomes a live
    cell, as if by reproduction.
 */

bool next_state_rule(int k)
{
  // Here is a "first crack" at a rule for each cell, k, of our fractal CA

  // first count the number of live neighbors to node k
  int live_neighs = 0;
  for (iw_ungraph::umapEdge::const_iterator it = g->adj(k).begin();
       it != g->adj(k).end(); ++it) {
    live_neighs += gstate[it->first];
  }

  if (debug)
    cerr << "live_neighbors = " << live_neighs << "\n";

  if (gstate[k]) {  // if k is a live cell
    if (live_neighs == 1 || live_neighs == 2)
      return true;  // the cell continues to live if 1 or 2 neighbors
  }
  else {            // if k is a dead cell
    if (live_neighs == 2)
      return true;  // the cell becomes alive if exactly 2 neighbors
  }

  return false;  // otherwise, the cell dies
}

void apply_rule_to_all_cells()
{
  vector<bool> new_gstate(width*height, false);  // new vector of next states

  for (int k = 0; k < g->numVerts(); k++) {      // for each cell
    if (g->adj(k).size() > 0) {                  // if cell has neighbors
      new_gstate[k] = next_state_rule(k);        // compute next cel state
    }
  } // end for each vertex loop

  gstate = new_gstate;  // copy in "one fell swoop" into gstate vector
}

void myinit()
{
  glClearColor(1.0, 1.0, 1.0, 0.0); // white opaque background

  // set up viewing
  // swidth x sheight window with origin lower left
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) swidth, 0.0, (GLdouble) sheight);
  glMatrixMode(GL_MODELVIEW);       // put back into modelling mode
}

void reshape (int w, int h)
{
  // maintain a square window
  swidth = (w > h) ? w : h;  // choose the larger of width, height
  sheight = swidth;

  glViewport(0, 0, (GLsizei) swidth, (GLsizei) sheight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) swidth, 0.0, (GLdouble) sheight);
}

void drawgrid()
{
  const double cellwidth = (double) swidth / width;
  const double cellheight = (double) sheight / height;

  glColor3f(0.3, 0.3, 0.3);      // draw in light grey
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // outline polys, not filled

  for (int i = 0; i < height; i++) {
    double y = i*cellheight;

    glBegin(GL_QUAD_STRIP);
    for (int j = 0; j <= width; j++) {
      double x = j*cellwidth;

      glVertex2f(x, y);
      glVertex2f(x, y+cellwidth);
    }
    glEnd();
  }
}

void drawgraph()
{
  const double cellwidth = (double) swidth / width;
  const double cellheight = (double) sheight / height;

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // filled polygon

  for (int k = 0; k < g->numVerts(); k++) {
    if (g->adj(k).size() > 0) {
      glBegin(GL_POLYGON);
         int i = k / height;
	 int j = k % height;

	 double x = j*cellwidth;
	 double y = i*cellheight;

	 if (gstate[k])
	   glColor3f(0.0, 1.0, 0.0);  // draw live cells in green
	 else
	   glColor3f(1.0, 0.0, 0.0);  // draw dead cells in red (blood)

         glVertex2f(x, y);
         glVertex2f(x+cellwidth, y);
	 glVertex2f(x+cellwidth, y+cellheight);
	 glVertex2f(x, y+cellheight);
      glEnd();
    }
  }
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);  // clear the window

  drawgrid();           // draw the grid

  drawgraph();          // draw the graph

  glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
  // have these two constants in three places, consolidate !

  const double cellwidth = (double) swidth / width;
  const double cellheight = (double) sheight / height;

  y = sheight - y;   // (0, 0) is upper left for glut and lower right for GL

  switch (button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
      apply_rule_to_all_cells();
      glutPostRedisplay();
    }
    break;
  case GLUT_MIDDLE_BUTTON:
    if (state == GLUT_DOWN) {
      // not currently using
    }
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN) {
      int row = y / cellwidth;
      int col = x / cellheight;
      int k = row*width+col;

      cerr << "toggling state of node number " << k << "\n";

      // toggle the state (live/dead)
      gstate[k] = !gstate[k];
      glutPostRedisplay();
    }
    break;
  default:
    break;
  }
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case 27:  //  Escape key
    exit(0);
  case 'c':
  case 'C':
    fill(gstate.begin(), gstate.end(), false);  // clear the state
    glutPostRedisplay();
  default:
    break;
  }
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    cerr << "USAGE: " << argv[0] << " k [debug]\n";
    cerr << "NOTE : width = height = 2^k+1\n";
    return 1;
  }

  int k = str2num<int>(argv[1]);
  if (k < 1 || k > 10) {
    cerr << "k must be an integer in range [1, 10]\n";
    return 2;
  }

  width = mypow2(k) + 1;
  height = width;

  if (argc == 3) {
    debug = str2num<int>(argv[2]);
  }

  g = new iw_ungraph(width*height);

  gstate.resize(width*height, false);

  // top middle, bottom left, bottom right vertices of a triangle
  int a = (width-1) / 2;
  int b = (height-1)*width;
  int c = height*width - 1;

  build_sierpinski_graph(a, b, c);

  string gstr = g->toDOT();

  cout << gstr;

  if (debug) {
    for (int i = 0; i < g->numVerts(); i++) {
      if (g->adj(i).size() > 0) {
	cerr << "vertex " << i << " degree is " << g->adj(i).size() << "\n";
      }
    }
  }

  cerr << "Two adjacent nodes on the grid DOES NOT imply these ";
  cerr << "two nodes are neighbors\n";

  // Standard GLUT initialization
  glutInit(&argc,argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(swidth, sheight); // swidth x sheight pixel window
  glutInitWindowPosition(0,0);         // place window top left on display
  glutCreateWindow("Fractal CA");      // window title */
  glutDisplayFunc(display);  // display callback invoked when window opened
  glutReshapeFunc(reshape);  // called when window is resized

  myinit();                  // initialize viewing and modeling paremeters

  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);

  glutMainLoop();            // enter event loop

  delete g;

  return 0;
}
