#include "OpenNI/ONIStreamer.hpp"

using namespace openni;

const char* getSensorTypeString(const SensorType type) {
  switch (type) {
  case SENSOR_IR:
    return "SENSOR_IR";
  case SENSOR_COLOR:
    return "SENSOR_COLOR";
  case SENSOR_DEPTH:
    return "SENSOR_DEPTH";
  }
}

const char* getPixelFormatString(const PixelFormat val) {
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
  case PIXEL_FORMAT_JPEG:
    return "PIXEL_FORMAT_JPEG";
  case PIXEL_FORMAT_YUYV:
    return "PIXEL_FORMAT_YUYV";
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

void ONIStreamer::initONI() {
  if (STATUS_OK != OpenNI::initialize())
    throw RuntimeError(__func__, ": Failed to initialize OpenNI.");
}

void ONIStreamer::quitONI() {
  OpenNI::shutdown();
}

ONIStreamer::ONIStreamer()
  : device_()
{}

ONIStreamer::~ONIStreamer() {
  if (device_.isValid())
    closeDevice();
}

void ONIStreamer::openDevice(const char* uri) {
  if (device_.isValid())
    throw RuntimeError(__func__, ": Device already opened.");
  if (STATUS_OK != device_.open(uri))
    throw RuntimeError(__func__,
		       ": Failed to open ", (uri)? uri:"ANY_DEVICE", ".");
}

void ONIStreamer::closeDevice() {
  device_.close();
}

void ONIStreamer::listAvailableSensors() {
  printf("IR Sensor:\n");
  if (device_.hasSensor(SENSOR_IR))
    printSupportedVideoModes(device_.getSensorInfo(SENSOR_IR));
  else
    printf("Not available.\n\n");

  printf("Depth Sensor:\n");
  if (device_.hasSensor(SENSOR_DEPTH))
    printSupportedVideoModes(device_.getSensorInfo(SENSOR_DEPTH));
  else
    printf("Not available.\n\n");

  printf("RGB Sensor:\n");
  if (device_.hasSensor(SENSOR_COLOR))
    printSupportedVideoModes(device_.getSensorInfo(SENSOR_COLOR));
  else
    printf("Not avaibale.\n\n");
}

void ONIStreamer::createStream(const openni::SensorType type,
			    const int mode, bool mirroring) {
  const SensorInfo* info = device_.getSensorInfo(type);
  if (!info)
    throw RuntimeError(__func__, ": ",
	  "No sensor for ", getSensorTypeString(type), " found.");

  auto& videomodes = info->getSupportedVideoModes();
  int nModes = videomodes.getSize();
  if (mode < 0 || nModes <= mode)
    throw RuntimeError(__func__, ":", getSensorTypeString(type), ": ",
		       "Invalid video mode (", mode, "). ",
		       "Value range [0, ", nModes, ").");

  auto& stream = streams_[type-1];
  if (STATUS_OK != stream.create(device_, type))
    throw RuntimeError(__func__, ": Failed to create ",
		       getSensorTypeString(type), " stream.");

  if (STATUS_OK != stream.setVideoMode(videomodes[mode]))
    throw RuntimeError(__func__, ":", getSensorTypeString(type), ": ",
		       "Failed to set video mode (", mode, ").");

  if (mirroring != stream.getMirroringEnabled()){
    if (STATUS_OK != stream.setMirroringEnabled(mirroring))
      throw RuntimeError(__func__, ":", getSensorTypeString(type), ": Failed ",
			 "to ", (mirroring)? "en" : "dis", "able mirroring.");
  }
}

void ONIStreamer::updateFrameRef(const openni::SensorType type) {
  while (streaming_) {
    // Wait for new frame to ready
    int dummy = 0;
    VideoStream* pStream = &streams_[type-1];
    OpenNI::waitForAnyStream(&pStream, 1, &dummy, WAIT_STREAM_TIMEOUT);
    // Acquire the frame
    std::lock_guard<std::mutex> _(frameMutex_[type-1]);
    if (STATUS_OK != streams_[type-1].readFrame(&frames_[type-1]))
      throw RuntimeError(__func__,
			 ": Failed to read ", getSensorTypeString(type), " Frame");
    timestamps_[type-1] = getCurrentTimestamp();
  }
};

void ONIStreamer::createDepthStream(const int mode, bool mirroring) {
  createStream(SENSOR_DEPTH, mode, mirroring);
}

void ONIStreamer::startStreaming() {
  SensorType types[] = {SENSOR_IR, SENSOR_COLOR, SENSOR_DEPTH};
  for (auto& type : types) {
    auto& stream = streams_[type-1];
    auto& thread = frameUpdater_[type-1];
    if (stream.isValid()) {
      if (STATUS_OK != stream.start())
	throw RuntimeError(__func__, "Failed to start ", getSensorTypeString(type));
      thread = std::thread(&ONIStreamer::updateFrameRef, this, type);
    }
  }
  streaming_.store(true);
}

void ONIStreamer::stopStreaming() {
  streaming_.store(false);
  SensorType types[] = {SENSOR_IR, SENSOR_COLOR, SENSOR_DEPTH};
  for (auto& type : types) {
    auto& stream = streams_[type-1];
    auto& thread = frameUpdater_[type-1];
    if (stream.isValid()) {
      thread.join();
      stream.stop();
    }
  }
}
