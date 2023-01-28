
#pragma once

#ifndef __GA_FeE_TopologyReference_h__
#define __GA_FeE_TopologyReference_h__

//#include "GA_FeE/GA_FeE_TopologyReference.h"

#include "GA/GA_Detail.h"
#include "GA/GA_SplittableRange.h"
#include "GA/GA_PageHandle.h"
#include "GA/GA_PageIterator.h"

#include "GA/GA_AttributeFilter.h"

#include "GA_FeE/GA_FeE_Type.h"

namespace GA_FeE_TopologyReference {


SYS_FORCE_INLINE
    static GA_Offset
    offsetPromote(
        const GA_Detail* const geo,
        const GA_AttributeOwner origClass,
        const GA_AttributeOwner newClass,
        const GA_Offset elemoff
    )
{
    //const GA_Topology& topo = geo->getTopology();
    switch (origClass)
    {
    case GA_ATTRIB_PRIMITIVE:
        switch (newClass)
        {
        case GA_ATTRIB_PRIMITIVE:
            return elemoff;
            break;
        case GA_ATTRIB_POINT:
            return geo->vertexPoint(geo->getPrimitiveVertexOffset(elemoff, 0));
            break;
        case GA_ATTRIB_VERTEX:
            return geo->getPrimitiveVertexOffset(elemoff, 0);
            break;
        default:
            UT_ASSERT_MSG(0, "Unhandled newClass!");
            break;
        }
        break;
    case GA_ATTRIB_POINT:
        switch (newClass)
        {
        case GA_ATTRIB_PRIMITIVE:
            return geo->vertexPrimitive(geo->pointVertex(elemoff));
            break;
        case GA_ATTRIB_POINT:
            return elemoff;
            break;
        case GA_ATTRIB_VERTEX:
            return geo->pointVertex(elemoff);
            break;
        default:
            UT_ASSERT_MSG(0, "Unhandled newClass!");
            break;
        }
        break;
    case GA_ATTRIB_VERTEX:
        switch (newClass)
        {
        case GA_ATTRIB_PRIMITIVE:
            return geo->vertexPrimitive(elemoff);
            break;
        case GA_ATTRIB_POINT:
            return geo->vertexPoint(elemoff);
            break;
        case GA_ATTRIB_VERTEX:
            return elemoff;
            break;
        default:
            UT_ASSERT_MSG(0, "Unhandled newClass!");
            break;
        }
        break;
    default:
        UT_ASSERT_MSG(0, "Unhandled origClass!");
        break;
    }
    return -1;
}



    

SYS_FORCE_INLINE
static void
outTopoAttrib(
    GA_Detail* const geo,
    const bool outTopo
)
{
    if (outTopo)
        return;
    GA_AttributeSet& AttribSet = geo->getAttributes();
    GA_AttributeFilter filter = GA_AttributeFilter::selectByPattern("__topo_*");
    filter = GA_AttributeFilter::selectAnd(filter, GA_AttributeFilter::selectPublic());
    filter = GA_AttributeFilter::selectAnd(filter, GA_AttributeFilter::selectNot(GA_AttributeFilter::selectGroup()));
    AttribSet.destroyAttributes(GA_ATTRIB_PRIMITIVE, filter);
    AttribSet.destroyAttributes(GA_ATTRIB_POINT,     filter);
    AttribSet.destroyAttributes(GA_ATTRIB_VERTEX,    filter);
    AttribSet.destroyAttributes(GA_ATTRIB_DETAIL,    filter);
        
    for (GA_GroupType groupType : {GA_GROUP_PRIMITIVE, GA_GROUP_POINT, GA_GROUP_VERTEX, GA_GROUP_EDGE})
    {
        GA_GroupTable* groupTable = geo->getGroupTable(groupType);
        //for (GA_GroupTable::iterator it = groupTable->beginTraverse(); !it.atEnd(); it.operator++())
        for (GA_GroupTable::iterator<GA_Group> it = groupTable->beginTraverse(); !it.atEnd(); ++it)
        {
            GA_Group* group = it.group();
            if (group->isDetached())
                continue;
            if (!group->getName().startsWith("__topo_"))
                continue;
            groupTable->destroy(group);
        }
    }
}




    //Get Vertex Prim Index
    static GA_Size
        vertexPrimIndex(
            const GA_Detail* const geo,
            const GA_Offset vtxoff
        )
    {
        return geo->getPrimitiveVertexList(geo->vertexPrimitive(vtxoff)).find(vtxoff);
    }

    static GA_Size
        vertexPrimIndex(
            const GA_Detail* const geo,
            const GA_Offset primoff,
            const GA_Offset vtxoff
        )
    {
        return geo->getPrimitiveVertexList(primoff).find(vtxoff);
    }




    //Get Vertex Destination Point
    //This is Faster than use linear vertex offset
    static GA_Offset
        vertexVertexDst(
            const GA_Detail* const geo,
            const GA_Offset primoff,
            const GA_Size vtxpnum
        )
    {
        const GA_Size vtxpnum_next = vtxpnum + 1;
        if (vtxpnum_next == geo->getPrimitiveVertexCount(primoff)) {
            if (geo->getPrimitiveClosedFlag(primoff))
            {
                return geo->getPrimitiveVertexOffset(primoff, 0);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return geo->getPrimitiveVertexOffset(primoff, vtxpnum_next);
        }
    }


    //Get Vertex Destination Point
    static GA_Offset
        vertexVertexDst(
            const GA_Detail* const geo,
            const GA_Offset vtxoff
        )
    {
        const GA_Offset primoff = geo->vertexPrimitive(vtxoff);
        return vertexVertexDst(geo, primoff, vertexPrimIndex(geo, primoff, vtxoff));
    }



    //Get Vertex Destination Point
#if 1
    //This is Faster than use linear vertex offset
    static GA_Offset
        vertexPointDst(
            const GA_Detail* const geo,
            const GA_Offset primoff,
            const GA_Size vtxpnum
        )
    {
        const GA_Size vtxpnum_next = vtxpnum + 1;
        if (vtxpnum_next == geo->getPrimitiveVertexCount(primoff)) {
            if (geo->getPrimitiveClosedFlag(primoff))
            {
                return geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, vtxpnum_next));
        }
    }

    static GA_Offset
    vertexPointDst(
        const GA_Detail* const geo,
        const GA_Offset vtxoff
    )
    {
        const GA_Offset primoff = geo->vertexPrimitive(vtxoff);
        return vertexPointDst(geo, primoff, vertexPrimIndex(geo, primoff, vtxoff));
    }
#else
    static GA_Offset
        vertexPointDst(
            const GA_Detail* const geo,
            const GA_Offset vtxoff
        )
    {
        const GA_Offset vertexVertexDst = vertexPointDst(geo, vtxoff);
        if (vertexVertexDst == -1)
        {
            return -1;
        }
        else
        {
            return geo->vertexPoint(vertexVertexDst);
        }
    }
    static GA_Offset
        vertexPointDst(
            const GA_Detail* const geo,
            const GA_Offset primoff,
            const GA_Size vtxpnum
        )
    {
        const GA_Offset vertexVertexDst = vertexPointDst(geo, primoff, vtxpnum);
        if (vertexVertexDst == -1)
        {
            return -1;
        }
        else
        {
            return geo->vertexPoint(vertexVertexDst);
        }
    }
#endif














    //Get Vertex neb Vertex in same Prim
    static void
        vertexVertexPrim(
            const GA_Detail* const geo,
            const GA_RWHandleT<GA_Offset>& attribPrev_h,
            const GA_RWHandleT<GA_Offset>& attribNext_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        if (geoGroup)
        {
            const GA_PrimitiveGroupUPtr promotedGroupUPtr = geo->createDetachedPrimitiveGroup();
            GA_PrimitiveGroup* promotedGroup = promotedGroupUPtr.get();
            promotedGroup->combine(geoGroup);

            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(promotedGroup));
            UTparallelFor(geo0SplittableRange0, [geo, &attribPrev_h, &attribNext_h, geoGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            attribPrev_h.set(vertices[0], vertices[numvtx -1]);
                            attribNext_h.set(vertices[numvtx -1], vertices[0]);
                        }
                        else
                        {
                            attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx -1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            if (!geoGroup->contains(vtxoff_next))
                                continue;
                            attribNext_h.set(vtxoff_prev, vtxoff_next);
                            attribPrev_h.set(vtxoff_next, vtxoff_prev);
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange());
            UTparallelFor(geo0SplittableRange0, [geo, &attribPrev_h, &attribNext_h](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            attribPrev_h.set(vertices[0], vertices[numvtx - 1]);
                            attribNext_h.set(vertices[numvtx - 1], vertices[0]);
                        }
                        else
                        {
                            attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx - 1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            attribNext_h.set(vtxoff_prev, vtxoff_next);
                            attribPrev_h.set(vtxoff_next, vtxoff_prev);
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }


    



    //Get Vertex Destination Vertex
    static void
        vertexVertexPrimNext(
        const GA_Detail* const geo,
        const GA_RWHandleT<GA_Offset>& attribNext_h,
        const GA_VertexGroup* const geoGroup = nullptr,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 256
    )
    {
        if (geoGroup)
        {
            const GA_PrimitiveGroupUPtr promotedGroupUPtr = geo->createDetachedPrimitiveGroup();
            GA_PrimitiveGroup* promotedGroup = promotedGroupUPtr.get();
            promotedGroup->combine(geoGroup);

            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(promotedGroup));
            UTparallelFor(geo0SplittableRange0, [geo, &attribNext_h, geoGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            //attribPrev_h.set(vertices[0], vertices[numvtx - 1]);
                            attribNext_h.set(vertices[numvtx - 1], vertices[0]);
                        }
                        else
                        {
                            //attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx - 1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            if (!geoGroup->contains(vtxoff_next))
                                continue;
                            attribNext_h.set(vtxoff_prev, vtxoff_next);
                            //attribPrev_h.set(vtxoff_next, vtxoff_prev);
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange());
            UTparallelFor(geo0SplittableRange0, [geo, &attribNext_h](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            //attribPrev_h.set(vertices[0], vertices[numvtx - 1]);
                            attribNext_h.set(vertices[numvtx - 1], vertices[0]);
                        }
                        else
                        {
                            //attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx - 1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            attribNext_h.set(vtxoff_prev, vtxoff_next);
                            //attribPrev_h.set(vtxoff_next, vtxoff_prev);
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }





    //Get Vertex Destination Point
    static void
        vertexVertexPrim1(
            GA_Detail* const geo,
            const GA_RWHandleT<GA_Size>& attribPrev_h,
            const GA_RWHandleT<GA_Size>& attribNext_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        const GA_Topology& topo = geo->getTopology();
        const GA_SplittableRange geo0SplittableRange0(geo->getVertexRange(geoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, &topo, &attribPrev_h, &attribNext_h, geoGroup](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            GA_Offset vtxoff_prev, vtxoff_next;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    topo.getAdjacentBoundaryVertices(elemoff, vtxoff_prev, vtxoff_next);
                    attribPrev_h.set(elemoff, vtxoff_prev);
                    attribNext_h.set(elemoff, vtxoff_next);
                }
            }
        }, subscribeRatio, minGrainSize);
    }







    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexPrimIndex(
            const GA_Detail* const geo,
            const GA_RWHandleT<GA_Size>& attrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 16
        )
    {
        if (geoGroup)
        {
            const GA_PrimitiveGroupUPtr promotedGroupUPtr = geo->createDetachedPrimitiveGroup();
            GA_PrimitiveGroup* promotedGroup = promotedGroupUPtr.get();
            promotedGroup->combine(geoGroup);

            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(promotedGroup));
            UTparallelFor(geo0SplittableRange0, [geo, &attrib_h, geoGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            if (!geoGroup->contains(vertices[vtxpnum]))
                                continue;
                            attrib_h.set(vertices[vtxpnum], vtxpnum);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(nullptr));
            UTparallelFor(geo0SplittableRange0, [geo, &attrib_h](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            attrib_h.set(vertices[vtxpnum], vtxpnum);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }





    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexPointDstByVtxpnum(
            GA_Detail* const geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            const GA_ROHandleT<GA_Size>& vtxpnumAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        GA_Topology& topo = geo->getTopology();
        topo.makePrimitiveRef();
        const GA_ATITopology* const vtxPrimRef = topo.getPrimitiveRef();

        const GA_SplittableRange geo0SplittableRange0(geo->getVertexRange(geoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, &attrib_h, &vtxpnumAttrib_h, vtxPrimRef](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, vertexPointDst(geo, vtxPrimRef->getLink(elemoff), vtxpnumAttrib_h.get(elemoff)));
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexPointDst(
            GA_Detail* const geo,
            GA_Attribute* const attrib_next,
            const GA_Attribute* const vtxPrimNextAttrib,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();

        const GA_SplittableRange geo0SplittableRange0(geo->getVertexRange(geoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, attrib_next, vtxPrimNextAttrib, vtxPointRef](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType dstpt_aph(attrib_next);
            GA_PageHandleScalar<GA_Offset>::ROType vtxPrimNext_aph(vtxPrimNextAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstpt_aph.setPage(start);
                    vtxPrimNext_aph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (vtxPrimNext_aph.value(elemoff) < 0)
                        {
                            dstpt_aph.value(elemoff) = GA_INVALID_OFFSET;
                        }
                        else
                        {
                            dstpt_aph.value(elemoff) = vtxPointRef->getLink(vtxPrimNext_aph.value(elemoff));
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    

    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexPointDst(
            GA_Detail* const geo,
            const GA_RWHandleT<GA_Offset>& attribNext_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        if (geoGroup)
        {
            const GA_PrimitiveGroupUPtr promotedGroupUPtr = geo->createDetachedPrimitiveGroup();
            GA_PrimitiveGroup* promotedGroup = promotedGroupUPtr.get();
            promotedGroup->combine(geoGroup);

            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(promotedGroup));
            UTparallelFor(geo0SplittableRange0, [geo, &attribNext_h, geoGroup, vtxPointRef](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            //attribPrev_h.set(vertices[0], vtxPointRef->getLink(vertices[numvtx - 1]));
                            attribNext_h.set(vertices[numvtx - 1], vtxPointRef->getLink(vertices[0]));
                        }
                        else
                        {
                            //attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx - 1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            if (!geoGroup->contains(vtxoff_next))
                                continue;
                            attribNext_h.set(vtxoff_prev, vtxPointRef->getLink(vtxoff_next));
                            //attribPrev_h.set(vtxoff_next, vtxPointRef->getLink(vtxoff_prev));
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange());
            UTparallelFor(geo0SplittableRange0, [geo, &attribNext_h, vtxPointRef](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (geo->getPrimitiveClosedFlag(elemoff))
                        {
                            //attribPrev_h.set(vertices[0], vtxPointRef->getLink(vertices[numvtx - 1]));
                            attribNext_h.set(vertices[numvtx - 1], vtxPointRef->getLink(vertices[0]));
                        }
                        else
                        {
                            //attribPrev_h.set(vertices[0], -1);
                            attribNext_h.set(vertices[numvtx - 1], -1);
                        }
                        GA_Offset vtxoff_prev = vertices[0];
                        GA_Offset vtxoff_next;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            attribNext_h.set(vtxoff_prev, vtxPointRef->getLink(vtxoff_next));
                            //attribPrev_h.set(vtxoff_next, vtxPointRef->getLink(vtxoff_prev));
                            vtxoff_prev = vtxoff_next;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }
















    //GA_FeE_Adjacency::addAttribVertexPrimIndex(geo, name, geoGroup, defaults, creation_args, attribute_options, storage, reuse, subscribeRatio, minGrainSize);

    static GA_Attribute* const
        addAttribVertexPrimIndex(
            GA_Detail* const geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_vtxpnum",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 64,
            const exint minGrainSize = 16
        )
    {
        GA_Attribute* attribPtr = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, name);
        if (attribPtr)
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GA_FeE_Type::getPreferredStorageI(geo) : storage;
        attribPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        //attribPtr = geo->addIntTuple(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, name, 1, defaults, creation_args, attribute_options, finalStorage, reuse);
        vertexPrimIndex(geo, attribPtr, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }












    //GA_FeE_Adjacency::addAttribVertexVertexPrim(geo, name, geoGroup, defaults, creation_args, attribute_options, storage, reuse, subscribeRatio, minGrainSize);

    static bool
        addAttribVertexVertexPrim(
            GA_Detail* const geo,
            GA_Attribute*& attribPtr_prev,
            GA_Attribute*& attribPtr_next,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& namePrev = "__topo_vtxPrimPrev",
            const UT_StringHolder& nameNext = "__topo_vtxPrimNext",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        attribPtr_prev = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, namePrev);
        attribPtr_next = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, nameNext);
        if (attribPtr_prev && attribPtr_next)
            return false;

        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GA_FeE_Type::getPreferredStorageI(geo) : storage;

        attribPtr_prev = geo->getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, namePrev, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        attribPtr_next = geo->getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, nameNext, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        //attribPtr_prev = geo->addIntTuple(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, namePrev, 1, defaults, creation_args, attribute_options, finalStorage, reuse);
        //attribPtr_next = geo->addIntTuple(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, nameNext, 1, defaults, creation_args, attribute_options, finalStorage, reuse);
        vertexVertexPrim(geo, attribPtr_prev, attribPtr_next, geoGroup, subscribeRatio, minGrainSize);
        return true;
    }









    //GA_FeE_TopologyReference::addAttribVertexVertexPrimNext(geo, name, geoGroup, defaults, creation_args, attribute_options, storage, reuse, subscribeRatio, minGrainSize);

    static GA_Attribute* const
        addAttribVertexVertexPrimNext(
            GA_Detail* const geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& nameNext = "__topo_vtxPrimNext",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        GA_Attribute* attribPtr_next = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, nameNext);
        if (attribPtr_next)
            return attribPtr_next;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GA_FeE_Type::getPreferredStorageI(geo) : storage;
        attribPtr_next = geo->getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, nameNext, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        //attribPtr_next = geo->addIntTuple(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, nameNext, 1, defaults, creation_args, attribute_options, finalStorage, reuse);
        vertexVertexPrimNext(geo, attribPtr_next, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr_next;
    }








    //GA_FeE_TopologyReference::addAttribVertexPointDst(geo, name, geoGroup, GA_Defaults(-1), GA_STORE_INT32, nullptr, nullptr, GA_ReuseStrategy(), subscribeRatio, minGrainSize);

    static GA_Attribute* const
        addAttribVertexPointDst(
            GA_Detail* const geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_dstpt",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        GA_Attribute* attribPtr = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, name);
        if (attribPtr)
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GA_FeE_Type::getPreferredStorageI(geo) : storage;

        attribPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        //attribPtr = geo->addIntTuple(GA_ATTRIB_VERTEX, GA_FEE_TOPO_SCOPE, name, 1, defaults, creation_args, attribute_options, finalStorage, reuse);

        GA_Attribute* refAttrib = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, "__topo_vtxPrimNext");
        if (refAttrib)
        {
            //GA_Attribute* const vtxPrimNextAttrib = addAttribVertexVertexPrimNext(geo, "__topo_vtxPrimNext", geoGroup, GA_Defaults(-1), finalStorage, nullptr);
            vertexPointDst(geo, attribPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
            return attribPtr;
        }

        refAttrib = geo->findVertexAttribute(GA_FEE_TOPO_SCOPE, "__topo_vtxpnum");
        if (refAttrib)
        {
            //GA_Attribute* const vtxPrimNextAttrib = addAttribVertexPrimIndex(geo, "__topo_vtxPrimNext", geoGroup, GA_Defaults(-1), finalStorage, nullptr);
            vertexPointDstByVtxpnum(geo, attribPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
            return attribPtr;
        }

        vertexPointDst(geo, attribPtr, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }









    //GA_FeE_TopologyReference::groupOneNeb(geo, outGroup, geoGroup, name, subscribeRatio, minGrainSize);
    static void
        groupOneNeb(
            GA_Detail* const geo,
            GA_PointGroup* const outGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        GA_Topology& topo = geo->getTopology();
        topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange());
        UTparallelFor(geoSplittableRange, [geo, outGroup,
            vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, ptoff;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, 0);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET)
                            outGroup->setElement(ptoff, true);

                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, geo->getPrimitiveVertexCount(elemoff) - 1);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET)
                            outGroup->setElement(ptoff, true);
                    }
                }
            }, subscribeRatio, minGrainSize);
    }
    

    //GA_FeE_TopologyReference::groupOneNeb(geo, outGroup, geoGroup, name, subscribeRatio, minGrainSize);
    static void
        groupOneNeb(
            GA_Detail* const geo,
            GA_PointGroup* const outGroup,
            const GA_PrimitiveGroup* const geoGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        GA_Topology& topo = geo->getTopology();
        topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(geoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, outGroup,
            vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, ptoff;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, 0);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET)
                            outGroup->setElement(ptoff, true);

                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, geo->getPrimitiveVertexCount(elemoff) - 1);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET)
                            outGroup->setElement(ptoff, true);
                    }
                }
            }, subscribeRatio, minGrainSize);
    }


    //GA_FeE_TopologyReference::groupOneNeb(geo, outGroup, geoGroup, name, subscribeRatio, minGrainSize);
    static void
        groupOneNeb(
            GA_Detail* const geo,
            GA_PointGroup* const outGroup,
            const GA_PointGroup* const geoGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        if (!geoGroup)
        {
            groupOneNeb(geo, outGroup, subscribeRatio, minGrainSize);
            return;
        }
        GA_Topology& topo = geo->getTopology();
        topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_PrimitiveGroupUPtr geoPromoGroupUPtr = geo->createDetachedPrimitiveGroup();
        GA_PrimitiveGroup* geoPromoGroup = geoPromoGroupUPtr.get();
        geoPromoGroup->combine(geoGroup);

        const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(geoPromoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, outGroup, geoGroup,
            vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, ptoff;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, 0);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff) && geoGroup->contains(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET && geoGroup->contains(ptoff))
                            outGroup->setElement(ptoff, true);

                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, geo->getPrimitiveVertexCount(elemoff) - 1);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff = pointVtxRef->getLink(ptoff);
                        vtxoff = vtxNextRef->getLink(vtxoff);
                        //if (!topo.isPointShared(ptoff) && geoGroup->contains(ptoff))
                        if (vtxoff == GA_INVALID_OFFSET && geoGroup->contains(ptoff))
                            outGroup->setElement(ptoff, true);
                    }
                }
            }, subscribeRatio, minGrainSize);
    }


    //GA_FeE_TopologyReference::groupOneNeb(geo, outGroup, geoGroup, name, subscribeRatio, minGrainSize);

    static void
        groupOneNeb(
            GA_Detail* const geo,
            GA_PointGroup* const outGroup,
            const GA_VertexGroup* const geoGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        if (!geoGroup)
        {
            groupOneNeb(geo, outGroup, subscribeRatio, minGrainSize);
            return;
        }
        GA_Topology& topo = geo->getTopology();
        topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_PrimitiveGroupUPtr geoPromoGroupUPtr = geo->createDetachedPrimitiveGroup();
        GA_PrimitiveGroup* geoPromoGroup = geoPromoGroupUPtr.get();
        geoPromoGroup->combine(geoGroup);

        const GA_SplittableRange geo0SplittableRange0(geo->getPrimitiveRange(geoPromoGroup));
        UTparallelFor(geo0SplittableRange0, [geo, outGroup, geoGroup,
            vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, ptoff, vtxoff_next;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, 0);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff_next = pointVtxRef->getLink(ptoff);
                        vtxoff_next = vtxNextRef->getLink(vtxoff_next);
                        //if (!topo.isPointShared(ptoff) && geoGroup->contains(ptoff))
                        if (vtxoff_next == GA_INVALID_OFFSET && geoGroup->contains(vtxoff))
                            outGroup->setElement(ptoff, true);

                        vtxoff = geo->getPrimitiveVertexOffset(elemoff, geo->getPrimitiveVertexCount(elemoff) - 1);
                        ptoff = vtxPointRef->getLink(vtxoff);
                        vtxoff_next = pointVtxRef->getLink(ptoff);
                        vtxoff_next = vtxNextRef->getLink(vtxoff_next);
                        //if (!topo.isPointShared(ptoff) && geoGroup->contains(ptoff))
                        if (vtxoff_next == GA_INVALID_OFFSET && geoGroup->contains(vtxoff))
                            outGroup->setElement(ptoff, true);
                    }
                }
            }, subscribeRatio, minGrainSize);
    }


    //GA_FeE_TopologyReference::groupOneNeb(geo, outGroup, geoGroup, name, subscribeRatio, minGrainSize);
    static void
        groupOneNeb(
            GA_Detail* const geo,
            GA_PointGroup* const outGroup,
            const GA_Group* const geoGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        if (!geoGroup)
        {
            groupOneNeb(geo, outGroup, subscribeRatio, minGrainSize);
            return;
        }
        switch (geoGroup->classType())
        {
        case GA_GROUP_PRIMITIVE:
            return groupOneNeb(geo, outGroup, UTverify_cast<const GA_PrimitiveGroup*>(geoGroup),
                subscribeRatio, minGrainSize);
            break;
        case GA_GROUP_POINT:
            return groupOneNeb(geo, outGroup, UTverify_cast<const GA_PointGroup*>(geoGroup),
                subscribeRatio, minGrainSize);
            break;
        case GA_GROUP_VERTEX:
            return groupOneNeb(geo, outGroup, UTverify_cast<const GA_VertexGroup*>(geoGroup),
                subscribeRatio, minGrainSize);
            break;
        case GA_GROUP_EDGE:
            return groupOneNeb(geo, outGroup, UTverify_cast<const GA_EdgeGroup*>(geoGroup),
                subscribeRatio, minGrainSize);
            break;
        default:
            break;
        }
    }



     
    //GA_FeE_TopologyReference::addGroupOneNeb(geo, geoGroup, name, subscribeRatio, minGrainSize);

    static GA_PointGroup*
        addGroupOneNeb(
            GA_Detail* const geo,
            const GA_Group* const geoGroup = nullptr,
            const UT_StringHolder& name = "__topo_oneNeb",
            const exint subscribeRatio = 64,
            const exint minGrainSize = 256
        )
    {
        GA_PointGroup* outGroup = geo->findPointGroup(name);
        if (outGroup)
            return outGroup;
        outGroup = geo->newPointGroup(name);
        groupOneNeb(geo, outGroup, geoGroup, subscribeRatio, minGrainSize);
        return outGroup;
    }







} // End of namespace GA_FeE_TopologyReference

#endif
