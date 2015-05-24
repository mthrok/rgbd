#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#include <stdio.h>
#include <stdexcept>

uint nFrames = 15000;
int depthMode=4, colorMode=9;

void main_exec() {
  NIDevice nid;
  nid.initDevice(depthMode, colorMode);
  //nid.listAllSensorModes();
  
  // Get stream resolutions
  uint depthW = 0, depthH = 0, colorW = 0, colorH = 0;

  nid.getDepthResolution(depthW, depthH);
  nid.getColorResolution(colorW, colorH);

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
      nid.convertDepthFrameToJet(visualizer.getDepthBuffer());
      nid.copyColorFrame(visualizer.getColorBuffer(), 1, 1);
      nid.releaseDepthFrame();
      nid.releaseColorFrame();
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    // Update window
    visualizer.refreshWindow();
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
