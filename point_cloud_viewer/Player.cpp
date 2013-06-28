#include "Player.h"

const string Player::TAG_PLAYER_INDEX = "PLAYER_INDEX";
const string Player::TAG_LABEL_MATRIX = "LABEL_MATRIX";
const string Player::TAG_NORMAL_MATRIX = "NORMAL_MATRIX";

void Player::Initialize(void)
{
	m_index = 0;
	m_storage = Storage();
	m_skeleton = Skeleton();
	m_labelMatrix = Mat();
	m_normalMatrix = Mat();
	//m_segment = Segmentation();
}

bool Player::IsValid(void) const
{
	return m_storage.IsValid();
}

bool Player::Initialize(
		const Storage& storage,
		const Mat& indexFrame,
		const int index,
		const NUI_SKELETON_DATA& skeletonData,
		INuiCoordinateMapper*& pMapper)
{
	if(!storage.IsValid()) return false;
	if(indexFrame.empty()) return false;
	if(index <= 0 || index > NUI_SKELETON_COUNT) return false;
	if(skeletonData.eTrackingState == NUI_SKELETON_NOT_TRACKED) return false;

	this->m_index = index;

	m_storage.Initialize();
	m_storage.UpdateColor(storage.GetColor(), indexFrame, index);
	m_storage.UpdateDepth(storage.GetDepth(), indexFrame, index);
	m_storage.UpdatePoint(pMapper);
	m_storage.UpdateSkeleton(pMapper);

	m_skeleton.Initialize();
	m_skeleton.Initialize(index, skeletonData, pMapper);
	return true;
}

bool Player::Segment(void)
{
	if(m_labelMatrix.empty())
	{
		Segmentation::ComputeLabel(
			m_skeleton.m_pointJoints,
			m_storage.GetPoint(),
			m_labelMatrix);
		return !m_labelMatrix.empty();
	}
	else return true;
}

bool Player::Normal(void)
{
	if(m_normalMatrix.empty())
	{
		m_normalMatrix = NormalVector::ComputeNormalVector(m_storage.GetPoint());
		return !m_normalMatrix.empty();
	}
	else return true;
}

bool Player::GraphCut(void)
{
	m_labelMatrix = GraphCutter::Run(
		m_skeleton.m_pointJoints,
		m_labelMatrix,
		m_storage.GetPoint(),
		m_normalMatrix);
	return true;
}

bool Player::Transform(const Player& refPlayer, Mapper& mapper)
{
	const Skeleton& newSkeleton = GetSkeleton();
	const Skeleton& refSkeleton = refPlayer.GetSkeleton();
	const NUI_SKELETON_POSITION_TRACKING_STATE* trackingStates =
		newSkeleton.m_skeletonData.eSkeletonPositionTrackingState;
	Transformation transformation;
	transformation.ComputeTransformation(refSkeleton, newSkeleton);
	
	const Mat transformed = transformation.TransformSkeletonFrame(
		GetStorage().GetSkeleton(), GetLabel(), trackingStates);
	if(transformed.empty()) return false;
	const Mat newPoint = mapper.transformSkeletonToPoint(transformed);
	if(newPoint.empty()) return false;
	m_storage.UpdateTransformed(newPoint);
	cout << "transformed" << endl;
	return true;
}

void Player::Smoothing(INuiCoordinateMapper*& pMapper)
{
	m_storage.Smoothing(pMapper);
}

void Player::UpdateColorFrame(const Mat& colorFrame)
{
	m_storage.UpdateColor(colorFrame);
}

void Player::UpdatePointFrame(const Mat& pointFrame)
{
	m_storage.UpdatePointFrame(pointFrame);
}


bool Player::Save(const string& path) const
{
	FileStorage fs;
	bool isSaved = true;
	isSaved &= fs.open(path, FileStorage::WRITE);
	if(isSaved) fs << TAG_PLAYER_INDEX << m_index;
	if(isSaved) isSaved &= m_storage.SaveStorage(fs);
	if(isSaved) isSaved &= m_skeleton.Save(fs);
	if(isSaved && !m_labelMatrix.empty()) isSaved &= SaveMatrix(fs, TAG_LABEL_MATRIX, m_labelMatrix);
	if(isSaved && !m_normalMatrix.empty()) isSaved &= SaveMatrix(fs, TAG_NORMAL_MATRIX, m_normalMatrix);
	if(fs.isOpened()) fs.release();
	return isSaved;
}

bool Player::Load(const string& path)
{
	FileStorage fs;
	bool isLoaded = true;
	if(path.find(".yml") == string::npos) return false;
	if(path.find("player") == string::npos) return false;
	isLoaded &= fs.open(path, FileStorage::READ);
	if(isLoaded) fs[TAG_PLAYER_INDEX] >> m_index;
	if(isLoaded) isLoaded &= m_storage.LoadStorage(fs);
	if(isLoaded) isLoaded &= m_skeleton.Load(fs);
	if(isLoaded) LoadMatrix(fs, TAG_LABEL_MATRIX, m_labelMatrix);
	if(isLoaded) LoadMatrix(fs, TAG_NORMAL_MATRIX, m_normalMatrix);
	if(fs.isOpened()) fs.release();
	return isLoaded;
}