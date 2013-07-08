#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "GCoptimization.h"
#include "../Common.h"
#include "../Segmentation.h"

class GraphCutter
{
public:
	GraphCutter(void);
	GraphCutter(
		const Skeleton& skeleton,
		const Mat& labelMatrix,
		const Mat& pointMatrix,
		const Mat& normalMatrix);
	~GraphCutter(void);

protected:
	static const int N_LABEL = NUI_SKELETON_POSITION_COUNT - 1;
	static const int N_QUERY = 3;
	static const int MAX_COST = 10000000;
	static const int LABEL_COST_SAME = 2;
	static const int LABEL_COST_DIFF = 3;
	static const int SCALE_DATA_COST = 100;
	static const int SCALE_SMOOTH_COST = 2000;
	static const float SIGMA;
	static const float SIGMA2;
	static const int N_ITERATION = 50;



public:
	static Mat Run(
		const Skeleton& skeleton,
		const Mat& labelMatrix,
		const Mat& pointMatrix,
		const Mat& normalMatrix);
};