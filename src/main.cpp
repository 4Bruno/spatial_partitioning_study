#include <windows.h>
#include <limits.h>

#include "platform.h"
#include "world.h"
#include "memory.h"

void *
Win32ReserveMemory(u32 Size)
{
    void * SystemMemory = VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS);
    
    Assert(SystemMemory);

    return SystemMemory;
}

int 
main()
{

    u32 MemSize = Megabytes(90);

    void * SystemMemory = Win32ReserveMemory(Megabytes(90));

    if (!SystemMemory)
    {
        return 1;
    }

    memory_arena Arena = NewMemoryArena(SystemMemory,MemSize);

    world World = NewWorld(&Arena, 16, 16, 16);

    world_pos P = WorldPosition((UINT_MAX / 2),(UINT_MAX / 2),(UINT_MAX / 2));
    world_pos P2 = WorldPosition((UINT_MAX / 2),(UINT_MAX / 2),(UINT_MAX / 2)-1);

    world_pos P3 = WorldPosition(20,20,20);
    world_pos P4 = WorldPosition(20,20,20-1);

    u32 Hash = WorldPosHash(&World,P);
    u32 Hash2 = WorldPosHash(&World,P2);

    Logn("P1: %i P2: %i Diff:%i", Hash, Hash2, Hash - Hash2);

    Hash = WorldPosHash(&World,P3);
    Hash2 = WorldPosHash(&World,P4);

    Logn("P1: %i P2: %i Diff:%i", Hash, Hash2, Hash - Hash2);


    return 0;

}
