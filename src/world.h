#ifndef WORLD_H
/*
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 */

#include "platform.h"
#include "memory.h"
#include "math.h"

#define MAX_WORLD_ENTITY_COUNT (1 << 14)

struct world_pos
{
    // Those are x,y,z positions in the world grid
    u32 x, y ,z;
    // Within our grid cell, float precission offset
    v3 _Offset;
};

/*
 * Large buffer to accommodate a big number of entities data
 * if cell needs to add more data and is not sufficient
 * it uses linked list to add new chunk of data
 */
#define WORLD_CELL_DATA_SIZE (1 << 16)
struct world_cell_data
{
    u16 DataSize;
    u8 Data[WORLD_CELL_DATA_SIZE];

    world_cell_data * Next;
};

struct cell_neighbor_offset
{
    // In 3D any cell has 26 cells around + 1 (self)
    i32 Offset[27];
};

/*
 * cells for a given x,y,z only exists as long as there are entities on it
 * they are to be removed otherwise and push back to a free list
 */
struct world_cell
{
    // uniquely identifies cell
    u32 x, y , z;
    u32 HashIndex;

    cell_neighbor_offset * Neighbor;

    // For this cell, all data chunks associated as a linked list
    world_cell_data * FirstCellData;

    world_cell * NextCell;
};

/*
 * Spatial partition simple grid
 * It uses hash based on x,y,z cell position
 */
struct world
{
    // arbitrary for memory/speed trade-off
    v3 GridCellDimInMeters;

    // used for unique entity ID
    u32 TotalWorldEntities;

    u32 HashGridX,HashGridY,HashGridZ;

    world_cell ** HashGrid;
    intptr_t * HashGridOccupancy;
    u32 HashGridSizeMinusOne;

    cell_neighbor_offset InnerNeighbors;
    // Based on Grid size
    cell_neighbor_offset * OuterNeighbors;

    world_cell * FreeListWorldCells;
    world_cell_data * FreeListWorldCellData;

    memory_arena * Arena;

};

struct neighbor_iterator
{
    world_cell * Current;
    u32 CurrentNeighborIndex;
    cell_neighbor_offset * Neighbors;
    u32 CenterHashIndex;
    b32 CanContinue;
};

struct entity_id
{
    u32 ID;
};

struct entity
{
    entity_id ID;    
    world_pos WorldP;
    v3 P;
};


world
NewWorld(memory_arena * Arena, u32 DimX, u32 DimY, u32 DimZ);

u32
WorldPosHash(world * World,world_pos P);

world_pos
WorldPosition(u32 X, u32 Y, u32 Z, v3 Offset = V30);

void
CellPrintNeighbors(world * World,entity * Entity);

entity *
AddEntity(world * World, world_pos P);

neighbor_iterator
GetNeighborIterator(world * World,entity * Entity);

void
AdvanceIterator(world * World, neighbor_iterator * Iterator);

#define WORLD
#endif
