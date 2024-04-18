#pragma once

namespace Vision
{

class RenderDevice
{
public:
  RenderDevice();
  virtual ~RenderDevice();

  std::size_t GetNextID() { return m_CurrentID++; }

protected:
  std::size_t m_CurrentID = 1; // Each render object will be assigned an ID at creation
};

}