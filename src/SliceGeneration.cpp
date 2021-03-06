#include <GL/glew.h>
#include <vorgl/SliceGeneration.hpp>
#include <soglu/OGLDrawing.hpp>
#include <array>


namespace vorgl {

std::vector<glm::fvec3>
generateVolumeSlice(
		const glm::fvec3        &aMin,
		const glm::fvec3        &aMax,
		float			        aSliceCoord,
		soglu::CartesianPlanes  aPlane
		)
{
	std::vector<glm::fvec3> points(4);

	float sliceTexCoord = (aSliceCoord - aMin[aPlane]) / (aMax[aPlane] - aMin[aPlane]);
	glm::fvec2 p1 = purgeDimension(aMin, aPlane);
	glm::fvec2 p2 = purgeDimension(aMax, aPlane);
	points[0] = insertDimension(p1, aSliceCoord, aPlane);
	points[2] = insertDimension(p2, aSliceCoord, aPlane);

	points[1] = insertDimension(glm::fvec2(p2[0], p1[1]), aSliceCoord, aPlane);
	points[3] = insertDimension(glm::fvec2(p1[0], p2[1]), aSliceCoord, aPlane);
	return points;
}

void
GLDrawVolumeSlice3D(
		const glm::fvec3 	&aMin,
		const glm::fvec3 	&aMax,
		float			sliceCoord,
		soglu::CartesianPlanes	plane
		)
{
	float sliceTexCoord = (sliceCoord - aMin[plane]) / (aMax[plane] - aMin[plane]);
	glm::fvec2 point1 = purgeDimension( aMin, plane );
	glm::fvec2 point3 = purgeDimension( aMax, plane );

	glm::fvec2 point2( point3[0], point1[1] );
	glm::fvec2 point4( point1[0], point3[1] );

	glm::fvec3 tex1 = insertDimension( point1, sliceCoord, plane );
	glm::fvec3 tex2 = insertDimension( point2, sliceCoord, plane );
	glm::fvec3 tex3 = insertDimension( point3, sliceCoord, plane );
	glm::fvec3 tex4 = insertDimension( point4, sliceCoord, plane );

	//std::cout << sliceCoord << "  " << sliceTexCoord << " tex\n";
	glBegin( GL_QUADS );
		soglu::GLTextureVector( tex1 );
		soglu::GLVertexVector( tex1 );

		soglu::GLTextureVector( tex2 );
		soglu::GLVertexVector( tex2 );

		soglu::GLTextureVector( tex3 );
		soglu::GLVertexVector( tex3 );

		soglu::GLTextureVector( tex4 );
		soglu::GLVertexVector( tex4 );
	glEnd();
}

void
generateVolumeSlices(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		float			cutPlane,
		soglu::VertexIndexBuffers &buffers
		)
{
	buffers.vertices.resize((numberOfSteps + 1) * 7);
	buffers.indices.resize((numberOfSteps + 1) * 7);
	buffers.primitiveRestartIndex = numberOfSteps * 20;

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

	glm::fvec3 *currentVertexPtr = buffers.vertices.data();
	unsigned *currentIndexPtr = buffers.indices.data();
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
		for( unsigned j = 0; j < count; ++j ) {
			*(currentIndexPtr++) = primitiveStartIndex + j;
		}
		*(currentIndexPtr++) = buffers.primitiveRestartIndex;
		primitiveStartIndex += count;

		indicesSize += count+1;
	}
	buffers.indices.resize(indicesSize);
	buffers.vertices.resize(primitiveStartIndex);

	/*GLuint buff;
	GL_CHECKED_CALL(glGenBuffers(1, &buff));

	GL_CHECKED_CALL(glBindBuffer(GL_ARRAY_BUFFER, buff));

	GL_CHECKED_CALL(glBufferData(GL_ARRAY_BUFFER,  GLsizeiptr  size,  vertices, GL_STREAM_DRAW));

	GL_CHECKED_CALL(glDeleteBuffers(1, &buff));*/

	/*GL_CHECKED_CALL( glEnableClientState(GL_VERTEX_ARRAY) );
	//GL_CHECKED_CALL( glVertexPointer( 3, GL_FLOAT, 0, vertices ) );
	glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	//GL_CHECKED_CALL( glDrawElements(GL_TRIANGLE_FAN, indicesSize, GL_UNSIGNED_INT, indices) );
	GL_CHECKED_CALL( glDrawElements(GL_LINE_LOOP, indicesSize, GL_UNSIGNED_INT, indices) );
	GL_CHECKED_CALL( glDisableClientState(GL_VERTEX_ARRAY) );
	GL_CHECKED_CALL( glDisable(GL_PRIMITIVE_RESTART) );*/
	/*GLuint g_verticesVBO;
	GLuint g_vao;
	glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * primitiveStartIndex * sizeof(GLfloat), (GLfloat*) vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    // Bind the only used VBO in this example.
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLocation);

	glDrawElements(GL_LINE_LOOP, indicesSize, GL_UNSIGNED_INT, indices);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &g_verticesVBO);
	glBindVertexArray(0);
    glDeleteVertexArrays(1, &g_vao);*/
}


void
GLDrawVolumeSlices_Buffered(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		glm::fvec3		*vertices,
		unsigned			*indices,
		float			cutPlane
		)
{
	GL_ERROR_CLEAR_AFTER_CALL();
	//Vector3f *vertices = new Vector3f[ (numberOfSteps+1) * 6 ];
	//unsigned *indices = new unsigned[ (numberOfSteps+1) * 7 ];
	unsigned primitiveRestart = numberOfSteps * 20;

	SOGLU_ASSERT(GL_VERSION_3_1);
	SOGLU_ASSERT(glPrimitiveRestartIndex != NULL);
	glEnable(GL_PRIMITIVE_RESTART);
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
		for( unsigned j = 0; j < count; ++j ) {
			*(currentIndexPtr++) = primitiveStartIndex + j;
		}
		*(currentIndexPtr++) = primitiveRestart;
		primitiveStartIndex += count;

		indicesSize += count+1;
	}

	/*GLuint buff;
	GL_CHECKED_CALL(glGenBuffers(1, &buff));

	GL_CHECKED_CALL(glBindBuffer(GL_ARRAY_BUFFER, buff));

	GL_CHECKED_CALL(glBufferData(GL_ARRAY_BUFFER,  GLsizeiptr  size,  vertices, GL_STREAM_DRAW));

	GL_CHECKED_CALL(glDeleteBuffers(1, &buff));*/

	GL_CHECKED_CALL( glEnableClientState(GL_VERTEX_ARRAY) );
	GL_CHECKED_CALL( glVertexPointer( 3, GL_FLOAT, 0, vertices ) );
	//GL_CHECKED_CALL( glDrawElements(GL_TRIANGLE_FAN, indicesSize, GL_UNSIGNED_INT, indices) );
	GL_CHECKED_CALL( glDrawElements(GL_LINE_LOOP, indicesSize, GL_UNSIGNED_INT, indices) );
	GL_CHECKED_CALL( glDisableClientState(GL_VERTEX_ARRAY) );
	GL_CHECKED_CALL( glDisable(GL_PRIMITIVE_RESTART) );
}

void
GLDrawVolumeSlices_Intermediate(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		glm::fvec3		*,
		unsigned		*,
		float			cutPlane
		)
{
	GL_ERROR_CLEAR_AFTER_CALL();
	//Vector3f *vertices = new Vector3f[ (numberOfSteps+1) * 6 ];
	//unsigned *indices = new unsigned[ (numberOfSteps+1) * 7 ];

	SOGLU_ASSERT(GL_VERSION_3_1);
	glEnable(GL_PRIMITIVE_RESTART);

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

	std::array<glm::fvec3, 6> vertices;
	for( unsigned i = 0; i < numberOfSteps; ++i ) {
		//Obtain intersection of the optical axis and the currently rendered plane
		planePoint -= stepSize * camera.targetDirection();
		//Get n-gon as intersection of the current plane and bounding box
		/*unsigned count = soglu::getPlaneVerticesInBoundingBox(
				bbox, planePoint, camera.targetDirection(), minId, currentVertexPtr
				);*/
		unsigned count = soglu::getPlaneVerticesInBoundingBox(
				bbox, soglu::Planef(planePoint, camera.targetDirection()), &(vertices[0])
				);

		GL_CHECKED_CALL(glBegin(GL_POLYGON));
			for (size_t i = 0; i < count; ++i) {
				glVertex3fv(reinterpret_cast<const GLfloat*>(&(vertices[i])));
			}
		glEnd();
	}
}


} //namespace vorgl
