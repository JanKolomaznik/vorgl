#include <vorgl/SliceGeneration.hpp>
#include <vorgl/SliceRenderer.hpp>
#include <soglu/OGLDrawing.hpp>
#include <soglu/OGLTools.hpp>

namespace vorgl
{

void
SliceRenderer::initialize(boost::filesystem::path aPath)
{
	soglu::initializeCg();
	mCgEffect.initialize(aPath);
}

void
SliceRenderer::finalize()
{
	//TODO
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

void
SliceRenderer::lutWindowRendering(
	const soglu::GLTextureImageTyped<3> &aImage,
	float aSlice,
	soglu::CartesianPlanes aPlane,
	glm::fvec2 aLutWindow,
	bool aEnableInterpolation,
	const soglu::GLViewSetup &aViewSetup
	)
{
	mCgEffect.setParameter("gPrimaryImageData3D", aImage);
	mCgEffect.setParameter("gMappedIntervalBands", aImage.getMappedInterval());

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation);

	mCgEffect.setParameter("gViewSetup", aViewSetup);

	mCgEffect.setParameter("gWLWindow", aLutWindow);
	std::string techniqueName = "WLWindow_3D";

	/*
	glColor3d( 0.0, 1.0, 1.0 );
	vorgl::GLDrawVolumeSlice3D(
				aImage.getExtents().realMinimum,
				aImage.getExtents().realMaximum,
				aSlice,
				aPlane
				);*/

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlice3D,
				aImage.getExtents().realMinimum,
				aImage.getExtents().realMaximum,
				aSlice,
				aPlane
				)
			);
}

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
	mCgEffect.setParameter("gPrimaryImageData3D", aImage);
	mCgEffect.setParameter("gMappedIntervalBands", aImage.getMappedInterval());

	mCgEffect.setParameter("gEnableInterpolation", aEnableInterpolation);

	mCgEffect.setParameter("gViewSetup", aViewSetup);

	mCgEffect.setParameter( "gTransferFunction1D", aTransferFunction );
	std::string techniqueName = "TransferFunction1D_3DNoBlending";

	//case ctSimpleColorMap:
	//	techniqueName = "SimpleRegionColorMap_3D";

	mCgEffect.executeTechniquePass(
			techniqueName,
			boost::bind( &vorgl::GLDrawVolumeSlice3D,
				aImage.getExtents().realMinimum,
				aImage.getExtents().realMaximum,
				aSlice,
				aPlane
				)
			);
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
	mCgEffect.setParameter("gPrimaryImageData3D", aImage);
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
			);
}


}//vorgl

