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

#include "xbmc_scr_dll.h"
#include <GL/gl.h>
#ifdef WIN32
#include <d3d11.h>
#endif

extern "C" {

#include "Pyro.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32

#define SAFE_RELEASE(_p)		{ if(_p) { _p->Release();	_p=NULL; } }

ID3D11DeviceContext* g_pContext = nullptr;
ID3D11Buffer*        g_pVBuffer = nullptr;
ID3D11PixelShader*   g_pPShader = nullptr;

const BYTE PixelShader[] =
{
     68,  88,  66,  67,  18, 124, 
    182,  35,  30, 142, 196, 211, 
     95, 130,  91, 204,  99,  13, 
    249,   8,   1,   0,   0,   0, 
    124,   1,   0,   0,   4,   0, 
      0,   0,  48,   0,   0,   0, 
    124,   0,   0,   0, 188,   0, 
      0,   0,  72,   1,   0,   0, 
     65, 111, 110,  57,  68,   0, 
      0,   0,  68,   0,   0,   0, 
      0,   2, 255, 255,  32,   0, 
      0,   0,  36,   0,   0,   0, 
      0,   0,  36,   0,   0,   0, 
     36,   0,   0,   0,  36,   0, 
      0,   0,  36,   0,   0,   0, 
     36,   0,   0,   2, 255, 255, 
     31,   0,   0,   2,   0,   0, 
      0, 128,   0,   0,  15, 176, 
      1,   0,   0,   2,   0,   8, 
     15, 128,   0,   0, 228, 176, 
    255, 255,   0,   0,  83,  72, 
     68,  82,  56,   0,   0,   0, 
     64,   0,   0,   0,  14,   0, 
      0,   0,  98,  16,   0,   3, 
    242,  16,  16,   0,   1,   0, 
      0,   0, 101,   0,   0,   3, 
    242,  32,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
    242,  32,  16,   0,   0,   0, 
      0,   0,  70,  30,  16,   0, 
      1,   0,   0,   0,  62,   0, 
      0,   1,  73,  83,  71,  78, 
    132,   0,   0,   0,   4,   0, 
      0,   0,   8,   0,   0,   0, 
    104,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
    116,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  15,  15,   0,   0, 
    122,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
    122,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,  12,   0,   0,   0, 
     83,  86,  95,  80,  79,  83, 
     73,  84,  73,  79,  78,   0, 
     67,  79,  76,  79,  82,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171,  79,  83, 
     71,  78,  44,   0,   0,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  83,  86,  95,  84, 
     65,  82,  71,  69,  84,   0, 
    171, 171
};

void InitDXStuff(void)
{
  ID3D11Device* pDevice = nullptr;
  g_pContext->GetDevice(&pDevice);

  CD3D11_BUFFER_DESC vbDesc(sizeof(MYCUSTOMVERTEX) * 5, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
  pDevice->CreateBuffer(&vbDesc, nullptr, &g_pVBuffer);

  pDevice->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, &g_pPShader);

  SAFE_RELEASE(pDevice);
}

#endif // WIN32

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
#ifdef WIN32
  g_pContext = reinterpret_cast<ID3D11DeviceContext*>(scrprops->device);
  InitDXStuff();
#endif
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
#ifdef WIN32
  g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  size_t strides = sizeof(MYCUSTOMVERTEX), offsets = 0;
  g_pContext->IASetVertexBuffers(0, 1, &g_pVBuffer, &strides, &offsets);
  g_pContext->PSSetShader(g_pPShader, NULL, 0);
#endif

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
#ifdef WIN32
  SAFE_RELEASE(g_pPShader);
  SAFE_RELEASE(g_pVBuffer);
#endif
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
	p->colour[0] = (float)red / 255.0f;
	p->colour[1] = (float)green / 255.0f;
	p->colour[2] = (float)blue / 255.0f;
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
#ifndef WIN32
    //Store each point of the triangle together with it's colour
    MYCUSTOMVERTEX cvVertices[] =
    {
      {(float)   x, (float) y+h, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
      {(float)   x, (float)   y, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
      {(float) x+w, (float)   y, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
      {(float) x+w, (float) y+h, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]}
    };
    glBegin(GL_QUADS);
    for (size_t j=0;j<4;++j)
    {
      glColor4f(cvVertices[j].r, cvVertices[j].g, cvVertices[j].b, 1.0);
      glVertex2f(cvVertices[j].x, cvVertices[j].y);
    }
    glEnd();
#else
    //Store each point of the triangle together with it's colour
    MYCUSTOMVERTEX cvVertices[] =
    {
        {(float)   x, (float) y+h, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
        {(float)   x, (float)   y, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
        {(float) x+w, (float) y+h, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
        {(float) x+w, (float)   y, 0.0f, dwColour[0], dwColour[1], dwColour[2], dwColour[3]}
    };
    D3D11_MAPPED_SUBRESOURCE res = {};
    if (SUCCEEDED(g_pContext->Map(g_pVBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res)))
    {
      memcpy(res.pData, cvVertices, sizeof(cvVertices));
      g_pContext->Unmap(g_pVBuffer, 0);
    }
    g_pContext->Draw(4, 0);
#endif
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
#ifdef WIN32
  SAFE_RELEASE(g_pPShader);
  SAFE_RELEASE(g_pVBuffer);
#endif
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
