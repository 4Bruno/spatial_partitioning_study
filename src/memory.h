#ifndef MEMORY_H

#include "platform.h"

struct memory_arena
{
    void * Base;
    u32 CurrentSize;
    u32 MaxSize;
};

#define PushStruct(Arena,s)   (s *) PushSize_(Arena, sizeof(s))
#define PushArray(Arena,s,c)  (s *) PushSize_(Arena, sizeof(s) * c)

memory_arena
NewMemoryArena(void * Base, u32 Size);

void *
PushSize_(memory_arena * Arena, u32 Size);


#define MEMORY_H
#endif
