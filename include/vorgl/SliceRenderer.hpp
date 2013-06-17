#pragma once
#include <soglu/Camera.hpp>
#include <soglu/BoundingBox.hpp>
#include <soglu/Primitives.hpp>
#include <soglu/GLTextureImage.hpp>
#include <soglu/GLViewSetup.hpp>
#include <soglu/OGLTools.hpp>

#include <vorgl/TransferFunctionBuffer.hpp>

#include <boost/filesystem/path.hpp>

namespace vorgl {

class SliceRenderer
{
public:

	void
	initialize(boost::filesystem::path aPath);

	void
	finalize();
	
	void
	lutWindowRendering(
		const soglu::GLTextureImageTyped<3> &aImage,
		float aSlice,
		soglu::CartesianPlanes aPlane,
		glm::fvec2 aLutWindow,
		bool aEnableInterpolation,
		const soglu::GLViewSetup &aViewSetup
		);
	
	void
	transferFunctionRendering(
		const soglu::GLTextureImageTyped<3> &aImage,
		float aSlice,
		soglu::CartesianPlanes aPlane,
		const vorgl::GLTransferFunctionBuffer1D &aTransferFunction,
		bool aEnableInterpolation,
		const soglu::GLViewSetup &aViewSetup
		);

	void
	overlayMaskRendering(
		const soglu::GLTextureImageTyped<3> &aImage,
		float aSlice,
		soglu::CartesianPlanes aPlane,
		float aTransparency,
		bool aEnableInterpolation,
		const soglu::GLViewSetup &aViewSetup
		);
protected:
	CGcontext   				mCgContext;
	soglu::CgFXShader			mCgEffect;
};


} //namespace vorgl