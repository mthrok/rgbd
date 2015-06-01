#ifndef __OPENNI_INCLUDE_RGBDVISUALIZER_HPP__
#define __OPENNI_INCLUDE_RGBDVISUALIZER_HPP__

#include "types.hpp"
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

  uint m_depthW, m_depthH, m_colorW, m_colorH, m_channel;
  uint8_t *m_pColorBuff;
  uint8_t *m_pDepthBuff;

  char m_windowTitle[MAX_WINDOW_TITLE_LENGTH];

  void createWindow();
  void createConversionBuffers();
  void updateTexture();
  void render();

public:
  static void initSDL(Uint32 flag=SDL_INIT_VIDEO);
  static void quitSDL();
  static SDL_Keysym getKeyPressed();

  RGBDVisualizer();
  ~RGBDVisualizer();

  void initWindow(uint depthW, uint depthH, uint colorW, uint colorH);
  void refreshWindow();

  uint8_t* getColorBuffer() const;
  uint8_t* getDepthBuffer() const;

  void setWindowTitle(const char* format, ...) const;
  void delay(Uint32 mSec) const;
  bool isStopped() const;
};

#endif
