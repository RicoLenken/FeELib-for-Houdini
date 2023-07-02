
#pragma once

#ifndef __GFE_Normal3D_h__
#define __GFE_Normal3D_h__

#include "GFE/GFE_Normal3D.h"

#include "GFE/GFE_GeoFilter.h"




#include "GEO/GEO_Normal.h"


class GFE_Normal3D : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const float cuspAngleDegrees = GEO_DEFAULT_ADJUSTED_CUSP_ANGLE,
            const GEO_NormalMethod normalMethod = GEO_NormalMethod::ANGLE_WEIGHTED,
            const bool copyOrigIfZero = false
        )
    {
        this->cuspAngleDegrees = cuspAngleDegrees;
        this->normalMethod = normalMethod;
        this->copyOrigIfZero = copyOrigIfZero;
    }

    SYS_FORCE_INLINE void setNotComputed()
    { setHasComputed(); }

    
    SYS_FORCE_INLINE void setComputedNormal3DAttrib(const GA_Attribute* const inAttrib)
    {
        setHasComputed(true);
        normal3DAttrib = inAttrib;
    }

    SYS_FORCE_INLINE GA_Attribute* setNormal3DAttrib(GA_Attribute* const inAttrib)
    {
        setHasComputed();
        normal3DAttrib = inAttrib;
        getOutAttribArray().set(inAttrib);
        return inAttrib;
    }
    
    SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
        const bool findNormal3D,
        const GFE_NormalSearchOrder normal3DSearchOrder = GFE_NormalSearchOrder::All,
        const UT_StringRef& attribName = "",
        const bool addNormal3DIfNoFind = true
    )
    {
        GA_Attribute* const attrib = getOutAttribArray().findOrCreateNormal3D(true,
            findNormal3D ? normal3DSearchOrder : GFE_NormalSearchOrder::Primitive, GA_STORE_INVALID,
            findNormal3D ? attribName : UT_StringHolder(""));
        normal3DAttrib = attrib;
        return attrib;
    }

    
    GA_Attribute*
    findOrCreateNormal3D(
        const bool findNormal3D = false,
        const bool detached = true,
        const GFE_NormalSearchOrder owner = GFE_NormalSearchOrder::All,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3,
        const GA_Defaults& defaults = GA_Defaults(0.0f),
        const bool emplaceBack = true,
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr,
        const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
    )
    {
        GA_Attribute* attrib;
        if (findNormal3D)
        {
            const GA_Storage finalStorage = GFE_Type::getPreferredStorage(geo, storageClass, storage);
        
            attrib = GFE_Attribute::findNormal3D(*geo, owner, attribName);
            if (GFE_Type::checkTupleAttrib(attrib, finalStorage, tupleSize, nullptr))
            {
                setComputedNormal3DAttrib(attrib);
                return attrib;
            }
        }
        attrib = getOutAttribArray().findOrCreateNormal3D(detached, owner, storage,
                findNormal3D ? attribName : UT_StringHolder(""),
                tupleSize, defaults, emplaceBack, create_args, attribute_options, storageClass);
        normal3DAttrib = attrib;
        return attrib;
    }
    
    
    GA_Attribute*
    findOrCreateNormal3D(
        const bool findNormal3D,
        const bool addNormal3DIfNoFind,
        const bool detached,
        const GFE_NormalSearchOrder owner = GFE_NormalSearchOrder::All,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3,
        const GA_Defaults& defaults = GA_Defaults(0.0f),
        const bool emplaceBack = true,
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr,
        const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
    )
    {
        GA_Attribute* attrib;
        if (findNormal3D)
        {
            const GA_Storage finalStorage = GFE_Type::getPreferredStorage(geo, storageClass, storage);
        
            attrib = GFE_Attribute::findNormal3D(*geo, owner, attribName);
            if (GFE_Type::checkTupleAttrib(attrib, finalStorage, tupleSize, nullptr))
            {
                setComputedNormal3DAttrib(attrib);
                return attrib;
            }
            else if (!addNormal3DIfNoFind)
            {
                return nullptr;
            }
        }
        attrib = getOutAttribArray().findOrCreateNormal3D(detached, owner, storage,
                findNormal3D ? attribName : UT_StringHolder(""),
                tupleSize, defaults, emplaceBack, create_args, attribute_options, storageClass);
        normal3DAttrib = attrib;
        return attrib;
    }

        
    SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
        const bool findNormal3D,
        const bool detached,
        const GA_GroupType groupType,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3
        )
    { return findOrCreateNormal3D(findNormal3D, detached, GFE_Attribute::toNormalSearchOrder(groupType), storage, attribName, tupleSize); }

    SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
        const bool findNormal3D,
        const bool detached,
        const GA_Group& group,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3
        )
    { return findOrCreateNormal3D(findNormal3D, detached, group.classType(), storage, attribName, tupleSize); }

    SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
        const bool findNormal3D,
        const bool detached,
        const GA_AttributeOwner owner,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3
        )
    { return findOrCreateNormal3D(findNormal3D, detached, GFE_Attribute::toNormalSearchOrder(owner), storage, attribName, tupleSize); }

    
    SYS_FORCE_INLINE const GA_Attribute* getAttrib() const
    {
        //UT_ASSERT_MSG(!getOutAttribArray().isEmpty(), "no attrib found");
        //return getOutAttribArray().isEmpty() ? nullptr : getOutAttribArray()[0];
        return normal3DAttrib;
    }


private:

    virtual bool
        computeCore() override
    {
        if (getHasComputed())
            return true;

        //if (getOutAttribArray().isEmpty())
        if (!normal3DAttrib)
            return false;
        
        geo->setValidPosAttrib(posAttrib);
        geo->setValidPosAttrib(posAttribNonConst);
        
        if (groupParser.isEmpty())
            return true;

        //normal3DAttrib = getOutAttribArray()[0];
        const GA_AIFTuple* const AIFTuple = normal3DAttrib->getAIFTuple();
        switch (normal3DAttrib->getAIFTuple()->getStorage(normal3DAttrib))
        {
            default:
            case GA_STORE_REAL64: computeNormal<fpreal64>(); break;
            case GA_STORE_REAL32: computeNormal<fpreal32>(); break;
        }
        return true;
    }

    template<typename FLOAT_T>
    void computeNormal()
    {
        const GA_ROHandleT<UT_Vector3T<fpreal64>> pos_h(posAttrib);
        const GA_RWHandleT<UT_Vector3T<fpreal64>> normal3D_h(attrib);
        GEOcomputeNormals(*geo->asGEO_Detail(), pos_h, normal3D_h, groupParser.getGroup(attrib),
            cuspAngleDegrees, normalMethod, copyOrigIfZero);
    }
    

public:
    fpreal cuspAngleDegrees = GEO_DEFAULT_ADJUSTED_CUSP_ANGLE;
    GEO_NormalMethod normalMethod = GEO_NormalMethod::ANGLE_WEIGHTED;
    bool copyOrigIfZero = false;

private:
    const GA_Attribute* normal3DAttrib = nullptr;
    GA_Attribute* attrib;
    
}; // End of class GFE_Normal3D

#endif
