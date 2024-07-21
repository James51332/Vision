#section type(compute) name(sum)
#version 450 core

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

#section type(compute) name(triple)
#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0) buffer computeBuffer
{
  float data[];
};

void main()
{
  uint gid = gl_GlobalInvocationID.x;
  data[gid] *= 3;
}