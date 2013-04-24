#pragma once
#include <boost/graph/graph_concepts.hpp>

namespace vorgl {
	
inline void
GLDrawVolumeSlices_Buffered(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		glm::fvec3		*vertices,
		unsigned			*indices,
		float			cutPlane
		)
{
	//Vector3f *vertices = new Vector3f[ (numberOfSteps+1) * 6 ];
	//unsigned *indices = new unsigned[ (numberOfSteps+1) * 7 ];
	unsigned primitiveRestart = numberOfSteps * 20;

	SOGLU_ASSERT(GL_VERSION_3_1);
	SOGLU_ASSERT(glPrimitiveRestartIndex != NULL);
	GL_CHECKED_CALL(glEnable(GL_PRIMITIVE_RESTART));
	GL_CHECKED_CALL(glPrimitiveRestartIndex(primitiveRestart));
	

	SOGLU_ASSERT(vertices);
	SOGLU_ASSERT(indices);

	float 				min = 0; 
	float 				max = 0;
	unsigned			minId = 0;	
	unsigned			maxId = 0;	
	soglu::getBBoxMinMaxDistance( 
		bbox, 
		camera.eyePosition(), 
		camera.targetDirection(), 
		min, 
		max,
		minId,	
		maxId	
		);
	
	float stepSize = cutPlane * (max - min) / numberOfSteps;
	glm::fvec3 planePoint = camera.eyePosition() + (camera.targetDirection() * max);

	glm::fvec3 *currentVertexPtr = vertices;
	unsigned *currentIndexPtr = indices;
	size_t primitiveStartIndex = 0;
	size_t indicesSize = 0;
	for( unsigned i = 0; i < numberOfSteps; ++i ) {
		//Obtain intersection of the optical axis and the currently rendered plane
		planePoint -= stepSize * camera.targetDirection();
		//Get n-gon as intersection of the current plane and bounding box
		unsigned count = soglu::getPlaneVerticesInBoundingBox( 
				bbox, planePoint, camera.targetDirection(), minId, currentVertexPtr
				);

		currentVertexPtr += count;
		primitiveStartIndex += count;
		//currentVertexPtr += 6;
		//primitiveStartIndex += 6;
		for( unsigned j = 0; j < count; ++j ) {
			*(currentIndexPtr++) = primitiveStartIndex + j;
		}
		*(currentIndexPtr++) = primitiveRestart;
		indicesSize += count+1;
		/*for( unsigned j = count; j <= 6; ++j ) {
			*(currentIndexPtr++) = primitiveRestart;
		}
		indicesSize += 7;*/
	}

	GL_CHECKED_CALL( glEnableClientState(GL_VERTEX_ARRAY) );
	GL_CHECKED_CALL( glVertexPointer( 3, GL_FLOAT, 0, vertices ) );
	GL_CHECKED_CALL( glDrawElements(GL_TRIANGLE_FAN, indicesSize, GL_UNSIGNED_INT, indices) );
	GL_CHECKED_CALL( glDisableClientState(GL_VERTEX_ARRAY) );
	GL_CHECKED_CALL( glDisable(GL_PRIMITIVE_RESTART) );
}
	
} //namespace vorgl