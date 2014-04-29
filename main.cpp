#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>

#include "strfuncs.hpp"
#include "iw_ungraph.hpp"

using namespace std;

static bool run = false;

// make screen width and height the same (ie square)
static int swidth  = 720;
static int sheight = swidth;

// width x height number of cells
static int width, height;
static int depth = 0;  // keep track of recursion depth (not currently using)

static int debug = 0;  // i.e. no debugging

static long gen = 0;   // generation number

static iw_ungraph *g;
vector<bool> gstate;

typedef struct {
  float r, g, b;
} mycolor_t;

mycolor_t mycolors[256];

vector<int> color_index_shift;

inline void init_color_index_shift()
{
  /*
    Consider the following ASCII Art 3x3 subgrid for the neighbors of
    node, N.  NOTE: width = 2^k + 1, where k is recursive level of
    fractal.


    |-------------|--------------|--------------|
    |             |              |              |
    |  N-1-width  |   N-width    |  N+1-width   |
    |     (2)     |     (1)      |     (0)      |
    |             |              |              |
    |-------------|--------------|--------------|
    |             |              |              |
    |     N-1     |      N       |     N+1      |
    |     (3)     |              |     (7)      |
    |             |              |              |
    |-------------|--------------|--------------|
    |             |              |              |
    |  N-1+width  |   N+width    |  N+1+width   |
    |     (4)     |     (5)      |     (6)      |
    |             |              |              |
    |-------------|--------------|--------------|

    If the upper left cell (N-1-width) is given an index of 0, then
    the neighbor cells are assigned shift values (in a clockwise order
    starting from the right neighbor).  The shift values are indicated
    in parentheses.  These shift values are used to "SHIFT and OR"
    each one bit into an 8-bit color index byte (see get_mycolor_index
    function below).  The color index is used with the mycolors[]
    array to obtain a color which encodes which of 8 possible
    neighbors are true neighbors.
   */

  color_index_shift.resize(2*(width+1)+1, 0);

  color_index_shift[width+2] = 7;     // immed. right cell, go clockwise...
  color_index_shift[2*(width+1)] = 6;
  color_index_shift[2*width+1] = 5;
  color_index_shift[2*width] = 4;
  color_index_shift[width] = 3;
  color_index_shift[0] = 2;
  color_index_shift[1] = 1;
  color_index_shift[2] = 0;
}

int get_mycolor_index(int k)
{
  int color_index = 0;
  if (debug)
    cout << "si = ";
  /*
    For each neighbor, it->first, of node k, compute the difference in
    node number values.  Add width+1 to this difference to yield a
    positive range, to be used as an index into the color_index_shift
    vector.  At most there can be eight neighbors.

    For the simple fractally-inspired sierpinski graph I initially
    created (circa Aug. 2013), there is a maximum of 4 neighbors, with
    the three triangle corner points only having two neighbors.  There
    are eight bits for the color index (256 different color shades).
    Thus, there are 8 choose 4 (equal to 70) different possible shades
    of color.  We could have discretized the color space into 70
    pieces instead of 256 for this sierpinski instance, but I didn't.
   */

  for (iw_ungraph::umapEdge::const_iterator it = g->adj(k).begin();
       it != g->adj(k).end(); ++it) {
    int diff = it->first - k;    // diff range       :  [-(width+1), width+1]
    int si = diff + width + 1;   // shift index range:  [0, 2*(width+1)]

    if (debug)
      cout << si << " ";

    color_index |= (1 << color_index_shift[si]);
  }
  if (debug)
    cout << "\n";
  return color_index;
}

void init_mycolors()
{
  vector<bool> colors(256, false);

  for (int k = 0; k < g->numVerts(); k++) {      // for each cell
    if (g->adj(k).size() > 0) {                  // if cell has neighbors
      int ci = get_mycolor_index(k);
      colors[ci] = true;
    }
  }

  int count = 0;
  for (size_t i = 0; i < colors.size(); i++) {
    if (colors[i])
      count++;
  }

  cout << "number of colors = " << count << "\n";

  float dgreen = 1.0/(count + 1.0);
  float green = dgreen;

  for (size_t i = 0; i < colors.size(); i++) {
    mycolors[i].r = mycolors[i].g = mycolors[i].b = 0.0;

    if (colors[i]) {
      mycolors[i].g = green;
      green += dgreen;
    }
  }
}

inline void set_mycolor(int k)
{
  int ci = get_mycolor_index(k);

  glColor3f(mycolors[ci].r, mycolors[ci].g, mycolors[ci].b);
}

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
//Modifications to be made:
//Mod4
void build_sierpinski_graph(int a, int b, int c)
{
  depth++;

  if (debug)
    cerr << "a = " << a << " b = " << b << " c = " << c << "\n";

  int ab = midpoint(a, b);
  int bc = midpoint(b, c);
  int ac = midpoint(a, c);

  //new ints for the other midpoints on the outer edge
  int ab1 = midpoint(a, ab);
  int ab2 = midpoint(b, ab);
  int bc1 = midpoint(b, bc);
  int bc2 = midpoint(c, bc);
  int ac1 = midpoint(a, ac);
  int ac2 = midpoint(c, ac);

  //new ints for the midpoints inside the triangle 
  int top = midpoint(ab, ac);
  int left = midpoint(ab, bc);
  int right = midpoint(ab, ac);

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

  //new triangles (mod4)
  build_sierpinski_graph(ab1, ab, top);
  build_sierpinski_graph(ac1, ac, top);
  build_sierpinski_graph(top, left, right);
  build_sierpinski_graph(ab2, ab, left);
  build_sierpinski_graph(ac, ac2, right);
  build_sierpinski_graph(bc1, bc, left);
  build_sierpinski_graph(bc2, bc, right);

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
  if (++gen % 100 == 0)
    cout << "\tgeneration = " << gen << "\n";

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

	 if (gstate[k]) {
	   set_mycolor(k);   // draw live cells color for neighborhood
	 }
	 else
	   glColor3f(1.0, 0.0, 0.0);  // draw dead cells in red

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

void timer_func(int value)
{
  if (run) {
    apply_rule_to_all_cells();
    glutPostRedisplay();
  }
  glutTimerFunc(200, timer_func, 0);
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
      run = !run;
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

  width = mypow2(k * 4) + 1; //4 for mod4
  height = width;

  if (argc == 3) {
    debug = str2num<int>(argv[2]);
  }

  g = new iw_ungraph(width*height);

  gstate.resize(width*height, false);

  init_color_index_shift();

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
  glutTimerFunc(500, timer_func, 0);  // last parameter not used for me
  glutReshapeFunc(reshape);  // called when window is resized

  myinit();                  // initialize viewing and modeling paremeters

  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);

  init_mycolors();

  glutMainLoop();            // enter event loop

  delete g;

  return 0;
}
