#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#include <string>
#include <stdexcept>

#define DEFAULT_DEPTH_MODE 0
#define DEFAULT_COLOR_MODE 0
#define DEFAULT_IR_MODE    -1

void listModes() {
  NIDevice nid;
  nid.openDevice();
  nid.listAllSensorModes();
}

void view(int depthMode, int colorMode, int IRMode=-1) {
  // Initialize device and get frame sizes
  uint w16B = 0, h16B = 0, w8B = 0, h8B = 0, cIR=1;
  uint16_t minDepth = 0, maxDepth = 0;
  NIDevice nid;
  RGBDVisualizer visualizer;
  nid.openDevice();
  if (-1 < IRMode) {
    nid.createIRStream(IRMode);
    cIR = nid.getIRNumChannels();
    w8B = nid.getIRResolutionX();
    h8B = nid.getIRResolutionY();
    visualizer.initWindow(0, 0, w8B, h8B);
  } else {
    if (-1 < depthMode) {
      nid.createDepthStream(depthMode);
      w16B = nid.getDepthResolutionX();
      h16B = nid.getDepthResolutionY();
      minDepth = nid.getDepthMinValue();
      maxDepth = nid.getDepthMaxValue();
    }
    if (-1 < colorMode) {
      nid.createColorStream(colorMode);
      w8B = nid.getColorResolutionX();
      h8B = nid.getColorResolutionY();
    }
    if (-1 < depthMode && -1 < colorMode) {
      nid.setImageRegistration();
      nid.setDepthColorSync();
    }
    visualizer.initWindow(w16B, h16B, w8B, h8B);
  }

  nid.startAllStreams();
  while (1) {
    try {
      nid.getAllFrames();
      if (-1 < IRMode) {
	if (3 == cIR)
	  nid.copyIRFrame(visualizer.getColorBuffer(), 1, 1);
	else
	  nid.convertIRFrameToJet(visualizer.getColorBuffer());
      } else {
	if (-1 < colorMode)
	  nid.copyColorFrame(visualizer.getColorBuffer(), 1, 1);
	if (-1 < depthMode)
	  nid.convertDepthFrameToJet(visualizer.getDepthBuffer(), 1, minDepth, maxDepth);
      }
      nid.releaseAllFrames();
    } catch(const std::runtime_error& e) {
      printf("%s\n", e.what());
    }
    visualizer.refreshWindow();
    if (visualizer.isStopped())
      break;
  }
  nid.stopAllStreams();
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
      break;
    } else if (arg == "--list-modes"){
      opt.listModes = true;
      break;
    } else if (arg == "--ir-mode") {
      i += 1;
      if (i == argc) goto fail2;
      std::string val(argv[i]);
      opt.IRMode = std::stoi(val);
      opt.depthMode = opt.colorMode = -1;
    } else if (arg == "--depth-mode") {
      i += 1;
      if (i == argc) goto fail2;
      std::string val(argv[i]);
      opt.depthMode = std::stoi(val);
    } else if (arg == "--color-mode") {
      i += 1;
      if (i == argc) goto fail2;
      std::string val(argv[i]);
      opt.colorMode = std::stoi(val);
    } else {
      goto fail1;
    }
  }
  return opt;
 fail1:
  throw std::runtime_error("Unexpected option was given.");
 fail2:
  throw std::runtime_error("Parameter not given.");
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
    if (opt.listModes)
      listModes();
    else
      view(opt.depthMode, opt.colorMode, opt.IRMode);
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    ret = -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return ret;
}
