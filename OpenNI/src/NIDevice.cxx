#include "NIDevice.hpp"
#include "io.hpp"

#include <stdexcept>
#include <cstdio>
#include <string>

#define WAIT_TIMEOUT 500 // [ms]

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
  for (int i=0; i<mode.getSize(); ++i)
    printf("  %2d, W:%5d, H:%5d, FPS:%3d, %s\n",
	   i,  mode[i].getResolutionX(), mode[i].getResolutionY(),
	   mode[i].getFps(), getPixelFormatString(mode[i].getPixelFormat()));
  printf("\n");
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
  , streams()
  , frames()
  , frame_mutex()    
{}

NIDevice::~NIDevice() {
  if (device.isValid()) {
    for (auto& stream: streams) {
      if (stream.isValid()) {
	stream.stop();
	stream.destroy();
      }
    }
    device.close();
  }
}

void NIDevice::openDevice(const char* uri) {
  if (!device.isValid()) {
    if (STATUS_OK != device.open(uri))
      throw std::runtime_error("Failed to open device.");
  }
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

void NIDevice::createStream(const SensorType type, const int mode, bool mirroring) {
  openni::VideoStream& stream = streams[type-1];
  const openni::SensorInfo* info = device.getSensorInfo(type);
  if (!info)
    throw std::runtime_error("No sensor for the specified type found.");

  if (STATUS_OK != stream.create(device, type))
    throw std::runtime_error("Failed to create new stream.");

  auto& videomodes = info->getSupportedVideoModes();
  if (mode < 0 || videomodes.getSize() <= mode)
    throw std::runtime_error("Invalid video mode was given.");

  if (STATUS_OK != stream.setVideoMode(videomodes[mode]))
    throw std::runtime_error("Failed to set video mode.");

  if (mirroring != stream.getMirroringEnabled()){
    if (STATUS_OK != stream.setMirroringEnabled(mirroring))
      throw std::runtime_error("Failed to set mirroring mode.");
  }
};

void NIDevice::createDepthStream(const int mode, bool mirroring) {
  createStream(SENSOR_DEPTH, mode, mirroring);
}

void NIDevice::createColorStream(const int mode, bool mirroring) {
  createStream(SENSOR_COLOR, mode, mirroring);
}

void NIDevice::createIRStream(const int mode, bool mirroring) {
  createStream(SENSOR_IR, mode, mirroring);
}

uint NIDevice::getResolutionX(SensorType type) const {
  return streams[type-1].getVideoMode().getResolutionX();
}

uint NIDevice::getResolutionY(SensorType type) const {
  return streams[type-1].getVideoMode().getResolutionY();
}

uint NIDevice::getNumChannels(SensorType type) const {
  PixelFormat format = streams[type-1].getVideoMode().getPixelFormat();
  switch (format) {
  case PIXEL_FORMAT_DEPTH_1_MM:
  case PIXEL_FORMAT_DEPTH_100_UM: 
  case PIXEL_FORMAT_GRAY8:
  case PIXEL_FORMAT_GRAY16:
    return 1;
  case PIXEL_FORMAT_RGB888:
  case PIXEL_FORMAT_YUV422:
    return 3;
  default:
    ;
  }
  throw std::logic_error("Not implemented for this video mode.");
}

uint NIDevice::getDepthResolutionX() const {
  return getResolutionX(SENSOR_DEPTH);
}

uint NIDevice::getDepthResolutionY() const {
  return getResolutionY(SENSOR_DEPTH);
}

uint NIDevice::getColorResolutionX() const {
  return getResolutionX(SENSOR_COLOR);
}

uint NIDevice::getColorResolutionY() const {
  return getResolutionY(SENSOR_COLOR);
}

uint NIDevice::getIRResolutionX() const {
  return getResolutionX(SENSOR_IR);
}

uint NIDevice::getIRResolutionY() const {
  return getResolutionY(SENSOR_IR);
}

uint NIDevice::getIRNumChannels() const {
  return getNumChannels(SENSOR_IR);
}

int NIDevice::getMinValue(SensorType type) const {
  auto& stream = streams[type-1];
  if (!stream.isValid())
    throw std::runtime_error("Video stream is not initialized.");
  return stream.getMinPixelValue();
}

int NIDevice::getMaxValue(SensorType type) const {
  auto& stream = streams[type-1];
  if (!stream.isValid())
    throw std::runtime_error("Video stream is not initialized.");
  return stream.getMaxPixelValue();
}

int NIDevice::getDepthMinValue() const {
  return getMinValue(SENSOR_DEPTH);
}

int NIDevice::getDepthMaxValue() const {
  return getMaxValue(SENSOR_DEPTH);
}

int NIDevice::getColorMinValue() const {
  return getMinValue(SENSOR_COLOR);
}

int NIDevice::getColorMaxValue() const {
  return getMaxValue(SENSOR_COLOR);
}

int NIDevice::getIRMinValue() const {
  return getMinValue(SENSOR_IR);
}

int NIDevice::getIRMaxValue() const {
  return getMaxValue(SENSOR_IR);
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

void NIDevice::startStream(SensorType type) {
  auto& stream = streams[type-1];
  if (stream.isValid()) {
    if (STATUS_OK != stream.start())
      throw std::runtime_error("Failed to start stream.");
  }
}

void NIDevice::startAllStreams() {
  SensorType types[] = {SENSOR_IR, SENSOR_DEPTH, SENSOR_COLOR};
  for (auto& type : types) 
    startStream(type);
}

void NIDevice::stopStream(SensorType type) {
  if (streams[type-1].isValid())
    streams[type-1].stop();
}

void NIDevice::stopAllStreams() {
  SensorType types[] = {SENSOR_IR, SENSOR_DEPTH, SENSOR_COLOR};
  for (auto& type : types) 
    stopStream(type);
}

void NIDevice::getFrame(openni::SensorType type) {
  int dummy = 0;
  VideoStream* pStream = &streams[type-1];
  if (!(pStream->isValid()))
    throw std::runtime_error("Stream not initialized.");
  releaseFrame(type);
  if (STATUS_OK != OpenNI::waitForAnyStream(&pStream, 1, &dummy, WAIT_TIMEOUT))
    throw std::runtime_error("Stream timeout.");
  if (STATUS_OK != streams[type-1].readFrame(&frames[type-1]))
    throw std::runtime_error("Failed to read frame.");
}

void NIDevice::getAllFrames() {
  SensorType types[] = {SENSOR_IR, SENSOR_DEPTH, SENSOR_COLOR};
  for (int i=0; i<3; ++i) {
    if (streams[i].isValid())
      getFrame(types[i]);
  }
}

void NIDevice::releaseFrame(openni::SensorType type) {
  auto& frame = frames[type-1];
  if (frame.isValid())
    frame.release();
}

void NIDevice::releaseAllFrames() {
  SensorType types[] = {SENSOR_IR, SENSOR_DEPTH, SENSOR_COLOR};
  for (auto& type : types)
    releaseFrame(type);
}

void NIDevice::copyFrame(SensorType type,
			 void* pDst, int offset, int padding) const {
  auto& frame = frames[type-1];
  uint width = frame.getWidth();
  uint height = frame.getHeight();
  uint BPP = frame.getStrideInBytes() / width;
  const void *pSrc = static_cast<const void *>(frame.getData());
  ::copyFrame(pSrc, pDst, width, height, BPP, offset, padding);
}

void NIDevice::copyDepthFrame(void* pDst, int offset, int padding) const {
  copyFrame(SENSOR_DEPTH, pDst, offset, padding);
}

void NIDevice::copyColorFrame(void* pDst, int offset, int padding) const {
  copyFrame(SENSOR_COLOR, pDst, offset, padding);
}

void NIDevice::copyIRFrame(void* pDst, int offset, int padding) const {
  copyFrame(SENSOR_IR, pDst, offset, padding);
}

void NIDevice::convert16BitFrameTo8Bit(SensorType type,
				       uint8_t* pDst,
				       const uint color_format,
				       const uint16_t v_min,
				       const uint16_t v_max) {
  auto& frame = frames[type-1];
  uint width = frame.getWidth();
  uint height = frame.getHeight();
  uint BPP = frame.getStrideInBytes() / width;
  const uint16_t *pSrc = static_cast<const uint16_t *>(frame.getData());
  convert16BitFrameToJet(pSrc, pDst, width, height, color_format, v_min, v_max);
}

void NIDevice::convertDepthFrameToJet(uint8_t* pDst,
				      const uint color_format,
				      const uint16_t v_min,
				      const uint16_t v_max){
  // Modification for PIXEL_FORMAT_DEPTH_100_UM
  auto format = streams[SENSOR_DEPTH-1].getVideoMode().getPixelFormat();
  uint16_t c = (PIXEL_FORMAT_DEPTH_100_UM == format)? 100:1;
  convert16BitFrameTo8Bit(SENSOR_DEPTH, pDst, color_format, v_min*c, v_max*c);
}

void NIDevice::convertIRFrameToJet(uint8_t* pDst,
				    const uint color_format,
				    const uint16_t v_min,
				    const uint16_t v_max){
  convert16BitFrameTo8Bit(SENSOR_IR, pDst, color_format, v_min, v_max);
}
