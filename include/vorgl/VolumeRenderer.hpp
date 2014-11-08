#pragma once

#include <soglu/Camera.hpp>
#include <soglu/BoundingBox.hpp>
#include <soglu/Primitives.hpp>
#include <soglu/GLTextureImage.hpp>
#include <soglu/GLViewSetup.hpp>
#include <soglu/GLSLShader.hpp>
#include <soglu/OGLDrawing.hpp>
#include <soglu/OpenGLWrappers.hpp>

#include <vorgl/TransferFunctionBuffer.hpp>

#include <boost/filesystem/path.hpp>

#include <unordered_map>

#include <type_traits>

struct MaskedImage {
	const soglu::GLTextureImageTyped<3> &image;
	const soglu::GLTextureImageTyped<3> &mask;
};

inline std::string
definesFromInput(const soglu::GLTextureImageTyped<3>&)
{
	return std::string();
}

inline std::string
definesFromInput(const MaskedImage&)
{
	return "#define USE_MASK\n";
}

// TODO - find better place
template <typename TEnum>
class Flags {
public:
	typedef typename std::underlying_type<TEnum>::type RawType;
	typedef TEnum Enum;
	Flags() : mRaw() {}
	Flags(RawType aInitValue) : mRaw(aInitValue) {}

	//Flags(Flags const&) = default;

	Flags<Enum> &
	set(TEnum e)
	{
		mRaw |=  static_cast<RawType>(e);
		return *this;
	}

	Flags<Enum> &
	reset(TEnum e)
	{
		mRaw &= ~static_cast<RawType>(e);
		return *this;
	}

	bool
	test(TEnum e) const
	{
		return mRaw & static_cast<RawType>(e);
	}

	RawType
	rawValue() const
	{ return mRaw; }

	bool
	operator==(const Flags &aFlags) const
	{
		return mRaw == aFlags.mRaw;
	}

private:
	RawType mRaw;
};

template <typename TEnum>
struct Hasher
{
	size_t operator()(const Flags<TEnum> &aFlags) const
	{
		return static_cast<size_t>(aFlags.rawValue());
	}

	size_t operator()(const TEnum &aFlag) const
	{
		return static_cast<size_t>(aFlag);
	}
};

namespace vorgl {

enum TFConfigurationFlags{
	tfShading	= 1,
	tfJittering	= 1 << 1,
	tfIntegral	= 1 << 2
};

struct ClipPlanes {

};

struct LightConfiguration {
	glm::fvec3 lightPosition;
	bool enableLight;
};

struct DensityRenderingOptions {
	glm::fvec2 lutWindow;
	bool enableMIP;
	//VolumeRenderer::DensityRenderFlags flags;
};

struct TransferFunctionRenderingOptions : LightConfiguration {
	//const GLTransferFunctionBuffer1D &transferFunction;
	/*vorgl::GLTransferFunctionBuffer1D::ConstWPtr	transferFunction;
	vorgl::GLTransferFunctionBuffer1D::ConstWPtr	integralTransferFunction;*/
	TFBufferInfoVariant transferFunction;

	bool preintegratedTransferFunction;
	//VolumeRenderer::TransferFunctionRenderFlags flags;
};

struct VolumeRenderingConfiguration {
	soglu::Camera camera;
	soglu::GLViewSetup viewSetup;
	soglu::BoundingBox3D boundingBox;
	glm::ivec2 windowSize;
	soglu::TextureId depthBuffer;
};

struct IsoSurfaceRenderingOptions : LightConfiguration {
	float isoValue;
	glm::fvec4 isoSurfaceColor;
};

struct RenderingQuality {
	int sliceCount;
	bool enableInterpolation;
	bool enableJittering;
	float jitterStrength;
};

struct VorglError : soglu::GLException {};

} // namespace vorgl

namespace std {
// TODO - find better place
/*template <typename TFirst, typename TSecond>
struct hash<std::pair<TFirst, TSecond>
{
	typedef size_t result_type;
	typedef std::pair<TFirst, TSecond> argument_type;
	size_t
	operator()(const argument_type &aValue) const
	{
		return firstHash(aValue.first) ^ secondHash(aValue.second);
	}

	std::hash<TFirst> firstHash;
	std::hash<TSecond> secondHash;
};*/

} // namespace std

namespace vorgl {

class VolumeRenderer
{
public:
	enum class TFFlags : uint16_t {
		NO_FLAGS         = 0,

		JITTERING        = 1,
		SHADING          = 1 << 1,
		PREINTEGRATED_TF = 1 << 2,

		ALL_FLAGS       = (1 << 3) - 1
	};

	enum class DensityFlags : uint16_t {
		NO_FLAGS         = 0,

		JITTERING        = 1,
		MIP             = 1 << 1,

		ALL_FLAGS       = (1 << 2) - 1
	};

	enum {
		cDepthBufferTextureUnit = 9,
		cJitteringTextureUnit = 10,
		cTransferFunctionTextureUnit = 11,
		cData1TextureUnit = 12,
		cMaskTextureUnit = 13,
	};

	typedef Flags<TFFlags> TransferFunctionRenderFlags;
	typedef Flags<DensityFlags> DensityRenderFlags;


	void
	initialize(const boost::filesystem::path &aPath);

	void
	finalize();

	template<typename TInputData>
	void
	densityRendering(
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
	transferFunctionRendering(
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
	isosurfaceRendering(
			const VolumeRenderingConfiguration &aViewConfiguration,
			const TInputData &aImage,
			const RenderingQuality &aRenderingQuality,
			const ClipPlanes &aCutPlanes,
			const IsoSurfaceRenderingOptions &aIsoSurfaceRenderingOptions
			)
	{
		renderVolume(aViewConfiguration, aImage, aRenderingQuality, aCutPlanes, aIsoSurfaceRenderingOptions);
	}

protected:
	void
	setupJittering(soglu::GLSLProgram &aShaderProgram, float aJitterStrength);

	void
	setupView(const soglu::Camera &aCamera, const soglu::GLViewSetup &aViewSetup);

	void
	setupSamplingProcess(soglu::GLSLProgram &aShaderProgram, const soglu::BoundingBox3D &aBoundingBox, const soglu::Camera &aCamera, size_t aSliceCount);

	void
	setupLights(soglu::GLSLProgram &aShaderProgram, const glm::fvec3 &aLightPosition);

	void
	initJitteringTexture();

	void
	loadShaders(const boost::filesystem::path &aPath);

	template<typename TInputData>
	soglu::GLSLProgram &
	getShaderProgram(
			const TInputData &aData,
			const DensityRenderingOptions &aDensityRenderingOptions,
			const RenderingQuality &aRenderingQuality)
	{
		std::string defines = definesFromInput(aData) + "#define DENSITY_RENDERING\n";
		if (aRenderingQuality.enableJittering) {
			defines += "#define ENABLE_JITTERING\n";
		}

		if (aDensityRenderingOptions.enableMIP) {
			defines += "#define ENABLE_MIP\n";
		}

		if (!mDensityShaderPrograms[defines]) {
			soglu::ShaderProgramSource densityProgramSources = soglu::loadShaderProgramSource(mShaderPath / "density_volume.cfg", mShaderPath);
			mDensityShaderPrograms[defines] = soglu::createShaderProgramFromSources(densityProgramSources, defines);
		}
		return mDensityShaderPrograms[defines];
	}

	class ConfigureShaderStaticVisitor : public boost::static_visitor<void> {
	public:
		ConfigureShaderStaticVisitor(std::string &aDefines, bool aPreintegrated)
			: defines(aDefines)
			, preintegrated(aPreintegrated)
		{}

		void
		operator()(const TransferFunctionBuffer1DInfo &aInfo) const {
			defines += "#define USE_TRANSFER_FUNCTION_1D\n";
			if (preintegrated) {
				defines += "#define ENABLE_PREINTEGRATED_TRANSFER_FUNCTION\n";
			}
		}

		void
		operator()(const TransferFunctionBuffer2DInfo &aInfo) const {
			defines += "#define USE_TRANSFER_FUNCTION_2D\n";
		}

		std::string &defines;
		bool preintegrated;
	};

	template<typename TInputData>
	soglu::GLSLProgram &
	getShaderProgram(
			const TInputData &aData,
			const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions,
			const RenderingQuality &aRenderingQuality)
	{
		std::string defines = definesFromInput(aData) + "#define TRANSFER_FUNCTION_RENDERING\n";
		if (aRenderingQuality.enableJittering) {
			defines += "#define ENABLE_JITTERING\n";
		}

		if (aTransferFunctionRenderingOptions.enableLight) {
			defines += "#define ENABLE_SHADING\n";
		}

		boost::apply_visitor(
			ConfigureShaderStaticVisitor(defines, aTransferFunctionRenderingOptions.preintegratedTransferFunction),
			aTransferFunctionRenderingOptions.transferFunction);

		if (!mTFShaderPrograms[defines]) {
			soglu::ShaderProgramSource tfProgramSources = soglu::loadShaderProgramSource(mShaderPath / "transfer_function_volume.cfg", mShaderPath);
			mTFShaderPrograms[defines] = soglu::createShaderProgramFromSources(tfProgramSources, defines);
		}
		return mTFShaderPrograms[defines];
	}

	template<typename TInputData>
	soglu::GLSLProgram &
	getShaderProgram(
			const TInputData &aData,
			const IsoSurfaceRenderingOptions &aIsosurfaceRenderingOptions,
			const RenderingQuality &aRenderingQuality)
	{
		std::string defines = definesFromInput(aData);
		if (!mIsoSurfaceShaderPrograms[defines]) {
			soglu::ShaderProgramSource isoSurfaceProgramSources = soglu::loadShaderProgramSource(mShaderPath / "iso_surface_volume.cfg", mShaderPath);
			mIsoSurfaceShaderPrograms[defines] = soglu::createShaderProgramFromSources(isoSurfaceProgramSources, defines);
		}
		return mIsoSurfaceShaderPrograms[defines];
	}

	void
	renderAuxiliaryGeometryForRaycasting(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const ClipPlanes &aCutPlanes
		);

	void
	setVolumeRenderingImageData(
		soglu::GLSLProgram &aShaderProgram,
		const soglu::GLTextureImageTyped<3> &aImage,
		bool aEnableInterpolation
		);

	void
	setVolumeRenderingImageData(
		soglu::GLSLProgram &aShaderProgram,
		const MaskedImage &aData,
		bool aEnableInterpolation
		);

	void
	setVolumeRenderingViewConfiguration(
		soglu::GLSLProgram &aShaderProgram,
		const VolumeRenderingConfiguration &aViewConfiguration
		);
	void
	setVolumeRenderingQuality(
		soglu::GLSLProgram &aShaderProgram,
		const RenderingQuality &aRenderingQuality,
		const VolumeRenderingConfiguration &aViewConfiguration
		);

	void
	setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const DensityRenderingOptions &aDensityRenderingOptions
		);

	void
	setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions
		);

	void
	setRenderingOptions(
		soglu::GLSLProgram &aShaderProgram,
		const IsoSurfaceRenderingOptions &aIsosurfaceRenderingOptions
		);

	template<typename TInputData, typename TRenderingOptions>
	void
	renderVolume(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const TInputData &aData,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const TRenderingOptions &aRenderingOptions
		)
	{
		soglu::GLSLProgram &shaderProgram = getShaderProgram(aData, aRenderingOptions, aRenderingQuality);

		auto cull_face_enabler = soglu::enable(GL_CULL_FACE);
		renderAuxiliaryGeometryForRaycasting(aViewConfiguration, aCutPlanes);

		auto depth_test_disabler = soglu::disable(GL_DEPTH_TEST);
		GL_CHECKED_CALL(glCullFace(GL_BACK));

		int vertexLocation = shaderProgram.getAttributeLocation("vertex");
		auto programBinder = getBinder(shaderProgram);

		setVolumeRenderingViewConfiguration(shaderProgram, aViewConfiguration);
		setVolumeRenderingQuality(shaderProgram, aRenderingQuality, aViewConfiguration);
		setVolumeRenderingImageData(shaderProgram, aData, aRenderingQuality.enableInterpolation);

		setRenderingOptions(shaderProgram, aRenderingOptions);

		soglu::drawVertexIndexBuffers(soglu::generateBoundingBoxBuffers(aViewConfiguration.boundingBox), GL_TRIANGLE_STRIP, vertexLocation);
	}

	//soglu::GLSLProgram mRayCastingProgram;
	soglu::GLSLProgram mBasicShaderProgram;

	std::unordered_map<std::string, soglu::GLSLProgram> mTFShaderPrograms;
	std::unordered_map<std::string, soglu::GLSLProgram> mDensityShaderPrograms;
	std::unordered_map<std::string, soglu::GLSLProgram> mIsoSurfaceShaderPrograms;
	//soglu::GLSLProgram mIsoSurfaceShaderProgram;


	soglu::TextureId mNoiseMap;
	soglu::VertexIndexBuffers mSliceBuffers;

	soglu::Sampler mLinearInterpolationSampler;
	soglu::Sampler mNoInterpolationSampler;
	soglu::Sampler mMaskSampler;
	float mJitterStrength;

	boost::filesystem::path mShaderPath;
};


} //namespace vorgl
