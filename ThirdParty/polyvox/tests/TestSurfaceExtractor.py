# -*- coding: utf-8 -*-

import sys
sys.path.append("../library/bindings/")

import unittest

class TestSurfaceExtractor(unittest.TestCase):
	def setUp(self):
		import PolyVoxCore
		
		#Create a small volume
		r = PolyVoxCore.Region(PolyVoxCore.Vector3Dint32_t(0,0,0), PolyVoxCore.Vector3Dint32_t(31,31,31))
		vol = PolyVoxCore.SimpleVolumeuint8(r)
		#Set one single voxel to have a reasonably high density
		vol.setVoxelAt(PolyVoxCore.Vector3Dint32_t(5, 5, 5), 200)
		self.mesh = PolyVoxCore.SurfaceMeshPositionMaterialNormal()
		extractor = PolyVoxCore.MarchingCubesSurfaceExtractorSimpleVolumeuint8(vol, r, self.mesh)
		extractor.execute()
	
	def test_num_vertices(self):
		self.assertEqual(self.mesh.getNoOfVertices(), 6)

if __name__ == '__main__':
	unittest.main()
