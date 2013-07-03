#include "Skeleton.h"

const int Skeleton::parent_indices[N_JOINT] =
	{0, 0, 1, 2, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 14, 0, 16, 17, 18};

const string Skeleton::TAG_PARTS = "PARTS";

void Skeleton::Initialize(void)
{
	m_valid = false;
	m_index = -1;
	m_joints.clear();
	m_parts.clear();
}

bool Skeleton::IsValid(void) const
{
	return m_valid &&
		m_parts.size() == N_PART &&
		m_joints.size() == N_JOINT;
}

void Skeleton::Initialize(
	const int index,
	const NUI_SKELETON_DATA& skeletonData)
{
	Initialize();
	m_index = index;
	this->SetParts(skeletonData);
	this->SetJoints(skeletonData);
}

const Part& Skeleton::GetPart(int index) const
{
	if(IsValid() == false) return Part();
	if(index < 0 || index >= N_PART) return Part();
	else return m_parts[index];
}

const Vec3f& Skeleton::GetJoint(int index) const
{
	if(IsValid() == false) return Vec3f(0, 0, 0);
	if(index < 0 || index >= N_JOINT) return Vec3f(0, 0, 0);
	else return m_joints[index];
}

const vector<Vec3f>& Skeleton::GetJointList(void) const
{
	if(IsValid() == false) return vector<Vec3f>();
	return m_joints;
}

void Skeleton::SetParts(const NUI_SKELETON_DATA& data)
{
	// set valid flag
	m_valid = data.eTrackingState == NUI_SKELETON_TRACKED;

	// set flags, joints
	vector<bool> trackingStates;
	vector<Vec3f> skeletonJoints;
	for(int ii = 0; ii < N_JOINT; ii++)
	{
		trackingStates.push_back(data.eSkeletonPositionTrackingState[ii] == NUI_SKELETON_POSITION_TRACKED);
		skeletonJoints.push_back(StorageHandler::Vector4_to_Vec3f(data.SkeletonPositions[ii]));
	}

	// set rotation matrices
	vector<Mat> absoluteMatrices;
	NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
	NuiSkeletonCalculateBoneOrientations(&data, boneOrientations);
	absoluteMatrices.resize(N_JOINT);
	for(int ii = 0; ii < N_JOINT; ii++)
	{
		const NUI_SKELETON_BONE_ORIENTATION& orientation = boneOrientations[ii];
		const int index = orientation.endJoint;
		absoluteMatrices[index] = StorageHandler::Matrix4_to_Mat3x3(orientation.absoluteRotation.rotationMatrix);
	}

	// set parts
	m_parts.clear();
	for(int ii = 0; ii < N_PART; ii++)
	{
		Part part;
		part.Initialize(trackingStates, skeletonJoints, absoluteMatrices, ii);
		m_parts.push_back(part);
	}
}

void Skeleton::SetJoints(const NUI_SKELETON_DATA& data)
{
	this->m_joints.clear();
	for(int ii = 0; ii < N_JOINT; ii++)
		m_joints.push_back(StorageHandler::Vector4_to_Vec3f(data.SkeletonPositions[ii]));
}

Vec3f Skeleton::ConvertSkeletonToPoint(const Vector4& point4, INuiCoordinateMapper*& pMapper)
{
	Vec3f fpoint;
	NUI_DEPTH_IMAGE_POINT dpoint;
	NUI_COLOR_IMAGE_POINT cpoint;
	USHORT dd;

	// convert skeleton to depth
	NuiTransformSkeletonToDepthImage(
		point4,
		&dpoint.x, &dpoint.y, &dd,
		m_depthResolution);
	dd = (dd & 0xfff8) >> 3;
	dpoint.depth = static_cast<LONG>(dd);

	// convert depth to color
	pMapper->MapDepthPointToColorPoint(
		m_depthResolution,
		&dpoint,
		NUI_IMAGE_TYPE_COLOR,
		m_depthResolution,
		&cpoint);
	dpoint.x = cpoint.x;
	dpoint.y = cpoint.y;

	// convert color to point
	fpoint[0] = -(cpoint.x - CX) * dpoint.depth / FX;
	fpoint[1] = -(cpoint.y - CY) * dpoint.depth / FY;
	fpoint[2] = dpoint.depth;

	return fpoint;
}



bool Skeleton::Save(FileStorage& fs) const
{
	if(!fs.isOpened() || m_parts.size() == 0) return false;
	bool valid = true;
	try
	{
		char buffer[MAX_PATH];
		for(int ii = 0; ii < m_parts.size() && valid; ii++)
		{
			sprintf(buffer, "%02d", ii);
			string _tag = TAG_PARTS + buffer;
			valid &= m_parts[ii].Save(fs);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return valid;
}

bool Skeleton::Load(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	bool valid = true;
	try
	{
		m_parts.clear();
		m_parts.resize(N_PART);
		char buffer[MAX_PATH];
		for(int ii = 0; ii < N_PART && valid; ii++)
		{
			sprintf(buffer, "%02d", ii);
			string _tag = TAG_PARTS + buffer;
			valid &= m_parts[ii].Load(fs);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return valid;
}