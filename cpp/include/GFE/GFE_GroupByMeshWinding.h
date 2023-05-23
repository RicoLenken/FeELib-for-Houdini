
#pragma once

#ifndef __GFE_GroupByMeshWinding_h__
#define __GFE_GroupByMeshWinding_h__

//#include "GFE/GFE_GroupByMeshWinding.h"




//#include "GEO/GEO_Hull.h"
#include "GU/GU_PolyFill.h"
#include "GU/GU_ConvexHull3D.h"
#include "UT/UT_BoundingBox.h"


#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_Attribute.h"
#include "GFE/GFE_Measure.h"
#include "GFE/GFE_WindingNumber.h"
//#include "GFE/GFE_MeshCap.h"



enum class GFE_GroupByMeshWindingMethod
{
	VOLUME,
	WINDINGNUMBER_BBOX,
	WINDINGNUMBER_CONVEX,
};



class GFE_GroupByMeshWinding : public GFE_AttribFilterWithRef {

public:
	using GFE_AttribFilterWithRef::GFE_AttribFilterWithRef;


	void
		setComputeParm(
			const GFE_GroupByMeshWindingMethod method = GFE_GroupByMeshWindingMethod::VOLUME,
			
			const bool reverseGroup = false,
			const bool reversePrim = false,
			const bool meshCap = false
		)
	{
		setHasComputed();
		this->method = method;
			
		this->reverseOutGroup = reverseGroup;
		this->reversePrim = reversePrim;
		this->meshCap = meshCap;
	}

	SYS_FORCE_INLINE bool getMeshWindingCorrect() const
	{ return meshWindingCorrect; }

	SYS_FORCE_INLINE GA_Attribute* findOrCreateTuple(
		const bool detached = false,
		const UT_StringRef& attribName = ""
	)
	{ return getOutAttribArray().findOrCreateTuple(false, GA_ATTRIB_DETAIL,
			GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }

	
    
	virtual void bumpDataId() const override
	{
		getOutAttribArray().bumpDataId();
		getOutGroupArray().bumpDataId();
		if (reversePrim && !(meshWindingCorrect ^ reverseOutGroup))
			bumpDataIdsForAddOrRemove(false, true, false);
	}

private:

	virtual bool
		computeCore() override
	{
		if (groupParser.isEmpty())
			return true;

		
		if (meshCap)
		{
			geoCapTmp = new GU_Detail();
			geoCap_h.allocateAndSet(geoCapTmp);
			geoCapTmp->replaceWith(*geo);
			
			UT_Array<GA_OffsetArray> rings;
			UT_Array<GA_OffsetArray> ringOrigs;
			GU_PolyFill::singlePolys(geoCapTmp, rings, ringOrigs, nullptr);
			
			geoCapTmp_GA = geoCapTmp;
		}
		else
		{
			geoCapTmp_GA = geo;
		}
		
		switch (method)
		{
			case GFE_GroupByMeshWindingMethod::VOLUME:               volume(); break;
			case GFE_GroupByMeshWindingMethod::WINDINGNUMBER_CONVEX:
			case GFE_GroupByMeshWindingMethod::WINDINGNUMBER_BBOX:   wn();     break;
			default: break;
		}

		const bool meshWindingCorrect_reverse = meshWindingCorrect ^ reverseOutGroup;
		if (!getOutAttribArray().isEmpty())
		{
			UT_ASSERT_P(getOutAttribArray()[0]->getAIFTuple());
			getOutAttribArray()[0]->getAIFTuple()->set(getOutAttribArray()[0], 0, static_cast<int32>(meshWindingCorrect_reverse));
		}
		
		if (!getOutGroupArray().isEmpty())
		{
			if (meshWindingCorrect_reverse)
				getOutGroupArray()[0]->clear();
			else
				getOutGroupArray()[0]->addAll();
		}
		
		if (reversePrim && !meshWindingCorrect_reverse)
		{
			geo->asGU_Detail()->reverse(nullptr, false);
			//geo->asGU_Detail()->reversePolys(nullptr);
		}
		
		
		return true;
	}

	
	void volume()
	{
		GFE_Measure measure(geoCapTmp_GA, cookparms);
		measure.measureType = GFE_MeasureType::MeshVolume;
		measure.groupParser.setGroup(groupParser.getPrimitiveGroup());
		//measure.setPositionAttrib(sopparms.getPosAttribName());
		//measure.findOrCreateTuple(!outIntermediateAttrib);
		//measure.compute();
		const fpreal64 volume = measure.computeMeshVolume();
		
		meshWindingCorrect = volume >= 0;
	}
	
	void wn()
	{

		GU_DetailHandle geoConvex_h;
		GU_Detail* geoConvex = new GU_Detail();
		geoConvex_h.allocateAndSet(geoConvex);
		
		switch (method)
		{
		case GFE_GroupByMeshWindingMethod::WINDINGNUMBER_CONVEX:
			{
				GU_ConvexHull3D convexHull3D;
#if 1
				convexHull3D.computeAndShrink(0.1, *geo->asGU_Detail(), groupParser.getPointGroup(), true);

				convexHull3D.getGeometry(*geoConvex, geo->asGU_Detail());
#else
				geoConvex->replaceWith(*geo);
				geoConvex->convex();
#endif
			}
			break;
		case GFE_GroupByMeshWindingMethod::WINDINGNUMBER_BBOX:
			{
				const UT_BoundingBoxT<fpreal32>& geoBBox = geo->stdBoundingBox<fpreal32>(groupParser.getPointRange(), posAttrib);
				geoConvex->appendPointBlock(8);
				UT_Vector3T<fpreal32> posArray[8];
				geoBBox.getBBoxPoints(posArray);
			
				for (GA_Size i = 0; i < 8; ++i)
				{
					geoConvex->setPos3(geoConvex->pointOffset(i), posArray[i]);
				}
			}
			break;
		default: break;
		}
		
		
		GFE_WindingNumber wn(*geoConvex, *geoCapTmp_GA, nullptr, cookparms);
		GA_Attribute* wnAttrib = wn.findOrCreateTuple(!outIntermediateAttrib);
		
		wn.computeAndBumpDataId();

		const fpreal64 wnSum = GFE_Attribute::computeAttribSum<fpreal64>(wnAttrib);
		
		meshWindingCorrect = wnSum >= 0;
	}
	

public:
	GFE_GroupByMeshWindingMethod method = GFE_GroupByMeshWindingMethod::VOLUME;
	
	//bool reverseGroup = false;
	bool reversePrim = false;
	bool meshCap = false;
	
	bool outIntermediateAttrib = false;
	
private:
	bool meshWindingCorrect = true;

	GU_DetailHandle geoCap_h;
	GU_Detail* geoCapTmp;
	GA_Detail* geoCapTmp_GA;
	
}; // End of class GFE_GroupByMeshWinding



#endif
