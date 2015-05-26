#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#include <string>
//#include <cstdio>
#include <stdexcept>

#define DEFAULT_DEPTH_MODE 0
#define DEFAULT_COLOR_MODE 0
#define DEFAULT_IR_MODE    -1

void listModes() {
  NIDevice nid;
  nid.openDevice();
  nid.listAllSensorModes();
}

void view_RGBD(int depthMode, int colorMode) {
  NIDevice nid;
  nid.initDevice(depthMode, colorMode);

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

void view_IR(int IRMode) {
  NIDevice nid;
  nid.initDevice(-1, -1, IRMode);

  // Get stream resolutions
  uint channel = nid.getIRChannel();
  uint w3ch=0, h3ch=0, w1ch=0, h1ch=0;
  if (3 == channel)
    nid.getIRResolution(w3ch, h3ch);
  else
    nid.getIRResolution(w1ch, h1ch);

  // Initialize visualizer
  RGBDVisualizer visualizer(w1ch, h1ch, w3ch, h3ch);
  visualizer.initWindow();

  // Start streams
  nid.startIRStream();

  // Main loop
  while (1) {
    if (visualizer.isStopped())
      break;
    try {
      nid.readIRStream();
      if (3==channel)
	nid.copyIRFrame(visualizer.getColorBuffer(), 1, 1);
      else
	nid.convertIRFrameToJet(visualizer.getDepthBuffer());
      nid.releaseIRFrame();
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    // Update window
    visualizer.refreshWindow();
  }
}

struct Option {
  bool printHelp = false;
  bool listModes = false;
  int IRMode = DEFAULT_IR_MODE;
  int depthMode = DEFAULT_DEPTH_MODE;
  int colorMode = DEFAULT_COLOR_MODE;
};

void printHelp() {
  printf("%-30s:%s\n", "--list-modes", "Show available camera modes and quit.");
  printf("%-30s:%s\n", "--help", "Show this message and quit.");
  printf("%-30s:%s\n", "--ir-mode IR-MODE", "IR camera mode.");
  printf("%-30s:%s\n", "--depth-mode DEPTH-MODE", "Depth camera mode.");
  printf("%-30s:%s\n", "--color-mode COLOR-MODE", "Color camera mode.");
}

Option parseArguments(int argc, char *argv[]) {
  Option opt;
  for (int i=1; i<argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "--help") {
      opt.printHelp = true;
    } else if (arg == "--list-modes"){
      opt.listModes = true;
    } else if (arg == "--ir-mode") {
      i += 1;
      std::string val(argv[i]);
      opt.IRMode = std::stoi(val);
      opt.depthMode = opt.colorMode = -1;
    } else if (arg == "--depth-mode") {
      i += 1;
      std::string val(argv[i]);
      opt.depthMode = std::stoi(val);
    } else if (arg == "--color-mode") {
      i += 1;
      std::string val(argv[i]);
      opt.colorMode = std::stoi(val);
    } else {
      throw std::runtime_error("Unexpected option was given.");
    }
  }
  return opt;
}

int main(int argc, char *argv[]) {
  Option opt;
  try {
    opt = parseArguments(argc, argv);
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    return -1;
  }

  if (opt.printHelp) {
    printHelp();
    return 0;
  }

  int ret = 0;
  NIDevice::initONI();
  RGBDVisualizer::initSDL();
  try{
    if (opt.listModes) {
      listModes();
    } else {
      if (opt.IRMode >= 0) {
	view_IR(opt.IRMode);
    } else {
	view_RGBD(opt.depthMode, opt.colorMode);
      }
    }
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    ret = -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return ret;
}
