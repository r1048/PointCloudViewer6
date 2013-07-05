#include "Skeleton.h"

const int Skeleton::parent_indices[N_JOINT] =
	{0, 0, 1, 2, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 14, 0, 16, 17, 18};

const string Skeleton::TAG_SKELETON_STATE = "SKELETON_STATE";
const string Skeleton::TAG_SKELETON_JOINTS = "SKELETON_JOINTS";
const string Skeleton::TAG_PART_STATES = "PART_STATES";
const string Skeleton::TAG_PART_ROTATIONS = "PART_ROTATIONS";

void Skeleton::Initialize(void)
{
	m_trackingState = 0;
	m_jointList.clear();
	m_partList.clear();
}

bool Skeleton::IsValid(void) const
{
	return m_trackingState == NUI_SKELETON_TRACKED &&
		m_partList.size() == N_PART &&
		m_jointList.size() == N_JOINT;
}

void Skeleton::Initialize(
	const int index,
	const NUI_SKELETON_DATA& skeletonData)
{
	Initialize();
	this->SetJoints(skeletonData);
	this->SetParts(skeletonData);
}

const Part& Skeleton::GetPart(int index) const
{
	if(IsValid() == false) return Part();
	if(index < 0 || index >= N_PART) return Part();
	else return m_partList[index];
}

const Vec3f& Skeleton::GetJoint(int index) const
{
	if(IsValid() == false) return Vec3f(0, 0, 0);
	if(index < 0 || index >= N_JOINT) return Vec3f(0, 0, 0);
	else return m_jointList[index];
}

const vector<Vec3f>& Skeleton::GetJointList(void) const
{
	if(IsValid() == false) return vector<Vec3f>();
	return m_jointList;
}

void Skeleton::SetParts(const NUI_SKELETON_DATA& data)
{
	// set valid flag
	m_trackingState = data.eTrackingState;

	// set flags, joints
	vector<int> trackingStates;
	vector<Vec3f> skeletonJoints;
	for(int ii = 0; ii < N_JOINT; ii++)
	{
		trackingStates.push_back(data.eSkeletonPositionTrackingState[ii]);
		skeletonJoints.push_back(StorageHandler::Vector4_to_Vec3f(data.SkeletonPositions[ii]));
	}

	// set rotation matrices
	vector<Mat> absoluteMatrices;
	NUI_SKELETON_BONE_ORIENTATION boneOrientations[N_JOINT];
	NuiSkeletonCalculateBoneOrientations(&data, boneOrientations);
	absoluteMatrices.resize(N_JOINT);
	for(int ii = 0; ii < N_JOINT; ii++)
	{
		const NUI_SKELETON_BONE_ORIENTATION& orientation = boneOrientations[ii];
		const int index = orientation.endJoint;
		absoluteMatrices[index] = StorageHandler::Matrix4_to_Mat3x3(orientation.absoluteRotation.rotationMatrix);
	}

	// set parts
	m_partList.clear();
	for(int ii = 0; ii < N_PART; ii++)
	{
		Part part;
		part.Initialize(trackingStates, skeletonJoints, absoluteMatrices, ii);
		m_partList.push_back(part);
	}
}

void Skeleton::SetJoints(const NUI_SKELETON_DATA& data)
{
	this->m_jointList.clear();
	for(int ii = 0; ii < N_JOINT; ii++)
		m_jointList.push_back(StorageHandler::Vector4_to_Vec3f(data.SkeletonPositions[ii]));
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


// Save skeleton data with manipulation
// Save joints, valid flags and rotation matrices
bool Skeleton::Save(FileStorage& fs) const
{
	if(!fs.isOpened() || !IsValid()) return false;
	try
	{
		fs << TAG_SKELETON_STATE << m_trackingState;

		vector<float> jointList;
		vector<Mat> rotationList;
		vector<int> stateList;
		for(int ii = 0; ii < N_PART; ii++)
		{
			const Part& part = m_partList[ii];
			stateList.push_back(part.GetTrackingState());
			rotationList.push_back(part.GetRotation());
		}
		for(int ii = 0; ii < N_JOINT; ii++)
			for(int jj = 0; jj < 3; jj++)
				jointList.push_back(m_jointList[ii][jj]);

		fs << TAG_SKELETON_JOINTS << jointList;
		fs << TAG_PART_STATES << stateList;
		fs << TAG_PART_ROTATIONS << rotationList;
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::Load(FileStorage& fs)
{
	Initialize();

	if(fs.isOpened() == false) return false;
	try
	{
		fs[TAG_SKELETON_STATE] >> m_trackingState;

		vector<float> joints;
		vector<int> stateList;
		vector<Mat> rotationList;
		fs[TAG_SKELETON_JOINTS] >> joints;
		fs[TAG_PART_STATES] >> stateList;
		fs[TAG_PART_ROTATIONS] >> rotationList;
		
		for(int ii = 0; ii < N_JOINT; ii++)
		{
			Vec3f joint;
			for(int jj = 0; jj < 3; jj++)
				joint[jj] = joints[3 * ii + jj];
			m_jointList.push_back(joint);
		}
		for(int ii = 0; ii < N_PART; ii++)
		{
			Part part;
			part.Initialize(stateList, joints, rotationList, ii);
			m_partList.push_back(part);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}