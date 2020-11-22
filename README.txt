Documentation Author: Niko Procopi 2020

This tutorial was designed for Visual Studio 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

AtlasEngine - Stack Allocators
Prerequisites
	Intro C tutorials, Basic OpenGL tutorials, and 
	physics tutorials are required before starting
	the engine series
	
This series takes heavy inspiration from the code 
design of Crash Team Racing (1999). This series is
not trying to rebuild the game in any way, but simply
use similar concepts from that game in a modern engine.
All knowledge of CTR's source code was discovered by 
reverse-engineering research, there was no leak of the
game's source code, so this is all clean and legal.

In all the ATLAS OpenGL and Vulkan tutorials,
we use malloc repeatedly, and we make new GPU buffers
for all different kinds of objects. This fit the
purpose for intro tutorials, but this is not common
in the industry.

Every time you call malloc, data is thrown around in 
different places, and when you call delete, now you 
have empty holes between your allocations, and malloc 
needs to search around for those holes to fill new 
allocations. Same thing happens with glBuffer in VRAM

Alternatively, we can build a Stack Allocator so we know
exactly where our memory is allocated, and how much of it
we are using. "Stack" allocators are not the same as 
"Stack" memory vs Heap memory, it is called a Stack Allocator
because memory gets stacked on top of memory.

In this example, we have a "Linear Allocator":

	Dont do this for allocating and deleting
		
		// allocate
		void* ptr1 = malloc(1024);
		void* ptr2 = malloc(512);
		void* ptr3 = malloc(2048);

		// erase
		delete ptr1;
		delete ptr2;
		delete ptr3;

	Do this for allocating and deleting
		
		void* startPtr = malloc(1gb);
		void* currentPtr = startPtr;

		void* NewMalloc(int size)
		{
			void* returnVal = currentPtr;
			currentPtr += size;
			return returnVal;
		}

		// allocate
		void* ptr1 = NewMalloc(1024);
		void* ptr2 = NewMalloc(512);
		void* ptr3 = NewMalloc(2048);

		// erase
		currentPtr = startPtr;

Linear Allocators and Stack Allocators both allocate memory
the same way. The difference is that Linear Allocators only
allow you to erase all your allocations and overwrite from
the beginning, while a Stack Allocator has bookmarks so that
you can rewind the allocator to different places.

For example, with a Stack Allocator, you can allocate 
a bunch of audio for music, then put a bookmark, then allocate
level geometry, then erase that level geometry by returning to
a bookmark, withot erasing the music, and loading more geometry.

mempack.cpp and mempack.h hold all the Stack Allocator code, most
of it is not used yet, but it will be used in later tutorials.

main.cpp shows how to initialize a stack allocator for RAM,
simply malloc a bunch of memory, and assign the starting pointer
to the stack allocator

graphics.cpp in InitOpenGL() shows how to initialize stack allocators 
in VRAM on the GPU. We have one allocator for textures and one for
vertex data. This time, the allocators start at zero (literally 0),
rather than being given a pointer. 

For the GPU stack allocators, we allocate one large buffer with 
glBufferData, we set the size, and we set 0 as the start, because
all memory allocations willl be handled relative to the start of 
the buffer. Raw GPU addresses are abstracted away by the API, (which
is not a problem at all).

Normally with allocating texture, we call something like this
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_BGR, GL_UNSIGNED_BYTE, (void*)cpuMem);

By passing cpuMem as the last parameter, OpenGL copies memory
from the cpu memory of the texture into somewhere in VRAM, and
we have no reasonable way of managing where it goes.

To manually handle the allocations of texture data, we need to
allocate a glBuffer of type GL_PIXEL_UNPACK_BUFFER. As long as 
we have a buffer binded with that type, we can copy texture data
from CPU to GPU the same way we would with Vertex Data, by using
	glBufferSubData(GL_PIXEL_UNPACK_BUFFER, offsetOfGpuBuffer, size, ptrCpuTexData);

Then, we can change the final paramter of glTexImage2D to use that
offset of the GPU buffer, instead of using a CPU pointer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_BGR, GL_UNSIGNED_BYTE, offsetOfGpuBuffer);

Now we have texture data working in a Stack Allocator, so at any time,
we can erase it and overwrite new data, without hassle, and we can allocate
texture data quickly. Because we aren't making several new buffers for each
allocation, the GPU doesn't need to search around for free data.

For Vertex Buffers, this is similar, data is copied with glBufferSubData
just like pixel data, but VBOs are handled with buffer type GL_ARRAY_BUFFER,
which has been used in many previous OpenGL tutorials

The two types GL_ARRAY_BUFFER and GL_PIXEL_UNPACK_BUFFER are not compatible
with each other, that is why they must be in separate stack allocators,
which is not a problem, just something to remember

globals.h is included in every file, Global struct is declared with "extern"
in the header so that each file knows the global struct exists, and in main.cpp
the structure is declared without "extern" to initialize it.

sprite.cpp uses the stack allocators to send texture and vertex data from 
CPU to GPU

instance.cpp handles drawing each sprite, and setting uniforms.
Some engines say "Entity", some say "Instance", same thing

main.cpp initializes everything, creates sprites, and instances, 
then handles user input, draws user input, and cleans everything
when the window should close.

You might notice main.cpp is lengthy. As tutorials go forward, the
size of main.cpp will continually shrink as the engine grows larger