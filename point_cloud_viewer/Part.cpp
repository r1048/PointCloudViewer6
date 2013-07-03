#include "Part.h"

const int Part::parent_indices[20] = 
	{0, 0, 1, 2, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 14, 0, 16, 17, 18};

const string Part::TAG_VALID = "VALID";
const string Part::TAG_LENGTH = "LENGTH";
const string Part::TAG_INDEX = "INDEX";
const string Part::TAG_STARTJOINT = "STARTINDEX";
const string Part::TAG_ENDJOINT = "ENDINDEX";
const string Part::TAG_ROTATION = "ROTATION";


Part::Part(void)
{
	Initialize();
}

Part::~Part(void)
{
}

void Part::Initialize(void)
{
	m_valid = false;
	m_index = -1;
	m_length = 0.0f;
	m_startJoint = Vec3f(0, 0, 0);
	m_endJoint = Vec3f(0, 0, 0);
	m_rotation = Mat::eye(3, 3, CV_32FC1);
}

// initialize part with joint and matrix information
void Part::Initialize(
	const vector<bool>& trackingStates,
	const vector<Vec3f>& skeletonJoints,
	const vector<Mat>& absoluteMatrices,
	const int partIndex)
{
	// initialize with default values
	Initialize();

	// set joints and valid flag for rest of procedures
	if(partIndex < 0 || partIndex > 18) return ;
	m_index = partIndex;
	const int endIndex = partIndex + 1;
	const int startIndex = parent_indices[endIndex];
	m_startJoint = skeletonJoints[startIndex];
	m_endJoint = skeletonJoints[endIndex];
	const Vec3f delta = m_endJoint - m_startJoint;
	m_length = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
	m_valid = trackingStates[startIndex] == NUI_SKELETON_POSITION_TRACKED &&
		trackingStates[endIndex] == NUI_SKELETON_POSITION_TRACKED;

	// compute rotation if valid
	if(!m_valid) return;
	m_rotation = absoluteMatrices[endIndex];
}

// Compute a 4-by-4 transformation matrix given reference part
Mat Part::GetTransform(const Part& ref) const
{
	// handle exceptions: invalid case, equal case
	if(this->IsValid() == false || ref.IsValid() == false) return Mat();
	if(this == &ref) return Mat::eye(4, 4, CV_32FC1);

	// declare transformation variables
	Mat transformation1 = Mat::eye(4, 4, CV_32FC1);
	Mat transformation2 = Mat::eye(4, 4, CV_32FC1);

	// prepare R, t for transformation1 from current part [R -Rt; 0 0 0 1];
	Mat translation1 = -StorageHandler::Multiply_Mat_from_Mat3x3_by_Vec3f(m_rotation, m_startJoint);
	Mat rotation1 = m_rotation;

	// prepare R, t for transformation2 from reference part [inv(R) t; 0 0 0 1];
	Mat translation2 = Mat::zeros(3, 1, CV_32FC1);
	for(int rr = 0; rr < 3; rr++)
		translation2.at<float>(rr, 0) = ref.GetStartJoint()[rr];
	Mat rotation2 = ref.GetRotation().inv();
	
	// set transformations
	for(int rr = 0; rr < 3; rr++)
	{
		for(int cc = 0; cc < 3; cc++) {
			transformation1.at<float>(rr, cc) = rotation1.at<float>(rr, cc);
			transformation2.at<float>(rr, cc) = rotation2.at<float>(rr, cc);
		}
		transformation1.at<float>(rr, 3) = translation1.at<float>(rr, 0);
		transformation2.at<float>(rr, 3) = translation2.at<float>(rr, 0);
	}

	// return transformation
	return transformation2 * transformation1;
}

bool Part::Save(FileStorage& fs) const
{
	bool valid = true;
	valid &= StorageHandler::Save(fs, m_valid, TAG_VALID);
	valid &= StorageHandler::Save(fs, m_length, TAG_LENGTH);
	valid &= StorageHandler::Save(fs, m_index, TAG_INDEX);
	valid &= StorageHandler::Save(fs, m_startJoint, TAG_STARTJOINT);
	valid &= StorageHandler::Save(fs, m_endJoint, TAG_ENDJOINT);
	valid &= StorageHandler::Save(fs, m_rotation, TAG_ROTATION);
	return valid;
}

bool Part::Load(FileStorage& fs)
{
	bool valid = true;
	valid &= StorageHandler::Load(fs, m_valid, TAG_VALID);
	valid &= StorageHandler::Load(fs, m_length, TAG_LENGTH);
	valid &= StorageHandler::Load(fs, m_index, TAG_INDEX);
	valid &= StorageHandler::Load(fs, m_startJoint, TAG_STARTJOINT);
	valid &= StorageHandler::Load(fs, m_endJoint, TAG_ENDJOINT);
	valid &= StorageHandler::Load(fs, m_rotation, TAG_ROTATION);
	return valid;
}

bool Part::operator==(const Part& part)
{
	bool equalRotation = true;
	for(int rr = 0; rr < 3 && equalRotation; rr++)
	{
		for(int cc = 0; cc < 3 && equalRotation; cc++)
		{
			if(m_rotation.at<float>(rr, cc) != part.m_rotation.at<float>(rr, cc))
				equalRotation = false;
		}
	}

	return equalRotation &&
		m_valid == part.m_valid &&
		m_startJoint == part.m_startJoint &&
		m_endJoint == part.m_endJoint;
}