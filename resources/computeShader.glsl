#type compute
#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0) buffer computeBuffer
{
  float data[];
};

void main()
{
  uint gid = gl_GlobalInvocationID.x;
  data[gid] += 2;
}