#include <GL/glew.h>
#include <vorgl/VolumeRenderer.hpp>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/verbose_operator.hpp>

#include <vorgl/SliceGeneration.hpp>

namespace vorgl
{



/*void
applyVolumeRestrictionsOnBoundingBox(soglu::BoundingBox3D &aBBox, const VolumeRestrictions &aVolumeRestrictions )
{
	glm::fvec3 corner1 = aBBox.getMin();
	glm::fvec3 corner2 = aBBox.getMax();
	glm::fvec3 size = corner2 - corner1;

	glm::fvec3 i1Tmp, i2Tmp;
	aVolumeRestrictions.get3D(i1Tmp, i2Tmp);

	glm::fvec3 i1(i1Tmp[0], i1Tmp[1], i1Tmp[2]);
	glm::fvec3 i2(i2Tmp[0], i2Tmp[1], i2Tmp[2]);
	//LOG( "Restrictions : " << i1 << " - " << i2 );

	corner2 = corner1 + (i2 * size);
	corner1 += (i1 * size);
	aBBox = soglu::BoundingBox3D(corner1, corner2);
}*/

void
VolumeRenderer::initialize(boost::filesystem::path aPath)
{
	//soglu::initializeCg();
	//mCgEffect.initialize(aPath/*"ImageRender.cgfx"*/);

	mShaderProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(aPath / "volume.vert.glsl", aPath / "volume.frag.glsl");

	initJitteringTexture();
}

void
VolumeRenderer::initJitteringTexture()
{
	//TODO make better - destroy
	int size = 32;
	uint8 * buf = new uint8[size*size];
	srand( (unsigned)time(NULL) );
	for( int i = 0; i < size*size; ++i ) {
		buf[i] = static_cast<uint8>( 255.0f * rand()/(float)RAND_MAX );
	}
	glGenTextures(1, &mNoiseMap );
	//glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture( GL_TEXTURE_2D, mNoiseMap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_LUMINANCE8,
			size,
			size,
			0,
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE,
			buf
		    );
	glBindTexture( GL_TEXTURE_2D, 0 );

	delete buf;
}

void
VolumeRenderer::finalize()
{
	//TODO
}


void
VolumeRenderer::setupView(const soglu::Camera &aCamera, const soglu::GLViewSetup &aViewSetup)
{
	//mCgEffect.setParameter("gCamera", aCamera);
	//mCgEffect.setParameter("gViewSetup", aViewSetup );
}

void
VolumeRenderer::setupLights(const glm::fvec3 &aLightPosition)
{
	//mCgEffect.setParameter( "gLight.position", aLightPosition );
	//mCgEffect.setParameter( "gLight.color", glm::fvec3( 1.0f, 1.0f, 1.0f ) );
	//mCgEffect.setParameter( "gLight.ambient", glm::fvec3( 0.3f, 0.3f, 0.3f ) );
}

void
VolumeRenderer::setupJittering(float aJitterStrength)
{
	//mCgEffect.setTextureParameter( "gNoiseMap", mNoiseMap );
	//mCgEffect.setParameter("gNoiseMapSize", glm::fvec2( 32.0f, 32.0f ) );
	//mCgEffect.setParameter("gJitterStrength", aJitterStrength  );
}

void
VolumeRenderer::setupSamplingProcess(const soglu::BoundingBox3D &aBoundingBox, const soglu::Camera &aCamera, size_t aSliceCount)
{
	/*static int edgeOrder[8*12] = {
		 10, 11,  9,  4,  8,  5,  1,  0,  6,  2,  3,  7,
		 11,  8, 10,  5,  9,  6,  2,  1,  7,  3,  0,  4,
		  8,  9, 11,  6, 10,  7,  3,  2,  4,  0,  1,  5,
		  9, 10,  8,  7, 11,  4,  0,  3,  5,  1,  2,  6,
		  1,  0,  2,  4,  3,  7, 10, 11,  6,  9,  8,  5,
		  2,  1,  3,  5,  0,  4, 11,  8,  7, 10,  9,  6,
		  3,  2,  0,  6,  1,  5,  8,  9,  4, 11, 10,  7,
		  0,  3,  1,  7,  2,  6,  9, 10,  5,  8, 11,  4
	};
	mCgEffect.setParameter( "edgeOrder", edgeOrder, 8*12 );
	mCgEffect.setParameter( "gMinID", (int)minId );
	mCgEffect.setParameter( "gBBox", aBoundingBox );*/

	float 		min = 0;
	float 		max = 0;
	unsigned		minId = 0;
	unsigned		maxId = 0;
	getBBoxMinMaxDistance(
			aBoundingBox,
			aCamera.eyePosition(),
			aCamera.targetDirection(),
			min,
			max,
		       	minId,
		       	maxId
			);
	float renderingSliceThickness = (max-min)/static_cast< float >( aSliceCount );
	mShaderProgram.setUniformByName("gRenderingSliceThickness", renderingSliceThickness);
	//mCgEffect.setParameter("gRenderingSliceThickness", renderingSliceThickness);
}



/*enum TFConfigurationFlags{
	tfShading	= 1,
	tfJittering	= 1 << 1,
	tfIntegral	= 1 << 2
};*/

namespace detail {

static const uint64 FLAGS_TO_NAME_SUFFIXES_MASK = tfShading | tfJittering | tfIntegral;
static const std::string gFlagsToNameSuffixes[] =
	{
		std::string("Simple"),
		std::string("Shading"),
		std::string("Jittering"),
		std::string("JitteringShading"),
		std::string("PreintegratedSimple"),
		std::string("PreintegratedShading"),
		std::string("PreintegratedJittering"),
		std::string("PreintegratedJitteringShading"),

		std::string("UNDEFINED_COMBINATION")
	};

}


void
VolumeRenderer::basicRendering(
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
      			)
{
	/*D_PRINT("FLAGS " << aFlags);

	mCgEffect.setParameter( "gPrimaryImageData3D", aImage );
	mCgEffect.setParameter( "gMappedIntervalBands", aImage.getMappedInterval() );

	setupView(aCamera, aViewSetup);
	setupJittering(aJitterStrength);
	setupSamplingProcess(aBoundingBox, aCamera, aSliceCount);

	mCgEffect.setParameter("gEnableCutPlane", aEnableCutPlane );
	mCgEffect.setParameter("gCutPlane", aCutPlane );

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation );



	mCgEffect.setParameter("gWLWindow", aLutWindow );

	std::string techniqueName = aMIP ? "WLWindowMIP_3D" : "WLWindowBasic_3D";

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlices_Intermediate,
				aBoundingBox,
				aCamera,
				aSliceCount,
				mVertices,
				mIndices,
				1.0f
				)
			);


	soglu::checkForGLError( "OGL error : " );*/
}

void
VolumeRenderer::transferFunctionRendering(
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
	)
{
	GL_ERROR_CLEAR_AFTER_CALL();
	aSliceCount = 3;
	mShaderProgram.setUniformByName("modelViewProj", glm::mat4(aViewSetup.modelViewProj));
	mShaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(GL_TEXTURE0));
	//mShaderProgram.setUniformByName("gTransferFunction1D", aTransferFunction); // TODO - setUniform for transfer function
	setUniform(mShaderProgram, "gTransferFunction1D", aTransferFunction, soglu::TextureUnitId(GL_TEXTURE1));

	mShaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());
	setupSamplingProcess(aBoundingBox, aCamera, aSliceCount);
	int vertexLocation = mShaderProgram.getAttributeLocation("vertex");
	mShaderProgram.bind();

	vorgl::generateVolumeSlices(
		aBoundingBox,
		aCamera,
		aSliceCount,
		1.0f,
		mSliceBuffers
		);
	soglu::drawVertexIndexBuffers(
		mSliceBuffers,
		GL_TRIANGLE_FAN,
		//GL_LINE_LOOP,
		vertexLocation
		);
	soglu::gl::useProgram(0);
	mShaderProgram.unbind();
	/*mCgEffect.setParameter( "gPrimaryImageData3D", aImage );
	mCgEffect.setParameter( "gMappedIntervalBands", aImage.getMappedInterval() );

	setupView(aCamera, aViewSetup);
	setupJittering(aJitterStrength);
	setupSamplingProcess(aBoundingBox, aCamera, aSliceCount);

	mCgEffect.setParameter("gEnableCutPlane", aEnableCutPlane );
	mCgEffect.setParameter("gCutPlane", aCutPlane );

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation );

	setupLights(aLightPosition);
	std::string techniqueName;
	if (aFlags & tfIntegral) {
		mCgEffect.setParameter( "gIntegralTransferFunction1D", aTransferFunction );
	} else {
		mCgEffect.setParameter( "gTransferFunction1D", aTransferFunction );
	}

	techniqueName = "TransferFunction1D";
	techniqueName += detail::gFlagsToNameSuffixes[aFlags & detail::FLAGS_TO_NAME_SUFFIXES_MASK];
	techniqueName += "_3D";

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlices_Intermediate,
				aBoundingBox,
				aCamera,
				aSliceCount,
				mVertices,
				mIndices,
				1.0f
				)
			);
	soglu::checkForGLError( "OGL error : " );*/
}


}//vorgl

