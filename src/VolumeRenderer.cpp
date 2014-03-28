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
	boost::filesystem::path vertexShaderPath = aPath / "volume.vert.glsl";
	boost::filesystem::path fragmentShaderPath = aPath / "volume.frag.glsl";

	std::string vertexShaderCode = soglu::loadFile(vertexShaderPath);
	std::string fragmentShaderCode = soglu::loadFile(fragmentShaderPath);
	auto vertexShader = std::make_shared<soglu::GLSLVertexShader>(vertexShaderCode);

	static std::unordered_map<TFRenderFlags, std::string, Hasher<TFRenderFlags>> flagDefines = [this]() {
			// TODO - use initializer_list (waiting for VS 2013)
			std::unordered_map<TFRenderFlags, std::string, Hasher<TFRenderFlags>> defines;
			defines[TFRenderFlags::JITTERING] = "#define ENABLE_JITTERING\n";
			defines[TFRenderFlags::SHADING] = "#define ENABLE_SHADING\n";
			defines[TFRenderFlags::PREINTEGRATED_TF] = "#define ENABLE_PREINTEGRATED_TRANSFER_FUNCTION\n";
			return defines;
		} ();

	for (int i = static_cast<int>(TFRenderFlags::NO_FLAGS); i <= static_cast<int>(TFRenderFlags::ALL_FLAGS); ++i) {
		TransferFunctionRenderFlags flags(i);
		std::string defines = "#version 150\n";
		for (const auto & def : flagDefines) {
			if (flags.test(def.first)) {
				defines += def.second;
			}
		}
		soglu::GLSLProgram program(true);
		program.attachShader(vertexShader);
		program.attachShader(std::make_shared<soglu::GLSLFragmentShader>(defines + fragmentShaderCode));
		program.link();
		program.validate();

		mTFShaderPrograms[flags] = std::move(program);
	}

//	mShaderProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(, aPath / "volume.frag.glsl");

	initJitteringTexture();
}

void
VolumeRenderer::initJitteringTexture()
{
	std::cout << "Init noise texture\n";
	int size = 32;
	std::vector<uint8> buf(size * size);
	srand( (unsigned)time(NULL) );
	for( int i = 0; i < size*size; ++i ) {
		buf[i] = static_cast<uint8>( 255.0f * rand()/(float)RAND_MAX );
	}
	GL_CHECKED_CALL(glGenTextures(1, &(mNoiseMap.value) ));
	soglu::gl::bindTexture(soglu::TextureUnitId(2), soglu::TextureTarget::Texture2D, mNoiseMap);
	//glActiveTextureARB(GL_TEXTURE3_ARB);
	GL_CHECKED_CALL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ));
	GL_CHECKED_CALL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ));
	GL_CHECKED_CALL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
	GL_CHECKED_CALL(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));
	GL_CHECKED_CALL(glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_R8,
			size,
			size,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			buf.data()
		    ));
	GL_CHECKED_CALL(glBindTexture( GL_TEXTURE_2D, 0 ));
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
VolumeRenderer::setupLights(soglu::GLSLProgram &aShaderProgram, const glm::fvec3 &aLightPosition)
{
	aShaderProgram.setUniformByName("gLight.position", aLightPosition);
	aShaderProgram.setUniformByName("gLight.color", glm::fvec3( 1.0f, 1.0f, 1.0f ));
	aShaderProgram.setUniformByName("gLight.ambient", glm::fvec3( 0.3f, 0.3f, 0.3f ));
	//mCgEffect.setParameter( "gLight.position", aLightPosition );
	//mCgEffect.setParameter( "gLight.color", glm::fvec3( 1.0f, 1.0f, 1.0f ) );
	//mCgEffect.setParameter( "gLight.ambient", glm::fvec3( 0.3f, 0.3f, 0.3f ) );
}

void
VolumeRenderer::setupJittering(soglu::GLSLProgram &aShaderProgram, float aJitterStrength)
{
	soglu::gl::bindTexture(soglu::TextureUnitId(2), soglu::TextureTarget::Texture2D, mNoiseMap);
	aShaderProgram.setUniformByName("gNoiseMap", soglu::TextureUnitId(2));
	aShaderProgram.setUniformByName("gNoiseMapSize", glm::fvec2(32.0f, 32.0f));
	aShaderProgram.setUniformByName("gJitterStrength", aJitterStrength);
	//mCgEffect.setTextureParameter( "gNoiseMap", mNoiseMap );
	//mCgEffect.setParameter("gNoiseMapSize", glm::fvec2( 32.0f, 32.0f ) );
	//mCgEffect.setParameter("gJitterStrength", aJitterStrength  );
}

void
VolumeRenderer::setupSamplingProcess(soglu::GLSLProgram &aShaderProgram, const soglu::BoundingBox3D &aBoundingBox, const soglu::Camera &aCamera, size_t aSliceCount)
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
	aShaderProgram.setUniformByName("gRenderingSliceThickness", renderingSliceThickness);
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
	float aJitterStrength,
	bool aEnableCutPlane,
	soglu::Planef aCutPlane,
	bool aEnableInterpolation,
	const soglu::GLViewSetup &aViewSetup,
	const GLTransferFunctionBuffer1D &aTransferFunction,
	glm::fvec3 aLightPosition,
	VolumeRenderer::TransferFunctionRenderFlags aFlags
	)
{
	GL_ERROR_CLEAR_AFTER_CALL();
	soglu::GLSLProgram &shaderProgram = mTFShaderPrograms[aFlags];
	shaderProgram.setUniformByName("gCamera", aCamera);
	shaderProgram.setUniformByName("gViewSetup", aViewSetup);
	//shaderProgram.setUniformByName("modelViewProj", glm::mat4(aViewSetup.modelViewProj));
	shaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(0));
	//shaderProgram.setUniformByName("gTransferFunction1D", aTransferFunction); // TODO - setUniform for transfer function
	setUniform(shaderProgram, "gTransferFunction1D", aTransferFunction, soglu::TextureUnitId(1));

	shaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());
	setupSamplingProcess(shaderProgram, aBoundingBox, aCamera, aSliceCount);
	setupJittering(shaderProgram, aJitterStrength);
	setupLights(shaderProgram, aLightPosition);
	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	shaderProgram.bind();

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
	shaderProgram.unbind();
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

