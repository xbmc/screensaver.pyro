/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2004 Chris Barnett (Forza)
 *  Adapted from the Pyro screen saver by
 *  Jamie Zawinski <jwz@jwz.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include <kodi/addon-instance/Screensaver.h>

#ifdef WIN32
#include <d3d11.h>
#else
#include <kodi/gui/gl/Shader.h>
#endif

#include "Pyro.h"
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef WIN32

#define SAFE_RELEASE(_p) { if(_p) { _p->Release(); _p=nullptr; } }

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

#endif // WIN32

class ATTRIBUTE_HIDDEN CScreensaverPyro
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver
#ifndef WIN32
  , public kodi::gui::gl::CShaderProgram
#endif
{
public:
  CScreensaverPyro();

  // override functions for kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

#ifndef WIN32
  // override functions for kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override { return true;  }
#endif

private:
  void DrawRectangle(int x, int y, int w, int h, float* dwColour);

  struct projectile *get_projectile(void);
  void free_projectile(struct projectile *p);
  void launch(int xlim, int ylim, int g);
  struct projectile *shrapnel(struct projectile *parent);
  void hsv_to_rgb(double hue, double saturation, double value, double *red, double *green, double *blue);

  int m_iWidth;
  int m_iHeight;

  struct projectile *m_projectiles;
  struct projectile *m_free_projectiles;

  int m_how_many;
  int m_frequency;
  int m_scatter;

  int m_xlim;
  int m_ylim;
  int m_real_xlim;
  int m_real_ylim;

#ifndef WIN32
  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;
  GLint m_aPosition = -1;
  GLint m_aColor = -1;
#else
  ID3D11DeviceContext* m_pContext;
  ID3D11Buffer*        m_pVBuffer;
  ID3D11PixelShader*   m_pPShader;
#endif
};

////////////////////////////////////////////////////////////////////////////
// Kodi has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
CScreensaverPyro::CScreensaverPyro()
  : m_how_many(1000),
    m_frequency(5),
    m_scatter(20)
{
  m_iWidth = Width();
  m_iHeight = Height();
  m_real_xlim = m_iWidth;
  m_real_ylim = m_iHeight;
}

bool CScreensaverPyro::Start()
{
  m_free_projectiles = nullptr;
  m_projectiles = static_cast<struct projectile*>(calloc(m_how_many, sizeof (struct projectile)));
  for (int i = 0; i < m_how_many; i++)
    free_projectile(&m_projectiles[i]);

#ifndef WIN32
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glGenBuffers(1, &m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);
#else
  m_pContext = reinterpret_cast<ID3D11DeviceContext*>(Device());
  ID3D11Device* pDevice = nullptr;
  m_pContext->GetDevice(&pDevice);

  CD3D11_BUFFER_DESC vbDesc(sizeof(MYCUSTOMVERTEX) * 5, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
  pDevice->CreateBuffer(&vbDesc, nullptr, &m_pVBuffer);
  pDevice->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, &m_pPShader);
  SAFE_RELEASE(pDevice);
#endif
  return true;
}

static int myrand()
{
  return (rand() << 15) + rand();
}

void CScreensaverPyro::Render()
{
#ifdef WIN32
  m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  UINT strides = sizeof(MYCUSTOMVERTEX), offsets = 0;
  m_pContext->IASetVertexBuffers(0, 1, &m_pVBuffer, &strides, &offsets);
  m_pContext->PSSetShader(m_pPShader, nullptr, 0);
#endif

  int g = 100;

  if ((myrand() % m_frequency) == 0)
  {
    m_real_xlim = m_iWidth;
    m_real_ylim = m_iHeight;
    m_xlim = m_real_xlim * 1000;
    m_ylim = m_real_ylim * 1000;
    launch(m_xlim, m_ylim, g);
  }

  for (int i = 0; i < m_how_many; i++)
  {
    struct projectile *p = &m_projectiles[i];
    int old_x, old_y, old_size;
    int size, x, y;
    if (p->dead)
      continue;
    old_x = p->x >> 10;
    old_y = p->y >> 10;
    old_size = p->size >> 10;
    size = (p->size += p->decay) >> 10;
    x = (p->x += p->dx) >> 10;
    y = (p->y += p->dy) >> 10;
    p->dy += (p->size >> 6);
    if (p->primary)
      p->fuse--;

    if ((p->primary ? (p->fuse > 0) : (p->size > 0)) &&
        x < m_real_xlim &&
        y < m_real_ylim &&
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
      int j = (myrand() % m_scatter) + (m_scatter/2);
      while (j-- > 0)
        shrapnel(p);
    }
  }
}

// Kodi tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void CScreensaverPyro::Stop()
{
  free(m_projectiles);
#ifndef WIN32
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
#else
  SAFE_RELEASE(m_pPShader);
  SAFE_RELEASE(m_pVBuffer);
#endif
}

void CScreensaverPyro::DrawRectangle(int x, int y, int w, int h, float* dwColour)
{
#ifndef WIN32
  EnableShader();

  GLfloat x1 = -1.0 + 2.0*x/m_iWidth;
  GLfloat y1 = -1.0 + 2.0*y/m_iHeight;
  GLfloat x2 = -1.0 + 2.0*(x+w)/m_iWidth;
  GLfloat y2 = -1.0 + 2.0*(y+h)/m_iHeight;

  //Store each point of the triangle together with it's colour
  MYCUSTOMVERTEX vertex[4] =
  {
    {x1, y1, 0.0, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
    {x2, y1, 0.0, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
    {x2, y2, 0.0, dwColour[0], dwColour[1], dwColour[2], dwColour[3]},
    {x1, y2, 0.0, dwColour[0], dwColour[1], dwColour[2], dwColour[3]}
  };

  GLubyte idx[] = {0, 1, 2, 2, 3, 0};

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(MYCUSTOMVERTEX)*4, &vertex[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*6, idx, GL_STATIC_DRAW);

  glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, 0, sizeof(MYCUSTOMVERTEX), BUFFER_OFFSET(offsetof(MYCUSTOMVERTEX, x)));
  glVertexAttribPointer(m_aColor, 4, GL_FLOAT, 0, sizeof(MYCUSTOMVERTEX), BUFFER_OFFSET(offsetof(MYCUSTOMVERTEX, r)));

  glEnableVertexAttribArray(m_aPosition);
  glEnableVertexAttribArray(m_aColor);

  glEnable(GL_BLEND);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

  glDisableVertexAttribArray(m_aPosition);
  glDisableVertexAttribArray(m_aColor);

  DisableShader();

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
  if (SUCCEEDED(m_pContext->Map(m_pVBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res)))
  {
    memcpy(res.pData, cvVertices, sizeof(cvVertices));
    m_pContext->Unmap(m_pVBuffer, 0);
  }
  m_pContext->Draw(4, 0);
#endif
}

struct projectile *CScreensaverPyro::get_projectile(void)
{
  if (m_free_projectiles)
  {
    struct projectile *p = m_free_projectiles;
    m_free_projectiles = p->next_free;
    p->next_free = 0;
    p->dead = false;
    return p;
  }

  return nullptr;
}

void CScreensaverPyro::free_projectile(struct projectile *p)
{
  p->next_free = m_free_projectiles;
  m_free_projectiles = p;
  p->dead = true;
}

void CScreensaverPyro::launch(int xlim, int ylim, int g)
{
  struct projectile *p = get_projectile();
  int x, dx, xxx;
  double red, green, blue;
  if (!p)
    return;

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

struct projectile* CScreensaverPyro::shrapnel(struct projectile *parent)
{
  struct projectile *p = get_projectile();
  if (!p)
    return nullptr;

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

void CScreensaverPyro::hsv_to_rgb(double hue, double saturation, double value,
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

#ifndef WIN32
void CScreensaverPyro::OnCompiledAndLinked()
{
  m_aPosition = glGetAttribLocation(ProgramHandle(), "a_position");
  m_aColor = glGetAttribLocation(ProgramHandle(), "a_color");
}
#endif

ADDONCREATOR(CScreensaverPyro);
