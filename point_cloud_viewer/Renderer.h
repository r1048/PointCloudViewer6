#pragma once

#include <math.h>
#include <iostream>
#include <fstream>

#include <opencv2\opencv.hpp>	// OpenCV header file
#include <Windows.h>			// Kinect SDK header files
#include <NuiApi.h>				// Kinect SDK header files
#include <gl\freeglut.h>		// OpenGL header files
#include <opencv2\ml\ml.hpp>	// Use KNN

#include "Grabber.h"			// grabber header file
#include "GraphCut\GraphCutter.h"s
#include "Transformation.h"

using namespace cv;
using namespace std;


// variables for GUI
const float TRACKBALLSIZE = 0.8f;
const int RENORMCOUNT = 97;
const int width = 640;
const int height = 480;

GLint drag_state = 0;
GLint button_state = 0;

GLint rot_x = 0;
GLint rot_y = 0;
GLint trans_x = 0;
GLint trans_y = 0;
GLint trans_z = 0;

float quat[4] = {0};
float t[3] = {0};

int window = 0;
GLuint all_texture = 0;
const float scale_factor = 0.01f;
char path[MAX_PATH];

// Use the RGB texture or just draw it as color
bool frame_texture_mode		= true;
bool player_texture_mode	= true;
bool near_mode			= false;
bool screen_mode		= true;
bool online_mode		= true;
bool clipping_mode		= false;
bool skeleton_mode		= true;
bool frame_mode			= true;
bool player_index_mode	= true;
bool is_paused			= false;
bool smoothing_mode		= true;
bool segmentation_mode	= false;
bool normal_mode		= false;
bool graphcut_mode		= false;
bool transformation_mode = true;
bool auto_mode = true;

const float pointSizeColor = 1.5f;
const float pointSizePoint = 1.0f;

Mat texcoords = Mat::zeros(height, width, CV_32FC2);
unsigned int* indices = NULL;

Grabber grabber = Grabber();
vector<Frame> frames;
HANDLE hMutex;

// initialize GUI with arguments
void Initialize(int argc, char* argv[]);
void Deallocate();

const Vec3f SKELETON_COLOR_LIST[NUI_SKELETON_COUNT] = 
{
	Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, 1.0f, 0.0f), Vec3f(1.0f, 0.0f, 0.0f), 
	Vec3f(0.0f, 1.0f, 1.0f), Vec3f(1.0f, 0.0f, 1.0f), Vec3f(1.0f, 1.0f, 0.0f)
};

const Vec3b SKELETON_POSITION_COLOR_LIST[NUI_SKELETON_POSITION_COUNT - 1] =
{
	Vec3b(255, 0, 0),
	Vec3b(0, 255, 0),
	Vec3b(0, 0, 255),
	Vec3b(0, 255, 255),
	Vec3b(255, 0, 255),
	Vec3b(255, 255, 0),
	Vec3b(0, 127, 127),
	Vec3b(127, 0, 127),
	Vec3b(127, 127, 0),
	Vec3b(127, 0, 0),
	Vec3b(0, 127, 0),
	Vec3b(0, 0, 127),
	Vec3b(255, 127, 127),
	Vec3b(127, 255, 127),
	Vec3b(127, 127, 255),
	Vec3b(127, 255, 255),
	Vec3b(255, 127, 255),
	Vec3b(255, 255, 127),
	Vec3b(127, 127, 127)
};

// display functions
void draw_frame(const Frame& frame, const GLuint& bindIndex, const bool texture_mode = true);
void draw_player(const Frame& frame, const Player& player, const GLuint& bindIndex, const bool texture_mode = true);
void draw_skeleton(const Skeleton& skeleton);
void draw_segmentation(const Player& player, const GLuint& bindIndex);
void draw_normal(const Player& player);

// high-level functions for GUI
void draw_center();
void display();
void close();
void special(int, int, int);
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void reshape(int, int);
void motion(int, int);

void auto_save();
void save();
void prepare_data();
void segmentation();
void graph_cut();
void compute_normal();
void transform();

void InitGL(int, int, const string);
void allocate_texture();
void assign_texture(const Mat& point, const Mat& coordinate, const Mat& color, const GLuint& bindIndex);
void update_texture();

void draw_triangle(const Vec3f pos, const float dl);
Mat assign_label(const int idx = 1);

// basic functions for computation/GUI
// trackball codes were imported from those of Gavin Bell
// which appeared in SIGGRAPH '88
void vzero(float*);
void vset(float*, float, float, float);
void vsub(const float*, const float*, float*);
void vcopy(const float*, float*);
void vcross(const float *v1, const float *v2, float *cross);
float vlength(const float *v);
void vscale(float *v, float div);
void vnormal(float *v);
float vdot(const float *v1, const float *v2);
void vadd(const float *src1, const float *src2, float *dst);

void trackball(float q[4], float, float, float, float);
void add_quats(float*, float*, float*);
void axis_to_quat(float a[3], float phi, float q[4]);
void normalize_quat(float q[4]);
float tb_project_to_sphere(float, float, float);
void build_rotmatrix(float m[4][4], float q[4]);



/*
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */


/*
 * Trackball code:
 *
 * Implementation of a virtual trackball.
 * Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 *   the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * Vector manip code:
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Much mucking with by:
 * Gavin Bell
 */