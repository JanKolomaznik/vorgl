#include <GL/glew.h>
#include <vorgl/VolumeRenderer.hpp>

#include <vector>

#include <boost/scope_exit.hpp>

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

	mSecondaryLinearInterpolationSampler.initialize();
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	mSecondaryLinearInterpolationSampler.setParameter(GL_TEXTURE_BORDER_COLOR, glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f));

	mSecondaryNoInterpolationSampler.initialize();
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	mSecondaryNoInterpolationSampler.setParameter(GL_TEXTURE_BORDER_COLOR, glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f));


	mMaskSampler.initialize();
	mMaskSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	mMaskSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	mMaskSampler.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	mMaskSampler.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	mMaskSampler.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	mMaskSampler.setParameter(GL_TEXTURE_BORDER_COLOR, glm::fvec4(0.0f, 0.0f, 0.0f, 0.0f));
	initJitteringTexture();
}

void
VolumeRenderer::loadShaders(const boost::filesystem::path &aPath)
{
	mShaderPath = aPath;

	mBasicShaderProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(aPath / "basic_vertex.glsl", aPath / "basic_fragment.glsl");

	mTFShaderPrograms.clear();
	mDensityShaderPrograms.clear();
	mIsoSurfaceShaderPrograms.clear();
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
	float min = 0;
	float max = 0;
	unsigned minId = 0;
	unsigned maxId = 0;
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

void
VolumeRenderer::setVolumeRenderingViewConfiguration(
		soglu::GLSLProgram &aShaderProgram,
		const VolumeRenderingConfiguration &aViewConfiguration
		)
{
	aShaderProgram.setUniformByName("gCamera", aViewConfiguration.camera);
	aShaderProgram.setUniformByName("gViewSetup", aViewConfiguration.viewSetup);
	aShaderProgram.setUniformByName("gWindowSize", glm::fvec2(aViewConfiguration.windowSize));

	soglu::gl::bindTexture(soglu::TextureUnitId(cDepthBufferTextureUnit), soglu::TextureTarget::Texture2D, aViewConfiguration.depthBuffer);
	aShaderProgram.setUniformByName("gDepthBuffer", soglu::TextureUnitId(cDepthBufferTextureUnit));
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

void VolumeRenderer::setVolumeRenderingImageData(soglu::GLSLProgram &aShaderProgram, const MaskedImage &aData, bool aEnableInterpolation)
{
	aShaderProgram.setUniformByName("gPrimaryImageData3D", aData.image, soglu::TextureUnitId(cData1TextureUnit));
	aShaderProgram.setUniformByName("gMappedIntervalBands", aData.image.getMappedInterval());

	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	}

	aShaderProgram.setUniformByName("gMaskData3D", aData.mask, soglu::TextureUnitId(cMaskTextureUnit));
	//aShaderProgram.setUniformByName("gMappedIntervalBands", aData.image.getMappedInterval());
	mMaskSampler.bind(soglu::TextureUnitId(cMaskTextureUnit));
}

void VolumeRenderer::setVolumeRenderingImageData(soglu::GLSLProgram &aShaderProgram, const MaskedImageWithSecondaryImage &aData, bool aEnableInterpolation)
{
	aShaderProgram.setUniformByName("gPrimaryImageData3D", aData.image, soglu::TextureUnitId(cData1TextureUnit));
	aShaderProgram.setUniformByName("gMappedIntervalBands", aData.image.getMappedInterval());

	aShaderProgram.setUniformByName("gSecondaryImageData3D", aData.secondary, soglu::TextureUnitId(cData2TextureUnit));
	aShaderProgram.setUniformByName("gSecondaryMappedIntervalBands", aData.secondary.getMappedInterval());

	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData2TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData2TextureUnit));
	}

	aShaderProgram.setUniformByName("gMaskData3D", aData.mask, soglu::TextureUnitId(cMaskTextureUnit));
	//aShaderProgram.setUniformByName("gMappedIntervalBands", aData.image.getMappedInterval());
	mMaskSampler.bind(soglu::TextureUnitId(cMaskTextureUnit));
}

void VolumeRenderer::setVolumeRenderingImageData(soglu::GLSLProgram &aShaderProgram, const ImageWithSecondaryImage &aData, bool aEnableInterpolation)
{
	aShaderProgram.setUniformByName("gPrimaryImageData3D", aData.image, soglu::TextureUnitId(cData1TextureUnit));
	aShaderProgram.setUniformByName("gMappedIntervalBands", aData.image.getMappedInterval());

	aShaderProgram.setUniformByName("gSecondaryImageData3D", aData.secondary, soglu::TextureUnitId(cData2TextureUnit));
	aShaderProgram.setUniformByName("gSecondaryMappedIntervalBands", aData.secondary.getMappedInterval());

	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData2TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData2TextureUnit));
	}
}


void
VolumeRenderer::setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const DensityRenderingOptions &aDensityRenderingOptions
		)
{
	aShaderProgram.setUniformByName("gWLWindow", aDensityRenderingOptions.lutWindow);
}

class SetTFUniformStaticVisitor : public boost::static_visitor<void> {
public:
	SetTFUniformStaticVisitor(soglu::GLSLProgram &aShaderProgram, bool aPreintegrated)
		: shaderProgram(aShaderProgram)
		, preintegrated(aPreintegrated)
	{}

	void
	operator()(const TransferFunctionBuffer1DInfo &aInfo) const {
		vorgl::GLTransferFunctionBuffer1D::ConstPtr tf;
		if (preintegrated) {
			tf = aInfo.tfGLIntegralBuffer;
		} else {
			tf = aInfo.tfGLBuffer;
		}
		if (!tf) {
			SOGLU_THROW(VorglError());
		}
		setUniform(shaderProgram, "gTransferFunction1D", *tf, soglu::TextureUnitId(VolumeRenderer::cTransferFunctionTextureUnit));
	}

	void
	operator()(const TransferFunctionBuffer2DInfo &aInfo) const {
		if (!aInfo.tfGLBuffer) {
			SOGLU_THROW(VorglError());
		}
		setUniform(shaderProgram, "gTransferFunction2D", *(aInfo.tfGLBuffer), soglu::TextureUnitId(VolumeRenderer::cTransferFunctionTextureUnit));
	}

	soglu::GLSLProgram &shaderProgram;
	bool preintegrated;
};

void
VolumeRenderer::setRenderingOptions(
	soglu::GLSLProgram &aShaderProgram,
	const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions
	)
{
	setupLights(aShaderProgram, aTransferFunctionRenderingOptions.lightPosition);

	boost::apply_visitor(
		SetTFUniformStaticVisitor(aShaderProgram, aTransferFunctionRenderingOptions.preintegratedTransferFunction),
		aTransferFunctionRenderingOptions.transferFunction);

	/*vorgl::GLTransferFunctionBuffer1D::ConstPtr tf;
	if (aTransferFunctionRenderingOptions.preintegratedTransferFunction) {
		tf = aTransferFunctionRenderingOptions.integralTransferFunction.lock();
	} else {
		tf = aTransferFunctionRenderingOptions.transferFunction.lock();
	}
	if (!tf) {
		SOGLU_THROW(VorglError());
	}
	setUniform(aShaderProgram, "gTransferFunction1D", *tf, soglu::TextureUnitId(cTransferFunctionTextureUnit));*/
}

void
VolumeRenderer::setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const IsoSurfaceRenderingOptions &aIsoSurfaceRenderingOptions
		)
{
	setupLights(aShaderProgram, aIsoSurfaceRenderingOptions.lightPosition);

	std::vector<float> isoValues(aIsoSurfaceRenderingOptions.isoSurfaces.size());
	std::vector<glm::vec4> surfaceColors(aIsoSurfaceRenderingOptions.isoSurfaces.size());
	for (int i = 0; i < isoValues.size(); ++i) {
		isoValues[i] = aIsoSurfaceRenderingOptions.isoSurfaces[i].isoValue;
		surfaceColors[i] = aIsoSurfaceRenderingOptions.isoSurfaces[i].isoSurfaceColor;
	}

	aShaderProgram.setUniformByName("gIsoValues", isoValues.data(), isoValues.size());
	aShaderProgram.setUniformByName("gIsoSurfacesColors", surfaceColors.data(), surfaceColors.size());

	//std::cout << "\n\n\nAttribute " << aShaderProgram.getAttributeLocation("gLight") << std::endl;
	//std::cout << "\n\n\nAttribute " << aShaderProgram.getAttributeLocation("gIsoSurfaces") << std::endl;
	//std::cout << "\n\n\nAttribute " << aShaderProgram.getAttributeLocation("gIsoSurfaces[0].surfaceColor") << std::endl;
	//std::cout << "\n\n\nAttribute2 " << aShaderProgram.getAttributeLocation("gIsoSurfaces[0].isoValue") << std::endl;
	//aShaderProgram.setUniformByName("gIsoValue", aIsoSurfaceRenderingOptions.isoValue);
	//aShaderProgram.setUniformByName("gSurfaceColor", aIsoSurfaceRenderingOptions.isoSurfaceColor);
}

void
VolumeRenderer::renderAuxiliaryGeometryForRaycasting(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const ClipPlanes &aCutPlanes
		)
{
		GL_CHECKED_CALL(glCullFace(GL_FRONT));
		GL_CHECKED_CALL(glColorMask(false, false, false, false));
		BOOST_SCOPE_EXIT_ALL(=) {
			GL_CHECKED_CALL(glColorMask(true, true, true, true));
		};

		int vertexLocation = mBasicShaderProgram.getAttributeLocation("vertex");
		auto programBinder = soglu::getBinder(mBasicShaderProgram);
		setVolumeRenderingViewConfiguration(mBasicShaderProgram, aViewConfiguration);

		soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aViewConfiguration.boundingBox), GL_TRIANGLE_STRIP, vertexLocation);
}










/*
template<typename TRenderingOptions>
void
VolumeRenderer::renderVolume(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const soglu::GLTextureImageTyped<3> &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const TRenderingOptions &aRenderingOptions
		)
{
	soglu::GLSLProgram &shaderProgram = getShaderProgram(aRenderingOptions, aRenderingQuality);

	auto cull_face_enabler = soglu::enable(GL_CULL_FACE);
	renderAuxiliaryGeometryForRaycasting(aViewConfiguration, aCutPlanes);

	auto depth_test_disabler = soglu::disable(GL_DEPTH_TEST);
	GL_CHECKED_CALL(glCullFace(GL_BACK));

	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	auto programBinder = getBinder(shaderProgram);

	setVolumeRenderingViewConfiguration(shaderProgram, aViewConfiguration);
	setVolumeRenderingQuality(shaderProgram, aRenderingQuality, aViewConfiguration);
	setVolumeRenderingImageData(shaderProgram, aImage, aRenderingQuality.enableInterpolation);

	setRenderingOptions(shaderProgram, aRenderingOptions);

	soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aViewConfiguration.boundingBox), GL_TRIANGLE_STRIP, vertexLocation);
}*/

/*

template<typename TInputData>
void
VolumeRenderer::densityRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const TInputData &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const DensityRenderingOptions &aDensityRenderingOptions
		)
{
	renderVolume(aViewConfiguration, aImage, aRenderingQuality, aCutPlanes, aDensityRenderingOptions);
}


template<typename TInputData>
void
VolumeRenderer::transferFunctionRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const TInputData &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions
		)
{
	renderVolume(aViewConfiguration, aImage, aRenderingQuality, aCutPlanes, aTransferFunctionRenderingOptions);
}

template<typename TInputData>
void
VolumeRenderer::isosurfaceRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const TInputData &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const IsoSurfaceRenderingOptions &aIsoSurfaceRenderingOptions
		)
{
	renderVolume(aViewConfiguration, aImage, aRenderingQuality, aCutPlanes, aIsoSurfaceRenderingOptions);
}
*/



}//vorgl

