#include "NIDevice.hpp"
#include "io.hpp"

#include <stdexcept>
#include <stdio.h>
#include <string>

#define WAIT_TIMEOUT 500 //2000ms

using namespace openni;

const char * getPixelFormatString(const PixelFormat& val) {
  switch (val) {
  case PIXEL_FORMAT_DEPTH_1_MM:
    return "PIXEL_FORMAT_DEPTH_1_MM";
  case PIXEL_FORMAT_DEPTH_100_UM:
    return "PIXEL_FORMAT_DEPTH_100_UM";
  case PIXEL_FORMAT_SHIFT_9_2:
    return "PIXEL_FORMAT_SHIFT_9_2";
  case PIXEL_FORMAT_SHIFT_9_3:
    return "PIXEL_FORMAT_SHIFT_9_3";
  case PIXEL_FORMAT_RGB888:
    return "PIXEL_FORMAT_RGB888";
  case PIXEL_FORMAT_YUV422:
    return "PIXEL_FORMAT_YUV422";
  case PIXEL_FORMAT_GRAY8:
    return "PIXEL_FORMAT_GRAY8";
  case PIXEL_FORMAT_GRAY16:
    return "PIXEL_FORMAT_GRAY16";
  default:
    return "UNKNOWN";
  }
}

void printSupportedVideoModes(const SensorInfo* info) {
  auto& mode = info->getSupportedVideoModes();
  for (int i=0; i<mode.getSize(); ++i) {
    printf("%15s: %d\n", "Video Mode", i);
    printf("  %13s: %d\n", "Width", mode[i].getResolutionX());
    printf("  %13s: %d\n", "Height", mode[i].getResolutionY());
    printf("  %13s: %d\n", "FPS", mode[i].getFps());
    printf("  %13s: %s\n", "Pixel Format",
	   getPixelFormatString(mode[i].getPixelFormat()));
    printf("\n");
  }
}

void NIDevice::initONI() {
  Status rc = OpenNI::initialize();
  if (rc != STATUS_OK)
    throw std::runtime_error("Failed to initialize OpenNI.");  
}

void NIDevice::quitONI() {
  OpenNI::shutdown();
}

NIDevice::NIDevice()
  : device()
  , depthStream()
  , colorStream()
  , IRStream()
  , depthFrame()
  , colorFrame()
  , IRFrame()
{}

NIDevice::~NIDevice() {
  IRStream.stop();
  IRStream.destroy();
  colorStream.stop();
  colorStream.destroy();
  depthStream.stop();
  depthStream.destroy();
  device.close();
}

void NIDevice::initDevice(int depthMode, int colorMode, int IRMode) {
  openDevice();
  if (-1 < depthMode) {
    createDepthStream();
    setDepthMode(depthMode);
  }
  if (-1 < colorMode) {
    createColorStream();
    setColorMode(colorMode);
  }
  if (-1 < IRMode) {
    createIRStream();
    setIRMode(IRMode);
  }
  if (-1 < depthMode && -1 < colorMode) {
    setImageRegistration();
    setDepthColorSync();
  }
}

void NIDevice::openDevice(const char* uri) {
  if (STATUS_OK != device.open(uri))
    throw std::runtime_error("Failed to open device.");
}

void NIDevice::listAllSensorModes() {
  printf("IR Sensor:\n");
  if (device.hasSensor(SENSOR_IR))
    printSupportedVideoModes(device.getSensorInfo(SENSOR_IR));
  else
    printf("Not available.\n\n");

  printf("Depth Sensor:\n");
  if (device.hasSensor(SENSOR_DEPTH))
    printSupportedVideoModes(device.getSensorInfo(SENSOR_DEPTH));
  else
    printf("Not available.\n\n");

  printf("RGB Sensor:\n");
  if (device.hasSensor(SENSOR_COLOR))
    printSupportedVideoModes(device.getSensorInfo(SENSOR_COLOR));
  else
    printf("Not avaibale.\n\n");
}

void NIDevice::createDepthStream() {
  if (device.getSensorInfo(SENSOR_DEPTH)) {
    if (STATUS_OK != depthStream.create(device, SENSOR_DEPTH))
      throw std::runtime_error("Failed to create depth stream.");
  } else {
    throw std::runtime_error("No depth sensor found in the device.");
  }
}

void NIDevice::createColorStream() {
  if (device.getSensorInfo(SENSOR_COLOR)) {
    if (STATUS_OK != colorStream.create(device, SENSOR_COLOR))
      throw std::runtime_error("Failed to create color stream.");
  } else {
    throw std::runtime_error("No color sensor found in the device.");
  }
}

void NIDevice::createIRStream() {
  if (device.getSensorInfo(SENSOR_IR)) {
    if (STATUS_OK != IRStream.create(device, SENSOR_IR))
      throw std::runtime_error("Failed to create IR stream.");
  } else {
    throw std::runtime_error("No IR sensor found in the device.");
  }
}

void NIDevice::setDepthMode(const int i) {
  auto* info = device.getSensorInfo(SENSOR_DEPTH);
  if (!info)
    throw std::runtime_error("No depth sensor in the device");
  auto& modes  = info->getSupportedVideoModes();
  if (i > modes.getSize())
    throw std::runtime_error("Invliad depth mode was given.");
  if (STATUS_OK != depthStream.setVideoMode(modes[i]))
    throw std::runtime_error("Failed to set depth mode.");
}

void NIDevice::setColorMode(const int i) {
  auto* info = device.getSensorInfo(SENSOR_COLOR);
  if (!info)
    throw std::runtime_error("No color sensor in the device");
  auto& modes  = info->getSupportedVideoModes();
  if (i > modes.getSize())
    throw std::runtime_error("Invliad color mode was given.");
  if (STATUS_OK != colorStream.setVideoMode(modes[i]))
    throw std::runtime_error("Failed to set color mode.");
}

void NIDevice::setIRMode(const int i) {
  auto* info = device.getSensorInfo(SENSOR_IR);
  if (!info)
    throw std::runtime_error("No IR sensor in the device");
  auto& modes  = info->getSupportedVideoModes();
  if (i > modes.getSize())
    throw std::runtime_error("Invliad IR mode was given.");
  if (STATUS_OK != IRStream.setVideoMode(modes[i]))
    throw std::runtime_error("Failed to set IR mode.");
}

void NIDevice::getDepthResolution(uint& w, uint& h) const {
  VideoMode mode = depthStream.getVideoMode();
  w = mode.getResolutionX();
  h = mode.getResolutionY();
}

void NIDevice::getColorResolution(uint& w, uint& h) const {
  VideoMode mode = colorStream.getVideoMode();
  w = mode.getResolutionX();
  h = mode.getResolutionY();
}

void NIDevice::getIRResolution(uint& w, uint& h) const {
  VideoMode mode = IRStream.getVideoMode();
  w = mode.getResolutionX();
  h = mode.getResolutionY();
}

uint NIDevice::getIRChannel() const {
  PixelFormat format = IRStream.getVideoMode().getPixelFormat();
  switch (format) {
  case PIXEL_FORMAT_RGB888:
    return 3;
  case PIXEL_FORMAT_GRAY16:
  default:
    return 1;
  }
}

void NIDevice::setImageRegistration(const bool enable) {
  if (enable) {
    if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
      if (STATUS_OK != device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	throw std::runtime_error("Failed to enable depth to color registration.");
    } else {
      throw std::runtime_error("The device does not support depth to color registration.");
    }
  } else {
    if (STATUS_OK != device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF))
	throw std::runtime_error("Failed to disable registration mode.");
  }
}

void NIDevice::setDepthColorSync(const bool enable) {
  if (STATUS_OK != device.setDepthColorSyncEnabled(enable))
    throw std::runtime_error("Failed to set depth/color sync mode.");
}

void NIDevice::startDepthStream() {
  if (STATUS_OK != depthStream.start())
    throw std::runtime_error("Failed to start depth stream.");
}

void NIDevice::startColorStream() {
  if (STATUS_OK != colorStream.start())
    throw std::runtime_error("Failed to start color stream.");
}

void NIDevice::startIRStream() {
  if (STATUS_OK != IRStream.start())
    throw std::runtime_error("Failed to start IR stream.");
}

void NIDevice::readDepthStream() {
  int dummy = 0;
  VideoStream* pStream = &depthStream;
  if (STATUS_OK != OpenNI::waitForAnyStream(&pStream, 1, &dummy, WAIT_TIMEOUT))
    throw std::runtime_error("Depth stream timeout.");
  if (STATUS_OK != depthStream.readFrame(&depthFrame))
    throw std::runtime_error("Failed to read depth frame.");
}

void NIDevice::readColorStream() {
  int dummy = 0;
  VideoStream* pStream = &colorStream;
  if (STATUS_OK != OpenNI::waitForAnyStream(&pStream, 1, &dummy, WAIT_TIMEOUT))
    throw std::runtime_error("Color stream timeout.");
  if (STATUS_OK != colorStream.readFrame(&colorFrame))
    throw std::runtime_error("Failed to read color frame.");
}

void NIDevice::readIRStream() {
  int dummy = 0;
  VideoStream* pStream = &IRStream;
  if (STATUS_OK != OpenNI::waitForAnyStream(&pStream, 1, &dummy, WAIT_TIMEOUT))
    throw std::runtime_error("IR stream timeout.");
  if (STATUS_OK != IRStream.readFrame(&IRFrame))
    throw std::runtime_error("Failed to read IR frame.");
}

void NIDevice::copyDepthFrame(uint16_t* pDst, int offset, int padding) {
  uint width=0, height=0;
  getDepthResolution(width, height);
  const void *pSrc = static_cast<const void *>(depthFrame.getData());
  copyFrame(pSrc, pDst, width, height, 2, offset, padding);
}

void NIDevice::copyColorFrame(uint8_t* pDst, int offset, int padding) {
  uint width=0, height=0;
  getColorResolution(width, height);
  const void *pSrc = static_cast<const void *>(colorFrame.getData());
  copyFrame(pSrc, pDst, width, height, 3, offset, padding);
}

void NIDevice::copyIRFrame(uint8_t* const pDst, int offset, int padding) {
  uint width=0, height=0;
  getIRResolution(width, height);
  const void *pSrc = static_cast<const void *>(IRFrame.getData());
  copyFrame(pSrc, pDst, width, height, 3, offset, padding);
}

void NIDevice::convertDepthFrameToJet(uint8_t* pDst, const uint color_format,
				      const uint16_t v_min, const uint16_t v_max){
  uint width=0, height=0;
  getDepthResolution(width, height);
  const uint16_t *pSrc = static_cast<const uint16_t *>(depthFrame.getData());
  // Modification for PIXEL_FORMAT_DEPTH_100_UM
  uint16_t c = 1;
  if (PIXEL_FORMAT_DEPTH_100_UM == depthStream.getVideoMode().getPixelFormat())
    c = 100;
  convert16BitFrameToJet(pSrc, pDst, width, height, color_format, v_min*c, v_max*c);
}

void NIDevice::convertIRFrameToJet(uint8_t* pDst, const uint color_format,
				   const uint16_t v_min, const uint16_t v_max){
  uint width=0, height=0;
  getIRResolution(width, height);
  const uint16_t *pSrc = static_cast<const uint16_t *>(IRFrame.getData());
  convert16BitFrameToJet(pSrc, pDst, width, height, color_format, v_min, v_max);
}

void NIDevice::releaseDepthFrame() {
  depthFrame.release();
}

void NIDevice::releaseColorFrame() {
  colorFrame.release();
}

void NIDevice::releaseIRFrame() {
  IRFrame.release();
}
