//
//  Q3BspCollision.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#include "Q3BspCollision.h"
#include "Q3BSPAsset.h"

struct trace_work
{
    glm::vec3 start;
    glm::vec3 end;
    
    glm::vec3 endpos;
    float frac;
    
    bool startsolid;
    bool allsolid;
    
    glm::vec3 mins;
    glm::vec3 maxs;
    glm::vec3 offsets[8];
    
    glm::vec3 normal;
    
    int surfaceFlags;
    int texture;
};

#define SURF_CLIP_EPSILON 0.125f

enum bsp_contents
{
    CONTENTS_SOLID = 1,
    LAST_CONTENTS
};

enum plane_type
{
    PLANE_X,
    PLANE_Y,
    PLANE_Z,
    PLANE_NONAXIAL,
    LAST_PLANE_TYPE
};

struct PlaneExtended
{
    glm::vec3 normal;
    float distance;
    
    int signbits;
    int type;
    
    void init(const tBSPPlane &plane)
    {
        normal = plane.normal;
        distance = plane.distance;

        signbits = 0;

        for (int i = 0; i < 3; ++i)
        {
            if (normal[i] < 0)
            {
                signbits |= 1 << i;
            }
        }
        
        type = PLANE_NONAXIAL;
        
        if (normal.x == 1.0f) {
            type = PLANE_X;
        }
        else if (normal.y == 1.0f) {
            type = PLANE_Y;
        }
        else if (normal.z == 1.0f) {
            type = PLANE_Z;
        }
    }
};

struct Q3BspCollision::Impl
{
    void trace(trace_work &work, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &mins, const glm::vec3 &maxs);
    
    void trace_node(trace_work &work, int index, float start_frac, float end_frac, const glm::vec3 &start, const glm::vec3 &end);
    void trace_leaf(trace_work &work, int index);
    void trace_brush(trace_work &work, const tBSPBrush &brush);
    
    std::vector<tBSPNode> m_nodes;
    std::vector<tBSPLeaf> m_leafs;
    std::vector<tBSPBrush> m_brushes;
    std::vector<tBSPBrushSide> m_brushSides;
    std::vector<int> m_leafBrushes;
    
    std::vector<PlaneExtended> m_planes;
    std::vector<tBSPTexture> m_textures;
};

Q3BspCollision::Q3BspCollision() : pImpl(new Impl) {}
Q3BspCollision::~Q3BspCollision() = default;

void Q3BspCollision::initFromBsp(Q3BSPAsset *bsp)
{
//    pImpl = std::make_unique<Impl>();
    
    pImpl->m_nodes = bsp->m_nodes;
    pImpl->m_leafs = bsp->m_leafs;
    pImpl->m_brushes = bsp->m_brushes;
    pImpl->m_brushSides = bsp->m_brushSides;
    pImpl->m_leafBrushes = bsp->m_leafBrushes;
    
    pImpl->m_planes.resize(bsp->m_planes.size());
    
    for (int i = 0; i < bsp->m_planes.size(); ++i)
    {
        pImpl->m_planes[i].init(bsp->m_planes[i]);
    }
    
    pImpl->m_textures = bsp->m_textures;
}

void Q3BspCollision::trace(HitResult &result, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &mins, const glm::vec3 &maxs) const
{
    trace_work work;
    pImpl->trace(work, start, end, mins, maxs);
    
    result.endpos = work.endpos;
    result.normal = work.normal;
    result.fraction = work.frac;
    
    result.startsolid = work.startsolid;
    result.allsolid = work.allsolid;
    
    result.textureId = work.texture;
    result.surfaceFlags = work.surfaceFlags;
}

void Q3BspCollision::findClusterArea(const glm::vec3 &pos, int &cluster, int &area)
{
    int i = 0;
    float distance = 0.0f;
    
    // Continue looping until we find a negative index
    while(i >= 0)
    {
        // Get the current node, then find the slitter plane from that
        // node's plane index.  Notice that we use a constant reference
        // to store the plane and node so we get some optimization.
        const tBSPNode& node = pImpl->m_nodes[i];
        const PlaneExtended& plane = pImpl->m_planes[node.plane];
        
        // Use the Plane Equation (Ax + by + Cz + D = 0) to find if the
        // camera is in front of or behind the current splitter plane.
        distance = plane.normal.x * pos.x + plane.normal.y * pos.y + plane.normal.z * pos.z - plane.distance;
        
        // If the camera is in front of the plane
        if(distance >= 0)
        {
            // Assign the current node to the node in front of itself
            i = node.child[0];
        }
        else
        {
            // Assign the current node to the node behind itself
            i = node.child[1];
        }
    }
    
    // Return the leaf index (same thing as saying:  return -(i + 1)).
    int leafIndex = ~i;  // Binary operation
    
    cluster = pImpl->m_leafs[leafIndex].cluster;
    area = pImpl->m_leafs[leafIndex].area;
}

void Q3BspCollision::Impl::trace(trace_work& work, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &mins, const glm::vec3 &maxs)
{
    work.frac = 1;
    work.startsolid = false;
    work.allsolid = false;
    work.texture = -1;
    work.surfaceFlags = 0;
    
    glm::vec3 offset = (mins + maxs) * 0.5f;
    
    work.mins = mins - offset;
    work.maxs = maxs - offset;
    work.start = start + offset;
    work.end = end + offset;


    work.offsets[0][0] = work.mins[0];
    work.offsets[0][1] = work.mins[1];
    work.offsets[0][2] = work.mins[2];

    work.offsets[1][0] = work.maxs[0];
    work.offsets[1][1] = work.mins[1];
    work.offsets[1][2] = work.mins[2];

    work.offsets[2][0] = work.mins[0];
    work.offsets[2][1] = work.maxs[1];
    work.offsets[2][2] = work.mins[2];

    work.offsets[3][0] = work.maxs[0];
    work.offsets[3][1] = work.maxs[1];
    work.offsets[3][2] = work.mins[2];

    work.offsets[4][0] = work.mins[0];
    work.offsets[4][1] = work.mins[1];
    work.offsets[4][2] = work.maxs[2];

    work.offsets[5][0] = work.maxs[0];
    work.offsets[5][1] = work.mins[1];
    work.offsets[5][2] = work.maxs[2];

    work.offsets[6][0] = work.mins[0];
    work.offsets[6][1] = work.maxs[1];
    work.offsets[6][2] = work.maxs[2];

    work.offsets[7][0] = work.maxs[0];
    work.offsets[7][1] = work.maxs[1];
    work.offsets[7][2] = work.maxs[2];

    trace_node(work, 0, 0, 1, work.start, work.end);

    if (work.frac == 1)
    {
        work.endpos = end;
    }
    else
    {
        work.endpos = start + work.frac * (end - start);
    }
}

void Q3BspCollision::Impl::trace_node(trace_work& work, int index, float start_frac, float end_frac, const glm::vec3 &start, const glm::vec3 &end)
{
    float start_distance;
    float end_distance;
    float offset;

    if (index < 0) {
        trace_leaf(work, -(index + 1));
        return;
    }

    tBSPNode* node = &m_nodes[index];
    PlaneExtended* plane = &m_planes[node->plane];
    int plane_type = plane->type;

    if (plane_type < 3)
    {
        start_distance = start[plane_type] - plane->distance;
        end_distance = end[plane_type] - plane->distance;
        offset = work.maxs[plane_type];
    }
    else
    {
        start_distance = glm::dot(start, plane->normal) - plane->distance;
        end_distance = glm::dot(end, plane->normal) - plane->distance;

        if (work.mins == work.maxs) {
            offset = 0;
        } else {
            /* "this is silly" - id Software */
            offset = 2048;
        }
    }

    if (start_distance >= offset + 1 && end_distance >= offset + 1) {
        trace_node(work, node->child[0], start_frac, end_frac, start, end);
        return;
    }

    if (start_distance < -offset - 1 && end_distance < -offset - 1) {
        trace_node(work, node->child[1], start_frac, end_frac, start, end);
        return;
    }
    
    
    float frac1;
    float frac2;
    int side;

    if (start_distance < end_distance)
    {
        
        float idistance = 1.0f / (start_distance - end_distance);
        frac1 = (start_distance - offset + SURF_CLIP_EPSILON) * idistance;
        frac2 = (start_distance + offset + SURF_CLIP_EPSILON) * idistance;
        side = 1;
    }
    else if (start_distance > end_distance)
    {
        
        float idistance = 1.0f / (start_distance - end_distance);
        frac1 = (start_distance + offset + SURF_CLIP_EPSILON) * idistance;
        frac2 = (start_distance - offset - SURF_CLIP_EPSILON) * idistance;
        side = 0;
    }
    else
    {
        frac1 = 1;
        frac2 = 0;
        side = 0;
    }

    frac1 = fmax(0, fmin(1, frac1));
    frac2 = fmax(0, fmin(1, frac2));
    
    float mid_frac;
    glm::vec3 mid;

    mid_frac = start_frac + (end_frac - start_frac) * frac1;
    mid = start + (end - start) * frac1;
    trace_node(work, node->child[side], start_frac, mid_frac, start, mid);

    mid_frac = start_frac + (end_frac - start_frac) * frac2;
    mid = start + (end - start) * frac2;
    trace_node(work, node->child[side^1], mid_frac, end_frac, mid, end);
}

void Q3BspCollision::Impl::trace_leaf(trace_work& work, int index)
{
    const tBSPLeaf &leaf = m_leafs[index];

    for (int i = 0; i < leaf.numOfLeafBrushes; ++i)
    {
        int brush_index = m_leafBrushes[leaf.leafBrush + i];
        const tBSPBrush &brush = m_brushes[brush_index];
        int contents = m_textures[brush.textureID].contents;

        if ((contents & CONTENTS_SOLID))
        {
            trace_brush(work, brush);

            if (!work.frac) return;
            
//            if (work.flags & TW_ALL_SOLID) return;
        }
    }

    /* TODO: collision with patches */
}

void Q3BspCollision::Impl::trace_brush(trace_work& work, const tBSPBrush &brush)
{
    /* TODO: do optimized check for the first 6 planes which are axial */

    float start_frac = -1;
    float end_frac = 1;
    
    PlaneExtended* closest_plane;
    int texture;
    int surfaceFlags;
    
    bool getout = false;
    bool startout = false;

    for (int i = 0; i < brush.numOfBrushSides; ++i)
    {
        int side_index = brush.brushSide + i;
        int plane_index = m_brushSides[side_index].plane;
        PlaneExtended* plane = &m_planes[plane_index];

        float dist = plane->distance - glm::dot(work.offsets[plane->signbits], plane->normal);

        float start_distance = glm::dot(work.start, plane->normal) - dist;
        float end_distance = glm::dot(work.end, plane->normal) - dist;

        if (start_distance > 0) {
            startout = true;
        }
        
        if (end_distance > 0) {
            // endpoint is not in solid
            getout = true;
        }

//        if (start_distance > 0) work.flags |= TW_STARTS_OUT;
//        if (end_distance > 0) work.flags |= TW_ENDS_OUT;

        if (start_distance > 0 && (end_distance >= SURF_CLIP_EPSILON || end_distance >= start_distance))
        {
            return;
        }

        if (start_distance <= 0 && end_distance <= 0) continue;

        if (start_distance > end_distance)
        {
            float frac = (start_distance - SURF_CLIP_EPSILON) / (start_distance - end_distance);

            if (frac > start_frac) {
                start_frac = frac;
                closest_plane = plane;
                texture = m_brushSides[side_index].textureID;
                surfaceFlags = m_textures[texture].flags;
            }
        }
        else
        {
            float frac = (start_distance + SURF_CLIP_EPSILON) / (start_distance - end_distance);
            end_frac = fmin(end_frac, frac);
        }
    }

//    if (start_frac < end_frac && start_frac > -1 && start_frac < work.frac)
//    {
//        work.frac = fmax(start_frac, 0);
//        work.normal = m_planes[closest_plane_index].normal;
//    }
//
//    if (!(work.flags & (TW_STARTS_OUT | TW_ENDS_OUT)))
//    {
//        work.frac = 0;
//    }
    
    if (!startout)
    {
        // original point was inside brush
        work.startsolid = true;
        
        if (!getout)
        {
            work.allsolid = true;
            work.frac = 0;
        }
        
        return;
    }
    
    if (start_frac < end_frac && start_frac > -1 && start_frac < work.frac)
    {
        work.frac = fmax(start_frac, 0);
        work.normal = closest_plane->normal;
        work.surfaceFlags = surfaceFlags;
        work.texture = texture;
    }
}

