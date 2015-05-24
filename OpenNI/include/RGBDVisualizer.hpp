#ifndef __SDLVIEWER_HPP__
#define __SDLVIEWER_HPP__

#include "SDL2/SDL.h"

#define MAX_WINDOW_TITLE_LENGTH 64

class EventHandler {
  
};

class RGBDVisualizer {
  SDL_Window *m_pWindow;
  SDL_Texture *m_pTexture;
  SDL_Renderer *m_pRenderer;
  SDL_Rect m_depthRect;
  SDL_Rect m_colorRect;

  int m_depthW, m_depthH, m_colorW, m_colorH, m_channel;
  uint8_t *m_pColorBuff;
  uint16_t *m_pDepthBuff;

  char m_windowTitle[MAX_WINDOW_TITLE_LENGTH];

  void createWindow();
  void createConversionBuffers();
  void updateTexture();
  void render();

public:
  static void initSDL(Uint32 flag=SDL_INIT_VIDEO);
  static void quitSDL();
  static SDL_Keysym getKeyPressed();

  RGBDVisualizer(int depthW, int depthH, int colorW, int colorH);
  ~RGBDVisualizer();

  void initWindow();
  void refreshWindow();
  
  uint8_t* getColorBuffer() const;
  uint16_t* getDepthBuffer() const;

  void setWindowTitle(const char* format, ...) const;
  void delay(Uint32 mSec) const;
  bool isStopped() const;
};

#endif
