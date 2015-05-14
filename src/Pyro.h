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

static struct projectile *projectiles, *free_projectiles;
static unsigned int default_fg_pixel;
static int how_many, frequency, scatter;

static struct projectile *get_projectile(void);
static void free_projectile(struct projectile *p);
static void launch(int xlim, int ylim, int g);
static struct projectile *shrapnel(struct projectile *parent);

struct MYCUSTOMVERTEX
{
	float x, y, z; // The transformed position for the vertex.
    float r, g, b, a; // The vertex colour.
};

//
#ifdef WIN32
void InitDXStuff(void);
#endif
void hsv_to_rgb(double hue, double saturation, double value, double *red, double *green, double *blue);
void DrawRectangle(int x, int y, int w, int h, float* dwColour);

int	m_iWidth;
int m_iHeight;
