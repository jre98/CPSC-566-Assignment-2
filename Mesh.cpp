#include "Mesh.h"

using namespace std;

void Mesh::load( const char* filename )
{

	// Open the OBJ file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        // Check if the line starts with 'v' (vertex)
        if (token == "v") {
            // Temp vector to hold vertex data
            Vector3f vertex;
            // Add to vector using stringstream and >> operator
            iss >> vertex[0] >> vertex[1] >> vertex[2];
            // Add individual vector to larger vector of vectors of all the vertices
            bindVertices.push_back(vertex);
        }
        // Check if the line starts with 'f' (face)
        else if (token == "f") {
			// Temp vector for face data
			Tuple3u face;
			unsigned index;
			unsigned vertexIndex = 0; // To keep track of which vertex index we are reading
			while (iss >> index) {
				// Assign the index to the appropriate component of the Tuple3u
				switch (vertexIndex) {
					case 0:
						face[0] = index;
						break;
					case 1:
						face[1] = index;
						break;
					case 2:
						face[2] = index;
						break;
				}
				// Move to the next vertex index
				vertexIndex++;
				// If we have read all three vertex indices for the face, add the Tuple3u to the faces vector
				if (vertexIndex == 3) {
					faces.push_back(face);
					// Reset vertexIndex for the next face
					vertexIndex = 0;
				}
			}
		}

	}
	// 2.1.1. load() should populate bindVertices, currentVertices, and faces

	// Add your code here.
	cout << "The size of the faces variable is: " << faces.size() << endl;
	cout << "The size of the bindVertices variable is: " << bindVertices.size() << endl;
	cout << "The size of the currentVertices variable is: " << currentVertices.size() << endl;
	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::draw()
{
	// Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".

	// First, loop through each face
    for (const auto& face : faces) {
        // Retrieve the vertices of the current face
        const Vector3f& v1 = currentVertices[face[0] - 1];
        const Vector3f& v2 = currentVertices[face[1] - 1];
        const Vector3f& v3 = currentVertices[face[2] - 1];

        // Compute the edges of the face
        Vector3f edge1 = v2 - v1;
        Vector3f edge2 = v3 - v1;

        // Compute the normal vector by taking the cross product of the edges
        Vector3f normal = Vector3f::cross(edge1, edge2);
		// Normalize
        normal.normalize();

        // Render the face with the computed normal for lighting
        glBegin(GL_TRIANGLES);
        glNormal3f(normal.x(), normal.y(), normal.z());
        glVertex3f(v1.x(), v1.y(), v1.z());
        glVertex3f(v2.x(), v2.y(), v2.z());
        glVertex3f(v3.x(), v3.y(), v3.z());
        glEnd();
    }
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
	// 2.2. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments

	// Use ifstream to open the attachment file
	std::ifstream file(filename);
	// Catch errors
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open attachment file " << filename << std::endl;
        return;
    }

	// Clear any existing attachment data
    attachments.clear(); 

	// This will hold the current line from the file
    std::string line;
	// Loop through each line in the file
    while (std::getline(file, line)) {
		// Convert current line to a stream of characters (use >> to read)
        std::istringstream iss(line);
		// This will hold the weights for a single vertex
        std::vector<float> vertexWeights;
		// To store the current weight
        float weight;
		// Read the weights in the current line and push the back into the array
        while (iss >> weight) {
            vertexWeights.push_back(weight);
        }
        // Ensure the vertex has attachment weights for all joints (including root)
        while (vertexWeights.size() < numJoints) {
			// Assume zero weight for missing joints
            vertexWeights.push_back(0.0f); 
        }
		// Add the current vector of weights to the attachments vector
        attachments.push_back(vertexWeights);
    }

    file.close();

}
