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

template <typename TEnum>
class Flags {
public:
	typedef typename std::underlying_type<TEnum>::type RawType;
	typedef TEnum Enum;
	Flags() : mRaw() {}
	Flags(RawType aInitValue) : mRaw(aInitValue) {}

	Flags(Flags const&) = default;

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


class VolumeRenderer
{
public:
	enum class TFRenderFlags : uint16_t {
		NO_FLAGS         = 0,

		JITTERING        = 1,
		SHADING          = 1 << 1,
		PREINTEGRATED_TF = 1 << 2,

		ALL_FLAGS       = (1 << 3) - 1
	};

	typedef Flags<TFRenderFlags> TransferFunctionRenderFlags;
	void
	initialize(boost::filesystem::path aPath);

	void
	finalize();


	void
	basicRendering(
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
     	);

	void
	transferFunctionRendering(
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
		TransferFunctionRenderFlags aFlags
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

	std::unordered_map<TransferFunctionRenderFlags, soglu::GLSLProgram, Hasher<TFRenderFlags>> mTFShaderPrograms;
	soglu::TextureId mNoiseMap;
	soglu::VertexIndexBuffers mSliceBuffers;
};


} //namespace vorgl
