# Vision

## *NOTICE: This is a dev branch, and the code is not guaranteed to be safe or to even compile.*

Vision is a rendering engine built in C++ with OpenGL. The engine powers manyother projects that I've built which need support for out of the box 2D rendering, as well as custom 3D rendering. I have many ambitions for the project, but here's where I'm at, and where my next big landmarks are:

### Features

* Mesh Generation for Cubes and Planes (Spheres are coming)
* Batched 2D Render
* User Input System
* ImGui Renderer

### Goals

* OpenGL, Metal, and WebGPU Renderer
* Versatile Compute Pipeline
* 2D Animation System
* Video Encoding System (Generate Videos for Demos)

### Why Vision?

With so many different tools to build projects in, it's a fair question to ask. I'm building vision for a few different reasons. I don't mind reinventing the wheel. The best way to learn is to rediscover, and Vision is a tool
for me to do exactly that. I'm not following a specific roadmap or working with a team to build a specific product, I
just want to learn.

### What's Broken?

This is my development branch, where I'm working on rewriting the renderer. There are a few things broken, and the API is undergoing changes right now. Here are the things that are broken in this branch, but are working in [master](https://github.com/james51332/vision):

* Renderer2D
* ImGuiRenderer

# Screenshots

![Waves and such](screenshots/waves.png)