#include "RGBDViewer.hpp"
#include <string.h>

void RGBDViewer::initSDL(Uint32 flag) {
  SDL_Init(flag);
}

void RGBDViewer::quitSDL() {
  SDL_Quit();
}

RGBDViewer::RGBDViewer(int depthW, int depthH, int colorW, int colorH)
  : m_pWindow(NULL)
  , m_pTexture(NULL)
  , m_pRenderer(NULL)
  , m_depthRect()
  , m_colorRect()
  , m_depthW(depthW)
  , m_depthH(depthH)
  , m_colorW(colorW)
  , m_colorH(colorH)
  , m_channel(4)
  , m_pColorBuff(NULL)
  , m_pDepthBuff(NULL)
{}

RGBDViewer::~RGBDViewer() {
  if (m_pRenderer)
    SDL_DestroyRenderer(m_pRenderer);
  if (m_pTexture)
    SDL_DestroyTexture(m_pTexture);
  if (m_pWindow)
    SDL_DestroyWindow(m_pWindow);
  if (m_pDepthBuff)
    delete[] m_pDepthBuff;
  if (m_pColorBuff)
    delete[] m_pColorBuff;
}

void RGBDViewer::initWindow() {
  createWindow();
  createConversionBuffers();
}

void RGBDViewer::createWindow() {
  // Check the size
  int width = m_depthW + m_colorW;
  int height = (m_depthH > m_colorH) ? m_depthH : m_colorH;
  // Create window, renderer and texture
  m_pWindow = SDL_CreateWindow("RGBDViewer", 0, 0, width, height, 0);
  m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
  m_pTexture = SDL_CreateTexture(m_pRenderer,
    SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  // Create region mask
  m_depthRect.x = m_depthRect.y = m_colorRect.y = 0;
  m_depthRect.w = m_colorRect.x = m_depthW; m_depthRect.h = m_depthH;
  m_colorRect.w = m_colorW; m_colorRect.h = m_colorH;
}

void RGBDViewer::createConversionBuffers() {
  uint64_t nDepthPix = uint64_t(m_depthW) * m_depthH * m_channel;
  uint64_t nColorPix = uint64_t(m_colorW) * m_colorH * m_channel;
  m_pDepthBuff = (uint16_t *)new uint8_t[nDepthPix];
  m_pColorBuff = new uint8_t[nColorPix];
  //memset_s(m_pDepthBuff, nDepthPix, 0, nDepthPix);
  //memset_s(m_pColorBuff, nColorPix, 0, nColorPix);
  memset(m_pDepthBuff, 0, nDepthPix);
  memset(m_pColorBuff, 0, nColorPix);
}

uint8_t* RGBDViewer::getColorBuffer() {
  return (uint8_t*)m_pColorBuff;
}

uint16_t* RGBDViewer::getDepthBuffer() {
  return (uint16_t*)m_pDepthBuff;
}

void RGBDViewer::refreshWindow() {
  updateTexture();
  render();
}

void RGBDViewer::updateTexture() {
  SDL_UpdateTexture(m_pTexture, &m_depthRect, (const void*)m_pDepthBuff, m_depthW * m_channel);
  SDL_UpdateTexture(m_pTexture, &m_colorRect, (const void*)m_pColorBuff, m_colorW * m_channel);
}

void RGBDViewer::render() {
  SDL_RenderClear(m_pRenderer);
  SDL_RenderCopy(m_pRenderer, m_pTexture, NULL, NULL);
  SDL_RenderPresent(m_pRenderer);
}
