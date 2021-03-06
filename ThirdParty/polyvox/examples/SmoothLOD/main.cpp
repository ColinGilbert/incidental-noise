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

#include "PolyVoxExample.h"

#include "PolyVox/Density.h"
#include "PolyVox/MarchingCubesSurfaceExtractor.h"
#include "PolyVox/Mesh.h"
#include "PolyVox/RawVolume.h"
#include "PolyVox/VolumeResampler.h"

#include <QApplication>

//Use the PolyVox namespace
using namespace PolyVox;

void createSphereInVolume(RawVolume<uint8_t>& volData, float fRadius)
{
	//This vector hold the position of the center of the volume
	Vector3DFloat v3dVolCenter(volData.getWidth() / 2, volData.getHeight() / 2, volData.getDepth() / 2);

	//This three-level for loop iterates over every voxel in the volume
	for (int z = 0; z < volData.getDepth(); z++)
	{
		for (int y = 0; y < volData.getHeight(); y++)
		{
			for (int x = 0; x < volData.getWidth(); x++)
			{
				//Store our current position as a vector...
				Vector3DFloat v3dCurrentPos(x,y,z);	
				//And compute how far the current position is from the center of the volume
				float fDistToCenter = (v3dCurrentPos - v3dVolCenter).length();

				if(fDistToCenter <= fRadius)
				{
					//Our new density value
					uint8_t uDensity = std::numeric_limits<uint8_t>::max();

					//Wrte the voxel value into the volume	
					volData.setVoxel(x, y, z, uDensity);
				}

				//144 in the middle, (144 - 32) at the edges. Threshold of 128 is between these
				//volData.setVoxel(x, y, z, 144 - fDistToCenter);
			}
		}
	}
}

class SmoothLODExample : public PolyVoxExample
{
public:
	SmoothLODExample(QWidget *parent)
		:PolyVoxExample(parent)
	{
	}

protected:
	void initializeExample() override
	{
		//Create an empty volume and then place a sphere in it
		RawVolume<uint8_t> volData(PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(63, 63, 63)));
		createSphereInVolume(volData, 28);

		//Smooth the data - should reimplement this using LowPassFilter
		//smoothRegion<PagedVolume, Density8>(volData, volData.getEnclosingRegion());
		//smoothRegion<PagedVolume, Density8>(volData, volData.getEnclosingRegion());
		//smoothRegion<PagedVolume, Density8>(volData, volData.getEnclosingRegion());

		RawVolume<uint8_t> volDataLowLOD(PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(15, 31, 31)));

		VolumeResampler< RawVolume<uint8_t>, RawVolume<uint8_t> > volumeResampler(&volData, PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(31, 63, 63)), &volDataLowLOD, volDataLowLOD.getEnclosingRegion());
		volumeResampler.execute();

		//Extract the surface
		auto meshLowLOD = extractMarchingCubesMesh(&volDataLowLOD, volDataLowLOD.getEnclosingRegion());
		// The returned mesh needs to be decoded to be appropriate for GPU rendering.
		auto decodedMeshLowLOD = decodeMesh(meshLowLOD);

		//Extract the surface
		auto meshHighLOD = extractMarchingCubesMesh(&volData, PolyVox::Region(Vector3DInt32(30, 0, 0), Vector3DInt32(63, 63, 63)));
		// The returned mesh needs to be decoded to be appropriate for GPU rendering.
		auto decodedMeshHighLOD = decodeMesh(meshHighLOD);

		//Pass the surface to the OpenGL window
		addMesh(decodedMeshHighLOD, Vector3DInt32(30, 0, 0));
		addMesh(decodedMeshLowLOD, Vector3DInt32(0, 0, 0), 63.0f / 31.0f);

		setCameraTransform(QVector3D(100.0f, 100.0f, 100.0f), -(PI / 4.0f), PI + (PI / 4.0f));
	}
};

int main(int argc, char *argv[])
{
	//Create and show the Qt OpenGL window
	QApplication app(argc, argv);
	SmoothLODExample openGLWidget(0);
	openGLWidget.show();

	//Run the message pump.
	return app.exec();
}
