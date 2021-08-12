#include "world.h"


u32
WorldPosHash(world * World,world_pos P)
{
    u32 HashX = (P.x ) & (World->HashGridX - 1);
    u32 HashY = (P.y ) & (World->HashGridY - 1);
    u32 HashZ = (P.z ) & (World->HashGridZ - 1);

    u32 HashKey = HashX +
                  HashY * World->HashGridX +
                  HashZ * (World->HashGridX * World->HashGridY);

    return HashKey;
}

u32
WorldPosHash(world * World,u32 X, u32 Y, u32 Z)
{
    world_pos P = WorldPosition(X,Y,Z);
    u32 HashKey = WorldPosHash(World,P);
    return HashKey;
}

u32
GridOuterCells(u32 DimX, u32 DimY, u32 DimZ)
{
    u32 TotalOuterCells = 2*DimX*DimY + 2*DimY*(DimZ - 2) + 2*(DimX - 2)*(DimZ - 2);

    return TotalOuterCells;
}

world_cell *
GetWorldCell(world * World, world_pos P)
{
    u32 Hash = WorldPosHash(World,P);

    world_cell * Cell = World->HashGrid[Hash];

    return Cell;
}


neighbor_iterator
GetNeighborIterator(world * World,entity * Entity)
{
    world_pos P = Entity->WorldP;
    world_cell * Cell  = GetWorldCell(World, Entity->WorldP);
    u32 CellHashIndex = Cell->HashIndex;

    neighbor_iterator Iterator;

    i32 IndexOffset = Cell->Neighbor->Offset[0];
    world_cell * BottomLeft = 
        World->HashGrid[(CellHashIndex + IndexOffset) % (World->HashGridSizeMinusOne)];

    Iterator.Current             = BottomLeft;     // world_cell * Current;
    Iterator.Neighbors           = Cell->Neighbor; // cell_neighbor_offset * Neighbors;
    Iterator.CurrentNeighborIndex= 0;
    Iterator.CenterHashIndex     = CellHashIndex;
    Iterator.CanContinue         = true;

    return Iterator;
}

void
AdvanceIterator(world * World, neighbor_iterator * Iterator)
{
    i32 IndexOffset = Iterator->Neighbors->Offset[++Iterator->CurrentNeighborIndex];

    if (Iterator->CurrentNeighborIndex > 26)
    {
        Iterator->CanContinue = false;
    }
    else
    {
        world_cell * Next = 
            World->HashGrid[(Iterator->CenterHashIndex + IndexOffset) % (World->HashGridSizeMinusOne)];
        Iterator->Current   = Next;     // world_cell * Current;
    }
}

void
CellPrintNeighbors(world * World,entity * Entity)
{
    world_pos P = Entity->WorldP;
    world_cell * Cell  = GetWorldCell(World, Entity->WorldP);
    u32 CellHashIndex = Cell->HashIndex;

    for (i32 NeighborIndex = 0;
                NeighborIndex < 27;
                ++NeighborIndex)
    {
        //Logn("Neighbor: %i",Cell->Neighbor->Offset[NeighborIndex]);
        i32 Z = (NeighborIndex / 9) - 1;
        i32 Y = (NeighborIndex % 3) - 1;
        i32 X = ((NeighborIndex / 3) % 3) - 1;
        u32 PX = Cell->x + X;
        u32 PY = Cell->y + Y;
        u32 PZ = Cell->z + Z;
        //if (Z == -1 && X == 1 && Y == 0) { Assert(0); }
        i32 IndexOffset = Cell->Neighbor->Offset[NeighborIndex];
        Logn("Neighbor X:%i Y:%i Z:%i V:%i", PX, PY , PZ, IndexOffset);
        world_cell * TestCell = World->HashGrid[(CellHashIndex + IndexOffset) % (World->HashGridSizeMinusOne)];
        if (TestCell)
        {
            Logn("Cell exists for X:%i Y:%i Z:%i",TestCell->x,TestCell->y,TestCell->z);
        }
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
                //Neighbors->Offset[NeighborIndex] = (ArbitraryWorldPHash - TestHash );
                //Neighbors->Offset[NeighborIndex] = (TestHash - ArbitraryWorldPHash );
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

entity_id
GetNextAvailableEntityID(world * World)
{
    // 0 is null entity pre-increment intended
    entity_id ID = { ++World->TotalWorldEntities };

    return ID;
}

entity *
AddEntity(world * World, world_pos WorldP)
{
    u32 Hash = WorldPosHash(World,WorldP);
    Logn("Creating entity with hash %i",Hash);

    world_cell ** Cell = (World->HashGrid + Hash);

    while ((*Cell) &&
            ((*Cell)->x != WorldP.x && (*Cell)->y != WorldP.y && (*Cell)->z != WorldP.z)
          )
    {
        Cell = &(*Cell)->NextCell;
    }

    if (!(*Cell))
    {
        if (!World->FreeListWorldCells)
        {
            World->FreeListWorldCells = PushStruct(World->Arena, world_cell);
        }

        world_cell * NextCell = World->FreeListWorldCells->NextCell;
        (*Cell) = World->FreeListWorldCells;
        World->FreeListWorldCells = NextCell;

        (*Cell)->x = WorldP.x;
        (*Cell)->y = WorldP.y;
        (*Cell)->z = WorldP.z;
        (*Cell)->HashIndex = Hash;

    }

    (*Cell)->Neighbor = &World->InnerNeighbors;

    u32 EntitySize = sizeof(entity);

    world_cell_data ** CellData = &(*Cell)->FirstCellData;

    while ( (*CellData) && 
            ((*CellData)->DataSize + EntitySize) < ArrayCount((*CellData)->Data) )
    {
        CellData = &(*CellData)->Next;
    }

    if (!(*CellData) || 
        ((*CellData)->DataSize + EntitySize) < ArrayCount((*CellData)->Data))
    {
        if (!World->FreeListWorldCellData)
        {
            World->FreeListWorldCellData = PushStruct(World->Arena, world_cell_data);
        }
        world_cell_data * NextCellData = World->FreeListWorldCellData->Next;
        (*CellData) = World->FreeListWorldCellData;
        World->FreeListWorldCellData = NextCellData;
    }

    entity * Entity = (entity *)((*CellData)->Data + (*CellData)->DataSize);
    Entity->WorldP = WorldP;
    Entity->ID = GetNextAvailableEntityID(World);

    (*CellData)->DataSize += (u16)EntitySize;

    return Entity;
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



