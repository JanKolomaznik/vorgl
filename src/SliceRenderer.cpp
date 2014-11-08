#include <vorgl/SliceGeneration.hpp>
#include <vorgl/SliceRenderer.hpp>
#include <soglu/OGLDrawing.hpp>
#include <soglu/OGLTools.hpp>

namespace vorgl
{

void
SliceRenderer::initialize(const boost::filesystem::path &aPath)
{
	mShaderPath = aPath;

	loadShaders(aPath);

	mLinearInterpolationSampler.initialize();
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	mLinearInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	mNoInterpolationSampler.initialize();
	mNoInterpolationSampler.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	mNoInterpolationSampler.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void
SliceRenderer::finalize()
{
	//mShaderProgram.finalize();
	mLinearInterpolationSampler.finalize();
	mNoInterpolationSampler.finalize();
}

void
SliceRenderer::brightnessContrastRendering(
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const SliceConfiguration &aSlice,
		const SliceRenderingQuality &aRenderingQuality,
		const BrightnessContrastRenderingOptions &aBCOptions)
{
	renderSlice(
		aViewSetup,
		aImage,
		aSlice,
		aRenderingQuality,
		aBCOptions);
}

void SliceRenderer::maskRendering(
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const SliceConfiguration &aSlice,
		const SliceRenderingQuality &aRenderingQuality,
		const MaskRenderingOptions &aOptions)
{
	renderSlice(
		aViewSetup,
		aImage,
		aSlice,
		aRenderingQuality,
		aOptions);
}

void
SliceRenderer::loadShaders(const boost::filesystem::path &aPath)
{
	mBrightnessContrastShaderPrograms.clear();
	mMaskShaderPrograms.clear();
	//SOGLU_DEBUG_PRINT("Loading slice renderer shader programs.");
	//mShaderProgram = soglu::createGLSLProgramFromVertexAndFragmentShader(aPath / "slice.vert.glsl", aPath / "slice.frag.glsl");
	//SOGLU_DEBUG_PRINT("Slice renderer shader programs loaded.");
}

/*
void
SliceRenderer::Render( SliceRenderer::RenderingConfiguration & aConfig, const GLViewSetup &aViewSetup )
{
	GLTextureImageTyped<3>::Ptr primaryData = aConfig.primaryImageData.lock();
	if ( !primaryData ) {
		_THROW_ ErrorHandling::EObjectUnavailable( "Primary texture not available" );
	}

	mCgEffect.SetParameter( "gPrimaryImageData3D", *primaryData );
	mCgEffect.SetParameter( "gMappedIntervalBands", primaryData->GetMappedInterval() );

	GLTextureImageTyped<3>::Ptr secondaryData = aConfig.secondaryImageData.lock();
	if( secondaryData ) {
		mCgEffect.SetParameter( "gSecondaryImageData3D", *secondaryData );
	}

	mCgEffect.SetParameter( "gEnableInterpolation", aConfig.enableInterpolation );

	mCgEffect.SetParameter( "gViewSetup", aViewSetup );

	std::string techniqueName;
	GLTransferFunctionBuffer1D::ConstPtr transferFunction;
	switch ( aConfig.colorTransform ) {
	case ctLUTWindow:
		{
			mCgEffect.SetParameter( "gWLWindow", aConfig.lutWindow );
			techniqueName = "WLWindow_3D";
		}
		break;
	case ctTransferFunction1D:
		{
			transferFunction = aConfig.transferFunction.lock();
			if ( !transferFunction ) {
				_THROW_ M4D::ErrorHandling::EObjectUnavailable( "Transfer function no available" );
			}
			mCgEffect.SetParameter( "gTransferFunction1D", *transferFunction );
			techniqueName = "TransferFunction1D_3DNoBlending";
		}
		break;
	case ctSimpleColorMap:
		techniqueName = "SimpleRegionColorMap_3D";
		break;
	case ctMaxIntensityProjection:
		ASSERT( false );
		break;
	default:
		ASSERT( false );
	}

	mCgEffect.ExecuteTechniquePass(
			techniqueName,
			boost::bind( &M4D::GLDrawVolumeSlice3D,
				primaryData->getExtents().realMinimum,
				primaryData->getExtents().realMaximum,
				float32(aConfig.currentSlice[ aConfig.plane ]+0.5f) * primaryData->getExtents().elementExtents[aConfig.plane],
				aConfig.plane
				)
			);

	if( secondaryData ) {
		mCgEffect.ExecuteTechniquePass(
			"OverlayMask_3D",
			boost::bind( &M4D::GLDrawVolumeSlice3D,
				secondaryData->getExtents().realMinimum,
				secondaryData->getExtents().realMaximum,
				float32(aConfig.currentSlice[ aConfig.plane ]+0.5f) * secondaryData->getExtents().elementExtents[aConfig.plane],
				aConfig.plane
				)
			);
	}
}
*/

/*void
SliceRenderer::lutWindowRendering(
	const soglu::GLTextureImageTyped<3> &aImage,
	float aSlice,
	soglu::CartesianPlanes aPlane,
	glm::fvec2 aLutWindow,
	bool aEnableInterpolation,
	const soglu::GLViewSetup &aViewSetup
	)
{
	mShaderProgram.setUniformByName("gPrimaryImageData3D", aImage, soglu::TextureUnitId(cData1TextureUnit));
	if (aEnableInterpolation) {
		mLinearInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	} else {
		mNoInterpolationSampler.bind(soglu::TextureUnitId(cData1TextureUnit));
	}
	mShaderProgram.setUniformByName("gMappedIntervalBands", aImage.getMappedInterval());
	mShaderProgram.setUniformByName("gWLWindow", aLutWindow);
	mShaderProgram.setUniformByName("gViewSetup", aViewSetup);

	int vertexLocation = mShaderProgram.getAttributeLocation("vertex");
	mShaderProgram.use([&aImage, aSlice, aPlane, vertexLocation]() {
			soglu::drawVertexBuffer(
				generateVolumeSlice(aImage.getExtents().realMinimum, aImage.getExtents().realMaximum, aSlice, aPlane),
				GL_TRIANGLE_FAN,
				vertexLocation
				);
		});

	soglu::Sampler::unbind(soglu::TextureUnitId(0));
}
*/
void
SliceRenderer::transferFunctionRendering(
	const soglu::GLTextureImageTyped<3> &aImage,
	float aSlice,
	soglu::CartesianPlanes aPlane,
	const vorgl::GLTransferFunctionBuffer1D &aTransferFunction,
	bool aEnableInterpolation,
	const soglu::GLViewSetup &aViewSetup
	)
{
	/*mCgEffect.setParameter("gPrimaryImageData3D", aImage);
	mCgEffect.setParameter("gMappedIntervalBands", aImage.getMappedInterval());

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation);

	mCgEffect.setParameter("gViewSetup", aViewSetup);

	mCgEffect.setParameter( "gTransferFunction1D", aTransferFunction );
	std::string techniqueName = "TransferFunction1D_3DNoBlending";

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlice3D,
				aImage.getExtents().realMinimum,
				aImage.getExtents().realMaximum,
				aSlice,
				aPlane
				)
			);*/
}

void
SliceRenderer::overlayMaskRendering(
	const soglu::GLTextureImageTyped<3> &aImage,
	float aSlice,
	soglu::CartesianPlanes aPlane,
	float aTransparency,
	bool aEnableInterpolation,
	const soglu::GLViewSetup &aViewSetup
	)
{
	/*mCgEffect.setParameter("gPrimaryImageData3D", aImage);
	mCgEffect.setParameter("gMappedIntervalBands", aImage.getMappedInterval());

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation);

	mCgEffect.setParameter("gViewSetup", aViewSetup);

	std::string techniqueName = "OverlayMask_3D";

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlice3D,
				aImage.getExtents().realMinimum,
				aImage.getExtents().realMaximum,
				aSlice,
				aPlane
				)
			);*/
}

void
SliceRenderer::setSliceRenderingImageData(
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
SliceRenderer::setViewSetup(
		soglu::GLSLProgram &aShaderProgram,
		const soglu::GLViewSetup &aViewSetup
		)
{
	aShaderProgram.setUniformByName("gViewSetup", aViewSetup);
}


soglu::GLSLProgram &
SliceRenderer::getShaderProgram(
		const BrightnessContrastRenderingOptions &aBCOptions,
		const SliceRenderingQuality &aRenderingQuality)
{
	std::string defines = "#define BRIGHTNESS_CONTRAST_RENDERING\n";

	if (!mBrightnessContrastShaderPrograms[defines]) {
		soglu::ShaderProgramSource brightnessContrastProgramSources = soglu::loadShaderProgramSource(mShaderPath / "brightness_contrast_slice.cfg", mShaderPath);
		mBrightnessContrastShaderPrograms[defines] = soglu::createShaderProgramFromSources(brightnessContrastProgramSources, defines);
	}
	return mBrightnessContrastShaderPrograms[defines];
}


soglu::GLSLProgram &
SliceRenderer::getShaderProgram(
		const MaskRenderingOptions &aOptions,
		const SliceRenderingQuality &aRenderingQuality)
{
	std::string defines;// = "#define BRIGHTNESS_CONTRAST_RENDERING\n";

	if (!mMaskShaderPrograms[defines]) {
		soglu::ShaderProgramSource maskProgramSources = soglu::loadShaderProgramSource(mShaderPath / "mask_slice.cfg", mShaderPath);
		mMaskShaderPrograms[defines] = soglu::createShaderProgramFromSources(maskProgramSources, defines);
	}
	return mMaskShaderPrograms[defines];
}

void
SliceRenderer::setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const BrightnessContrastRenderingOptions &aBCOptions
		)
{
	aShaderProgram.setUniformByName("gWLWindow", aBCOptions.lutWindow);
}

void SliceRenderer::setRenderingOptions(soglu::GLSLProgram &aShaderProgram, const MaskRenderingOptions &aOptions)
{
	aShaderProgram.setUniformByName("gMaskColor", aOptions.maskColor);
}


template<typename TRenderingOptions>
void
SliceRenderer::renderSlice(
	const soglu::GLViewSetup &aViewSetup,
	const soglu::GLTextureImageTyped<3> &aImage,
	const SliceConfiguration &aSlice,
	const SliceRenderingQuality &aRenderingQuality,
	const TRenderingOptions &aRenderingOptions
	)
{
	soglu::GLSLProgram &shaderProgram = getShaderProgram(aRenderingOptions, aRenderingQuality);

	int vertexLocation = shaderProgram.getAttributeLocation("vertex");
	auto programBinder = getBinder(shaderProgram);

	setViewSetup(shaderProgram, aViewSetup);
	setSliceRenderingImageData(shaderProgram, aImage, aRenderingQuality.enableInterpolation);

	setRenderingOptions(shaderProgram, aRenderingOptions);

	shaderProgram.use([&aImage, &aSlice, vertexLocation]() {
			soglu::drawVertexBuffer(
				generateVolumeSlice(aImage.getExtents().realMinimum, aImage.getExtents().realMaximum, aSlice.slice, aSlice.plane),
				GL_TRIANGLE_FAN,
				vertexLocation
				);
		});
}


}//vorgl

