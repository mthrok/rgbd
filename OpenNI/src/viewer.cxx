#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#include <stdio.h>
#include <stdexcept>

void main_exec() {
  NIDevice nid;
  nid.initDevice();
  //nid.listAllSensorModes();
  
  // Get stream resolutions
  int depthW = 0, depthH = 0;
  int colorW = 0, colorH = 0;

  nid.getDepthResolution(depthW, depthH);
  nid.getColorResolution(colorW, colorH);

  // Initialize visualizer
  RGBDVisualizer visualizer{depthW, depthH, colorW, colorH};
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
