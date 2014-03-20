#pragma once

#include <soglu/Camera.hpp>
#include <soglu/BoundingBox.hpp>
#include <soglu/Primitives.hpp>
#include <soglu/GLTextureImage.hpp>
#include <soglu/GLViewSetup.hpp>
#include <soglu/GLSLShader.hpp>
#include <soglu/OGLDrawing.hpp>

#include <vorgl/TransferFunctionBuffer.hpp>

#include <boost/filesystem/path.hpp>

namespace vorgl {

enum TFConfigurationFlags{
	tfShading	= 1,
	tfJittering	= 1 << 1,
	tfIntegral	= 1 << 2
};


class VolumeRenderer
{
public:
	void
	initialize(boost::filesystem::path aPath);

	void
	finalize();

	
	void
	basicRendering( 
		const soglu::Camera &aCamera, 
		const soglu::GLTextureImageTyped<3> &aImage, 
		const soglu::BoundingBox3D &aBoundingBox, 
		size_t aSliceCount,
		bool aJitterEnabled,
		float aJitterStrength, 
		bool aEnableCutPlane,
		soglu::Planef aCutPlane,
		bool aEnableInterpolation,
		glm::fvec2 aLutWindow,
		const soglu::GLViewSetup &aViewSetup,
		bool aMIP,
		uint64 aFlags
     	);
	
	void
	transferFunctionRendering(
		const soglu::Camera &aCamera,
		const soglu::GLTextureImageTyped<3> &aImage,
		const soglu::BoundingBox3D &aBoundingBox,
		int aSliceCount,
		bool aJitterEnabled,
		float aJitterStrength,
		bool aEnableCutPlane,
		soglu::Planef aCutPlane,
		bool aEnableInterpolation,
		const soglu::GLViewSetup &aViewSetup,
		const GLTransferFunctionBuffer1D &aTransferFunction,
		glm::fvec3 aLightPosition,
		uint64 aFlags
		);
	
	void
	setupJittering(float aJitterStrength);
	
	void
	setupView(const soglu::Camera &aCamera, const soglu::GLViewSetup &aViewSetup);
	
	void
	setupSamplingProcess(const soglu::BoundingBox3D &aBoundingBox, const soglu::Camera &aCamera, size_t aSliceCount);
	
	void
	setupLights(const glm::fvec3 &aLightPosition);
	
	void
	initJitteringTexture();

	/*void
	reallocateArrays( size_t aNewMaxSampleCount )
	{
		if( mVertices ) {
			delete [] mVertices;
		}
		if( mIndices ) {
			delete [] mIndices;
		}

		mVertices = new glm::fvec3[ (aNewMaxSampleCount+1) * 6 ];
		mIndices = new unsigned[ (aNewMaxSampleCount+1) * 7 ];
		mMaxSampleCount = aNewMaxSampleCount;
	}*/

	soglu::GLSLProgram mShaderProgram;

	CGcontext   				mCgContext;
	soglu::CgFXShader			mCgEffect;
	GLuint					mNoiseMap;

	soglu::VertexIndexBuffers mSliceBuffers;
	
	/*glm::fvec3 *mVertices;
	unsigned *mIndices;
	size_t		mMaxSampleCount;*/
};


} //namespace vorgl
