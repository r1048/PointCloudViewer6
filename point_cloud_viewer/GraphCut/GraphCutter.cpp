#include "GraphCutter.h"

//const float GraphCutter::MAX_COST = 1.0e+10F;
//const float GraphCutter::LABEL_COST_SAME = 1.00f;
//const float GraphCutter::LABEL_COST_DIFF = 1.05f;
//const float GraphCutter::SCALE_DATA_COST = 10.0f;
//const float GraphCutter::SCALE_SMOOTH_COST = 10.0f;
const float GraphCutter::SIGMA = 7.0f;
const float GraphCutter::SIGMA2 = 2.0f;


GraphCutter::GraphCutter(void)
{
}

GraphCutter::~GraphCutter(void)
{
}

Mat GraphCutter::Run(
	const Skeleton& skeleton,
	const Mat& labelMatrix,
	const Mat& pointMatrix,
	const Mat& normalMatrix)
{
	Mat m_labelMatrix;

	// assertion
	if(skeleton.IsValid() == false) return Mat();
	if(pointMatrix.rows != depthHeight || pointMatrix.cols != depthWidth) return Mat();
	if(pointMatrix.empty()) return Mat();
	if(normalMatrix.empty()) return Mat();

	// count valid points
	int count = 0;
	vector<Vec3f> pointList;
	vector<Vec3f> normList;
	vector< pair<int, int> > indexList;
	Mat indexMap = Mat::ones(depthHeight, depthWidth, CV_32SC1) * -1;
	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			const Vec3f& point = pointMatrix.at<Vec3f>(rr, cc);
			const Vec3f& norm = normalMatrix.at<Vec3f>(rr, cc);
			if(point == Vec3f(0.0f, 0.0f, 0.0f)) continue;
			else
			{
				pointList.push_back(point);
				normList.push_back(norm);
				indexList.push_back(pair<int, int>(rr, cc));
				indexMap.at<int>(rr, cc) = count++;
			}
		}
	}

	// compute normSim and distances
	Mat dists, normSimilarity;
	Segmentation::ComputeDistanceAndNorm(
		skeleton,
		pointList,
		normList,
		dists,
		normSimilarity);

	// set label wrt distance and norm
	vector<int> labelList;
	const float maxRatio = 0.4f;
	for(int ii = 0; ii < count; ii++)
	{
		int index1 = -1;
		int index2 = -1;
		float minDist1 = FLT_MAX;
		float minDist2 = FLT_MAX;
		for(int jj = 0; jj < N_LABEL; jj++)
		{
			const float dist = dists.at<float>(ii, jj);
			if(dist < minDist1)
			{
				minDist2 = minDist1;
				minDist1 = dist;
				index2 = index1;
				index1 = jj;
			}
		}

		const float ratio = minDist1 / minDist2;
		if(ratio < maxRatio) labelList.push_back(index1);
		else
		{
			if(index1 < 0 || index2 < 0) cout << "invalid index" << endl;
			const float normSim1 = normSimilarity.at<float>(ii, index1);
			const float normSim2 = normSimilarity.at<float>(ii, index2);
			const float ratio1 = minDist1 / normSim1;
			const float ratio2 = minDist2 / normSim2;
			labelList.push_back(ratio1 < ratio2 ? index1 : index2);
			//labelList.push_back(normSim1 > normSim2 ? index1 : index2);
		}
	}


	/////////////////////////////////////////////////////////////
	// MRF CODE
	/////////////////////////////////////////////////////////////

	// allocate
	int* m_dataCost = new int[count * N_LABEL];
	int* m_smoothCost = new int[N_LABEL * N_LABEL];
	memset(m_dataCost, 0, sizeof(int) * count * N_LABEL);
	memset(m_smoothCost, 0, sizeof(int) * N_LABEL * N_LABEL);

	// set data cost
	for(int ii = 0; ii < count * N_LABEL; ii++)
		m_dataCost[ii] = MAX_COST;

	int index = 0;
	int minCost = INT_MAX;
	int maxCost = 0;
	for(int ii = 0; ii < count; ii++)
	{
		for(int jj = 0; jj < N_LABEL; jj++, index++)
		{
			const float dist = dists.at<float>(ii, jj);
			const float sim = normSimilarity.at<float>(ii, jj);
			const float value1 = (1.0f / (1.0f + std::expf(-dist / SIGMA / SIGMA)) - 0.5f) * 2.0f;
			const float value2 = 1.0f / (1.0f + expf(sim / SIGMA2 / SIGMA2));
			//const float value2 = expf(-sim / SIGMA2 / SIGMA2);
			const float value = value1 * value2 * SCALE_DATA_COST;
			//const float value = SCALE_DATA_COST * dist;
			const int ivalue = static_cast<int>(value + 0.5f);

			if(ivalue > maxCost) maxCost = ivalue;
			if(ivalue < minCost) minCost = ivalue;
			if(m_dataCost[index] > ivalue)
				m_dataCost[index] = ivalue;
		}
	}

	cout << "dataMin: " << minCost << endl;
	cout << "dataMax: " << maxCost << endl;

	// set label cost for smoothness
	index = 0;
	for(int ii = 0; ii < N_LABEL; ii++)
	{
		for(int jj = 0; jj < N_LABEL; jj++, index++)
		{
			if(ii == jj)
				m_smoothCost[index] = LABEL_COST_SAME;
			else
				m_smoothCost[index] = LABEL_COST_DIFF;
		}
	}

	// set graph
	try
	{
		GCoptimizationGeneralGraph* m_graph = new GCoptimizationGeneralGraph(count, N_LABEL);
		m_graph->setDataCost(m_dataCost);
		m_graph->setSmoothCost(m_smoothCost);

		// set initial label
		for(int ii = 0; ii < count; ii++)
			m_graph->setLabel(ii, labelList[ii]);

		minCost = INT_MAX;
		maxCost = 0;

		// set a grid neighborhood
		for(int rr = 0; rr < depthHeight; rr++)
		{
			for(int cc = 1; cc < depthWidth; cc++)
			{
				int indexLeft = indexMap.at<int>(rr, cc - 1);
				int indexRight = indexMap.at<int>(rr, cc);
				if(indexLeft < 0 || indexRight < 0) continue;
				const pair<int, int>& ipl = indexList[indexLeft];
				const pair<int, int>& ipr = indexList[indexRight];
				const Vec3f& normLeft = normalMatrix.at<Vec3f>(ipl.first, ipl.second);
				const Vec3f& normRight = normalMatrix.at<Vec3f>(ipr.first, ipr.second);
				const float value = 1.0f - Segmentation::InnerProduct(normLeft, normRight);
				const int ivalue = static_cast<int>(value * SCALE_SMOOTH_COST + 0.5f);
				if(ivalue < minCost) minCost = ivalue;
				if(ivalue > maxCost) maxCost = ivalue;
				m_graph->setNeighbors(indexLeft, indexRight, ivalue);
			}
		}

		for(int rr = 1; rr < depthHeight; rr++)
		{
			for(int cc = 0; cc < depthWidth; cc++)
			{
				int indexTop = indexMap.at<int>(rr - 1, cc);
				int indexBottom = indexMap.at<int>(rr, cc);
				if(indexTop < 0 || indexBottom < 0) continue;
				const pair<int, int>& ipt = indexList[indexTop];
				const pair<int, int>& ipb = indexList[indexBottom];
				const Vec3f& normTop = normalMatrix.at<Vec3f>(ipt.first, ipt.second);
				const Vec3f& normBottom = normalMatrix.at<Vec3f>(ipb.first, ipb.second);
				const float value = 1.0f - Segmentation::InnerProduct(normTop, normBottom);
				const int ivalue = static_cast<int>(value * SCALE_SMOOTH_COST + 0.5f);
				if(ivalue < minCost) minCost = ivalue;
				if(ivalue > maxCost) maxCost = ivalue;
				m_graph->setNeighbors(indexTop, indexBottom, ivalue);
			}
		}

		cout << "smoothMin: " << minCost << endl;
		cout << "smoothMax: " << maxCost << endl;

		// compute energy
		cout << "E0: " << m_graph->compute_energy() << endl;
		m_graph->expansion(N_ITERATION);
		cout << "E1: " << m_graph->compute_energy() << endl;

		// update label matrix
		m_labelMatrix = Mat::zeros(depthHeight, depthWidth, CV_32SC1);

		for(int rr = 0; rr < depthHeight; rr++)
		{
			for(int cc = 0; cc < depthWidth; cc++)
			{
				const int index = indexMap.at<int>(rr, cc);
				if(index < 0) continue;
				m_labelMatrix.at<int>(rr, cc) = m_graph->whatLabel(index);
			}
		}
		
		// deallocate
		delete m_graph;
	}
	catch(GCException e)
	{
		e.Report();
	}

	delete [] m_dataCost;
	delete [] m_smoothCost;

	return m_labelMatrix;
}