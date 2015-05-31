#include "NIDevice.hpp"
#include "io.hpp"

#define WAIT_TIMEOUT 500 // [ms]

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

Streamer::Streamer()
  : m_stream()
  , m_frame()
  , m_listener()
  , m_bStreaming(false)
{};

Streamer::~Streamer() {
  if (m_frame.isValid())
    m_frame.release();
  if (m_stream.isValid()) {
    m_stream.stop();
    m_stream.destroy();
  }
}

void Streamer::create(Device &device, const SensorType type,
				const int mode, const bool mirroring) {
  const openni::SensorInfo* info = device.getSensorInfo(type);
  if (!info)
    throw RuntimeError({__func__, ":",
	  "No sensor for ", getSensorTypeString(type), " found."});

  if (STATUS_OK != m_stream.create(device, type))
    throw RuntimeError({__func__, ":",
	  "Failed to create ", getSensorTypeString(type), " stream."});

  auto& videomodes = info->getSupportedVideoModes();
  int nModes = videomodes.getSize();
  if (mode < 0 || nModes <= mode)
    throw RuntimeError({__func__, ":", getSensorTypeString(type), ":",
	  "Invalid video mode ",  std::to_string(mode), " was given. ",
	  "Value range [0, ", std::to_string(nModes)});

  if (STATUS_OK != m_stream.setVideoMode(videomodes[mode]))
    throw RuntimeError({__func__, ":", getSensorTypeString(type), ":",
	  "Failed to set video mode."});

  if (mirroring != m_stream.getMirroringEnabled()){
    if (STATUS_OK != m_stream.setMirroringEnabled(mirroring))
      throw RuntimeError({__func__, ":", getSensorTypeString(type), ":",
	    "Failed to set mirroring mode."});
  }

  m_listener.setCallbackFcn([=](){
      std::lock_guard<std::mutex> _(m_frameMutex);
      if (m_frame.isValid())
	m_frame.release();
      if (STATUS_OK != m_stream.readFrame(&m_frame))
	throw RuntimeError({"Callback", ":", getSensorTypeString(type), ":",
	      "Failed to read frame."});
    });
  if (STATUS_OK != m_stream.addNewFrameListener(&m_listener))
    throw RuntimeError({__func__, ":", getSensorTypeString(type), ":",
	  "Failed to add event listener."});
}

void Streamer::start() {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Video stream is not initialized."});

  if (STATUS_OK != m_stream.start())
    throw RuntimeError({__func__, ":", "Faild to start stream."});

  m_bStreaming = true;
};

void Streamer::stop() {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Stream is not initialized."});

  m_stream.stop();

  m_bStreaming = false;
}

bool Streamer::isStreamValid() const {
  return m_stream.isValid();
};

bool Streamer::isStreaming() const {
  return m_bStreaming;
}

bool Streamer::isFrameValid() const {
  return m_frame.isValid();
};

uint Streamer::getWidth() const {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Video stream is not initialized."});
  return m_stream.getVideoMode().getResolutionX();
}

uint Streamer::getHeight() const {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Video stream is not initialized."});
  return m_stream.getVideoMode().getResolutionY();
}

uint Streamer::getNumChannels() const {
  PixelFormat format = m_stream.getVideoMode().getPixelFormat();
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
    throw RuntimeError({__func__, ":", "Not implemented for this video mode."});
  }
}

uint Streamer::getMinValue() const {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Video stream is not initialized."});
  return m_stream.getMinPixelValue();
}

uint Streamer::getMaxValue() const {
  if (!m_stream.isValid())
    throw RuntimeError({__func__, ":", "Video stream is not initialized."});
  return m_stream.getMaxPixelValue();
}

void Streamer::copyTo(void* pDst, const uint offset, const uint padding) {
  std::lock_guard<std::mutex> _(m_frameMutex);
  uint width = m_frame.getWidth();
  uint height = m_frame.getHeight();
  uint BPP = m_frame.getStrideInBytes() / width;
  const void *pSrc = static_cast<const void *>(m_frame.getData());
  ::copyFrame(pSrc, pDst, width, height, BPP, offset, padding);
}

Listener::Listener()
  : m_callbackFcn()
{};

Listener::~Listener() {
};

void Listener::onNewFrame (VideoStream &stream) {
  m_callbackFcn();
};

void Listener::setCallbackFcn(std::function<void()> fcn) {
  m_callbackFcn = fcn;
};

void NIDevice::initONI() {
  Status rc = OpenNI::initialize();
  if (rc != STATUS_OK)
    throw RuntimeError({__func__, ":", "Failed to initialize OpenNI."});
}

void NIDevice::quitONI() {
  OpenNI::shutdown();
}

NIDevice::NIDevice()
  : m_device()
  , m_streamers()
{}

NIDevice::~NIDevice() {
  if (m_device.isValid())
    m_device.close();
}

void NIDevice::openDevice(const char* uri) {
  if (!m_device.isValid()) {
    if (STATUS_OK != m_device.open(uri))
      throw RuntimeError({__func__, ":",
	    "Failed to open ", (uri)? uri:"ANY_DEVICE", "."});
  }
}

void NIDevice::listAllSensorModes() {
  printf("IR Sensor:\n");
  if (m_device.hasSensor(SENSOR_IR))
    printSupportedVideoModes(m_device.getSensorInfo(SENSOR_IR));
  else
    printf("Not available.\n\n");

  printf("Depth Sensor:\n");
  if (m_device.hasSensor(SENSOR_DEPTH))
    printSupportedVideoModes(m_device.getSensorInfo(SENSOR_DEPTH));
  else
    printf("Not available.\n\n");

  printf("RGB Sensor:\n");
  if (m_device.hasSensor(SENSOR_COLOR))
    printSupportedVideoModes(m_device.getSensorInfo(SENSOR_COLOR));
  else
    printf("Not avaibale.\n\n");
}

void NIDevice::createStream(const SensorType type, const int mode, bool mirroring) {
  m_streamers[type-1].create(m_device, type, mode, mirroring);
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

uint NIDevice::getWidth(SensorType type) const {
  return m_streamers[type-1].getWidth();
}

uint NIDevice::getHeight(SensorType type) const {
  return m_streamers[type-1].getHeight();
}

uint NIDevice::getNumChannels(SensorType type) const {
  return m_streamers[type-1].getNumChannels();
}

uint NIDevice::getDepthWidth() const {
  return getWidth(SENSOR_DEPTH);
}

uint NIDevice::getDepthHeight() const {
  return getHeight(SENSOR_DEPTH);
}

uint NIDevice::getColorWidth() const {
  return getWidth(SENSOR_COLOR);
}

uint NIDevice::getColorHeight() const {
  return getHeight(SENSOR_COLOR);
}

uint NIDevice::getIRWidth() const {
  return getWidth(SENSOR_IR);
}

uint NIDevice::getIRHeight() const {
  return getHeight(SENSOR_IR);
}

uint NIDevice::getIRNumChannels() const {
  return getNumChannels(SENSOR_IR);
}

int NIDevice::getMinValue(SensorType type) const {
  return m_streamers[type-1].getMinValue();
}

int NIDevice::getMaxValue(SensorType type) const {
  return m_streamers[type-1].getMaxValue();
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
    if (m_device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
      if (STATUS_OK != m_device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	throw RuntimeError({__func__,
	      "Failed to enable depth to color registration."});
    } else {
      throw RuntimeError({__func__,
	    "The device does not support depth to color registration."});
    }
  } else {
    if (STATUS_OK != m_device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF))
      throw RuntimeError({__func__,
	    "Failed to disable registration mode."});
  }
}

void NIDevice::setDepthColorSync(const bool enable) {
  if (STATUS_OK != m_device.setDepthColorSyncEnabled(enable))
    throw RuntimeError({__func__, ":", "Failed to set depth/color sync mode."});
}

void NIDevice::startStream(SensorType type) {
  m_streamers[type-1].start();
}

void NIDevice::startStreams() {
  for (int i=0; i<3; ++i) {
    if (m_streamers[i].isStreamValid())
      m_streamers[i].start();
  }
}

void NIDevice::waitStreamsToGetReady() const {
  // Check if any stream is valid
  bool isAnyStreamStreaming = false;
  for (int i=0; i<3; ++i) {
    if (m_streamers[i].isStreaming())
      isAnyStreamStreaming = true;
  }
  if (!isAnyStreamStreaming)
    throw RuntimeError({__func__, ":", "No streaming sensor."});

  bool allFramesReady = false;
  while (!allFramesReady) {
    for (int i=0; i<3; ++i) {
      if (m_streamers[i].isStreaming())
	allFramesReady = (m_streamers[i].isFrameValid())? true : false;
    }
  }
}

void NIDevice::stopStream(SensorType type) {
  m_streamers[type-1].stop();
}

void NIDevice::stopStreams() {
  for (int i=0; i<3; ++i) {
    if (m_streamers[i].isStreaming())
      m_streamers[i].stop();
  }
}

void NIDevice::copyFrame(SensorType type,
			 void* pDst, int offset, int padding) {
  m_streamers[type-1].copyTo(pDst, offset, padding);
}

void NIDevice::copyDepthFrame(void* pDst, int offset, int padding) {
  copyFrame(SENSOR_DEPTH, pDst, offset, padding);
}

void NIDevice::copyColorFrame(void* pDst, int offset, int padding) {
  copyFrame(SENSOR_COLOR, pDst, offset, padding);
}

void NIDevice::copyIRFrame(void* pDst, int offset, int padding) {
  copyFrame(SENSOR_IR, pDst, offset, padding);
}
