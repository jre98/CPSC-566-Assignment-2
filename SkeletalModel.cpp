#include "SkeletalModel.h"

#include <FL/Fl.H>

using namespace std;

const int MAX_BUFFER_SIZE = 10000;

int callStack = 0;

Matrix4f   matrix = Matrix4f::identity();

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
	loadSkeleton(skeletonFile);

	m_mesh.load(meshFile);
	m_mesh.loadAttachments(attachmentsFile, m_joints.size());

	computeBindWorldToJointTransforms();
	updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(Matrix4f cameraMatrix, bool skeletonVisible)
{
	// draw() gets called whenever a redraw is required
	// (after an update() occurs, when the camera moves, the window is resized, etc)

	m_matrixStack.clear();
	m_matrixStack.push(cameraMatrix);

	if( skeletonVisible )
	{
		//glutSolidSphere( 0.025f, 12, 12 );

		drawJoints();

		drawSkeleton();
	}
	else
	{
		// Clear out any weird matrix we may have been using for drawing the bones and revert to the camera matrix.
		glLoadMatrixf(m_matrixStack.top());

		// Tell the mesh to draw itself.
		m_mesh.draw();
	}
}



void SkeletalModel::loadSkeleton( const char* filename )
{
	ifstream in;

	char buffer[MAX_BUFFER_SIZE];

	float x, y, z;

	int parentIndex;

	int i = 1;

	// open .skel file
	in.open(filename);

	if(in.fail())
	{
		cerr << "file opening failed\n";

		exit(1);
	}

	// load each line of file into buffer
	while(in.getline(buffer, MAX_BUFFER_SIZE))
    {
        stringstream ss(buffer);

		// get x,y,z coordinates for the joints tranformation and parent index
		ss >> x >> y >> z >> parentIndex;

		Joint *joint = new Joint;

		// updated transformation matrix for joint
		joint->transform = joint->transform.translation(x,y,z);

		// if parent index is -1, then this is the root joint
		if(parentIndex == -1)
		{
			m_rootJoint = joint;
		}

		// else, add joint to its parent's list of children
		else
		{

			m_joints[parentIndex]->children.push_back(joint);
		}

		// add joint to list
		m_joints.push_back(joint);
    }

	in.close();
}





void SkeletalModel::drawJoints( )
{
	// process, but don't draw, the root joint
	int numChildren = m_rootJoint->children.size();

	// push transformation to stack
	m_matrixStack.push(m_rootJoint->transform);

	// load matrix stack top
	glLoadMatrixf(m_matrixStack.top());

	// traverse subtrees
	for(int i = 0; i < numChildren; i++)
	{
		traverseJoints(m_rootJoint->children[i]);
	}

	// pop the transformation matrix stack and then load the new top
	m_matrixStack.pop();

	glLoadMatrixf( m_matrixStack.top() );
}



void SkeletalModel::traverseJoints(Joint* joint)
{
	int numChildren = joint->children.size();

	// visit node
	// push joint's transform onto stack
	m_matrixStack.push(joint->transform);

	// pass stack top to glLoadMatrixf()
	glLoadMatrixf(m_matrixStack.top());

	// draw sphere
	glutSolidSphere( 0.025f, 12, 12 );

	// traverse subtrees
	for(int i = 0; i < numChildren; i++)
	{
		traverseJoints(joint->children[i]);
	}

	// pop joint off stack and apply new top
	m_matrixStack.pop();

	glLoadMatrixf( m_matrixStack.top() );
}


// Draw boxes between the joints. You will need to add a recursive 
// helper function to traverse the joint hierarchy.
void SkeletalModel::drawSkeleton( )
{
	// process the root joint
	Matrix4f transform = m_rootJoint->transform;
	
	m_matrixStack.push(transform);
	
	// traverse the child subtrees
	for (int i = 0; i < m_rootJoint->children.size(); i++)
	{
		traverseSkeleton(m_rootJoint->children[i]);
	}
}


// draw a box between each joint node and each of its children
void SkeletalModel::traverseSkeleton(Joint* joint)
{
	int numChildren = joint->children.size();

	// only draw starting from joints with children
	if(!joint->children.empty())
	{
		// push current transformation
		m_matrixStack.push(joint->transform);	
		
		// draw bone between each of the child joints
		for(int i = 0; i < numChildren; i++)
		{
			// rotate matrix
			// z vector to rotate around
			Vector3f rotation = Vector3f(0,0,1);
			
			// get the z normalized coordinates from the transformation matrix
			Vector3f zNormalized = joint->children[i]->transform.getCol(3).xyz().normalized();
			
			// get the x and y coordinates for the basis with cross products
			Vector3f y = Vector3f::cross(zNormalized, rotation).normalized();

			Vector3f x = Vector3f::cross(y, zNormalized).normalized();
			
			Matrix3f  basis = Matrix3f(x,y,zNormalized);
			
			Matrix4f basisCoords = Matrix4f::identity();
			
			basisCoords.setSubmatrix3x3(0,0,basis);
			
			// combine the basis with the identity matrix and push it to the stack
			m_matrixStack.push(basisCoords);
			
			// scale matrix
			// scale the matrix by .025, .025 and the z coordinates of the transform
			Matrix4f scaleCoords = Matrix4f::scaling(0.025, 0.025, joint->children[i]->transform.getCol(3).xyz().abs());
			
			m_matrixStack.push(scaleCoords);
			
			// translate matrix
			Matrix4f transCoords = Matrix4f::translation(0.0, 0.0, 0.5);
			
			m_matrixStack.push(transCoords);
			
			// finally load the matrix and draw the cube
			glLoadMatrixf(m_matrixStack.top());
			
			glutSolidCube(1.0f);
			
			// pop the 3 transformations
			m_matrixStack.pop();
		
			m_matrixStack.pop();
			
			m_matrixStack.pop();
			
			glLoadMatrixf(m_matrixStack.top());
			
			// traverse child subtrees
			traverseSkeleton(joint->children[i]);
		}
		
		m_matrixStack.pop();
	}
}



void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
	// Set the rotation part of the joint's 
	// transformation matrix based on the passed in Euler angles.
	Matrix4f rotateTransform = m_joints[jointIndex]->transform;

	rotateTransform.setSubmatrix3x3(0, 0, (Matrix4f::rotateZ(rZ) * Matrix4f::rotateY(rY) * 
	Matrix4f::rotateX(rX)).getSubmatrix3x3(0,0));

	m_joints[jointIndex]->transform = rotateTransform;

}


// Recursive helper function to compute bind pose transformations
void computeBindTransformRecursive(Joint* joint, const Matrix4f& parentTransform) {

    // Compute joint's transformation relative to its parent
    Matrix4f jointTransform = parentTransform * joint->transform;

    // Set the joint's bindWorldToJointTransform
    joint->bindWorldToJointTransform = jointTransform.inverse();
	
    // Recursively compute bind pose transformations for children
    for (Joint* child : joint->children) {
        computeBindTransformRecursive(child, jointTransform);
    }
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
	// 2.3.1. Implement this method to compute a per-joint transform from
	// world-space to joint space in the BIND POSE.
	//
	// Note that this needs to be computed only once since there is only
	// a single bind pose.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.

	 // Start recursive computation from the root joint
    computeBindTransformRecursive(m_rootJoint, Matrix4f::identity());

	
}

// Recursive helper function to update current pose transformations
void updateCurrentTransformRecursive(Joint* joint, const Matrix4f& parentTransform) 
{
    // Compute joint's transformation relative to its parent
    Matrix4f jointTransform = parentTransform * joint->transform;

    // Set the joint's currentJointToWorldTransform
    joint->currentJointToWorldTransform = jointTransform;

    // Recursively update current pose transformations for children
    for (Joint* child : joint->children) 
    {
        updateCurrentTransformRecursive(child, jointTransform);
    }
}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
	// 2.3.2. Implement this method to compute a per-joint transform from
	// joint space to world space in the CURRENT POSE.
	//
	// The current pose is defined by the rotations you've applied to the
	// joints and hence needs to be *updated* every time the joint angles change.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.

	  // Start recursive update from the root joint
    updateCurrentTransformRecursive(m_rootJoint, Matrix4f::identity());

}

void SkeletalModel::updateMesh()
{
	// 2.3.2. This is the core of SSD.
	// Implement this method to update the vertices of the mesh
	// given the current state of the skeleton.
	// You will need both the bind pose world --> joint transforms.
	// and the current joint --> world transforms.

	// Iterate through each vertex in the mesh
    for (size_t i = 0; i < m_mesh.currentVertices.size(); ++i)
    {
        // Get the bind pose vertex position
        Vector3f bindPoseVertex = m_mesh.bindVertices[i];

        // Initialize the transformed vertex to the bind pose vertex
        Vector3f transformedVertex = bindPoseVertex;

        // Apply bind pose world to joint transform
        for (size_t j = 0; j < m_mesh.attachments[i].size(); ++j)
        {
            int jointIndex = j;
            float weight = m_mesh.attachments[i][j];

            // Compute vertex position in joint space
            Vector4f vertexInJointSpace = m_joints[jointIndex]->bindWorldToJointTransform * Vector4f(bindPoseVertex, 1.0f);

            // Blend with attachment weight
            transformedVertex += weight * Vector3f(vertexInJointSpace.x(), vertexInJointSpace.y(), vertexInJointSpace.z());
        }

		// Access the joint index for this vertex from the faces member
		// Assuming the first index in faces corresponds to the joint affecting this vertex
        int jointIndex = m_mesh.faces[i][0]; 
        // Apply current joint to world transform
        transformedVertex = (m_joints[jointIndex]->currentJointToWorldTransform * Vector4f(transformedVertex, 1.0f)).xyz();

        // Update the transformed vertex in currentVertices
        m_mesh.currentVertices[i] = transformedVertex;
    }
 }

