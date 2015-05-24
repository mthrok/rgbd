#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#include <stdio.h>
#include <stdexcept>

class RGBDFrames {
  uint m_depthW, m_depthH;
  uint m_colorW, m_colorH;
  uint m_nFrames;
  uint m_currentFrame;

  uint8_t *m_pColorFrame;
  uint16_t *m_pDepthFrame;
public:
  RGBDFrames(uint depthW=0, uint depthH=0,
	     uint colorW=0, uint colorH=0, uint nFrames=0)
    : m_depthW(depthW)
    , m_depthH(depthH)
    , m_colorW(colorW)
    , m_colorH(colorH)
    , m_nFrames(nFrames)
    , m_currentFrame(0)
    , m_pColorFrame(NULL)
    , m_pDepthFrame(NULL)
  {}

  ~RGBDFrames() {
    deallocate();
  }

  void deallocate() {
    delete[] m_pColorFrame;
    delete[] m_pDepthFrame;
  }

  void allocate() {
    if (m_pColorFrame || m_pDepthFrame)
      deallocate();
    m_pColorFrame = new uint8_t[size_t(m_nFrames) * m_colorW * m_colorH * 3];
    m_pDepthFrame = new uint16_t[size_t(m_nFrames) * m_depthW * m_depthH * 1];
  }

  int getCurrentFrame() {
    return m_currentFrame;
  }

  int getNFrames() {
    return m_nFrames;
  }
  
  void forwardFrame() {
    m_currentFrame = (m_currentFrame + 1) % m_nFrames;
  }

  void setCurrentFrame(int nFrame) {
    m_currentFrame = nFrame % m_nFrames;
  }

  uint8_t* getCurrentColorFrame() {
    return m_pColorFrame + size_t(m_currentFrame) * m_colorW * m_colorH * 3;
  }

  uint16_t* getCurrentDepthFrame() {
    return m_pDepthFrame + size_t(m_currentFrame) * m_depthW * m_depthH * 1;
  }

  void copyDepthFrame(uint16_t* pBuffer, int offset=1, int skip=1) {
    uint16_t *pDepth = getCurrentDepthFrame();
    pBuffer += offset;
    for (int h=0; h<m_depthH; ++h) {
      for (int w=0; w<m_depthW; ++w) {
	for (int c=0; c<1; ++c) {
	  *pBuffer = *pDepth;
	  ++pBuffer; ++pDepth;
	}
	pBuffer += skip;
      }
    }
  }
  
  void copyColorFrame(uint8_t* pBuffer, int offset=1, int skip=1) {
    uint8_t *pColor = getCurrentColorFrame();
    pBuffer += offset;
    for (int h=0; h<m_colorH; ++h) {
      for (int w=0; w<m_colorW; ++w) {
	for (int c=0; c<3; ++c) {
	  *pBuffer = *pColor;
	  ++pBuffer; ++pColor;
	}
	pBuffer += skip;
      }
    }
  }
};

void main_exec() {
  NIDevice nid;
  nid.initDevice();
  //nid.listAllSensorModes();
  
  // Get stream resolutions
  int depthW = 0, depthH = 0;
  int colorW = 0, colorH = 0;

  nid.getDepthResolution(depthW, depthH);
  nid.getColorResolution(colorW, colorH);

  // Allocate Recording buffer
  int nFrames = 2000;
  RGBDFrames rgbdf(depthW, depthH, colorW, colorH, nFrames);
  rgbdf.allocate();
  
  // Initialize visualizer
  RGBDVisualizer visualizer(depthW, depthH, colorW, colorH);
  visualizer.initWindow();

  // Start streams
  nid.startDepthStream();
  nid.startColorStream();

  // Main loop
  while (1) {
    if (visualizer.isStopped())
      break;
    // Read depth and color
    try {
      nid.readDepthStream();
      nid.readColorStream();
      nid.copyDepthData(visualizer.getDepthBuffer());
      nid.copyColorData(visualizer.getColorBuffer());
      nid.copyDepthData(rgbdf.getCurrentDepthFrame(), 0, 0);
      nid.copyColorData(rgbdf.getCurrentColorFrame(), 0, 0);
      rgbdf.forwardFrame();
      nid.releaseDepthFrame();
      nid.releaseColorFrame();
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    // Update window
    visualizer.setWindowTitle("Frame %5d/%5d", rgbdf.getCurrentFrame(), rgbdf.getNFrames());
    visualizer.refreshWindow();
  }

  // Playback
  printf("Playing back...\n");
  rgbdf.setCurrentFrame(0);
  while (1) {
    if (visualizer.isStopped())
      break;
    rgbdf.copyColorFrame(visualizer.getColorBuffer());
    rgbdf.copyDepthFrame(visualizer.getDepthBuffer());
    rgbdf.forwardFrame();
    visualizer.setWindowTitle("Frame %5d/%5d", rgbdf.getCurrentFrame(), rgbdf.getNFrames());
    visualizer.refreshWindow();
    visualizer.delay(33);
  }
}

int main(int argc, char *argv[]) {  
  NIDevice::initONI();
  RGBDVisualizer::initSDL();
  try {
    main_exec();
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    return -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return 0;
}
