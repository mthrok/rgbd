// c++ test_window.cpp -lOpenNi2 -framework SDL2 -std=c++11
#include <iostream>
#include <string>

#include "OpenNI2/OpenNI.h"
#include "SDL2/SDL.h"

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms

void printOpenNIError(const char * message) {
  std::cout << message << "\n"
	    << openni::OpenNI::getExtendedError() << "\n";
}

const char * getPixelFormatAsString(const int& val) {
  switch (val) {
  case openni::PIXEL_FORMAT_DEPTH_1_MM:
    return "PIXEL_FORMAT_DEPTH_1_MM";
  case openni::PIXEL_FORMAT_DEPTH_100_UM:
    return "PIXEL_FORMAT_DEPTH_100_UM";
  case openni::PIXEL_FORMAT_SHIFT_9_2:
    return "PIXEL_FORMAT_SHIFT_9_2";
  case openni::PIXEL_FORMAT_SHIFT_9_3:
    return "PIXEL_FORMAT_SHIFT_9_3";
  case openni::PIXEL_FORMAT_RGB888:
    return "PIXEL_FORMAT_RGB888";
  case openni::PIXEL_FORMAT_YUV422:
    return "PIXEL_FORMAT_YUV422";
  case openni::PIXEL_FORMAT_GRAY8:
    return "PIXEL_FORMAT_GRAY8";
  case openni::PIXEL_FORMAT_GRAY16:
    return "PIXEL_FORMAT_GRAY16";
  default:
    return "UNKNOWN";
  }
}

void printSupportedVideoModes(const openni::SensorInfo* const s_info) {
  auto& modes = s_info->getSupportedVideoModes();
  for (int i=0; i<modes.getSize(); ++i) {
    std::cout << "Depth Mode : " << i << "\n"
	      << "Width  : " << modes[i].getResolutionX() << "\n"
	      << "Hieght : " << modes[i].getResolutionY() << "\n"
	      << "FPS    : " << modes[i].getFps() << "\n"
	      << "Pixel Fromat : "
	      << getPixelFormatAsString(modes[i].getPixelFormat())<< "\n\n";
  }
}

int main(int argc, char *argv[]) {
  // Initialize SDL
  SDL_Init(SDL_INIT_VIDEO);

  // Initialize OpenNI
  openni::Status rc = openni::OpenNI::initialize();
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to initialize.");
    return 1;
  }

  // Get device
  openni::Device device;
  rc = device.open(openni::ANY_DEVICE);
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to open device.");
    return 2;
  }
  // Get depth stream
  auto* s_info = device.getSensorInfo(openni::SENSOR_DEPTH);
  if (!s_info) {
    std::cout << "No depth sensor found.\n";
    return 3;
  }
  openni::VideoStream depth;
  rc = depth.create(device, openni::SENSOR_DEPTH);
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to create depth stream.");
    return 3;
  }
  // Show depth modes
  //printSupportedVideoModes(s_info);
  // Set depth mode
  rc = depth.setVideoMode(s_info->getSupportedVideoModes()[0]);
  if (openni::STATUS_OK != rc) {
    std::cout << "Failed to set depth mode.\n";
    return 4;
  }
  // Get depth resolution
  openni::VideoMode mode = depth.getVideoMode();
  size_t depthW = mode.getResolutionX();
  size_t depthH = mode.getResolutionY();
  // Get color stream
  s_info = device.getSensorInfo(openni::SENSOR_COLOR);
  if (!s_info) {
    std::cout << "No color sensor found.\n";
    return 3;
  }
  openni::VideoStream color;
  rc = color.create(device, openni::SENSOR_COLOR);
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to create color stream.");
    return 3;
  }
  // Show color modes
  //printSupportedVideoModes(s_info);
  // Set color mode
  rc = color.setVideoMode(s_info->getSupportedVideoModes()[0]);
  if (openni::STATUS_OK != rc) {
    std::cout << "Failed to set color mode.\n";
    return 4;
  }
  // Get color resolution
  mode = color.getVideoMode();
  size_t colorW = mode.getResolutionX();
  size_t colorH = mode.getResolutionY();

  // Set image registration mode
  if (device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
    rc = device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
    if (rc != openni::STATUS_OK)
      printOpenNIError("Failed to set registration mode.\n");
  } else {
    std::cout << "Device DOES NOT support image registration mode.\n";
  }

  // Syncronyze depth and color
  device.setDepthColorSyncEnabled(true);
  
  // Start depth stream
  rc = depth.start();
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to start the depth stream.\n");
    return 5;
  }
  
  // Start color stream
  rc = color.start();
  if (rc != openni::STATUS_OK) {
    printOpenNIError("Failed to start the color stream\n");
    return 5;
  }
  
  // Create window and renderer
  SDL_Window *pWin = SDL_CreateWindow("Hello Xtion", 0, 0, depthW+colorW, depthH+colorH, 0);
  SDL_Renderer *pRndr = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED);

  // Create texture
  int channel = 4;
  SDL_Texture *pTxt = SDL_CreateTexture(pRndr, SDL_PIXELFORMAT_BGRA8888,
					SDL_TEXTUREACCESS_STREAMING, depthW+colorW, depthH+colorH);
  // Mask
  SDL_Rect depthRect, colorRect;
  depthRect.x = 0; depthRect.y = 0; depthRect.w = depthW; depthRect.h = depthH;
  colorRect.x = depthW; colorRect.y = depthH; colorRect.w = colorW; colorRect.h = colorH;

  // Create conversion buffer
  uint8_t* pDepthBuff = new uint8_t[depthW * depthH * channel];
  uint8_t* pColorBuff = new uint8_t[colorW * colorH * channel];
  for (size_t i=0; i<depthW * depthH * channel; ++i)
    pDepthBuff[i] = 0;
  for (size_t i=0; i<colorW * colorH * channel; ++i)
    pColorBuff[i] = 0;

  // Main loop
  openni::VideoFrameRef depthFrame, colorFrame;
  while (1) {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
	break;
      }
    }
    // Wait for the next depth frame to be ready
    int changedStreamDummy;
    openni::VideoStream* pDepthStream = &depth;
    rc = openni::OpenNI::waitForAnyStream(&pDepthStream, 1,
					  &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    if (rc != openni::STATUS_OK) {
      printOpenNIError("Wait failed!");
      continue;
    }
    // Read depth frame
    rc = depth.readFrame(&depthFrame);
    if (rc != openni::STATUS_OK) {
      printOpenNIError("Failed to read depth frame!");
      continue;
    }
    // Wait for the next color frame to be ready
    openni::VideoStream* pColorStream = &color;
    rc = openni::OpenNI::waitForAnyStream(&pColorStream, 1,
					  &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    if (rc != openni::STATUS_OK) {
      printOpenNIError("Wait failed!");
      continue;
    }
    // Read color frame
    rc = color.readFrame(&colorFrame);
    if (rc != openni::STATUS_OK) {
      printOpenNIError("Failed to read color frame!");
      continue;
    }

    // Convert (1 Channel [uint16] -> 3 Channel [uint8])
    openni::DepthPixel* pDepth = (openni::DepthPixel*)depthFrame.getData();
    for (int h=0; h<depthH; ++h) {
      for (int w=0; w<depthW; ++w) {
	pDepthBuff[h*depthW*channel + w*channel + 0] = *pDepth / 256;
	pDepthBuff[h*depthW*channel + w*channel + 2] = *pDepth % 256;
	++pDepth;
      }
    }
    // Convert (3 Channel [uint8] -> 4 Channel [uint8])
    uint8_t* pColor = (uint8_t*)colorFrame.getData();
    for (int h=0; h<colorH; ++h) {
      for (int w=0; w<colorW; ++w) {
	pColorBuff[h*colorW*channel + w*channel + 1] = pColor[0];
	pColorBuff[h*colorW*channel + w*channel + 2] = pColor[1];
	pColorBuff[h*colorW*channel + w*channel + 3] = pColor[2];
	pColor += 3;
      }
    }
    // Release frame
    depthFrame.release();
    colorFrame.release();
    // Copy to texture
    SDL_UpdateTexture(pTxt, &depthRect, (const void*)pDepthBuff, depthW*channel);
    SDL_UpdateTexture(pTxt, &colorRect, (const void*)pColorBuff, colorW*channel);
    // Render
    SDL_RenderClear(pRndr);
    SDL_RenderCopy(pRndr, pTxt, NULL, NULL);
    SDL_RenderPresent(pRndr);
    //SDL_Delay(10);
  }
  delete[] pDepthBuff;
  SDL_DestroyTexture(pTxt);
  SDL_DestroyRenderer(pRndr);
  SDL_DestroyWindow(pWin);
  SDL_Quit();

  return 0;
}

