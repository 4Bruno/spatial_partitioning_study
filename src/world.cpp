#include "world.h"


u32
WorldPosHash(world * World,world_pos P)
{
    u32 HashX = (P.x) & World->HashGridXMinusOne;
    u32 HashY = (P.y) & World->HashGridYMinusOne;
    u32 HashZ = (P.z) & World->HashGridZMinusOne;

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

    while (Cell && 
           (Cell->x != P.x || Cell->y != P.y || Cell->z != P.z)
           )
    {
        Cell = Cell->NextCell;
    }

    return Cell;
}


inline world_cell *
GetWorldCellInternal(world * World, u32 StartHashIndex, i32 OffsetIndex, u32 X, u32 Y, u32 Z)
{
    world_cell * Cell = 
        World->HashGrid[(StartHashIndex + OffsetIndex) & (World->HashGridSizeMinusOne)];

    while (Cell && 
           (Cell->x != X || Cell->y != Y || Cell->z != Z)
           )
    {
        Cell = Cell->NextCell;
    }

    return Cell;
}

neighbor_iterator
GetNeighborIterator(world * World,entity * Entity)
{
    world_pos P = Entity->WorldP;
    world_cell * Cell  = GetWorldCell(World, Entity->WorldP);

    Assert(Cell);

    u32 CellHashIndex = Cell->HashIndex;

    neighbor_iterator Iterator;

    i32 IndexOffset = Cell->Neighbor->Offset[0];
    world_cell * BottomLeft = 
        GetWorldCellInternal(World, CellHashIndex, IndexOffset, P.x - 1, P.y - 1, P.z - 1);

    Iterator.Current             = BottomLeft;     // world_cell * Current;
    Iterator.Neighbors           = Cell->Neighbor; // cell_neighbor_offset * Neighbors;
    Iterator.CurrentNeighborIndex= 0;
    Iterator.CenterHashIndex     = CellHashIndex;
    Iterator.CanContinue         = true;
    Iterator.x = P.x;
    Iterator.y = P.y;
    Iterator.z = P.z;

    return Iterator;
}

void
AdvanceIterator(world * World, neighbor_iterator * Iterator)
{
    i32 CurrentIndex = ++Iterator->CurrentNeighborIndex;
    i32 IndexOffset = Iterator->Neighbors->Offset[CurrentIndex];

    if (CurrentIndex > 26)
    {
        Iterator->CanContinue = false;
    }
    else
    {
        i32 Z = (i32)(CurrentIndex * (1.0f / 9.0f)) - 1;
        i32 X = (i32)(CurrentIndex % 3) - 1;
        i32 Y = ((i32)(CurrentIndex * (1.0f / 3.0f)) % 3) - 1;
        //Logn("Neighbor X:%i Y:%i Z:%i V:%i", X, Y , Z, CurrentIndex);

        world_cell * Next = 
            GetWorldCellInternal(World, Iterator->CenterHashIndex, IndexOffset, 
                                 Iterator->x + X,Iterator->y + Y,Iterator->z + Z);;
        Iterator->Current   = Next;     // world_cell * Current;
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

    World.HashGridXMinusOne = DimX - 1;
    World.HashGridYMinusOne = DimY - 1;
    World.HashGridZMinusOne = DimZ - 1;

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
    //Logn("Creating entity with hash %i",Hash);

    world_cell ** Cell = (World->HashGrid + Hash);

    while ((*Cell) &&
            ((*Cell)->x != WorldP.x || (*Cell)->y != WorldP.y || (*Cell)->z != WorldP.z)
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
            ((*CellData)->DataSize + EntitySize) > ArrayCount((*CellData)->Data) )
    {
        CellData = &(*CellData)->Next;
    }

    if (!(*CellData) || 
        ((*CellData)->DataSize + EntitySize) > ArrayCount((*CellData)->Data))
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



