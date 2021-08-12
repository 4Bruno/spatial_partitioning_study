#include <windows.h>
#include <limits.h>

#include "platform.h"
#include "world.h"
#include "memory.h"

void *
Win32ReserveMemory(u32 Size)
{
    void * SystemMemory = VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    Assert(SystemMemory);

    return SystemMemory;
}

void
TestPrintHashAtPos(world * World, u32 X, u32 Y, u32 Z)
{
    world_pos P = WorldPosition(X,Y,Z);
    world_pos P2 = WorldPosition(X+1,Y,Z-1);

    u32 Hash = WorldPosHash(World,P);
    u32 Hash2 = WorldPosHash(World,P2);

    Logn("P1: %i P2: %i Diff:%i", Hash, Hash2, Hash - Hash2);
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

#if 1
    entity * Entity = AddEntity(&World, WorldPosition(20,20,20));
    entity * Entity2 = AddEntity(&World, WorldPosition(19,19,19));
    entity * Entity3 = AddEntity(&World, WorldPosition(21,20,19));

#if 1
    CellPrintNeighbors(&World, Entity);
#else
    for (neighbor_iterator Iterator = GetNeighborIterator(&World,Entity);
            Iterator.CanContinue;
            AdvanceIterator(&World,&Iterator))
    {
        world_cell * NeighborCell = Iterator.Current;
        if (NeighborCell)
        {
            Logn("Neighbor exists for X:%i Y:%i Z:%i",NeighborCell->x,NeighborCell->y,NeighborCell->z);
        }
    }
#endif

#else

    TestPrintHashAtPos(&World, UINT_MAX / 2,UINT_MAX / 2, UINT_MAX / 2);
    TestPrintHashAtPos(&World, 20,20,20);
    //TestPrintHashAtPos(&World, 15,34,84);
    
#endif

    return 0;

}
