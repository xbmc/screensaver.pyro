// include file for screensaver template

struct projectile {
  int x, y;	/* position */
  int dx, dy;	/* velocity */
  int decay;
  int size;
  int fuse;
  bool primary;
  bool dead;
  //XColor color;
  //D3DCOLORVALUE colour;
  float colour[4];
  struct projectile *next_free;
};

struct MYCUSTOMVERTEX
{
  float x, y, z; // The transformed position for the vertex.
  float r, g, b, a; // The vertex colour.
};
