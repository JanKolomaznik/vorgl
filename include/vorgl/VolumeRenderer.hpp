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

struct DensityRenderingOptions {
	glm::fvec2 lutWindow;
	//VolumeRenderer::DensityRenderFlags flags;
};

struct TransferFunctionRenderingOptions {
	const GLTransferFunctionBuffer1D &transferFunction;
	glm::fvec3 lightPosition;
	bool enableLight;
	bool preintegratedTransferFunction;
	//VolumeRenderer::TransferFunctionRenderFlags flags;
};

struct VolumeRenderingConfiguration {
	const soglu::Camera &camera;
	const soglu::GLViewSetup &viewSetup;
	const soglu::BoundingBox3D &boundingBox;
	const glm::ivec2 &windowSize;
	soglu::TextureId depthBuffer;
};

struct RenderingQuality {
	int sliceCount;
	bool enableInterpolation;
	bool enableJittering;
};
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
		cJitteringTextureUnit = 10,
		cTransferFunctionTextureUnit = 11,
		cData1TextureUnit = 12,
	};

	typedef Flags<TFFlags> TransferFunctionRenderFlags;
	typedef Flags<DensityFlags> DensityRenderFlags;


	void
	initialize(const boost::filesystem::path &aPath);

	void
	finalize();

	void
	rayCasting(
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
		);

	void
	densityRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const soglu::GLTextureImageTyped<3> &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlane,
		const DensityRenderingOptions &aDensityRenderingOptions
		);

	void
	transferFunctionRendering(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const soglu::GLTextureImageTyped<3> &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions
		);

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

	soglu::GLSLProgram &
	getShaderProgram(
		const DensityRenderingOptions &aDensityRenderingOptions,
		const RenderingQuality &aRenderingQuality);

	soglu::GLSLProgram &
	getShaderProgram(
		const TransferFunctionRenderingOptions &aTransferFunctionRenderingOptions,
		const RenderingQuality &aRenderingQuality);


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

	template<typename TRenderingOptions>
	void
	renderVolume(
		const VolumeRenderingConfiguration &aViewConfiguration,
		const soglu::GLTextureImageTyped<3> &aImage,
		const RenderingQuality &aRenderingQuality,
		const ClipPlanes &aCutPlanes,
		const TRenderingOptions &aRenderingOptions
		);

	soglu::GLSLProgram mRayCastingProgram;
	soglu::GLSLProgram mBasicShaderProgram;

	std::unordered_map<std::string, soglu::GLSLProgram> mTFShaderPrograms;
	std::unordered_map<DensityRenderFlags, soglu::GLSLProgram, Hasher<DensityFlags>> mDensityShaderPrograms;
	soglu::TextureId mNoiseMap;
	soglu::VertexIndexBuffers mSliceBuffers;

	soglu::Sampler mLinearInterpolationSampler;
	soglu::Sampler mNoInterpolationSampler;
	float mJitterStrength;

	boost::filesystem::path mShaderPath;
};


} //namespace vorgl
