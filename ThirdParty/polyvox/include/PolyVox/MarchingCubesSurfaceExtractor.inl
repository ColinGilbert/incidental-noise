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

#include "Impl/Timer.h"

namespace PolyVox
{
	template< typename Sampler, typename ControllerType>
	Vector3DFloat computeCentralDifferenceGradient(const Sampler& volIter, ControllerType& controller)
	{
		//FIXME - Should actually use DensityType here, both in principle and because the maths may be
		//faster (and to reduce casts). So it would be good to add a way to get DensityType from a voxel.
		//But watch out for when the DensityType is unsigned and the difference could be negative.
		float voxel1nx = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx0py0pz()));
		float voxel1px = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px0py0pz()));

		float voxel1ny = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1ny0pz()));
		float voxel1py = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1py0pz()));

		float voxel1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px0py1nz()));
		float voxel1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px0py1pz()));

		return Vector3DFloat
			(
			voxel1nx - voxel1px,
			voxel1ny - voxel1py,
			voxel1nz - voxel1pz
			);
	}

	// This 'sobel' version of gradient estimation provides better (smoother) normals than the central difference version.
	// Even with the 16-bit normal encoding it does seem to make a difference, so is probably worth keeping. However, there
	// is no way to call it at the moment beyond modifying the main Marching Cubes function below to call this function
	// instead of the central difference one. We should provide a way to control the normal generation method, perhaps
	// including *no* normals incase the user wants to generate them afterwards (e.g. from the mesh).
	template< typename Sampler, typename ControllerType>
	Vector3DFloat computeSobelGradient(const Sampler& volIter, ControllerType& controller)
	{
		static const int weights[3][3][3] = { { { 2, 3, 2 }, { 3, 6, 3 }, { 2, 3, 2 } }, {
			{ 3, 6, 3 }, { 6, 0, 6 }, { 3, 6, 3 } }, { { 2, 3, 2 }, { 3, 6, 3 }, { 2, 3, 2 } } };

		//FIXME - Should actually use DensityType here, both in principle and because the maths may be
		//faster (and to reduce casts). So it would be good to add a way to get DensityType from a voxel.
		//But watch out for when the DensityType is unsigned and the difference could be negative.
		const float pVoxel1nx1ny1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1ny1nz()));
		const float pVoxel1nx1ny0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1ny0pz()));
		const float pVoxel1nx1ny1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1ny1pz()));
		const float pVoxel1nx0py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx0py1nz()));
		const float pVoxel1nx0py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx0py0pz()));
		const float pVoxel1nx0py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx0py1pz()));
		const float pVoxel1nx1py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1py1nz()));
		const float pVoxel1nx1py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1py0pz()));
		const float pVoxel1nx1py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1nx1py1pz()));

		const float pVoxel0px1ny1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1ny1nz()));
		const float pVoxel0px1ny0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1ny0pz()));
		const float pVoxel0px1ny1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1ny1pz()));
		const float pVoxel0px0py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px0py1nz()));
		//const float pVoxel0px0py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px0py0pz()));
		const float pVoxel0px0py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px0py1pz()));
		const float pVoxel0px1py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1py1nz()));
		const float pVoxel0px1py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1py0pz()));
		const float pVoxel0px1py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel0px1py1pz()));

		const float pVoxel1px1ny1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1ny1nz()));
		const float pVoxel1px1ny0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1ny0pz()));
		const float pVoxel1px1ny1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1ny1pz()));
		const float pVoxel1px0py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px0py1nz()));
		const float pVoxel1px0py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px0py0pz()));
		const float pVoxel1px0py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px0py1pz()));
		const float pVoxel1px1py1nz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1py1nz()));
		const float pVoxel1px1py0pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1py0pz()));
		const float pVoxel1px1py1pz = static_cast<float>(controller.convertToDensity(volIter.peekVoxel1px1py1pz()));

		const float xGrad(-weights[0][0][0] * pVoxel1nx1ny1nz -
			weights[1][0][0] * pVoxel1nx1ny0pz - weights[2][0][0] *
			pVoxel1nx1ny1pz - weights[0][1][0] * pVoxel1nx0py1nz -
			weights[1][1][0] * pVoxel1nx0py0pz - weights[2][1][0] *
			pVoxel1nx0py1pz - weights[0][2][0] * pVoxel1nx1py1nz -
			weights[1][2][0] * pVoxel1nx1py0pz - weights[2][2][0] *
			pVoxel1nx1py1pz + weights[0][0][2] * pVoxel1px1ny1nz +
			weights[1][0][2] * pVoxel1px1ny0pz + weights[2][0][2] *
			pVoxel1px1ny1pz + weights[0][1][2] * pVoxel1px0py1nz +
			weights[1][1][2] * pVoxel1px0py0pz + weights[2][1][2] *
			pVoxel1px0py1pz + weights[0][2][2] * pVoxel1px1py1nz +
			weights[1][2][2] * pVoxel1px1py0pz + weights[2][2][2] *
			pVoxel1px1py1pz);

		const float yGrad(-weights[0][0][0] * pVoxel1nx1ny1nz -
			weights[1][0][0] * pVoxel1nx1ny0pz - weights[2][0][0] *
			pVoxel1nx1ny1pz + weights[0][2][0] * pVoxel1nx1py1nz +
			weights[1][2][0] * pVoxel1nx1py0pz + weights[2][2][0] *
			pVoxel1nx1py1pz - weights[0][0][1] * pVoxel0px1ny1nz -
			weights[1][0][1] * pVoxel0px1ny0pz - weights[2][0][1] *
			pVoxel0px1ny1pz + weights[0][2][1] * pVoxel0px1py1nz +
			weights[1][2][1] * pVoxel0px1py0pz + weights[2][2][1] *
			pVoxel0px1py1pz - weights[0][0][2] * pVoxel1px1ny1nz -
			weights[1][0][2] * pVoxel1px1ny0pz - weights[2][0][2] *
			pVoxel1px1ny1pz + weights[0][2][2] * pVoxel1px1py1nz +
			weights[1][2][2] * pVoxel1px1py0pz + weights[2][2][2] *
			pVoxel1px1py1pz);

		const float zGrad(-weights[0][0][0] * pVoxel1nx1ny1nz +
			weights[2][0][0] * pVoxel1nx1ny1pz - weights[0][1][0] *
			pVoxel1nx0py1nz + weights[2][1][0] * pVoxel1nx0py1pz -
			weights[0][2][0] * pVoxel1nx1py1nz + weights[2][2][0] *
			pVoxel1nx1py1pz - weights[0][0][1] * pVoxel0px1ny1nz +
			weights[2][0][1] * pVoxel0px1ny1pz - weights[0][1][1] *
			pVoxel0px0py1nz + weights[2][1][1] * pVoxel0px0py1pz -
			weights[0][2][1] * pVoxel0px1py1nz + weights[2][2][1] *
			pVoxel0px1py1pz - weights[0][0][2] * pVoxel1px1ny1nz +
			weights[2][0][2] * pVoxel1px1ny1pz - weights[0][1][2] *
			pVoxel1px0py1nz + weights[2][1][2] * pVoxel1px0py1pz -
			weights[0][2][2] * pVoxel1px1py1nz + weights[2][2][2] *
			pVoxel1px1py1pz);

		//Note: The above actually give gradients going from low density to high density.
		//For our normals we want the the other way around, so we switch the components as we return them.
		return Vector3DFloat(-xGrad, -yGrad, -zGrad);
	}

	template< typename VolumeType, typename MeshType, typename ControllerType >
	void extractMarchingCubesMeshCustom(VolumeType* volData, Region region, MeshType* result, ControllerType controller)
	{		
		// Validate parameters
		POLYVOX_THROW_IF(volData == nullptr, std::invalid_argument, "Provided volume cannot be null");
		POLYVOX_THROW_IF(result == nullptr, std::invalid_argument, "Provided mesh cannot be null");

		// For profiling this function
		Timer timer;

		// Performance note: Profiling indicates that simply adding vertices and indices to the std::vector is one 
		// of the bottlenecks when generating the mesh. Reserving space in advance helps here but is wasteful in the 
		// common case that no/few vertices are generated. Maybe it's worth reserving a couple of thousand or so?
		// Alternatively, maybe the docs should suggest the user reserves some space in the mesh they pass in?
		result->clear();

		// Store some commonly used values for performance and convienience
		const uint32_t uRegionWidthInVoxels = region.getWidthInVoxels();
		const uint32_t uRegionHeightInVoxels = region.getHeightInVoxels();
		const uint32_t uRegionDepthInVoxels = region.getDepthInVoxels();

		typename ControllerType::DensityType tThreshold = controller.getThreshold();		

		// A naive implemetation of Marching Cubes might sample the eight corner voxels of every cell to determine the cell index. 
		// However, when processing the cells sequentially we cn observe that many of the voxels are shared with previous adjacent 
		// cells, and so we can obtain these by careful bit-shifting. These variables keep track of previous cells for this purpose.
		// We don't clear the arrays because the algorithm ensures that we only read from elements we have previously written to.
		uint8_t uPreviousCellIndex = 0;
		Array1DUint8 pPreviousRowCellIndices(uRegionWidthInVoxels);
		Array2DUint8 pPreviousSliceCellIndices(uRegionWidthInVoxels, uRegionHeightInVoxels);

		// A given vertex may be shared by multiple triangles, so we need to keep track of the indices into the vertex array.
		// We don't clear the arrays because the algorithm ensures that we only read from elements we have previously written to.
		Array<2, Vector3DInt32> pIndices(uRegionWidthInVoxels, uRegionHeightInVoxels);
		Array<2, Vector3DInt32> pPreviousIndices(uRegionWidthInVoxels, uRegionHeightInVoxels);

		// A sampler pointing at the beginning of the region, which gets incremented to always point at the beginning of a slice.
		typename VolumeType::Sampler startOfSlice(volData);
		startOfSlice.setPosition(region.getLowerX(), region.getLowerY(), region.getLowerZ());

		for (uint32_t uZRegSpace = 0; uZRegSpace < uRegionDepthInVoxels; uZRegSpace++)
		{
			// A sampler pointing at the beginning of the slice, which gets incremented to always point at the beginning of a row.
			typename VolumeType::Sampler startOfRow = startOfSlice;

			for (uint32_t uYRegSpace = 0; uYRegSpace < uRegionHeightInVoxels; uYRegSpace++)
			{
				// Copying a sampler which is already pointing at the correct location seems (slightly) faster than
				// calling setPosition(). Therefore we make use of 'startOfRow' and 'startOfSlice' to reset the sampler.
				typename VolumeType::Sampler sampler = startOfRow;

				for (uint32_t uXRegSpace = 0; uXRegSpace < uRegionWidthInVoxels; uXRegSpace++)
				{
					// Note: In many cases the provided region will be (mostly) empty which means mesh vertices/indices 
					// are not generated and the only thing that is done for each cell is the computation of uCellIndex.
					// It appears that retriving the voxel value is not so expensive and that it is the bitwise combining
					// which actually carries the cost.
					//
					// If we really need to speed this up more then it may be possible to pack 4 8-bit cell indices into
					// a single 32-bit value and then perform the bitwise logic on all four of them at the same time. 
					// However, this complicates the code and there would still be the cost of packing/unpacking so it's
					// not clear if there is really a benefit. It's something to consider in the future.

					// Each bit of the cell index specifies whether a given corner of the cell is above or below the threshold.
					uint8_t uCellIndex = 0;

					// Four bits of our cube index are obtained by looking at the cube index for
					// the previous slice and copying four of those bits into their new positions.
					uint8_t uPreviousCellIndexZ = pPreviousSliceCellIndices(uXRegSpace, uYRegSpace);
					uPreviousCellIndexZ >>= 4;
					uCellIndex |= uPreviousCellIndexZ;

					// Two bits of our cube index are obtained by looking at the cube index for
					// the previous row and copying two of those bits into their new positions.
					uint8_t uPreviousCellIndexY = pPreviousRowCellIndices(uXRegSpace);
					uPreviousCellIndexY &= 204; //204 = 128+64+8+4
					uPreviousCellIndexY >>= 2;
					uCellIndex |= uPreviousCellIndexY;

					// One bit of our cube index are obtained by looking at the cube index for
					// the previous cell and copying one of those bits into it's new position.
					uint8_t UPreviousCellIndexX = uPreviousCellIndex;
					UPreviousCellIndexX &= 170; //170 = 128+32+8+2
					UPreviousCellIndexX >>= 1;
					uCellIndex |= UPreviousCellIndexX;

					// The last bit of our cube index is obtained by looking
					// at the relevant voxel and comparing it to the threshold
					typename VolumeType::VoxelType v111 = sampler.getVoxel();
					if (controller.convertToDensity(v111) < tThreshold) uCellIndex |= 128;

					// The current value becomes the previous value, ready for the next iteration.
					uPreviousCellIndex = uCellIndex;
					pPreviousRowCellIndices(uXRegSpace) = uCellIndex;
					pPreviousSliceCellIndices(uXRegSpace, uYRegSpace) = uCellIndex;

					// 12 bits of uEdge determine whether a vertex is placed on each of the 12 edges of the cell.
					uint16_t uEdge = edgeTable[uCellIndex];

					// Test whether any vertices and indices should be generated for the current cell (i.e. it is occupied).
					// Performance note: This condition is usually false because most cells in a volume are completely above
					// or below the threshold and hence unoccupied. However, even when it is always false (testing on an empty
					// volume) it still incurs significant overhead, probably because the code is large and bloats the for loop
					// which contains it. On my empty volume test case the code as given runs in 34ms, but if I replace the
					// condition with 'false' it runs in 24ms and gives the same output (i.e. none).
					//
					// An improvement is to move the code into a seperate function which does speed things up (30ms), but this
					// is messy as the function needs to be passed about 10 differnt parameters, probably adding some overhead 
					// in its self. This does indeed seem to slow down the case when cells are occupied, by about 10-20%.
					//
					// Overall I don't know the right solution, but I'm leaving the code as-is to avoid making it messy. If we
					// can reduce the number of parameters which need to be passed then it might be worth moving it into a
					// function, or otherwise it may simply be worth trying to shorten the code (e.g. adding other function
					// calls). For now we will leave it as-is, until we have more information from real-world profiling.
					if (uEdge != 0)
					{
						auto v111Density = controller.convertToDensity(v111);

						// Performance note: Computing normals is one of the bottlencks in the mesh generation process. The
						// central difference approach actually samples the same voxel more than once as we call it on two
						// adjacent voxels. Perhaps we could expand this and eliminate dupicates in the future. Alternatively, 
						// we could compute vertex normals from adjacent face normals instead of via central differencing, 
						// but not for vertices on the edge of the region (as this causes visual discontinities).
						const Vector3DFloat n111 = computeCentralDifferenceGradient(sampler, controller);

						/* Find the vertices where the surface intersects the cube */
						if ((uEdge & 64) && (uXRegSpace > 0))
						{
							sampler.moveNegativeX();
							typename VolumeType::VoxelType v011 = sampler.getVoxel();
							auto v011Density = controller.convertToDensity(v011);
							const float fInterp = static_cast<float>(tThreshold - v011Density) / static_cast<float>(v111Density - v011Density);

							// Compute the position
							const Vector3DFloat v3dPosition(static_cast<float>(uXRegSpace - 1) + fInterp, static_cast<float>(uYRegSpace), static_cast<float>(uZRegSpace));

							// Compute the normal
							const Vector3DFloat n011 = computeCentralDifferenceGradient(sampler, controller);
							Vector3DFloat v3dNormal = (n111*fInterp) + (n011*(1 - fInterp));

							// The gradient for a voxel can be zero (e.g. solid voxel surrounded by empty ones) and so
							// the interpolated normal can also be zero (e.g. a grid of alternating solid and empty voxels).
							if (v3dNormal.lengthSquared() > 0.000001f)
							{
								v3dNormal.normalise();
							}

							// Allow the controller to decide how the material should be derived from the voxels.
							const typename VolumeType::VoxelType uMaterial = controller.blendMaterials(v011, v111, fInterp);

							MarchingCubesVertex<typename VolumeType::VoxelType> surfaceVertex;
							const Vector3DUint16 v3dScaledPosition(static_cast<uint16_t>(v3dPosition.getX() * 256.0f), static_cast<uint16_t>(v3dPosition.getY() * 256.0f), static_cast<uint16_t>(v3dPosition.getZ() * 256.0f));
							surfaceVertex.encodedPosition = v3dScaledPosition;
							surfaceVertex.encodedNormal = encodeNormal(v3dNormal);
							surfaceVertex.data = uMaterial;

							const uint32_t uLastVertexIndex = result->addVertex(surfaceVertex);
							pIndices(uXRegSpace, uYRegSpace).setX(uLastVertexIndex);

							sampler.movePositiveX();
						}
						if ((uEdge & 32) && (uYRegSpace > 0))
						{
							sampler.moveNegativeY();
							typename VolumeType::VoxelType v101 = sampler.getVoxel();
							auto v101Density = controller.convertToDensity(v101);
							const float fInterp = static_cast<float>(tThreshold - v101Density) / static_cast<float>(v111Density - v101Density);

							// Compute the position
							const Vector3DFloat v3dPosition(static_cast<float>(uXRegSpace), static_cast<float>(uYRegSpace - 1) + fInterp, static_cast<float>(uZRegSpace));

							// Compute the normal
							const Vector3DFloat n101 = computeCentralDifferenceGradient(sampler, controller);
							Vector3DFloat v3dNormal = (n111*fInterp) + (n101*(1 - fInterp));

							// The gradient for a voxel can be zero (e.g. solid voxel surrounded by empty ones) and so
							// the interpolated normal can also be zero (e.g. a grid of alternating solid and empty voxels).
							if (v3dNormal.lengthSquared() > 0.000001f)
							{
								v3dNormal.normalise();
							}

							// Allow the controller to decide how the material should be derived from the voxels.
							const typename VolumeType::VoxelType uMaterial = controller.blendMaterials(v101, v111, fInterp);

							MarchingCubesVertex<typename VolumeType::VoxelType> surfaceVertex;
							const Vector3DUint16 v3dScaledPosition(static_cast<uint16_t>(v3dPosition.getX() * 256.0f), static_cast<uint16_t>(v3dPosition.getY() * 256.0f), static_cast<uint16_t>(v3dPosition.getZ() * 256.0f));
							surfaceVertex.encodedPosition = v3dScaledPosition;
							surfaceVertex.encodedNormal = encodeNormal(v3dNormal);
							surfaceVertex.data = uMaterial;

							uint32_t uLastVertexIndex = result->addVertex(surfaceVertex);
							pIndices(uXRegSpace, uYRegSpace).setY(uLastVertexIndex);

							sampler.movePositiveY();
						}
						if ((uEdge & 1024) && (uZRegSpace > 0))
						{
							sampler.moveNegativeZ();
							typename VolumeType::VoxelType v110 = sampler.getVoxel();
							auto v110Density = controller.convertToDensity(v110);
							const float fInterp = static_cast<float>(tThreshold - v110Density) / static_cast<float>(v111Density - v110Density);

							// Compute the position
							const Vector3DFloat v3dPosition(static_cast<float>(uXRegSpace), static_cast<float>(uYRegSpace), static_cast<float>(uZRegSpace - 1) + fInterp);

							// Compute the normal
							const Vector3DFloat n110 = computeCentralDifferenceGradient(sampler, controller);
							Vector3DFloat v3dNormal = (n111*fInterp) + (n110*(1 - fInterp));

							// The gradient for a voxel can be zero (e.g. solid voxel surrounded by empty ones) and so
							// the interpolated normal can also be zero (e.g. a grid of alternating solid and empty voxels).
							if (v3dNormal.lengthSquared() > 0.000001f)
							{
								v3dNormal.normalise();
							}

							// Allow the controller to decide how the material should be derived from the voxels.
							const typename VolumeType::VoxelType uMaterial = controller.blendMaterials(v110, v111, fInterp);

							MarchingCubesVertex<typename VolumeType::VoxelType> surfaceVertex;
							const Vector3DUint16 v3dScaledPosition(static_cast<uint16_t>(v3dPosition.getX() * 256.0f), static_cast<uint16_t>(v3dPosition.getY() * 256.0f), static_cast<uint16_t>(v3dPosition.getZ() * 256.0f));
							surfaceVertex.encodedPosition = v3dScaledPosition;
							surfaceVertex.encodedNormal = encodeNormal(v3dNormal);
							surfaceVertex.data = uMaterial;

							const uint32_t uLastVertexIndex = result->addVertex(surfaceVertex);
							pIndices(uXRegSpace, uYRegSpace).setZ(uLastVertexIndex);

							sampler.movePositiveZ();
						}

						// Now output the indices. For the first row, column or slice there aren't
						// any (the region size in cells is one less than the region size in voxels)
						if ((uXRegSpace != 0) && (uYRegSpace != 0) && (uZRegSpace != 0))
						{

							int32_t indlist[12];

							/* Find the vertices where the surface intersects the cube */
							if (uEdge & 1)
							{
								indlist[0] = pPreviousIndices(uXRegSpace, uYRegSpace - 1).getX();
							}
							if (uEdge & 2)
							{
								indlist[1] = pPreviousIndices(uXRegSpace, uYRegSpace).getY();
							}
							if (uEdge & 4)
							{
								indlist[2] = pPreviousIndices(uXRegSpace, uYRegSpace).getX();
							}
							if (uEdge & 8)
							{
								indlist[3] = pPreviousIndices(uXRegSpace - 1, uYRegSpace).getY();
							}
							if (uEdge & 16)
							{
								indlist[4] = pIndices(uXRegSpace, uYRegSpace - 1).getX();
							}
							if (uEdge & 32)
							{
								indlist[5] = pIndices(uXRegSpace, uYRegSpace).getY();
							}
							if (uEdge & 64)
							{
								indlist[6] = pIndices(uXRegSpace, uYRegSpace).getX();
							}
							if (uEdge & 128)
							{
								indlist[7] = pIndices(uXRegSpace - 1, uYRegSpace).getY();
							}
							if (uEdge & 256)
							{
								indlist[8] = pIndices(uXRegSpace - 1, uYRegSpace - 1).getZ();
							}
							if (uEdge & 512)
							{
								indlist[9] = pIndices(uXRegSpace, uYRegSpace - 1).getZ();
							}
							if (uEdge & 1024)
							{
								indlist[10] = pIndices(uXRegSpace, uYRegSpace).getZ();
							}
							if (uEdge & 2048)
							{
								indlist[11] = pIndices(uXRegSpace - 1, uYRegSpace).getZ();
							}

							for (int i = 0; triTable[uCellIndex][i] != -1; i += 3)
							{
								const int32_t ind0 = indlist[triTable[uCellIndex][i]];
								const int32_t ind1 = indlist[triTable[uCellIndex][i + 1]];
								const int32_t ind2 = indlist[triTable[uCellIndex][i + 2]];

								if ((ind0 != -1) && (ind1 != -1) && (ind2 != -1))
								{
									result->addTriangle(ind0, ind1, ind2);
								}
							} // For each triangle
						}
					} // For each cell
					sampler.movePositiveX();
				} // For X
				startOfRow.movePositiveY();
			} // For Y
			startOfSlice.movePositiveZ();

			pIndices.swap(pPreviousIndices);
		} // For Z

		result->setOffset(region.getLowerCorner());

		POLYVOX_LOG_TRACE("Marching cubes surface extraction took ", timer.elapsedTimeInMilliSeconds(),
			"ms (Region size = ", region.getWidthInVoxels(), "x", region.getHeightInVoxels(),
			"x", region.getDepthInVoxels(), ")");
	}		
}
