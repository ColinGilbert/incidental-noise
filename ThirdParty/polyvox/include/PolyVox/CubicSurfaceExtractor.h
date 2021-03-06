/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*******************************************************************************/

#ifndef __PolyVox_CubicSurfaceExtractor_H__
#define __PolyVox_CubicSurfaceExtractor_H__

#include "Impl/PlatformDefinitions.h"

#include "Array.h"
#include "BaseVolume.h" //For wrap modes... should move these?
#include "DefaultIsQuadNeeded.h"
#include "Mesh.h"
#include "Vertex.h"

namespace PolyVox
{
	template<typename _DataType>
	struct  CubicVertex
	{
		typedef _DataType DataType;

		// Each component of the position is stored as a single unsigned byte.
		// The true position is found by offseting each component by 0.5f.
		Vector3DUint8 encodedPosition;

		// User data
		DataType data;
	};

	// Convienient shorthand for declaring a mesh of 'cubic' vertices
	// Currently disabled because it requires GCC 4.7
	//template <typename VertexDataType, typename IndexType = DefaultIndexType>
	//using CubicMesh = Mesh< CubicVertex<VertexDataType>, IndexType >;

	/// Decodes a position from a CubicVertex
	inline Vector3DFloat decodePosition(const Vector3DUint8& encodedPosition)
	{
		Vector3DFloat result(encodedPosition.getX(), encodedPosition.getY(), encodedPosition.getZ());
		result -= 0.5f; // Apply the required offset
		return result;
	}

	/// Decodes a MarchingCubesVertex by converting it into a regular Vertex which can then be directly used for rendering.
	template<typename DataType>
	Vertex<DataType> decodeVertex(const CubicVertex<DataType>& cubicVertex)
	{
		Vertex<DataType> result;
		result.position = decodePosition(cubicVertex.encodedPosition);
		result.normal.setElements(0.0f, 0.0f, 0.0f); // Currently not calculated
		result.data = cubicVertex.data; // Data is not encoded
		return result;
	}

	struct Quad
	{
		Quad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
		{
			vertices[0] = v0;
			vertices[1] = v1;
			vertices[2] = v2;
			vertices[3] = v3;
		}

		uint32_t vertices[4];
	};

	// This constant defines the maximum number of quads which can share a vertex in a cubic style mesh.
	//
	// We try to avoid duplicate vertices by checking whether a vertex has already been added at a given position.
	// However, it is possible that vertices have the same position but different materials. In this case, the
	// vertices are not true duplicates and both must be added to the mesh. As far as I can tell, it is possible to have
	// at most eight vertices with the same position but different materials. For example, this worst-case scenario
	// happens when we have a 2x2x2 group of voxels, all with different materials and some/all partially transparent.
	// The vertex position at the center of this group is then going to be used by all eight voxels all with different
	// materials.
	const uint32_t MaxVerticesPerPosition = 8;

	template<typename VolumeType>
	struct IndexAndMaterial
	{
		int32_t iIndex;
		typename VolumeType::VoxelType uMaterial;
	};

	enum FaceNames
	{
		PositiveX,
		PositiveY,
		PositiveZ,
		NegativeX,
		NegativeY,
		NegativeZ,
		NoOfFaces
	};

	template<typename MeshType>
	bool mergeQuads(Quad& q1, Quad& q2, MeshType* m_meshCurrent)
	{
		//All four vertices of a given quad have the same data,
		//so just check that the first pair of vertices match.
		if (m_meshCurrent->getVertices()[q1.vertices[0]].data == m_meshCurrent->getVertices()[q2.vertices[0]].data)
		{
			//Now check whether quad 2 is adjacent to quad one by comparing vertices.
			//Adjacent quads must share two vertices, and the second quad could be to the
			//top, bottom, left, of right of the first one. This gives four combinations to test.
			if ((q1.vertices[0] == q2.vertices[1]) && ((q1.vertices[3] == q2.vertices[2])))
			{
				q1.vertices[0] = q2.vertices[0];
				q1.vertices[3] = q2.vertices[3];
				return true;
			}
			else if ((q1.vertices[3] == q2.vertices[0]) && ((q1.vertices[2] == q2.vertices[1])))
			{
				q1.vertices[3] = q2.vertices[3];
				q1.vertices[2] = q2.vertices[2];
				return true;
			}
			else if ((q1.vertices[1] == q2.vertices[0]) && ((q1.vertices[2] == q2.vertices[3])))
			{
				q1.vertices[1] = q2.vertices[1];
				q1.vertices[2] = q2.vertices[2];
				return true;
			}
			else if ((q1.vertices[0] == q2.vertices[3]) && ((q1.vertices[1] == q2.vertices[2])))
			{
				q1.vertices[0] = q2.vertices[0];
				q1.vertices[1] = q2.vertices[1];
				return true;
			}
		}

		//Quads cannot be merged.
		return false;
	}

	template<typename MeshType>
	bool performQuadMerging(std::list<Quad>& quads, MeshType* m_meshCurrent)
	{
		bool bDidMerge = false;
		for (typename std::list<Quad>::iterator outerIter = quads.begin(); outerIter != quads.end(); outerIter++)
		{
			typename std::list<Quad>::iterator innerIter = outerIter;
			innerIter++;
			while (innerIter != quads.end())
			{
				Quad& q1 = *outerIter;
				Quad& q2 = *innerIter;

				bool result = mergeQuads(q1, q2, m_meshCurrent);

				if (result)
				{
					bDidMerge = true;
					innerIter = quads.erase(innerIter);
				}
				else
				{
					innerIter++;
				}
			}
		}

		return bDidMerge;
	}

	template<typename VolumeType, typename MeshType>
	int32_t addVertex(uint32_t uX, uint32_t uY, uint32_t uZ, typename VolumeType::VoxelType uMaterialIn, Array<3, IndexAndMaterial<VolumeType> >& existingVertices, MeshType* m_meshCurrent)
	{
		for (uint32_t ct = 0; ct < MaxVerticesPerPosition; ct++)
		{
			IndexAndMaterial<VolumeType>& rEntry = existingVertices(uX, uY, ct);

			if (rEntry.iIndex == -1)
			{
				//No vertices matched and we've now hit an empty space. Fill it by creating a vertex. The 0.5f offset is because vertices set between voxels in order to build cubes around them.
				CubicVertex<typename VolumeType::VoxelType> cubicVertex;
				cubicVertex.encodedPosition.setElements(static_cast<uint8_t>(uX), static_cast<uint8_t>(uY), static_cast<uint8_t>(uZ));
				cubicVertex.data = uMaterialIn;
				rEntry.iIndex = m_meshCurrent->addVertex(cubicVertex);
				rEntry.uMaterial = uMaterialIn;

				return rEntry.iIndex;
			}

			//If we have an existing vertex and the material matches then we can return it.
			if (rEntry.uMaterial == uMaterialIn)
			{
				return rEntry.iIndex;
			}
		}

		// If we exit the loop here then apparently all the slots were full but none of them matched.
		// This shouldn't ever happen, so if it does it is probably a bug in PolyVox. Please report it to us!
		POLYVOX_THROW(std::runtime_error, "All slots full but no matches during cubic surface extraction. This is probably a bug in PolyVox");
		return -1; //Should never happen.
	}
	
	/// Do not use this class directly. Use the 'extractCubicSurface' function instead (see examples).
	template<typename VolumeType, typename MeshType, typename IsQuadNeeded>
	class CubicSurfaceExtractor
	{		

	public:
		CubicSurfaceExtractor(VolumeType* volData, Region region, MeshType* result, IsQuadNeeded isQuadNeeded = IsQuadNeeded(), bool bMergeQuads = true);

	private:
		
	};

	// This version of the function performs the extraction into a user-provided mesh rather than allocating a mesh automatically.
	// There are a few reasons why this might be useful to more advanced users:
	//
	//   1. It leaves the user in control of memory allocation and would allow them to implement e.g. a mesh pooling system.
	//   2. The user-provided mesh could have a different index type (e.g. 16-bit indices) to reduce memory usage.
	//   3. The user could provide a custom mesh class, e.g a thin wrapper around an openGL VBO to allow direct writing into this structure.
	//
	// We don't provide a default MeshType here. If the user doesn't want to provide a MeshType then it probably makes
	// more sense to use the other variant of this function where the mesh is a return value rather than a parameter.
	//
	// Note: This function is called 'extractCubicMeshCustom' rather than 'extractCubicMesh' to avoid ambiguity when only three parameters
	// are provided (would the third parameter be a controller or a mesh?). It seems this can be fixed by using enable_if/static_assert to emulate concepts,
	// but this is relatively complex and I haven't done it yet. Could always add it later as another overload.
	template<typename VolumeType, typename MeshType, typename IsQuadNeeded = DefaultIsQuadNeeded<typename VolumeType::VoxelType> >
	void extractCubicMeshCustom(VolumeType* volData, Region region, MeshType* result, IsQuadNeeded isQuadNeeded = IsQuadNeeded(), bool bMergeQuads = true)
	{
		CubicSurfaceExtractor<VolumeType, MeshType, IsQuadNeeded> extractor(volData, region, result, isQuadNeeded, bMergeQuads);
		//extractor.execute();
	}

	/// The CubicSurfaceExtractor creates a mesh in which each voxel appears to be rendered as a cube
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Introduction
	/// ------------
	/// Games such as Minecraft and Voxatron have a unique graphical style in which each voxel in the world appears to be rendered as a single cube. Actually rendering a cube for each voxel would be very expensive, but in practice the only faces which need to be drawn are those which lie on the boundary between solid and empty voxels. The CubicSurfaceExtractor can be used to create such a mesh from PolyVox volume data. As an example, images from Minecraft and Voxatron are shown below:
	///
	/// \image html MinecraftAndVoxatron.jpg
	///
	/// Before we get into the specifics of the CubicSurfaceExtractor, it is useful to understand the principles which apply to *all* PolyVox surface extractors and which are described in the Surface Extraction document (ADD LINK). From here on, it is assumed that you are familier with PolyVox regions and how they are used to limit surface extraction to a particular part of the volume. The principles of allowing dynamic terrain are also common to all surface extractors and are described here (ADD LINK).
	///
	/// Basic Operation
	/// ---------------
	/// At its core, the CubicSurfaceExtractor works by by looking at pairs of adjacent voxels and determining whether a quad should be placed between then. The most simple situation to imagine is a binary volume where every voxel is either solid or empty. In this case a quad should be generated whenever a solid voxel is next to an empty voxel as this represents part of the surface of the solid object. There is no need to generate a quad between two solid voxels (this quad would never be seen as it is inside the object) and there is no need to generate a quad between two empty voxels (there is no object here). PolyVox allows the principle to be extended far beyond such simple binary volumes but they provide a useful starting point for understanding how the algorithm works.
	///
	/// As an example, lets consider the part of a volume shown below. We are going to explain the principles in only two dimensions as this makes it much simpler to illustrate, so you will need to mentally extend the process into the third dimension. Hopefully you will find this intuitive. The diagram below shows a small part of a larger volume (as indicated by the voxel coordinates on the axes) which contains only solid and empty voxels represented by solid and hollow circles respectively. The region on which we are running the surface extractor is marked in pink, and for the purpose of this example it corresponds to the whole of the diagram.
	///
	/// \image html CubicSurfaceExtractor1.png
	///
	/// The output of the surface extractor is the mesh marked in red. As you can see, this forms a closed object which corrsponds to the shape of the underlying voxel data. We won't describe the rendering of such meshes here - for details of this please see (SOME LINK HERE).
	///
	/// Working with Regions
	/// --------------------
	/// So far the behaviour is easy to understand, but let's look at what happens when the extraction is limited to a particular region of the volume. The figure below shows the same data set as the previous figure, but the extraction region (still marked in pink) has been limited to 13 to 16 in x and 47 to 51 in y:
	///
	/// \image html CubicSurfaceExtractor2.png
	/// 
	/// As you can see, the extractor continues to generate a number of quads as indicated by the solid red lines. However, you can also see that the shape is no longer closed. This is because the solid voxels actually extend outside the region which is being processed, and so the extractor does not encounter a boundary between solid and empty voxels. Although this may initially appear problematic, the hole in the mesh does not actually matter because it will be hidden by the mesh corresponding to the region adjacent to it (see next diagram).
	///
	/// More interestingly, the diagram also contains a couple of dotted red lines lying on the bottom and right hand side of the extracted region. These are present to illustrate a common point of confusion, which is that *no quads are generated at this position even though it is a boundary between solid and empty voxels*. This is indeed somewhat counter intuitive but there is a rational reasaoning behind it.
	/// If you consider the dashed line on the righthand side of the extracted region, then it is clear that this lies on a boundary between solid and empty voxels and so we do need to create quads here. But what is not so clear is whether these quads should be assigned to the mesh which corresponds to the region in pink, or whether they should be assigned to the region to the right of it which is marked in blue in the diagram below:
	///
	/// \image html CubicSurfaceExtractor3.png
	///
	/// We could choose to add the quads to *both* regions, but this can cause confusion when one of the region is modified (causing the face to disappear or a new one to be created) as *both* regions need to have their mesh regenerated to correctly represent the new state of the volume data. Such pairs of coplanar quads can also cause problems with physics engines, and may prevent transparent voxels from rendering correctly. Therefore we choose to instead only add the quad to one of the the regions and we always choose the one with the greater coordinate value in the direction in which they differ. In the above example the regions differ by the 'x' component of their position, and so the quad is added to the region with the greater 'x' value (the one marked in blue).
	///
	/// **Note:** *This behaviour has changed recently (September 2012). Earlier versions of PolyVox tried to be smart about this problem by looking beyond the region which was being processed, but this complicated the code and didn't work very well. Ultimatly we decided to simply stick with the convention outlined above.*
	///
	/// One of the practical implications of this is that when you modify a voxel *you may have to re-extract the mesh for regions other than region which actually contains the voxel you modified.* This happens when the voxel lies on the upper x,y or z face of a region. Assuming that you have some management code which can mark a region as needing re-extraction when a voxel changes, you should probably extend this to mark the regions of neighbouring voxels as invalid (this will have no effect when the voxel is well within a region, but will mark the neighbouring region as needing an update if the voxel lies on a region face).
	///
	/// Another scenario which sometimes results in confusion is when you wish to extract a region which corresponds to the whole volume, partcularly when solid voxels extend right to the edge of the volume.  
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename VolumeType, typename IsQuadNeeded = DefaultIsQuadNeeded<typename VolumeType::VoxelType> >
	Mesh<CubicVertex<typename VolumeType::VoxelType> > extractCubicMesh(VolumeType* volData, Region region, IsQuadNeeded isQuadNeeded = IsQuadNeeded(), bool bMergeQuads = true)
	{
		Mesh< CubicVertex<typename VolumeType::VoxelType> > result;
		extractCubicMeshCustom(volData, region, &result, isQuadNeeded, bMergeQuads);
		return result;
	}
}

#include "CubicSurfaceExtractor.inl"

#endif
