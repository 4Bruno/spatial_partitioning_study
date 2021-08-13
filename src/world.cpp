#include "world.h"
#include <intrin.h>

//#define MAX_LENGHTSQR_DISTANCE_ALLOWED (1000.0f * 1000.0f)
#define MAX_LENGHTSQR_DISTANCE_ALLOWED (50.0f * 50.0f)

i32 NeighborOffsets[27][3] = {
	-1, -1, -1,	 0, -1, -1,	 1, -1, -1,
	-1,  0, -1,	 0,  0, -1,	 1,  0, -1,
	-1,  1, -1,	 0,  1, -1,	 1,  1, -1,
	-1, -1,  0,	 0, -1,  0,	 1, -1,  0,
	-1,  0,  0,	 0,  0,  0,	 1,  0,  0,
	-1,  1,  0,	 0,  1,  0,	 1,  1,  0,
	-1, -1,  1,	 0, -1,  1,	 1, -1,  1,
	-1,  0,  1,	 0,  0,  1,	 1,  0,  1,
	-1,  1,  1,	 0,  1,  1,	 1,  1,  1
};

u32
WorldPosHash(world * World,world_pos P)
{
    u32 HashX = (P.x) & World->HashGridXMinusOne;
    u32 HashY = (P.y) & World->HashGridYMinusOne;
    u32 HashZ = (P.z) & World->HashGridZMinusOne;

    u32 HashKey = HashX +
                  HashY * World->HashGridX +
                  HashZ * (World->HashGridX * World->HashGridY);

    if (HashKey < 0) HashKey += World->HashGridSizeMinusOne;

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

inline b32
SameCellPosition(world_cell * Cell, world_pos WorldP)
{
    b32 Matches = Cell->x == WorldP.x && 
                  Cell->y == WorldP.y && 
                  Cell->z == WorldP.z;
    return Matches;
}

inline b32
DifferentCellPosition(world_cell * Cell, world_pos WorldP)
{
    b32 Matches = SameCellPosition(Cell,WorldP);
    return !Matches;
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

world_cell *
GetWorldCellAndRemove(world * World, world_pos P)
{
    u32 Hash = WorldPosHash(World,P);

    world_cell ** Parent = World->HashGrid + Hash;
    world_cell * Cell = (*Parent);

    while (IS_NOT_NULL(Cell) && 
            DifferentCellPosition(Cell, P))
    {
        Parent = &Cell->NextCell;
        Cell = Cell->NextCell;
    }

    if (IS_NOT_NULL(Cell))
    {
        (* Parent) = Cell->NextCell;
    }

    return Cell;
}


inline world_cell *
GetWorldCellInternal(world * World, u32 StartHashIndex, i32 OffsetIndex, i32 X, i32 Y, i32 Z)
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
#if 0
        i32 Z = (i32)(CurrentIndex * (1.0f / 9.0f)) - 1;
        i32 X = (i32)(CurrentIndex % 3) - 1;
        i32 Y = ((i32)(CurrentIndex * (1.0f / 3.0f)) % 3) - 1;
#else
        i32 Z = NeighborOffsets[CurrentIndex][2];
        i32 Y = NeighborOffsets[CurrentIndex][1];
        i32 X = NeighborOffsets[CurrentIndex][0];
        //Logn("Neighbor X:%i Y:%i Z:%i V:%i", X == Xt, Y == Yt , Z == Zt, CurrentIndex);
#endif
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

    // Push data first so hash table is together
    World.ActiveEntitiesCount = 0;
    World.ActiveEntities = PushArray(Arena, entity, MAX_WORLD_ENTITY_COUNT);
    World.CurrentWorldP = {};

    u32 CountCells = DimX * DimY * DimZ;
    World.HashGridSizeMinusOne = CountCells - 1;

    World.GridCellDimInMeters   = V3(1.5f) ; // RECORD   GridCellDimInMeters;
    World.OneOverGridCellDimInMeters = 1.0f / World.GridCellDimInMeters;

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


inline i32
R32ToInt32(r32 r)
{
    // TODO: floor operation expected? why not?
    //i32 Result = (i32)_mm_cvtss_si32(_mm_set_ss(r));
    i32 Result = (i32)r;
    return Result;
}

world_pos
MapIntoCell(world * World, world_pos P, v3 dP)
{
    /*
     * Cells are mapped as [-x, x] with center equals to (0,0)
     * GridCellDim is the radius to get from 0,0 to the edge
     */
    P._Offset.x += dP.x;
    P._Offset.y += dP.y;
    P._Offset.z += dP.z;

    i32 OffsetX = R32ToInt32(P._Offset.x * World->OneOverGridCellDimInMeters.x);
    i32 OffsetY = R32ToInt32(P._Offset.y * World->OneOverGridCellDimInMeters.y);
    i32 OffsetZ = R32ToInt32(P._Offset.z * World->OneOverGridCellDimInMeters.z);

    P.x += OffsetX;
    P.y += OffsetY;
    P.z += OffsetZ;

    P._Offset.x -= ((r32)OffsetX * World->GridCellDimInMeters.x);
    P._Offset.y -= ((r32)OffsetY * World->GridCellDimInMeters.y);
    P._Offset.z -= ((r32)OffsetZ * World->GridCellDimInMeters.z);

    return P;
}


inline b32
CellHasEnoughRoomForEntity(world_cell_data * CellData)
{
    u32 EntitySize = sizeof(entity);
    b32 HasRoom = (CellData->DataSize + EntitySize) < ArrayCount(CellData->Data);
    return HasRoom;
}

entity *
GetPtrToFreeCellData(world * World, world_pos WorldP)
{
    /*
     * Entities are mapped in a single cell
     */
    u32 Hash = WorldPosHash(World,WorldP);
    //Logn("Creating entity with hash %i",Hash);

    world_cell ** CellPtr = (World->HashGrid + Hash);

    while (IS_NOT_NULL(*CellPtr) &&
            DifferentCellPosition((*CellPtr),WorldP)
          )
    {
        CellPtr = &(*CellPtr)->NextCell;
    }
    
    world_cell * Cell = (*CellPtr);

    if (IS_NULL(Cell))
    {
        if (IS_NULL(World->FreeListWorldCells))
        {
            World->FreeListWorldCells = PushStruct(World->Arena, world_cell);
        }

        world_cell * NextCell = World->FreeListWorldCells->NextCell;
        Cell = World->FreeListWorldCells;
        World->FreeListWorldCells = NextCell;

        Cell->x = WorldP.x;
        Cell->y = WorldP.y;
        Cell->z = WorldP.z;
        Cell->HashIndex = Hash;
        Cell->Neighbor = &World->InnerNeighbors;
        (*CellPtr) = Cell;
    }

    u32 EntitySize = sizeof(entity);

    world_cell_data ** CellDataPtr = &Cell->FirstCellData;

    while ( IS_NOT_NULL(*CellDataPtr) && 
            !CellHasEnoughRoomForEntity((*CellDataPtr))
          )
    {
        CellDataPtr = &(*CellDataPtr)->Next;
    }

    world_cell_data * CellData = (*CellDataPtr);

    if (IS_NULL(CellData) || 
        !CellHasEnoughRoomForEntity((CellData))
       )
    {
        if (IS_NULL(World->FreeListWorldCellData))
        {
            World->FreeListWorldCellData = PushStruct(World->Arena, world_cell_data);
        }
        world_cell_data * NextCellData = World->FreeListWorldCellData->Next;
        CellData = World->FreeListWorldCellData;
        World->FreeListWorldCellData = NextCellData;
        (*CellDataPtr) = CellData;
    }

    entity * Entity = (entity *)(CellData->Data + CellData->DataSize);
    Entity->WorldP = WorldP;

    CellData->DataSize += (u16)EntitySize;

    return Entity;
}

entity *
AddEntity(world * World, world_pos WorldP)
{
    entity_id ID  = GetNextAvailableEntityID(World);
    entity * Entity = GetPtrToFreeCellData(World, WorldP);
    Entity->ID = ID;

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

/*
 * Visualize as vector diff:
 * a - b = vector distance from b to a
 * a <----- b
 */
v3
Substract(world * World, world_pos To, world_pos From)
{
    i32 XCells = To.x - From.x;
    i32 YCells = To.y - From.y;
    i32 ZCells = To.z - From.z;

    v3 dP = To._Offset - From._Offset;

    dP.x += XCells * World->GridCellDimInMeters.x;
    dP.y += YCells * World->GridCellDimInMeters.y;
    dP.z += ZCells * World->GridCellDimInMeters.z;
    
    return V3(dP.x, dP.y, dP.z);
}

/*
 * We load a big chunk of world data at once
 * By loading we cache (copy) data in the spatial grid into
 * a dense array where game logic operates for fast access.
 * The game is not supposed to loop all the active data. Instead,
 * reduced size simulations can be run simultaneously
 */
void
UpdateWorldLocation(world * World, world_pos CenterSim, v3 Radius)
{
    v3 MinCorner = CenterSim._Offset - Radius; 
    v3 MaxCorner = CenterSim._Offset + Radius; 

    world_pos MinCell = MapIntoCell(World, CenterSim, MinCorner);
    world_pos MaxCell = MapIntoCell(World, CenterSim, MaxCorner);

    Logn("Query min: %i %i %i",MinCell.x,MinCell.y,MinCell.z);
    Logn("Query max: %i %i %i",MaxCell.x,MaxCell.y,MaxCell.z);

    world_pos OldWorldP = World->CurrentWorldP;
    World->CurrentWorldP = CenterSim;
    v3 OldWorldPToNewP = Substract(World,CenterSim, OldWorldP);


    b32 EntityNoLongerActive = false;
    u32 EntitySize = sizeof(entity);

    // Check if current cached entities are still valid (not too far, not deleted)
    for (u32 EntityIndex = 0;
                EntityIndex < World->ActiveEntitiesCount;
                /*Index advances under special condition */
                )
    {
        entity * Entity = World->ActiveEntities + EntityIndex;
        if (EntityHasFlag(Entity,component_flag_delete))
        {
            EntityNoLongerActive = true;
            // We will not storage back to the grid. Effectively deleted
        }
        else
        {
            // TODO: Check cell boundaries against entity size
            v3 EntityInSimulationP = Entity->P - OldWorldPToNewP;
            b32 RoomForEntity = (World->ActiveEntitiesCount < MAX_WORLD_ENTITY_COUNT);
            world_pos NewWorldP = MapIntoCell(World, OldWorldP, Entity->P);
            b32 EntityTooFar = LengthSqr(EntityInSimulationP) > MAX_LENGHTSQR_DISTANCE_ALLOWED;
            if (!RoomForEntity || EntityTooFar)
            {
                EntityNoLongerActive = true;
                // Storage back to grid
                Entity->WorldP = NewWorldP;
                entity * Dest = GetPtrToFreeCellData(World, NewWorldP);
                MemCopy((u8 *)Dest, (u8 *)Entity,EntitySize);
            }
            else
            {
                Entity->P -= OldWorldPToNewP;
            }
        }
        if (EntityNoLongerActive)
        {
            // Keep data packed. 
            // Copy last active entity here
            // re-use the iteration index
            *Entity = World->ActiveEntities[--World->ActiveEntitiesCount];
        }
        else
        {
            ++EntityIndex;
        }
    }

    /*
     * Fetch all cells within boundaries:
     * 1) Remove them from the grid
     * 2) Copy its contents to the dense entity array
     * 3) Set cells as free list
     */

    i32 CountLoops = 0;
    for (i32 DimZ = MinCell.z;
                DimZ <= MaxCell.z;
                ++DimZ)
    {
        for (i32 DimY = MinCell.y;
                DimY <= MaxCell.y;
                ++DimY)
        {
            for (i32 DimX = MinCell.x;
                    DimX <= MaxCell.x;
                    ++DimX)
            {
                world_pos P = WorldPosition(DimX, DimY, DimZ);
                //if (DimX == 13 && DimY == 0 && DimZ == 0) { Assert(0); }
                world_cell * Cell = GetWorldCellAndRemove(World, P);
                if (Cell)
                {
                    //Logn("Cell found at:");LOG_WORLD_POS(P);
                    for (world_cell_data * Data = Cell->FirstCellData;
                            IS_NOT_NULL(Data);
                            Data = Data->Next)
                    {
                        for (u32 DataRead = 0;
                                DataRead < Data->DataSize;
                                DataRead += EntitySize)

                        {
                            u8 * Src = (Data->Data + DataRead);
                            Assert((World->ActiveEntitiesCount + 1) < MAX_WORLD_ENTITY_COUNT);
                            entity * Dest = (World->ActiveEntities + World->ActiveEntitiesCount++);
                            Assert(Dest);
                            MemCopy((u8 *)Dest,Src,EntitySize);
                            Dest->P = Substract(World,Dest->WorldP,CenterSim);
                        }
                    }
                    world_cell * FirstFreeCell = World->FreeListWorldCells;
                    World->FreeListWorldCells = Cell;
                    Cell->NextCell = FirstFreeCell;
                }
                ++CountLoops;
            }
        }
    }

    Logn("Count: %i",CountLoops);
    Assert(CountLoops < 10000);
}



