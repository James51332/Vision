#section common
#version 450 core

int method()
{
    return 2;
}

#section type(computfe) name(horizontalFFT)    

layout (local_size_x = 50) in;
void main()
{

}

#section type(compute) name(verticalFFT)   

layout (local_size_y = 50) in;
void main()
{

}
