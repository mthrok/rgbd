// c++ viewer_tmp.cxx NIDevice.cxx -lOpenNi2 -framework SDL2 --std=c++11
#include "NIDevice.hpp"
#include "RGBDViewer.hpp"

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

  // Initialize viewer
  RGBDViewer viewer{depthW, depthH, colorW, colorH};
  viewer.initWindow();

  // Start streams
  nid.startDepthStream();
  nid.startColorStream();

  // Main loop
  while (1) {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
	break;
      }
    }
    // Read depth and color
    try {
      nid.readDepthStream();
      nid.readColorStream();
      nid.copyDepthData(viewer.getDepthBuffer());
      nid.copyColorData(viewer.getColorBuffer());
      nid.releaseDepthFrame();
      nid.releaseColorFrame();
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    // Update window
    viewer.refreshWindow();
  }
}

int main(int argc, char *argv[]) {  
  NIDevice::initONI();
  RGBDViewer::initSDL();
  try {
    main_exec();
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    return -1;
  }
  RGBDViewer::quitSDL();
  NIDevice::quitONI();
  return 0;
}
