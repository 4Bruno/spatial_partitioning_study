#include "world.h"


u32
WorldPosHash(world * World,world_pos P)
{
    u32 HashKey = ((P.x * 45691) + (P.y * 95009) + (P.z * 26041)) 
                     & 
                     World->HashGridSizeMinusOne; // must be power of 2
    return HashKey;
}

u32
GridOuterCells(u32 DimX, u32 DimY, u32 DimZ)
{
    u32 TotalOuterCells = 2*DimX*DimY + 2*DimY*(DimZ - 2) + 2*(DimX - 2)*(DimZ - 2);

    return TotalOuterCells;
}


void
CellGetNeighbors(world_cell * Cell)
{
    for (i32 NeighborIndex = 0;
                NeighborIndex < 27;
                ++NeighborIndex)
    {
        Logn("Neighbor: %i",Cell->Neightbor->Offset[NeighborIndex]);
    }
}

void
BuildHierarchicalGridInnerNeighbors(cell_neighbor_offset * Neighbors, u32 DimX, u32 DimY, u32 DimZ)
{

    Assert(DimZ >= 4 && DimX >= 4 && DimY >= 4);
    u32 NeighborIndex = 0;

    for (i32 Z = -1;
                Z <= 1;
                ++Z)
    {
        i32 OffsetZ = Z * (DimX * DimY);
        for (i32 Y = -1;
                Y <= 1;
                ++Y)
        {
            i32 OffsetY = Y * DimX;
            for (i32 X = -1;
                    X <= 1;
                    ++X)
            {
                i32 OffsetX = X;
                Neighbors->Offset[NeighborIndex] = OffsetZ + OffsetY + OffsetX;
                //Logn("Neighbor Z:%i Y:%i X:%i V:%i", Z, Y , X, Neighbors->Offset[NeighborIndex]);
                ++NeighborIndex;
            }
        }
    }
}

world
NewWorld(memory_arena * Arena, u32 DimX, u32 DimY, u32 DimZ)
{
    world World;

    u32 CountCells = DimX * DimY * DimZ;
    World.HashGridSizeMinusOne = CountCells - 1;

    World.GridCellDimInMeters   = V3(1.5f) ; // RECORD   GridCellDimInMeters;
    World.TotalWorldEntities    = 0; // u32   TotalWorldEntities;
    World.FreeListWorldCells    = 0; // world_cell * FreeListWorldCells;
    World.FreeListWorldCellData = 0; // world_cell_data * FreeListWorldCellData;
    World.Arena                 = Arena; // memory_arena * Arena;

    World.HashGridX = DimX;
    World.HashGridY = DimY;
    World.HashGridZ = DimZ;

    World.HashGrid = PushArray(Arena, world_cell *, CountCells);
    // We will write bit via accesing 64 bits at a time
    u32 BitsOccupancyCount = CountCells / 64; 
    World.HashGridOccupancy = PushArray(Arena, intptr_t, BitsOccupancyCount);

    BuildHierarchicalGridInnerNeighbors(&World.InnerNeighbors, DimX, DimY, DimZ);

    return World;
}

world_pos
WorldPosition(u32 X, u32 Y, u32 Z, v3 Offset)
{
    world_pos P;
    P.x       = X; // u32   x;
    P.y       = Y; // u32   y;
    P.z       = Z; // u32   z;
    P._Offset = Offset; // RECORD   _Offset;

    return P;
}



