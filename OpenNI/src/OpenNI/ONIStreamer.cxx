#include "OpenNI/ONIStreamer.hpp"

using namespace openni;

const char* sensorStr(const SensorType type) {
  switch (type) {
  case SENSOR_IR:
    return "SENSOR_IR";
  case SENSOR_COLOR:
    return "SENSOR_COLOR";
  case SENSOR_DEPTH:
    return "SENSOR_DEPTH";
  }
}

const char* formatStr(const PixelFormat val) {
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
	   mode[i].getFps(), formatStr(mode[i].getPixelFormat()));
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
  , streams_()
  , frames_()
  , streaming_(false)
  , frameMutex_()
  , frameUpdater_()
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
	  "No sensor for ", sensorStr(type), " found.");

  auto& videomodes = info->getSupportedVideoModes();
  int nModes = videomodes.getSize();
  if (mode < 0 || nModes <= mode)
    throw RuntimeError(__func__, ":", sensorStr(type), ": ",
		       "Invalid video mode (", mode, "). ",
		       "Value range [0, ", nModes, ").");

  auto& stream = streams_[type-1];
  if (STATUS_OK != stream.create(device_, type))
    throw RuntimeError(__func__, ": Failed to create ",
		       sensorStr(type), " stream.");

  if (STATUS_OK != stream.setVideoMode(videomodes[mode]))
    throw RuntimeError(__func__, ":", sensorStr(type), ": ",
		       "Failed to set video mode (", mode, ").");

  if (mirroring != stream.getMirroringEnabled()){
    if (STATUS_OK != stream.setMirroringEnabled(mirroring))
      throw RuntimeError(__func__, ":", sensorStr(type), ": Failed ",
			 "to ", (mirroring)? "en" : "dis", "able mirroring.");
  }
}

void ONIStreamer::createDepthStream(const int mode, bool mirroring) {
  createStream(SENSOR_DEPTH, mode, mirroring);
}

void ONIStreamer::updateFrameRef(const openni::SensorType type) {
  int dummy = 0;
  VideoStream* pStream = &streams_[type-1];
  while (streaming_) {
    OpenNI::waitForAnyStream(&pStream, 1, &dummy); // <- Is this fine?
    std::lock_guard<std::mutex> _(frameMutex_[type-1]);
    if (frames_[type-1].isValid())
      frames_[type-1].release();
    if (STATUS_OK != pStream->readFrame(&frames_[type-1]))
      printf("%s:Failed to read frame %s.", __func__, sensorStr(type));
    else
      printf("Frame %s, updated: %llu.\n", sensorStr(type), frames_[type-1].getTimestamp());
  }
};

void ONIStreamer::getFrame(const SensorType type, void* pBuff, uint64_t *time) {
  if (!pBuff)
    throw RuntimeError(__func__, ": Invalid destination buffer.");

  std::lock_guard<std::mutex> _(frameMutex_[type-1]);
  auto& frame = frames_[type-1];
  if (!frame.isValid())
    throw RuntimeError(__func__, ": Invalid frame.");
  if (time)
    *time = frame.getTimestamp();
  uint size = frame.getDataSize();
  auto *pDst = static_cast<uint8_t*>(pBuff);
  auto *pSrc = static_cast<const uint8_t*>(frame.getData());
  for (uint i=0; i<size; ++i)
    pDst[i] = pSrc[i];

}

void ONIStreamer::startStreaming() {
  SensorType types[] = {SENSOR_IR, SENSOR_COLOR, SENSOR_DEPTH};
  for (auto& type : types) {
    auto& stream = streams_[type-1];
    auto& thread = frameUpdater_[type-1];
    if (stream.isValid()) {
      printf("Starting stream %s\n", sensorStr(type));
      if (STATUS_OK != stream.start())
	throw RuntimeError(__func__, "Failed to start ", sensorStr(type));
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
