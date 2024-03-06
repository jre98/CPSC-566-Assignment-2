#include "SkeletalModel.h"

#include <FL/Fl.H>

using namespace std;

const int MAX_BUFFER_SIZE = 10000;

int callStack = 0;

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
		glutSolidSphere( 0.025f, 12, 12 );

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

		Joint *joint = new Joint;

		// get x,y,z coordinates for the joints tranformation and parent index
		ss >> x >> y >> z >> parentIndex;

		//cout << "line " << i << ": " << x << ", " << y << ", " << z << endl;

		// updated transformation matrix for joint
		joint->transform = joint->transform.translation(x,y,z);

		//cout << "loaded transformation:\n";

		//joint->transform.print();

		//cout << endl;

		//cout << "parent index: " << parentIndex << endl;

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
	// Draw a sphere at each joint. You will need to add a recursive helper function to traverse the joint hierarchy.
	//
	// We recommend using glutSolidSphere( 0.025f, 12, 12 )
	// to draw a sphere of reasonable size.
	//
	// You are *not* permitted to use the OpenGL matrix stack commands
	// (glPushMatrix, glPopMatrix, glMultMatrix).
	// You should use your MatrixStack class
	// and use glLoadMatrix() before your drawing call.

	traverseHiearchy(m_rootJoint);

	cout << "finished traversal\n";

}



void SkeletalModel::traverseHiearchy(Joint* joint)
{
	// visit node
	// push joint's transform onto stack
	m_matrixStack.push(joint->transform);

	//cout << "pushed transformation: \n";

	//joint->transform.print();

	// pass stack top to glLoadMatrixf()
	glLoadMatrixf(m_matrixStack.top());

	// draw sphere
	glutSolidSphere( 0.025f, 12, 12 );

	// base case, no children
	if(joint->children.empty())
	{
		//cout <<"returning\n";
		return;
	}

	// traverse subtrees
	for(int i = 0; i < joint->children.size() - 1; i++)
	{
		//cout << "child " << i+1 << endl;
		
		traverseHiearchy(joint->children[i]);
	}

	// visit last child
	

	// pop joint off stack
	m_matrixStack.pop();
}



void SkeletalModel::drawSkeleton( )
{
	// Draw boxes between the joints. You will need to add a recursive helper function to traverse the joint hierarchy.
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
	// Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
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
}

void SkeletalModel::updateMesh()
{
	// 2.3.2. This is the core of SSD.
	// Implement this method to update the vertices of the mesh
	// given the current state of the skeleton.
	// You will need both the bind pose world --> joint transforms.
	// and the current joint --> world transforms.
}

