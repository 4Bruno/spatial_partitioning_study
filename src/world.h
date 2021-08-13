#ifndef WORLD_H
#define WORLD_H
/*
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 */

#include "platform.h"
#include "memory.h"
#include "math.h"

#define MAX_WORLD_ENTITY_COUNT (1 << 14)
#define LOG_WORLD_POS(P) Logn("WorldPos X:%i Y:%i Z:%i Offsets: X:%f Y:%f Z:%f", P.x, P.y, P.z, P._Offset.x,P._Offset.y,P._Offset.z);


struct world_pos
{
    // Those are x,y,z positions in the world grid
    // using float allows for a 4 Kilometers world
    // (where objects are size of 0.02 mm - grand of sand)
    // by using u32 x/y/z we have more size
    i32 x, y ,z;
    // Within our grid cell, float precission offset
    v3 _Offset;
};

enum component_flag
{
    component_flag_none       = 0,
    component_flag_delete     = (1 << 1),
    component_flag_collision  = (1 << 2)
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
    component_flag Flags;
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
    i32 x, y , z;
    u32 HashIndex;

    cell_neighbor_offset * Neighbor;

    // For this cell, all data chunks associated as a linked list
    world_cell_data * FirstCellData;

    world_cell * NextCell;
};

/*
 * Spatial partition:
 * uniform grid open hashing
 * Christer Ericson Chapter 7
 * Implementation based on other paper:
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 *
 * World has:
 * - HashGrid 
 *   For storaging world data
 * - ActiveEntities
 *   Cached (duplicated) data for entities unpacked
 */
struct world
{
    // arbitrary for memory/speed trade-off
    // MUST be power of 2 - preferred 16
    v3 GridCellDimInMeters;
    v3 OneOverGridCellDimInMeters;

    // used for unique entity ID
    u32 TotalWorldEntities;

    // Grid dimensions
    u32 HashGridX,HashGridY,HashGridZ;
    u32 HashGridXMinusOne,HashGridYMinusOne,HashGridZMinusOne;

    u32 HashGridSizeMinusOne;
    world_cell ** HashGrid;
    intptr_t * HashGridOccupancy;

    // Constant for all cells not in edge
    cell_neighbor_offset InnerNeighbors;
    // Based on Grid size
    cell_neighbor_offset * OuterNeighbors;
    
    world_cell      * FreeListWorldCells;
    world_cell_data * FreeListWorldCellData;

    memory_arena * Arena;

    world_pos CurrentWorldP;
    entity *  ActiveEntities;
    u32       ActiveEntitiesCount;

};

struct neighbor_iterator
{
    world_cell * Current;
    u32 CurrentNeighborIndex;
    cell_neighbor_offset * Neighbors;
    u32 CenterHashIndex;
    i32 x,y,z;
    b32 CanContinue;
};


world
NewWorld(memory_arena * Arena, u32 DimX, u32 DimY, u32 DimZ);

u32
WorldPosHash(world * World,world_pos P);

world_pos
WorldPosition(u32 X, u32 Y, u32 Z, v3 Offset = V30);

entity *
AddEntity(world * World, world_pos P);

neighbor_iterator
GetNeighborIterator(world * World,entity * Entity);

void
AdvanceIterator(world * World, neighbor_iterator * Iterator);

world_pos
MapIntoCell(world * World, world_pos P, v3 dP);

void
UpdateWorldLocation(world * World, world_pos Center, v3 QueryDim);

v3
Substract(world * World, world_pos To, world_pos From);

inline b32
EntityHasFlag(entity * Entity, component_flag Flag)
{
    b32 HasFlag = (Entity->Flags & Flag);
    return HasFlag;
}

#endif
