/*
 * Pyro Screensaver for XBox Media Center
 * Copyright (c) 2004 Team XBMC
 *
 * Ver 1.0 15 Nov 2004	Chris Barnett (Forza)
 *
 * Adapted from the Pyro screen saver by
 *
 *  Jamie Zawinski <jwz@jwz.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "xbmc/xbmc_scr_dll.h"
#include <GL/gl.h>

extern "C" {

#include "Pyro.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////
// XBMC has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;

  SCR_PROPS* scrprops = (SCR_PROPS*)props;

  m_iWidth = scrprops->width;
  m_iHeight = scrprops->height;

  return ADDON_STATUS_OK;
}

extern "C" void Start()
{
	int i;

	how_many = 1000;
	frequency = 5;
	scatter = 20;

	projectiles = 0;
	free_projectiles = 0;
	projectiles = (struct projectile *) calloc(how_many, sizeof (struct projectile));
	for (i = 0; i < how_many; i++)
		free_projectile (&projectiles [i]);
}

static int myrand()
{
  return (rand() << 15) + rand();
}

extern "C" void Render()
{
	static int xlim, ylim, real_xlim, real_ylim;
	int g = 100;
	int i;

	if ((myrand() % frequency) == 0)
	{
		real_xlim = m_iWidth;
		real_ylim = m_iHeight;
		xlim = real_xlim * 1000;
		ylim = real_ylim * 1000;
		launch (xlim, ylim, g);
	}

	for (i = 0; i < how_many; i++)
	{
		struct projectile *p = &projectiles [i];
		int old_x, old_y, old_size;
		int size, x, y;
		if (p->dead) continue;
		old_x = p->x >> 10;
		old_y = p->y >> 10;
		old_size = p->size >> 10;
		size = (p->size += p->decay) >> 10;
		x = (p->x += p->dx) >> 10;
		y = (p->y += p->dy) >> 10;
		p->dy += (p->size >> 6);
		if (p->primary) p->fuse--;

		if ((p->primary ? (p->fuse > 0) : (p->size > 0)) &&
		 x < real_xlim &&
		 y < real_ylim &&
		 x > 0 &&
		 y > 0)
		{
			DrawRectangle(x, y, size, size, p->colour);
		}
		else
		{
			free_projectile (p);
		}

		if (p->primary && p->fuse <= 0)
		{
			int j = (myrand() % scatter) + (scatter/2);
			while (j-- > 0)	shrapnel(p);
		}
	}
}


extern "C" void Stop()
{
	free(projectiles);
}

static struct projectile *get_projectile (void)
{
	struct projectile *p;

	if (free_projectiles)
	{
		p = free_projectiles;
		free_projectiles = p->next_free;
		p->next_free = 0;
		p->dead = false;
		return p;
	}
	else
		return 0;
}

static void free_projectile (struct projectile *p)
{
	p->next_free = free_projectiles;
	free_projectiles = p;
	p->dead = true;
}

static void launch (int xlim, int ylim, int g)
{
	struct projectile *p = get_projectile ();
	int x, dx, xxx;
	double red, green, blue;
	if (! p) return;

	do {
		x = (myrand() % xlim);
		dx = 30000 - (myrand() % 60000);
		xxx = x + (dx * 200);
	} while (xxx <= 0 || xxx >= xlim);

	p->x = x;
	p->y = ylim;
	p->dx = dx;
	p->size = 20000;
	p->decay = 0;
	p->dy = (myrand() % 10000) - 20000;
	p->fuse = ((((myrand() % 500) + 500) * abs (p->dy / g)) / 1000);
	p->primary = true;

	hsv_to_rgb ((double)(myrand() % 360)/360.0, 1.0, 255.0,	&red, &green, &blue);
	p->colour[0] = red;
        p->colour[1] = green;
        p->colour[2] = blue;
        p->colour[3] = 1.0;
}

static struct projectile *shrapnel(struct projectile *parent)
{
	struct projectile *p = get_projectile ();
	if (! p) return 0;

	p->x = parent->x;
	p->y = parent->y;
	p->dx = (myrand() % 5000) - 2500 + parent->dx;
	p->dy = (myrand() % 5000) - 2500 + parent->dy;
	p->decay = (myrand() % 50) - 60;
	p->size = (parent->size * 2) / 3;
	p->fuse = 0;
	p->primary = false;
	p->colour[0] = parent->colour[0];
	p->colour[1] = parent->colour[1];
	p->colour[2] = parent->colour[2];
	p->colour[3] = parent->colour[3];
	return p;
}

void DrawRectangle(int x, int y, int w, int h, float* dwColour)
{
    //Store each point of the triangle together with it's colour
    MYCUSTOMVERTEX cvVertices[] =
    {
        {(float) x, (float) y+h, 0.0f, 1.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
        {(float) x, (float) y, 0.0f, 1.0f,  dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
	{(float) x+w, (float) y, 0.0f, 1.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
        {(float) x+w, (float) y+h, 0.0f, 1.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]}
    };

    glBegin(GL_QUADS);
    for (size_t j=0;j<4;++j)
    {
      glColor4f(cvVertices[j].r/255.0, cvVertices[j].g/255.0, cvVertices[j].b/255.0, 1.0);
      glVertex2f(cvVertices[j].x, cvVertices[j].y);
    }
    glEnd();
}


void hsv_to_rgb (double hue, double saturation, double value, 
		 double *red, double *green, double *blue)
{
  double f, p, q, t;

  if (saturation == 0.0)
    {
      *red   = value;
      *green = value;
      *blue  = value;
    }
  else
    {
      hue *= 6.0; // 0 -> 360 * 6

      if (hue == 6.0)
        hue = 0.0;

      f = hue - (int) hue;
      p = value * (1.0 - saturation);
      q = value * (1.0 - saturation * f);
      t = value * (1.0 - saturation * (1.0 - f));

      switch ((int) hue)
        {
        case 0:
          *red = value;
          *green = t;
          *blue = p;
          break;

        case 1:
          *red = q;
          *green = value;
          *blue = p;
          break;

        case 2:
          *red = p;
          *green = value;
          *blue = t;
          break;

        case 3:
          *red = p;
          *green = q;
          *blue = value;
          break;
        case 4:
          *red = t;
          *green = p;
          *blue = value;
          break;

        case 5:
          *red = value;
          *green = p;
          *blue = q;
          break;
        }
    }
}

// XBMC tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
extern "C" void ADDON_Stop()
{
}

void ADDON_Destroy()
{
}

ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

bool ADDON_HasSettings()
{
  return false;
}

unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void *value)
{
  return ADDON_STATUS_OK;
}

void ADDON_FreeSettings()
{
}

void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

void GetInfo(SCR_INFO *info)
{
}

};
