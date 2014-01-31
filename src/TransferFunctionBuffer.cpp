#include <GL/glew.h>
#include <vorgl/TransferFunctionBuffer.hpp>
#include <soglu/CgFXShader.hpp>
#include <soglu/TextureUtils.hpp>
#include <soglu/ErrorHandling.hpp>
#include <cmath>

namespace vorgl
{

TransferFunctionBuffer1D::Iterator
TransferFunctionBuffer1D::GetNearest( float aValue )
{
	int idx = floor(0.5f + ( aValue - mMappedInterval[0] ) / ( mMappedInterval[1] - mMappedInterval[0] ) * (float)size() );
	if ( idx < 0 || idx >= (int)size() ) {
		return end();
	}

	return begin() + idx;
}

TransferFunctionBuffer1D::ConstIterator
TransferFunctionBuffer1D::GetNearest( float aValue )const
{
	int idx = floor(0.5f + ( aValue - mMappedInterval[0] ) / ( mMappedInterval[1] - mMappedInterval[0] ) * (float)size() );
	if ( idx < 0 || idx >= (int)size() ) {
		return end();
	}

	return begin() + idx;
}

int
TransferFunctionBuffer1D::GetNearestIndex( float aValue )const
{
	int idx = floor(0.5f + ( aValue - mMappedInterval[0] ) / ( mMappedInterval[1] - mMappedInterval[0] ) * (float)size());
	if ( idx < 0 ) {
		return -1;
	}
	if( idx >= (int)size() ) {
		return static_cast<int>(size());
	}
	return idx;
}

void
TransferFunctionBuffer1D::SetMappedInterval( TransferFunctionBuffer1D::MappedInterval aMappedInterval )
{
	mMappedInterval = aMappedInterval;
}

GLTransferFunctionBuffer1D::Ptr
createGLTransferFunctionBuffer1D(const TransferFunctionBuffer1D &aTransferFunction)
{
	if ( aTransferFunction.size() == 0 ) {
		throw "ErrorHandling::EBadParameter";//( "Transfer function buffer of 0 size" );
	}

	GLuint texName;

	try {
		GL_CHECKED_CALL( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );
		GL_CHECKED_CALL( glPixelStorei(GL_PACK_ALIGNMENT, 1) );
		GL_CHECKED_CALL( glGenTextures( 1, &texName ) );
		GL_CHECKED_CALL( glBindTexture ( GL_TEXTURE_1D, texName ) );
		GL_CHECKED_CALL( glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE ) );

		GL_CHECKED_CALL( glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
		GL_CHECKED_CALL( glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
		GL_CHECKED_CALL( glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );


		GL_CHECKED_CALL( glEnable( GL_TEXTURE_1D ) );

		GL_CHECKED_CALL( glBindTexture( GL_TEXTURE_1D, texName ) );

		GL_CHECKED_CALL(
			glTexImage1D(
				GL_TEXTURE_1D,
				0,
				GL_RGBA32F,
				static_cast<GLsizei>(aTransferFunction.size()),
				0,
				GL_RGBA,
				GL_FLOAT,
				aTransferFunction.data()
				)
			);


		// soglu::checkForGLError( "OGL building texture for transfer function: " );
	}
	catch(std::exception &) {
		if( texName != 0 ) {
			glDeleteTextures( 1, &texName );
		}
		throw;
	}

	return GLTransferFunctionBuffer1D::Ptr(new GLTransferFunctionBuffer1D( texName, aTransferFunction.getMappedInterval(), aTransferFunction.size()));
	//return GLTransferFunctionBuffer1D::Ptr()
}


void
setCgFXParameter(CGeffect &aEffect, std::string aName, const vorgl::GLTransferFunctionBuffer1D &aTransferFunction)
{
	//TODO
	{
		CGparameter cgParameter = cgGetNamedEffectParameter(aEffect, (aName + ".data").c_str());
		cgGLSetupSampler( cgParameter, aTransferFunction.getTextureID() );
	}
	soglu::detail::setCgFXParameter(aEffect, aName + ".interval", aTransferFunction.getMappedInterval() );

	soglu::detail::setCgFXParameter(aEffect, aName + ".sampleCount", aTransferFunction.getSampleCount() );
}

} /*vorgl*/
