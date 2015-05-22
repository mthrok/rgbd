#ifndef __SDLVIEWER_HPP__
#define __SDLVIEWER_HPP__

#include "SDL2/SDL.h"

class RGBDViewer {
  SDL_Window *m_pWindow;
  SDL_Texture *m_pTexture;
  SDL_Renderer *m_pRenderer;
  SDL_Rect m_depthRect;
  SDL_Rect m_colorRect;

  int m_depthW, m_depthH, m_colorW, m_colorH, m_channel;
  uint8_t *m_pColorBuff;
  uint16_t *m_pDepthBuff;

  void createWindow();
  void createConversionBuffers();
  void updateTexture();
  void render();


public:
  static void initSDL(Uint32 flag=SDL_INIT_VIDEO);
  static void quitSDL();
  
  RGBDViewer(int depthW, int depthH, int colorW, int colorH);
  ~RGBDViewer();

  void initWindow();
  
  uint8_t* getColorBuffer();
  uint16_t* getDepthBuffer();

  void refreshWindow();
};

#endif
