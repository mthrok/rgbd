#include "RGBDVisualizer.hpp"
#include "NIDevice.hpp"
#include "io.hpp"

#include <string>
#include <stdexcept>

#define DEFAULT_DEPTH_MODE 0
#define DEFAULT_COLOR_MODE 0
#define DEFAULT_IR_MODE    -1
#define DEFAULT_NUM_FRAMES 9000

void listModes() {
  NIDevice nid;
  nid.openDevice();
  nid.listAllSensorModes();
}

void recordIR(uint nFrames, int IRMode) {
  NIDevice nid;
  Frames IRFrame;
  RGBDVisualizer visualizer;
  nid.openDevice();
  nid.createIRStream(IRMode);
  uint wIR = nid.getIRWidth();
  uint hIR = nid.getIRHeight();
  uint cIR = nid.getIRNumChannels();
  if (3 == cIR)
    IRFrame.allocate(wIR, hIR, 3, nFrames);
  else
    IRFrame.allocate(wIR, hIR, 2, nFrames);
  nid.startStreams();
  nid.waitStreamsToGetReady();
  visualizer.initWindow(wIR, hIR, 0, 0);
  uint iFrame = 0;
  while (1) {
    try {
      nid.copyIRFrame(IRFrame.getFrame());
      if (3 == cIR)
	IRFrame.copyCurrentFrameTo(visualizer.getDepthBuffer(), 1, 1);
      else
	IRFrame.convertCurrent16BitFrameToJet(visualizer.getDepthBuffer(), 0, 1024);
      IRFrame.incrementFrameIndex();
      iFrame = (++iFrame) % nFrames;
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    visualizer.setWindowTitle("Frame %5d/%5d", iFrame+1, nFrames);
    visualizer.refreshWindow();
    if (visualizer.isStopped())
      break;
  }
  nid.stopStreams();

  iFrame = 0;
  while (1) {
    if (3 == cIR)
      IRFrame.copyFrameTo(visualizer.getDepthBuffer(), iFrame, 1, 1);
    else
      IRFrame.convert16BitFrameToJet(visualizer.getDepthBuffer(), iFrame, 0, 1024);
    iFrame = (++iFrame) % nFrames;
    visualizer.setWindowTitle("Frame %5d/%5d", iFrame+1, nFrames);
    visualizer.refreshWindow();
    visualizer.delay(20);
    if (visualizer.isStopped()) {
      break;
    }
  }
  IRFrame.deallocate();
}

void recordRGBD(uint nFrames, int depthMode, int colorMode) {
  NIDevice nid;
  Frames depthFrame, colorFrame;
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
    depthFrame.allocate(wDepth, hDepth, 2, nFrames);
  }
  if (-1 < colorMode) {
    nid.createColorStream(colorMode);
    wColor = nid.getColorWidth();
    hColor = nid.getColorHeight();
    colorFrame.allocate(wColor, hColor, 3, nFrames);
  }
  if (-1 < depthMode && -1 < colorMode) {
    nid.setImageRegistration();
    nid.setDepthColorSync();
  }
  nid.startStreams();
  nid.waitStreamsToGetReady();
  visualizer.initWindow(wDepth, hDepth, wColor, hColor);
  uint iFrame = 0;
  while (1) {
    try {
      if (-1 < colorMode) {
	nid.copyColorFrame(colorFrame.getFrame());
	colorFrame.copyCurrentFrameTo(visualizer.getColorBuffer(), 1, 1);
	colorFrame.incrementFrameIndex();
      }
      if (-1 < depthMode) {
	nid.copyDepthFrame(depthFrame.getFrame());
	depthFrame.convertCurrent16BitFrameToJet(visualizer.getDepthBuffer(), minDepth, maxDepth);
	depthFrame.incrementFrameIndex();
      }
      iFrame = (++iFrame) % nFrames;
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    visualizer.setWindowTitle("Frame %5d/%5d", iFrame+1, nFrames);
    visualizer.refreshWindow();
    if (visualizer.isStopped())
      break;
  }
  nid.stopStreams();

  iFrame = 0;
  while (1) {
    if (-1 < colorMode)
      colorFrame.copyFrameTo(visualizer.getColorBuffer(), iFrame, 1, 1);
    if (-1 < depthMode)
      depthFrame.convert16BitFrameToJet(visualizer.getDepthBuffer(), iFrame, minDepth, maxDepth);
    iFrame = (++iFrame) % nFrames;
    visualizer.setWindowTitle("Frame %5d/%5d", iFrame+1, nFrames);
    visualizer.refreshWindow();
    visualizer.delay(20);
    if (visualizer.isStopped()) {
      break;
    }
  }
  colorFrame.deallocate();
  depthFrame.deallocate();
}

struct Option {
  bool printHelp = false;
  bool listModes = false;
  int IRMode = DEFAULT_IR_MODE;
  int depthMode = DEFAULT_DEPTH_MODE;
  int colorMode = DEFAULT_COLOR_MODE;
  uint nFrames = DEFAULT_NUM_FRAMES;
};

void printHelp() {
  printf("%-30s:%s\n", "--list-modes", "Show available camera modes and quit.");
  printf("%-30s:%s\n", "--help", "Show this message and quit.");
  printf("%-30s:%s\n", "--ir-mode IR-MODE", "IR camera mode.");
  printf("%-30s:%s\n", "--depth-mode DEPTH-MODE", "Depth camera mode.");
  printf("%-30s:%s\n", "--color-mode COLOR-MODE", "Color camera mode.");
  printf("%-30s:%s\n", "--n-frames N-FRAMES", "Number of frames.");}

Option parseArguments(int argc, char *argv[]) {
  Option opt;
  for (int i=1; i<argc; ++i) {
    std::string arg, val;
    arg = argv[i];
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
      int mode = std::stoi(val);
      if (0 <= mode) {
	opt.depthMode = mode;
	opt.IRMode = -1;
      }
    } else if (arg == "--color-mode") {
      i += 1;
      if (i == argc) goto fail2;
      val = argv[i];
      int mode = std::stoi(val);
      if (0 <= mode) {
	opt.colorMode = mode;
	opt.IRMode = -1;
      }
    } else if (arg == "--n-frames") {
      i += 1;
      if (i == argc) goto fail2;
      opt.nFrames = std::stoi(argv[i]);
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
  try {
    if (opt.listModes) {
      listModes();
    } else {
      if (opt.IRMode >= 0)
	recordIR(opt.nFrames, opt.IRMode);
      else
	recordRGBD(opt.nFrames, opt.depthMode, opt.colorMode);
    }
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    ret = -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return ret;
}
