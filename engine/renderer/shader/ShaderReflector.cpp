#include "ShaderReflector.h"

namespace Vision
{

ShaderReflector::ShaderReflector(const ShaderSPIRV& shaderCode)
  : reflector(shaderCode.SPIRV)
{
  reflector.compile();
}

glm::ivec3 ShaderReflector::GetThreadgroupSize() const
{
  auto entries = reflector.get_entry_points_and_stages();
  auto workSize = reflector.get_entry_point(entries.front().name, entries.front().execution_model).workgroup_size;
  return { workSize.x, workSize.y, workSize.z };
}

std::vector<ShaderReflector::UniformBuffer> ShaderReflector::GetUniformBuffers() const
{
  std::vector<UniformBuffer> uniforms;
  
  auto res = reflector.get_shader_resources();
  for (auto uniform : res.uniform_buffers)
  {
    UniformBuffer ubo;
    ubo.Name = uniform.name;
    ubo.Binding = reflector.get_decoration(uniform.id, spv::DecorationBinding);

    uniforms.push_back(ubo);
  }

  return std::move(uniforms);
}

std::vector<ShaderReflector::SampledImage> ShaderReflector::GetSampledImages() const
{
  std::vector<SampledImage> images;

  auto res = reflector.get_shader_resources();
  for (auto image : res.sampled_images)
  {
    SampledImage s;
    s.Name = image.name;
    s.Binding = reflector.get_decoration(image.id, spv::DecorationBinding);

    const spirv_cross::SPIRType& type = reflector.get_type(image.type_id);
    if (type.array.size())
      s.ArraySize = type.array.front();
    else
      s.ArraySize = 1;

    images.push_back(s);
  }

  return std::move(images);
}

}