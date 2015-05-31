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

void record(uint nFrames, int depthMode, int colorMode, int IRMode=-1) {
  // Initialize device and get frame sizes
  uint w16B = 0, h16B = 0, w8B = 0, h8B = 0, cIR=1;
  NIDevice nid;
  RGBDFrames rgbdf;
  RGBDVisualizer visualizer;
  nid.openDevice();
  if (-1 < IRMode) {
    nid.createIRStream(IRMode);
    cIR = nid.getIRNumChannels();
    if (3 == cIR) {
      w8B = nid.getIRWidth();
      h8B = nid.getIRHeight();
      rgbdf.allocate(0, 0, w8B, h8B, nFrames);
      visualizer.initWindow(0, 0, w8B, h8B);
    } else {
      w16B = nid.getIRWidth();
      h16B = nid.getIRHeight();
      rgbdf.allocate(w16B, h16B, 0, 0, nFrames);
      visualizer.initWindow(0, 0, w16B, h16B);
    }
  } else {
    if (-1 < depthMode) {
      nid.createDepthStream(depthMode);
      w16B = nid.getDepthWidth();
      h16B = nid.getDepthHeight();
    }
    if (-1 < colorMode) {
      nid.createColorStream(colorMode);
      w8B = nid.getColorWidth();
      h8B = nid.getColorHeight();
    }
    if (-1 < depthMode && -1 < colorMode) {
      nid.setImageRegistration();
      nid.setDepthColorSync();
    }
    rgbdf.allocate(w16B, h16B, w8B, h8B, nFrames);
    visualizer.initWindow(w16B, h16B, w8B, h8B);
  }

  // Start streams
  nid.startStreams();

  // Main loop
  while (1) {
    try {
      //nid.getAllFrames();
      if (-1 < IRMode) {
	if (3 == cIR) {
	  nid.copyIRFrame(visualizer.getColorBuffer(), 1, 1);
	  nid.copyIRFrame(rgbdf.getColorFrame());
	} else {
	  //nid.convertIRFrameToJet(visualizer.getColorBuffer());
	  //nid.copyIRFrame(rgbdf.getDepthFrame());
	}
      } else {
	if (-1 < colorMode) {
	  nid.copyColorFrame(visualizer.getColorBuffer(), 1, 1);
	  nid.copyColorFrame(rgbdf.getColorFrame());
	} if (-1 < depthMode) {
	  //nid.convertDepthFrameToJet(visualizer.getDepthBuffer());
	  //nid.copyDepthFrame(rgbdf.getDepthFrame());
	}
      }
      rgbdf.incrementFrameIndex();
      //nid.releaseAllFrames();
    } catch(const std::runtime_error& e) {
      printf("%s/n", e.what());
    }
    visualizer.setWindowTitle("Frame %5d/%5d", rgbdf.getFrameIndex()+1, nFrames);
    visualizer.refreshWindow();    
    if (visualizer.isStopped())
      break;
  }

  // Playback
  printf("Playing back...\n");
  uint iFrame = 0;
  while (1) {
    if (-1 < IRMode) {
      if (3 == cIR)
	rgbdf.copyColorFrameTo(visualizer.getColorBuffer(), iFrame, 1, 1);
      else
	rgbdf.convert16BitFrameToJet(visualizer.getColorBuffer(),
				     iFrame, DEFAULT_IR_MIN, DEFAULT_IR_MAX, 1);
    } else {
      if (-1 < colorMode)
	rgbdf.copyColorFrameTo(visualizer.getColorBuffer(), iFrame, 1, 1);
      if (-1 < depthMode)
	rgbdf.convert16BitFrameToJet(visualizer.getDepthBuffer(),
				     iFrame, DEFAULT_DEPTH_MIN, DEFAULT_DEPTH_MAX, 1);
    }
    iFrame = (iFrame + 1) % rgbdf.getNumFrames();
    visualizer.setWindowTitle("Frame %5d/%5d", iFrame+1, rgbdf.getNumFrames());
    visualizer.refreshWindow();
    visualizer.delay(20);
    if (visualizer.isStopped())
      break;
  }
}

void printHelp() {
  printf("%-30s:%s\n", "--list-modes", "Show available camera modes and quit.");
  printf("%-30s:%s\n", "--help", "Show this message and quit.");
  printf("%-30s:%s\n", "--ir-mode IR-MODE", "IR camera mode.");
  printf("%-30s:%s\n", "--depth-mode DEPTH-MODE", "Depth camera mode.");
  printf("%-30s:%s\n", "--color-mode COLOR-MODE", "Color camera mode.");
  printf("%-30s:%s\n", "--n-frames N-FRAMES", "Number of frames.");
}

struct Option {
  bool printHelp = false;
  bool listModes = false;
  int IRMode = DEFAULT_IR_MODE;
  int depthMode = DEFAULT_DEPTH_MODE;
  int colorMode = DEFAULT_COLOR_MODE;
  int nFrames = DEFAULT_NUM_FRAMES;
};

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
    } else if (arg == "--n-frames") {
      i += 1;
      if (i == argc) goto fail2;
      std::string val(argv[i]);
      opt.nFrames = std::stoi(val);
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
  
  NIDevice::initONI();
  RGBDVisualizer::initSDL();
  try {
    if (opt.listModes)
      listModes();
    else
      record(opt.nFrames, opt.depthMode, opt.colorMode, opt.IRMode);
  } catch (const std::exception& e) {
    printf("%s\n", e.what());
    return -1;
  }
  RGBDVisualizer::quitSDL();
  NIDevice::quitONI();
  return 0;
}
