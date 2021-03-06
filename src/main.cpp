#include <windows.h>
#include <limits.h>
#include <intrin.h>

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
TestPrintHashAtPos_(world * World, u32 X, u32 Y, u32 Z)
{
    world_pos P = WorldPosition(X,Y,Z);
    world_pos P2 = WorldPosition(X+1,Y,Z-1);

    u32 Hash = WorldPosHash(World,P);
    u32 Hash2 = WorldPosHash(World,P2);

    Logn("P1: %i P2: %i Diff:%i", Hash, Hash2, Hash - Hash2);
}

void
TestPrintHashAtPos(world * World)
{
    TestPrintHashAtPos_(World, UINT_MAX / 2,UINT_MAX / 2, UINT_MAX / 2);
    TestPrintHashAtPos_(World, 20,20,20);
    TestPrintHashAtPos_(World, 15,34,84);
}

void
TestPrintInnerNeighborArrayOffsets(world * World)
{
    for (i32 NeighborIndex = 0;
                NeighborIndex < 27;
                ++NeighborIndex)
    {
        //Logn("Neighbor: %i",Cell->Neighbor->Offset[NeighborIndex]);
        i32 Z = (NeighborIndex / 9) - 1;
        i32 X = (NeighborIndex % 3) - 1;
        i32 Y = ((NeighborIndex / 3) % 3) - 1;
        i32 IndexOffset = World->InnerNeighbors.Offset[NeighborIndex];
        Logn("Neighbor X:%i Y:%i Z:%i V:%i", X, Y , Z, IndexOffset);
    }
}

void
TestCollision(world * World)
{
    world_pos PA = WorldPosition(20,20,20);
    world_pos PB = WorldPosition(20,36,20);

    entity * EntityA = AddEntity(World, PA);
    entity * EntityB = AddEntity(World, PB);
}

void
TestNeighbors(world * World, world_pos WorldP)
{
    world_pos RefWP = WorldP;
    entity * Entity = AddEntity(World, RefWP);
    entity * Entity2 = AddEntity(World, WorldPosition(RefWP.x - 1,RefWP.y - 1,RefWP.z - 1));
    entity * Entity3 = AddEntity(World, WorldPosition(RefWP.x - 1,RefWP.y - 0,RefWP.z - 1));

    for (neighbor_iterator Iterator = GetNeighborIterator(World,Entity);
            Iterator.CanContinue;
            AdvanceIterator(World,&Iterator))
    {
        world_cell * NeighborCell = Iterator.Current;
        if (NeighborCell)
        {
            Logn("Neighbor exists for X:%i Y:%i Z:%i",
                    NeighborCell->x - Entity->WorldP.x,
                    NeighborCell->y - Entity->WorldP.y,
                    NeighborCell->z - Entity->WorldP.z);
        }
    }
}

void
TestRecanonicalize(world * World)
{
    world_pos RefWP = WorldPosition(20,20,20, V3(0.8f, 0.5f, 0));
    Logn("Start at:");LOG_WORLD_POS(RefWP);
    entity * Entity = AddEntity(World, RefWP);
    v3 dP = V3(1.0f,0,0);
    world_pos P = MapIntoCell(World, RefWP, dP);
    LOG_WORLD_POS(P);
    dP = V3(-1.0f - 0.8f,0,0);
    P = MapIntoCell(World, RefWP, dP);
    LOG_WORLD_POS(P);

    RefWP = WorldPosition(20,20,20, V3(0.0f, 0.0f, 0));
    Logn("Start at:");LOG_WORLD_POS(RefWP);
    Entity = AddEntity(World, RefWP);
    dP = V3(1.0f,0,0);
    P = MapIntoCell(World, RefWP, dP);
    LOG_WORLD_POS(P);
    dP = V3(-1.0f - 0.8f,-3.3f,0);
    P = MapIntoCell(World, RefWP, dP);
    LOG_WORLD_POS(P);
}

void
DoTests(world * World)
{
    //TestCollision(World);

    //Logn(" ------------------ Test 1 ------------------");
    //TestPrintHashAtPos(World);

    //Logn(" ------------------ Test 2 ------------------");
    //TestPrintInnerNeighborArrayOffsets(World);

    //Logn(" ------------------ Test 3 ------------------");
    TestNeighbors(World,WorldPosition(INT32_MAX/2,INT32_MAX/2,INT32_MAX/2));
    TestNeighbors(World,WorldPosition(20,20,20));
    TestNeighbors(World,WorldPosition(20,36,20));
    
    //TestRecanonicalize(World);
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

    //DoTests(World)
    world_pos P = WorldPosition(0,0,0);
    world_pos P2 = WorldPosition(13,0,0);
    v3 Dim = V3(20.0f,3.0f,20.0f);
    entity * EntityA = AddEntity(&World, P);
    entity * EntityB = AddEntity(&World, P2);

    UpdateWorldLocation(&World, P, Dim);

    P = WorldPosition(20,0,0);
    UpdateWorldLocation(&World, P, Dim);

    UpdateWorldLocation(&World, P, Dim);

    v3 DiffP = Substract(&World, P, P2);
    Logn("Diff: %f %f %f",DiffP.x, DiffP.y, DiffP.z);

    return 0;

}
