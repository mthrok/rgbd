#include "io.hpp"
#include "NIDevice.hpp"
#include "RGBDVisualizer.hpp"

#define DEFAULT_DEPTH_MODE 0
#define DEFAULT_COLOR_MODE 0
#define DEFAULT_IR_MODE    -1

void listModes() {
  NIDevice nid;
  nid.openDevice();
  nid.listAllSensorModes();
}

void viewIR(int IRMode) {
  NIDevice nid;
  Frames frame;
  RGBDVisualizer visualizer;
  nid.openDevice();
  nid.createIRStream(IRMode);
  uint wIR = nid.getIRWidth();
  uint hIR = nid.getIRHeight();
  uint cIR = nid.getIRNumChannels();
  nid.startStreams();
  nid.waitStreamsToGetReady();
  frame.allocate(wIR, hIR, 2, 1);
  visualizer.initWindow(wIR, hIR, 0, 0);
  while (1) {
    try {
      nid.copyIRFrame(frame.getFrame());
      if (3 == cIR)
	frame.copyFrameTo(visualizer.getDepthBuffer(), 1, 1);
      else
	frame.convert16BitFrameToJet(visualizer.getDepthBuffer(), -1, 0, 1024); // Check
    } catch(const std::exception& e) {
      printf("%s\n", e.what());
    }
    visualizer.refreshWindow();
    if (visualizer.isStopped())
      break;
  }
  frame.deallocate();
  nid.stopStreams();
}

void viewRGBD(int depthMode, int colorMode) {
  NIDevice nid;
  Frames frame;
  RGBDVisualizer visualizer;
  uint wDepth = 0, hDepth = 0, wColor = 0, hColor = 0;
  uint16_t minDepth = DEFAULT_DEPTH_MIN, maxDepth = DEFAULT_DEPTH_MAX;
  nid.openDevice();
 if (-1 < depthMode) {
    nid.createDepthStream(depthMode);
    wDepth = nid.getDepthWidth();
    hDepth = nid.getDepthHeight();
    minDepth = nid.getDepthMinValue();
    maxDepth = nid.getDepthMaxValue();
    frame.allocate(wDepth, hDepth, 2, 1);
  }
  if (-1 < colorMode) {
    nid.createColorStream(colorMode);
    wColor = nid.getColorWidth();
    hColor = nid.getColorHeight();
  }
  if (-1 < depthMode && -1 < colorMode) {
    nid.setImageRegistration();
    nid.setDepthColorSync();
  }
  nid.startStreams();
  nid.waitStreamsToGetReady();
  visualizer.initWindow(wDepth, hDepth, wColor, hColor);
  while (1) {
    try {
      if (-1 < colorMode)
	nid.copyColorFrame(visualizer.getColorBuffer(), 1, 1);
      if (-1 < depthMode) {
	nid.copyDepthFrame(frame.getFrame(), 0, 0);
	frame.convert16BitFrameToJet(visualizer.getDepthBuffer(), -1, minDepth, maxDepth);
      }
    } catch(const std::exception& e) {
      printf("%s\n", e.what());
    }
    visualizer.refreshWindow();
    if (visualizer.isStopped())
      break;
  }
  frame.deallocate();
  nid.stopStreams();
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
  std::string arg, val;
  for (int i=1; i<argc; ++i) {
    arg = (argv[i]);
    if (arg == "--help") {
      opt.printHelp = true;
      break;
    } else if (arg == "--list-modes"){
      opt.listModes = true;
      break;
    } else if (arg == "--ir-mode") {
      i += 1;
      if (i == argc) goto fail2;
      val = argv[i];
      int mode = std::stoi(val);
      if (0 <= mode) {
	opt.IRMode = mode;
	opt.depthMode = opt.colorMode = -1;
      }
    } else if (arg == "--depth-mode") {
      i += 1;
      if (i == argc) goto fail2;
      val = argv[i];
      opt.depthMode = std::stoi(val);
      opt.IRMode = -1;
    } else if (arg == "--color-mode") {
      i += 1;
      if (i == argc) goto fail2;
      val = argv[i];
      opt.colorMode = std::stoi(val);
      opt.IRMode = -1;
    } else {
      goto fail1;
    }
  }
  return opt;
 fail1:
  throw RuntimeError({"Unexpected option ", arg, " was given."});
 fail2:
  throw RuntimeError({"Parameter for ", arg, " is missing."});
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
      if (opt.IRMode >= 0)
	viewIR(opt.IRMode);
      else
	viewRGBD(opt.depthMode, opt.colorMode);
    }
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    ret = -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return ret;
}
