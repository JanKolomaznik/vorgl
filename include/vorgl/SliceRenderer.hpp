#pragma once
#include <soglu/Camera.hpp>
#include <soglu/BoundingBox.hpp>
#include <soglu/Primitives.hpp>
#include <soglu/GLTextureImage.hpp>
#include <soglu/GLViewSetup.hpp>
#include <soglu/utils.hpp>
#include <unordered_map>
//#include <soglu/OGLTools.hpp>

#include <vorgl/TransferFunctionBuffer.hpp>

#include <boost/filesystem/path.hpp>

namespace vorgl {

struct SliceRenderingQuality {
	bool enableInterpolation;
};

struct BrightnessContrastRenderingOptions {
	glm::fvec2 lutWindow;
};

struct SliceConfiguration {
	float slice;
	soglu::CartesianPlanes plane;
};

class SliceRenderer
{
public:
	enum {
		cData1TextureUnit = 12
	};

	void
	initialize(const boost::filesystem::path &aPath);

	void
	finalize();

	void
	brightnessContrastRendering(
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const SliceConfiguration &aSlice,
		const SliceRenderingQuality &aRenderingQuality,
		const BrightnessContrastRenderingOptions &aBCOptions
		);

	/*void
	lutWindowRendering(
		const soglu::GLTextureImageTyped<3> &aImage,
		float aSlice,
		soglu::CartesianPlanes aPlane,
		glm::fvec2 aLutWindow,
		bool aEnableInterpolation,
		const soglu::GLViewSetup &aViewSetup
		);*/

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

	void
	loadShaders(const boost::filesystem::path &aPath);

protected:
	void
	setSliceRenderingImageData(
			soglu::GLSLProgram &aShaderProgram,
			const soglu::GLTextureImageTyped<3> &aImage,
			bool aEnableInterpolation
			);

	void
	setViewSetup(
			soglu::GLSLProgram &aShaderProgram,
			const soglu::GLViewSetup &aViewSetup
			);

	void
	setRenderingOptions(
			soglu::GLSLProgram &aShaderProgram,
			const BrightnessContrastRenderingOptions &aBCOptions
			);

	soglu::GLSLProgram &
	getShaderProgram(
			const BrightnessContrastRenderingOptions &aBCOptions,
			const SliceRenderingQuality &aRenderingQuality
			);

	template<typename TRenderingOptions>
	void
	renderSlice(
		const soglu::GLViewSetup &aViewSetup,
		const soglu::GLTextureImageTyped<3> &aImage,
		const SliceConfiguration &aSlice,
		const SliceRenderingQuality &aRenderingQuality,
		const TRenderingOptions &aRenderingOptions
		);

	std::unordered_map<std::string, soglu::GLSLProgram> mBrightnessContrastShaderPrograms;

	//soglu::GLSLProgram mShaderProgram;
	soglu::Sampler mLinearInterpolationSampler;
	soglu::Sampler mNoInterpolationSampler;

	boost::filesystem::path mShaderPath;
};


} //namespace vorgl
