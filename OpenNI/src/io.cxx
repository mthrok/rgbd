#include "io.hpp"
#include <cmath>
#include <stdexcept>

//double interpolate(double val, double x0, double x1, double y0, double y1) {
//  return (val-x0) * (y1-y0) / (x1-x0) + y0;
//}

void jet(const uint16_t val, uint8_t& R, uint8_t& G, uint8_t& B,
	 const uint16_t v_min, const uint16_t v_max) {
  if (val < v_min) {
    R = G = B = 0;
    return;
  } else if (val > v_max) {
    R = G = B = 255;
    return;
  }
  double r = ((double)val - v_min) / (v_max - v_min);
  // MATLAB Jet style
  // http://stackoverflow.com/questions/7706339/grayscale-to-red-green-blue-matlab-jet-color-scale
  /*
  if (r < 0.125) {
    R = 0;
    G = 0;
    B = (uint8_t) (255 * interpolate(r, 0.000, 0.125, 0.5, 1.0));
  } else if (r < 0.375) {
    R = 0;
    G = (uint8_t) (255 * interpolate(r, 0.125, 0.375, 0.0, 1.0));
    B = 255;
  } else if (r < 0.625) {
    R = (uint8_t) (255 * interpolate(r, 0.375, 0.625, 0.0, 1.0));
    G = 255;
    B = (uint8_t) (255 * interpolate(r, 0.375, 0.625, 1.0, 0.0));
  } else if (r < 0.875) {
    R = 255;
    G = (uint8_t) (255 * interpolate(r, 0.625, 0.875, 1.0, 0.0));
    B = 0;
  } else {
    R = (uint8_t) (255 * interpolate(r, 0.875, 1.000, 1.0, 0.5));
    G = 0;
    B = 0;
  }
  */
  // Smoother version
  double t = 2.0 * M_PI * r;
  R = (uint8_t) (255 * (-sin(t) + 1.0) / 2.0);
  B = (uint8_t) (255 * ( sin(t) + 1.0) / 2.0);
  G = (uint8_t) (255 * (-cos(t) + 1.0) / 2.0);
  if (r < 0.25)
    R = 0;
  if (r > 0.75)
    B = 0;
}

void convert16BitFrameToJet(const uint16_t* pSrc, uint8_t* pDst,
			    const uint width, const uint height, const uint mode,
			    const uint16_t v_min, const uint16_t v_max) {
  uint8_t R=0, G=1, B=2, channel=3;
  switch (mode) {
  case 1: // ARGB == SDL_PIXELFORMAT_BGRA8888
    R=1; G=2; B=3;
    channel = 4;
    break;
  default:
    break;
  }
  for (uint h=0; h<height; ++h) {
    for (uint w=0; w<width; ++w) {
      jet(*pSrc, pDst[R], pDst[G], pDst[B], v_min, v_max);
      ++pSrc; pDst += channel;
    }
  }
}

void copyFrame(const void* pSrc, void* pDst,
	       const uint width, const uint height,
	       const uint BPP, const int offset, const int padding) {
  const uint8_t* pSrcBuff = static_cast<const uint8_t *>(pSrc);
  uint8_t* pDstBuff = static_cast<uint8_t *>(pDst);
  pDstBuff += offset;
  for (uint h=0; h<height; ++h) {
    for (uint w=0; w<width; ++w) {
      for (uint b=0; b<BPP; ++b) {
	*pDstBuff = *pSrcBuff;
	++pDstBuff; ++pSrcBuff;
      }
      pDstBuff += padding;
    }
  }
}

Frames::Frames()
  : m_width(0)
  , m_height(0)
  , m_BPP(0)
  , m_nFrames(0)
  , m_currentFrame(0)
  , m_pBuffer(NULL)
{}

Frames::~Frames() {
  deallocate();
}

void Frames::deallocate() {
  delete[] static_cast<uint8_t *>(m_pBuffer);
  m_pBuffer = NULL;
}

void Frames::allocate(uint width, uint height, uint BPP, uint nFrames) {
  if (nFrames == 0)
    throw std::runtime_error("#Frames cannnot be 0.");
  m_width = width; m_height = height; m_BPP = BPP; m_nFrames = nFrames;
  deallocate();
  size_t buffersize = size_t(m_nFrames) * m_width * m_height * m_BPP;
  m_pBuffer = static_cast<void *>(new uint8_t[buffersize]);
}

uint Frames::getWidth() { return m_width; }

uint Frames::getHeight() { return m_height; }

uint Frames::getNumFrames() { return m_nFrames; }

uint Frames::getFrameIndex() { return m_currentFrame; }

void Frames::setFrameIndex(int iFrame) {
  iFrame %= m_nFrames;
  if (iFrame < 0)
    iFrame += m_nFrames;
  m_currentFrame = iFrame;
}

void Frames::incrementFrameIndex() {
  setFrameIndex(m_currentFrame + 1);
}

void* Frames::getFrame(int iFrame){
  if (iFrame < 0)
    iFrame = m_currentFrame;
  uint8_t * pFrame = static_cast<uint8_t *>(m_pBuffer);
  pFrame += (size_t)iFrame * m_width * m_height * m_BPP;
  return static_cast<void *>(pFrame);
}

void Frames::copyFrameTo(void* pDst, int iFrame, int offset, int padding) {
  const void *pSrc = static_cast<const void *>(getFrame(iFrame));
  copyFrame(pSrc, pDst, m_width, m_height, m_BPP, offset, padding);
}

RGBDFrames::RGBDFrames()
  : m_depthFrames()
  , m_colorFrames()
{}

RGBDFrames::~RGBDFrames() {
  deallocate();
}

void RGBDFrames::deallocate() {
  m_depthFrames.deallocate();
  m_colorFrames.deallocate();
}

void RGBDFrames::allocate(uint depthW, uint depthH, uint colorW, uint colorH, uint nFrames) {
  m_depthFrames.allocate(depthW, depthH, 2, nFrames);
  m_colorFrames.allocate(colorW, colorH, 3, nFrames);
}

uint RGBDFrames::getNumFrames() {
  return m_depthFrames.getNumFrames();
}

void RGBDFrames::incrementFrameIndex() {
  m_depthFrames.incrementFrameIndex();
  m_colorFrames.incrementFrameIndex();
}

uint RGBDFrames::getFrameIndex() {
  return m_depthFrames.getFrameIndex();
}

void RGBDFrames::setFrameIndex(int iFrame) {
  m_depthFrames.setFrameIndex(iFrame);
  m_colorFrames.setFrameIndex(iFrame);
}

uint8_t* RGBDFrames::getColorFrame(int iFrame) {
  return static_cast<uint8_t *>(m_colorFrames.getFrame(iFrame));
}

uint16_t* RGBDFrames::getDepthFrame(int iFrame) {
  return static_cast<uint16_t *>(m_depthFrames.getFrame(iFrame));
}

void RGBDFrames::copyDepthFrameTo(uint16_t* pDst, int iFrame,
				  uint offset, uint padding) {
  m_depthFrames.copyFrameTo(pDst, iFrame, offset, padding);
}

void RGBDFrames::copyColorFrameTo(uint8_t* pDst, int iFrame,
				  uint offset, uint padding) {
  m_colorFrames.copyFrameTo(pDst, iFrame, offset, padding);
};

void RGBDFrames::convert16BitFrameToJet(uint8_t* pDst, int iFrame,
					const uint16_t v_min,
					const uint16_t v_max,
					const uint color_format) {
  uint width = m_depthFrames.getWidth();
  uint height = m_depthFrames.getHeight();
  const uint16_t *pSrc = getDepthFrame(iFrame);
  ::convert16BitFrameToJet(pSrc, pDst, width, height, color_format, v_min, v_max);
}
