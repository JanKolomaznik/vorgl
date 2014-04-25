#include <GL/glew.h>
#include <vorgl/VolumeRenderer.hpp>

#include <vector>

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
VolumeRenderer::initialize(const boost::filesystem::path &aPath)
{
	mJitterStrength = 1.0f;

	loadShaders(aPath);

	mLinearInterpolationSampler.initialize();
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_BORDER_COLOR, glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f));

	mNoInterpolationSampler.initialize();
	mNoInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_BORDER_COLOR, glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f));

	initJitteringTexture();
}

void
VolumeRenderer::loadShaders(const boost::filesystem::path &aPath)
{
	mShaderPath = aPath;

	boost::filesystem::path vertexShaderPath = mShaderPath / "volume.vert.glsl";
	boost::filesystem::path fragmentShaderPath = mShaderPath / "volume.frag.glsl";

	soglu::ShaderProgramSource volumeProgramSources = soglu::loadShaderProgramSource(aPath / "volume.cfg", aPath);

	/*SOGLU_DEBUG_PRINT("Loading raycasting renderer shader program.");
	mRayCastingProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(vertexShaderPath, aPath / "testvolume.frag.glsl");
	SOGLU_DEBUG_PRINT("Raycasting renderer shader program loaded.");*/

	mBasicShaderProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(aPath / "basic_vertex.glsl", aPath / "basic_fragment.glsl");

	/*std::string vertexShaderCode = soglu::loadFile(vertexShaderPath);
	std::string fragmentShaderCode = soglu::loadFile(fragmentShaderPath);
	auto vertexShader = std::make_shared<soglu::GLSLVertexShader>(vertexShaderCode);
	*/
	static std::unordered_map<TFFlags, std::string, Hasher<TFFlags>> tfFlagDefines = [this]() {
			// TODO - use initializer_list (waiting for VS 2013)
			std::unordered_map<TFFlags, std::string, Hasher<TFFlags>> defines;
			defines[TFFlags::JITTERING] = "#define ENABLE_JITTERING\n";
			defines[TFFlags::SHADING] = "#define ENABLE_SHADING\n";
			defines[TFFlags::PREINTEGRATED_TF] = "#define ENABLE_PREINTEGRATED_TRANSFER_FUNCTION\n";
			return defines;
		} ();

	for (int i = static_cast<int>(TFFlags::NO_FLAGS); i <= static_cast<int>(TFFlags::ALL_FLAGS); ++i) {
		TransferFunctionRenderFlags flags(i);
		std::string defines = "#define TRANSFER_FUNCTION_RENDERING\n";
		for (const auto & def : tfFlagDefines) {
			if (flags.test(def.first)) {
				defines += def.second;
			}
		}
		SOGLU_DEBUG_PRINT("Creating volume renderer shader program. Defines: \n" << defines);
		mTFShaderPrograms[flags] = soglu::createShaderProgramFromSources(volumeProgramSources, defines);
		SOGLU_DEBUG_PRINT("Shader program : " << mTFShaderPrograms[flags].id());
		/*soglu::GLSLProgram program(true);
		program.attachShader(vertexShader);
		program.attachShader(std::make_shared<soglu::GLSLFragmentShader>(defines + fragmentShaderCode));
		program.link();
		program.validate();*/

		//mTFShaderPrograms[flags] = std::move(program);
	}

	/*static std::unordered_map<DensityFlags, std::string, Hasher<DensityFlags>> densityFlagDefines = [this]() {
			// TODO - use initializer_list (waiting for VS 2013)
			std::unordered_map<DensityFlags, std::string, Hasher<DensityFlags>> defines;
			defines[DensityFlags::JITTERING] = "#define ENABLE_JITTERING\n";
			defines[DensityFlags::MIP] = "";
			return defines;
		} ();

	for (int i = static_cast<int>(DensityFlags::NO_FLAGS); i <= static_cast<int>(DensityFlags::ALL_FLAGS); ++i) {
		DensityRenderFlags flags(i);
		std::string defines = "#version 150\n#define DENSITY_RENDERING\n";
		for (const auto & def : densityFlagDefines) {
			if (flags.test(def.first)) {
				defines += def.second;
			}
		}
		SOGLU_DEBUG_PRINT("Creating density volume renderer shader program. Defines: \n" << defines);
		soglu::GLSLProgram program(true);
		program.attachShader(vertexShader);
		program.attachShader(std::make_shared<soglu::GLSLFragmentShader>(defines + fragmentShaderCode));
		program.link();
		program.validate();

		mDensityShaderPrograms[flags] = std::move(program);
	}*/
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
	soglu::gl::bindTexture(soglu::TextureUnitId(cJitteringTextureUnit), soglu::TextureTarget::Texture2D, mNoiseMap);
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
VolumeRenderer::setupLights(soglu::GLSLProgram &aShaderProgram, const glm::fvec3 &aLightPosition)
{
	aShaderProgram.setUniformByName("gLight.position", aLightPosition);
	aShaderProgram.setUniformByName("gLight.color", glm::fvec3( 1.0f, 1.0f, 1.0f ));
	aShaderProgram.setUniformByName("gLight.ambient", glm::fvec3( 0.3f, 0.3f, 0.3f ));
}

void
VolumeRenderer::setupJittering(soglu::GLSLProgram &aShaderProgram, float aJitterStrength)
{
	soglu::gl::bindTexture(soglu::TextureUnitId(cJitteringTextureUnit), soglu::TextureTarget::Texture2D, mNoiseMap);
	aShaderProgram.setUniformByName("gNoiseMap", soglu::TextureUnitId(cJitteringTextureUnit));
	aShaderProgram.setUniformByName("gNoiseMapSize", glm::fvec2(32.0f, 32.0f));
	aShaderProgram.setUniformByName("gJitterStrength", aJitterStrength);
}

void
VolumeRenderer::setupSamplingProcess(soglu::GLSLProgram &aShaderProgram, const soglu::BoundingBox3D &aBoundingBox, const soglu::Camera &aCamera, size_t aSliceCount)
{
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
}


/*void
VolumeRenderer::densityRendering(
		const soglu::Camera &aCamera,
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const soglu::BoundingBox3D &aBoundingBox,
		glm::fvec2 aLutWindow,
		size_t aSliceCount,
		bool aEnableCutPlane,
		soglu::Planef aCutPlane,
		bool aEnableInterpolation,
		VolumeRenderer::DensityRenderFlags aFlags
		)*/

void
setVolumeRenderingViewConfiguration(
		soglu::GLSLProgram &aShaderProgram,
		const VolumeRenderingConfiguration &aViewConfiguration
		)
{
	aShaderProgram.setUniformByName("gCamera", aViewConfiguration.camera);
	aShaderProgram.setUniformByName("gViewSetup", aViewConfiguration.viewSetup);
	aShaderProgram.setUniformByName("gWindowSize", glm::fvec2(aViewConfiguration.windowSize));

	SOGLU_DEBUG_PRINT("----------------------------------------------------\n");
	SOGLU_DEBUG_PRINT("AAA " << aViewConfiguration.depthBuffer.value);
	soglu::gl::bindTexture(soglu::TextureUnitId(5), soglu::TextureTarget::Texture2D, aViewConfiguration.depthBuffer);
	aShaderProgram.setUniformByName("gDepthBuffer", soglu::TextureUnitId(5));
}

void
VolumeRenderer::setVolumeRenderingQuality(
		soglu::GLSLProgram &aShaderProgram,
		const RenderingQuality &aRenderingQuality,
		const VolumeRenderingConfiguration &aViewConfiguration
		)
{
	setupSamplingProcess(aShaderProgram, aViewConfiguration.boundingBox, aViewConfiguration.camera, aRenderingQuality.sliceCount);
	setupJittering(aShaderProgram, mJitterStrength);
}


void
VolumeRenderer::setVolumeRenderingImageData(
		soglu::GLSLProgram &aShaderProgram,
		const soglu::GLTextureImageTyped<3> &aImage,
		bool aEnableInterpolation
		)
{
	aShaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(cData1TextureUnit));
	aShaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());

	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	}
}


void
VolumeRenderer::setDensityRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const DensityRenderingOptions &aDensityRenderingOptions
		)
{
	aShaderProgram.setUniformByName("gWLWindow", aDensityRenderingOptions.lutWindow);
}

void
VolumeRenderer::renderAuxiliaryGeometryForRaycasting(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const ClipPlanes &aCutPlanes
		)
{
		GL_CHECKED_CALL(glCullFace(GL_FRONT));
		int vertexLocation = mBasicShaderProgram.getAttributeLocation("vertex");
		auto programBinder = soglu::getBinder(mBasicShaderProgram);
		setVolumeRenderingViewConfiguration(mBasicShaderProgram, aViewConfiguration);

		soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aViewConfiguration.boundingBox), GL_TRIANGLE_STRIP, vertexLocation);
}

soglu::GLSLProgram &
VolumeRenderer::getDensityShaderProgram(
		const DensityRenderingOptions &aDensityRenderingOptions,
		const RenderingQuality &aRenderingQuality)
{
	if (!mRayCastingProgram) {
		soglu::ShaderProgramSource densityProgramSources = soglu::loadShaderProgramSource(mShaderPath / "density_volume.cfg", mShaderPath);
		mRayCastingProgram = soglu::createShaderProgramFromSources(densityProgramSources, "");
	}
	return mRayCastingProgram;
}

void
VolumeRenderer::densityRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const soglu::GLTextureImageTyped<3> &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const DensityRenderingOptions &aDensityRenderingOptions
		)
{
	soglu::GLSLProgram &shaderProgram = getDensityShaderProgram(aDensityRenderingOptions, aRenderingQuality);

	auto cull_face_enabler = soglu::enable(GL_CULL_FACE);
	renderAuxiliaryGeometryForRaycasting(aViewConfiguration, aCutPlanes);

	auto depth_test_disabler = soglu::disable(GL_DEPTH_TEST);
	GL_CHECKED_CALL(glCullFace(GL_BACK));

	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	auto programBinder = getBinder(shaderProgram);

	setVolumeRenderingViewConfiguration(shaderProgram, aViewConfiguration);
	setVolumeRenderingQuality(shaderProgram, aRenderingQuality, aViewConfiguration);
	setVolumeRenderingImageData(shaderProgram, aImage, aRenderingQuality.enableInterpolation);
	setDensityRenderingOptions(shaderProgram, aDensityRenderingOptions);

	//shaderProgram.setUniformByName("gWindowSize", glm::fvec2(aViewConfiguration.windowSize));

	soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aViewConfiguration.boundingBox), GL_TRIANGLE_STRIP, vertexLocation);
}


void
VolumeRenderer::rayCasting(
		const soglu::Camera &aCamera,
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const soglu::BoundingBox3D &aBoundingBox,
		glm::fvec2 aLutWindow,
		size_t aSliceCount,
		bool aEnableCutPlane,
		soglu::Planef aCutPlane,
		bool aEnableInterpolation,
		VolumeRenderer::TransferFunctionRenderFlags aFlags,
		soglu::TextureId aDepthBuffer,
		glm::ivec2 aWindowSize
		)
{
		SOGLU_DEBUG_PRINT("----------------------------------------------------\n");
	SOGLU_DEBUG_PRINT("BBB " << aDepthBuffer.value);
	GL_CHECKED_CALL(glEnable(GL_CULL_FACE));
	{
		GL_CHECKED_CALL(glCullFace(GL_FRONT));
		int vertexLocation = mBasicShaderProgram.getAttributeLocation("vertex");
		auto programBinder = getBinder(mBasicShaderProgram);
		mBasicShaderProgram.setUniformByName("gCamera", aCamera);
		mBasicShaderProgram.setUniformByName("gViewSetup", aViewSetup);
		soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aBoundingBox), GL_TRIANGLE_STRIP, vertexLocation);
	}
	GL_CHECKED_CALL(glDisable(GL_DEPTH_TEST));
	soglu::GLSLProgram &shaderProgram = mRayCastingProgram;
	//soglu::GLSLProgram &shaderProgram = mBasicShaderProgram;
	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	auto programBinder = getBinder(shaderProgram);

	GL_CHECKED_CALL(glCullFace(GL_BACK));

	shaderProgram.setUniformByName("gCamera", aCamera);
	shaderProgram.setUniformByName("gViewSetup", aViewSetup);
	shaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(cData1TextureUnit));
	shaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());
	shaderProgram.setUniformByName("gWLWindow", aLutWindow);
	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	}

	soglu::gl::bindTexture(soglu::TextureUnitId(5), soglu::TextureTarget::Texture2D, aDepthBuffer);
	shaderProgram.setUniformByName("gDepthBuffer", soglu::TextureUnitId(5));
	shaderProgram.setUniformByName("gWindowSize", glm::fvec2(aWindowSize));

	soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aBoundingBox), GL_TRIANGLE_STRIP, vertexLocation);
	GL_CHECKED_CALL(glDisable(GL_CULL_FACE));
	GL_CHECKED_CALL(glEnable(GL_DEPTH_TEST));
}

void
VolumeRenderer::transferFunctionRendering(
	const soglu::Camera &aCamera,
	const soglu::GLViewSetup &aViewSetup,
	const soglu::GLTextureImageTyped<3> &aImage,
	const soglu::BoundingBox3D &aBoundingBox,
	const GLTransferFunctionBuffer1D &aTransferFunction,
	int aSliceCount,
	bool aEnableCutPlane,
	soglu::Planef aCutPlane,
	bool aEnableInterpolation,
	glm::fvec3 aLightPosition,
	VolumeRenderer::TransferFunctionRenderFlags aFlags
	)
{
	soglu::GLSLProgram &shaderProgram = mTFShaderPrograms[aFlags];
	shaderProgram.setUniformByName("gCamera", aCamera);
	shaderProgram.setUniformByName("gViewSetup", aViewSetup);
	shaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(cData1TextureUnit));
	shaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());
	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	}

	setupSamplingProcess(shaderProgram, aBoundingBox, aCamera, aSliceCount);
	setupJittering(shaderProgram, mJitterStrength);
	setupLights(shaderProgram, aLightPosition);

	setUniform(shaderProgram, "gTransferFunction1D", aTransferFunction, soglu::TextureUnitId(cTransferFunctionTextureUnit));

	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	auto programBinder = getBinder(shaderProgram);

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
}


}//vorgl

