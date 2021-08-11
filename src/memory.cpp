#include "memory.h"

memory_arena
NewMemoryArena(void * Base, u32 Size)
{
    memory_arena Arena;

    Arena.Base        = Base; // Typedef * Base;
    Arena.CurrentSize = 0; // u32   CurrentSize;
    Arena.MaxSize     = Size; // u32   MaxSize;

    return Arena;
}

void *
PushSize_(memory_arena * Arena, u32 Size)
{
    u32 NewSize = Arena->CurrentSize + Size;
    Assert(Arena->MaxSize >= NewSize);
    void * StartingAddress = (void *)((u8 *)Arena->Base + Arena->CurrentSize);
    Arena->CurrentSize = NewSize;

    return StartingAddress;
}
