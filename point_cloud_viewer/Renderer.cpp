#include "Renderer.h"

void allocate_texture()
{
	glGenTextures(1, &all_texture);
	glBindTexture(GL_TEXTURE_2D, all_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void assign_texture(const Mat& point, const Mat& coordinate, const Mat& color, const GLuint& bindIndex)
{
	memcpy_s(texcoords.data, sizeof(float) * width * height * 2,
		coordinate.data, sizeof(float) * width * height * 2);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, point.data);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords.data);
	glBindTexture(GL_TEXTURE_2D, bindIndex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, color.ptr<unsigned char>(0));
	glPointSize(pointSizeColor);
	glDrawElements(GL_POINTS, width * height, GL_UNSIGNED_INT, indices);
	glDisable(GL_TEXTURE_2D);
}

void InitGL(int width, int height, const string path)
{
	// initialize
	frames.clear();

	// apply operations checked
	if(online_mode == false)
	{
		destroyAllWindows();

		// get file information using list
		FileLoader fileList(path, "*.*");
		FileLoader colorList(path, "png", 1, CV_8UC3);
		
		// collect timestamps
		vector<string> timestamps;
		for(int ii = 0; ii < colorList.size(); ii++) {
			string colorName = colorList.at(ii);
			int strlen = colorName.length();
			int suflen = Frame::SUFFIX_COLOR.length();
			string newstr = colorName.substr(strlen - suflen - 15, 15);
			timestamps.push_back(newstr);
		}

		// load storages
		for(int ii = 0; ii < colorList.size(); ii++)
		{
			Frame frame;
			frame.Load(path, timestamps[ii], fileList.filelist);
			frames.push_back(frame);
		}

		if(segmentation_mode || graphcut_mode) normal_mode = true;
		if(normal_mode) compute_normal();
		if(segmentation_mode) segmentation();
		if(graphcut_mode) graph_cut();
		if(transformation_mode) transform();
	}
	else frames.resize(1);
	
	// bind textures
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glEnable( GL_DEPTH_TEST );
	reshape( width, height );
}

void Initialize(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH );
	glutInitWindowSize( width, height );
	glutInitWindowPosition( 0, 0 );
    
	window = glutCreateWindow("point");

	trackball( quat, 90.0, 0.0, 0.0, 0.0 );

	glutDisplayFunc( display );
	glutIdleFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( special );
	glutMotionFunc( motion );
	glutMouseFunc( mouse );
	glutCloseFunc( close );

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// initialize _* variables
	indices = new unsigned int[width * height];
	for(int ii = 0; ii < width * height; ii++) indices[ii] = ii;

	// set directory and path
	sprintf(path, "../../data/%s/", JunhaLibrary::GetDate());
	JunhaLibrary::FileLoader::mkdir(path, false);
	InitGL(width, height, path);
	allocate_texture();
	hMutex = CreateMutex(NULL, TRUE, NULL);
}

void Deallocate()
{
	CloseHandle(hMutex);
	delete [] indices;
	glDeleteTextures(1, &all_texture);
}

void auto_save(void)
{
	if(!online_mode) return ;

	static string refTime = JunhaLibrary::GetTimeStamp();
	string currTime = JunhaLibrary::GetTimeStamp();
	const int iRefTime = atoi(refTime.substr(9, 15).c_str());
	const int iCurrTime = atoi(currTime.substr(9, 15).c_str());
	const int timeInterval = 10;

	if(iCurrTime - iRefTime >= timeInterval)
	{
		refTime = currTime;
		online_mode = false;
		graphcut_mode = true;
		graph_cut();
		save();
		online_mode = true;
	}
}

void save(void)
{
	if(online_mode)
	{
		// set the mutex for preserve current data
		WaitForSingleObject(hMutex, INFINITE);
		grabber.Save(path);
		ReleaseMutex(hMutex);
	}
	else
	{
		for(int ii = 0; ii < frames.size(); ii++)
			frames[ii].Save(path);
	}
	cout << "files are saved" << endl;
}

void draw_center(void)
{
	glBegin( GL_LINES );
	glColor3f( 1.0f, 0.0f, 0.0f ); /* R */
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( 0.2f, 0.0f, 0.0f );
	glEnd();
	glRasterPos3f( 0.2f, 0.0f, 0.0f );
	glutBitmapCharacter( GLUT_BITMAP_9_BY_15, 'x' );
  
	glBegin( GL_LINES );
	glColor3f( 0.0f, 1.0f, 0.0f ); /* G */
		glVertex3f( 0.0f,  0.2f, 0.0f );
		glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();
	glRasterPos3f( 0.0f, 0.2f, 0.0f );
	glutBitmapCharacter( GLUT_BITMAP_9_BY_15, 'y' );

	glBegin(GL_LINES);
	glColor3f( 0.0f, 0.0f, 1.0f ); /* B */
		glVertex3f( 0.0f, 0.0f, -0.2f );
		glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();  
	glRasterPos3f( 0.0f, 0.0f, -0.2f );
	glutBitmapCharacter( GLUT_BITMAP_9_BY_15, 'z' );
}


void draw_triangle(const Vec3f pos, const float dl)
{
	float xx = pos[0];
	float yy = pos[1];
	float zz = pos[2];

	glBegin(GL_TRIANGLES);
	glVertex3f(xx + 00, yy + dl, zz);
	glVertex3f(xx - dl, yy - dl, zz);
	glVertex3f(xx + dl, yy - dl, zz);
	glEnd();
}


void prepare_data()
{
	if(online_mode == false) return ;

	// retrieve data
	WaitForSingleObject(hMutex, INFINITE);
	grabber.Update(0, smoothing_mode);
	ReleaseMutex(hMutex);

	// copy data
	frames.clear();
	frames.push_back(grabber.GetFrame());
}

void segmentation()
{
	if(segmentation_mode == false) return;

	for(int ii = 0; ii < frames.size(); ii++)
	{
		Frame& frame = frames[ii];
		frame.SegmentPlayers();
	}
}

void graph_cut()
{
	if(graphcut_mode == false) return ;
	if(normal_mode == false) normal_mode = true;
	compute_normal();

	for(int ii = 0; ii < frames.size(); ii++)
	{
		Frame& frame = frames[ii];
		frame.GraphCutPlayers();
	}
}

void compute_normal()
{
	if(normal_mode == false) return;

	for(int ii = 0; ii < frames.size(); ii++)
	{
		Frame& frame = frames[ii];
		frame.NormalPlayers();
	}
	cout << "Compute Normal" << endl;
}

void transform()
{
	if(transformation_mode == false) return ;
	if(frames.size() < 2) return ;
	const Frame& refFrame = frames[0];
	for(int ii = 1; ii < frames.size(); ii++)
	{
		Frame& newFrame = frames[ii];
		newFrame.TransformPlayers(refFrame);
	}
}

void draw_frame(const Frame& frame, const GLuint& bindIndex, const bool texture_mode)
{
	const Storage& storage = frame.GetStorage();
	const Mat& coordinate = storage.GetCoordinate();
	Mat point;
	storage.GetPoint().copyTo(point, frame.m_indexFrame == 0);
	point *= scale_factor;

	Mat color = storage.GetColor();
	resize(color, color, Size(width, height), 0.0, 0.0, INTER_NEAREST);

	if(texture_mode)
	{
		assign_texture(point, coordinate, color, bindIndex);
	}
	else 
	{
		glColor3f(0.5f, 0.5f, 0.5f);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, point.data);
		glPointSize(pointSizePoint);
		glDrawElements(GL_POINTS, width * height, GL_UNSIGNED_INT, indices);
	}
}

void draw_player(
	const Frame& frame,
	const Player& player,
	const GLuint& bindIndex,
	const bool texture_mode)
{
	const int index = player.GetIndex() - 1;
	const Vec3f colorLabel = SKELETON_COLOR_LIST[index];
	glColor3f(colorLabel[0], colorLabel[1], colorLabel[2]);

	const Storage& storage = player.GetStorage();
	const Mat& coordinate = storage.GetCoordinate();
	Mat point = storage.GetPoint() * scale_factor;
	Mat color = frame.GetStorage().GetColor();
	resize(color, color, Size(width, height), 0.0, 0.0, INTER_NEAREST);

	if(texture_mode)
	{
		assign_texture(point, coordinate, color, bindIndex);
	}
	else 
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, point.data);
		glPointSize(pointSizeColor);
		glDrawElements(GL_POINTS, width * height, GL_UNSIGNED_INT, indices);
	}
}

void draw_skeleton(const Player& player, const Mapper& mapper)
{
	const float radius = 0.2f;
	const int nSlice = 25;

	// set color
	// TODO: check if getIndex returns non-zero based number
	const int index = player.GetIndex() - 1;
	const Vec3f color = SKELETON_COLOR_LIST[index];
	glColor3f(color[0], color[1], color[2]);

	const Skeleton& skeleton = player.GetSkeleton();
	// draw lines
	{
		glLineWidth(2.5f);
		glBegin(GL_LINES);
		for(int ii = 0; ii < N_PART; ii++)
		{
			const Part& skeletonPart = skeleton.GetPart(ii);
			const Part part = mapper.transformSkeletonPartToPointPart(skeletonPart);
			const Vec3f& startJoint = part.GetStartJoint();
			const Vec3f& endJoint = part.GetEndJoint();
			glVertex3f(startJoint[0], startJoint[1], startJoint[2]);
			glVertex3f(endJoint[0], endJoint[1], endJoint[2]);
		}
		glEnd();
	}

	// draw joints
	vector<Vec3f> pointList = mapper.transformSkeletonJointToPointJoint(skeleton.GetJointList());
	for(int ii = 0; ii < N_JOINT; ii++)
	{
		const Vec3f& point = pointList[ii] * scale_factor;
		if(point == Vec3f(0, 0, 0)) continue;
		glPushMatrix();
		glTranslatef(point[0], point[1], point[2]);
		glutSolidSphere(radius, nSlice, nSlice);
		glPopMatrix();
	}
}

void draw_segmentation(const Player& player, const GLuint& bindIndex)
{
	glEnable(GL_TEXTURE_2D);
	Mat color = Mat::zeros(height, width, CV_8UC3);
	const Storage& storage = player.GetStorage();
	const Mat& labelMatrix = player.GetLabel();
	const Mat& pointMatrix = storage.GetPoint();
	for(int rr = 0; rr < height; rr++) 
	{
		for(int cc = 0; cc < width; cc++)
		{
			const Vec3f point = pointMatrix.at<Vec3f>(rr, cc);
			const int label = labelMatrix.at<int>(rr, cc);
			if(label < 0 || label >= NUI_SKELETON_POSITION_COUNT - 1) continue;
			if(point[0] == 0.0f && point[1] == 0.0f && point[2] == 0.0f) continue;
			color.at<Vec3b>(rr, cc) = SKELETON_POSITION_COLOR_LIST[label];
		}
	}
	Mat point = pointMatrix * scale_factor;
	assign_texture(point, storage.GetCoordinate(), color, all_texture);
}

void draw_graphcut(const Player& player, const GLuint& bindIndex)
{
	draw_segmentation(player, bindIndex);
}

void draw_normal(const Player& player)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);

	const Mat& normalMatrix = player.GetNormal();
	const Mat& pointMatrix = player.GetStorage().GetPoint();
	if(!normalMatrix.empty() && !pointMatrix.empty())
	{
		for(int rr = 0; rr < height; rr++)
		{
			for(int cc = 0; cc < width; cc++)
			{
				const Vec3f& normalVector = normalMatrix.at<Vec3f>(rr, cc) * scale_factor;
				if(normalVector == Vec3f(0.0f, 0.0f, 0.0f)) continue;
				else 
				{
					const Vec3f fromPoint = pointMatrix.at<Vec3f>(rr, cc) * scale_factor;
					const Vec3f toPoint = fromPoint + normalVector;
					glVertex3f(fromPoint[0], fromPoint[1], fromPoint[2]);
					glVertex3f(toPoint[0], toPoint[1], toPoint[2]);
				}
			}
		}
	}
	glEnd();
}

void display()
{
	if(online_mode) prepare_data();
	if(is_paused) return ;

	// clear buffers
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// set matrix
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(t[0], t[1], t[2] - 1.0f);

	GLfloat m[4][4];
	build_rotmatrix( m, quat );
	glMultMatrixf( &m[ 0 ][ 0 ]);
	draw_center();

	// Set the projection from the XYZ to the texture image
	glMatrixMode(GL_TEXTURE);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glLoadIdentity();
	glScalef(1 / static_cast<float>(width), 1 / static_cast<float>(height),1);
	glMatrixMode( GL_MODELVIEW );


	// draw
	if(screen_mode)
	{
		for(int ii = 0; ii < frames.size(); ii++)
		{
			const Frame& frame = frames[ii];
			const vector<Player>& players = frame.GetPlayers();
			if(frame_mode)
				draw_frame(frame, all_texture, frame_texture_mode);

			for(int jj = 0; jj < players.size(); jj++)
			{
				const Player& player = players[jj];
				if(segmentation_mode && !online_mode)
					draw_segmentation(player, all_texture);
				else if(graphcut_mode && !online_mode)
					draw_graphcut(player, all_texture);
				else
					draw_player(frame, player, all_texture, player_texture_mode);
				if(normal_mode)
					draw_normal(player);
			}
		}
	}

	if(skeleton_mode)
	{
		for(int ii = 0; ii < frames.size(); ii++)
		{
			Frame& frame = frames[ii];
			const vector<Player>& players = frame.GetPlayers();
			for(int jj = 0; jj < players.size(); jj++)
			{
				const Player& player = players[jj];
				draw_skeleton(player, frame.m_mapper);
			}
		}
	}

	if(auto_mode && online_mode)
	{
		auto_save();
	}
	
	glPopMatrix();
	glutSwapBuffers();
}

void close()
{
	destroyAllWindows();
	glutDestroyWindow(window);
}

void special(int key, int x, int y)
{
	if( key == GLUT_KEY_LEFT )
	{

	}
	else if( key == GLUT_KEY_RIGHT )
	{

	}
	else if( key == GLUT_KEY_UP )
	{
		grabber.ElevationAngleIncrease();
	}
	else if( key == GLUT_KEY_DOWN )
	{
		grabber.ElevationAngleDecrease();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if( key == 27 ) 
	{
		glutLeaveMainLoop();
	}

	else if( key == 'r' )
	{
		rot_x = rot_y = trans_x = trans_y = trans_z = 0;
		//scale_factor = scale_factor_init;
		trackball( quat , 90.0, 0.0, 0.0, 0.0 );
		t[ 0 ] = t[ 1 ] = t[ 2 ] = 0;
	}

	else if( key == 'n' ) {
		grabber.ToggleNearMode();
	}

	else if( key == 'i') 
	{
		player_index_mode = !player_index_mode;
	}

	else if( key == 'p')
	{
		is_paused = !is_paused;
	}

	else if( key == 's' )
	{
		save();
	}

	else if(key == 'q') screen_mode = !screen_mode;
	else if(key == 'w') frame_mode = !frame_mode;

	else if(key == 'o')
	{
		online_mode = !online_mode;
		InitGL(width, height, path);
	}

	else if(key == 'a') {
		auto_mode = !auto_mode;
		if(auto_mode && online_mode)
		{
			auto_save();
		}
	}

	else if(key == '1')	frame_texture_mode = !frame_texture_mode;
	else if(key == '2') player_texture_mode = !player_texture_mode;
	else if(key == '3')	skeleton_mode = !skeleton_mode;
	else if(key == '4') {
		segmentation_mode = !segmentation_mode;
		if(segmentation_mode && online_mode) 
		{
			online_mode = false;
			segmentation();
		}
	}
	else if(key == '5') {
		normal_mode = !normal_mode;
		if(normal_mode && online_mode) {
			online_mode = false;
			compute_normal();
		}
	}
	else if(key == '6') {
		graphcut_mode = !graphcut_mode;
		if(graphcut_mode && online_mode)
		{
			online_mode = false;
			graph_cut();
		}
	}

	else if(key == 'c') clipping_mode = !clipping_mode;
	else if(key == 'm') smoothing_mode = !smoothing_mode;

	if(key != 27)
		glutPostRedisplay();
}

void reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 58, 4/3., 0.1, 100 );
	glMatrixMode(GL_MODELVIEW);
}



void motion( int x, int y )
 {
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if( drag_state == GLUT_DOWN )
	{
		if( button_state == GLUT_LEFT_BUTTON )
		{
			trackball( spin_quat,
				( gain * rot_x - width   ) / width,
		        ( height  - gain * rot_y ) / height,
		        (       gain * x - width   ) / width,
		        ( height  - gain * y       ) / height);
			add_quats( spin_quat, quat, quat );
		}
		else if( button_state == GLUT_RIGHT_BUTTON )
		{
			t[ 0 ] -= ((( float )trans_x - x ) / width );
			t[ 1 ] += ((( float )trans_y - y ) / height );
		}
		else if( button_state == GLUT_MIDDLE_BUTTON )
			t[ 2 ] -= ((( float )trans_z - y ) / height * 3 );
		else if( button_state == 3 || button_state == 4 ) // scroll
		{
			
		}
		glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if( state == GLUT_DOWN )
	{
		if( button == GLUT_LEFT_BUTTON )
		{
			rot_x = x;
			rot_y = y;
		}
		else if( button == GLUT_RIGHT_BUTTON )
		{
			trans_x = x;
			trans_y = y;
		}
		else if( button == GLUT_MIDDLE_BUTTON )
		{
			trans_z = y;
		}
		else if( button == 3 || button == 4 )
		{
			const float sign = (static_cast<float>(button) - 3.5f) * 2.0f;
			t[2] -= sign * height * 0.00015;
		}
	}

	drag_state = state;
	button_state = button;
}


void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

float vlength(const float *v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}

void vnormal(float *v)
{
    vscale(v,1.0f/vlength(v));
}

float vdot(const float *v1, const float *v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
    float a[3]; /* Axis of rotation */
    float phi;  /* how much to rotate about axis */
    float p1[3], p2[3], d[3];
    float t;

    if (p1x == p2x && p1y == p2y) {
        /* Zero rotation */
        vzero(q);
        q[3] = 1.0;
        return;
    }

    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */
    vset(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
    vset(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

    /*
     *  Now, we want the cross product of P1 and P2
     */
    vcross(p2,p1,a);

    /*
     *  Figure out how much to rotate around that axis.
     */
    vsub(p1,p2,d);
    t = vlength(d) / (2.0f*TRACKBALLSIZE);

    /*
     * Avoid problems with out-of-control values...
     */
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    phi = 2.0f * asin(t);

    axis_to_quat(a,phi,q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
    vnormal(a);
    vcopy(a,q);
    vscale(q,sin(phi/2.0f));
    q[3] = cos(phi/2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
    float d, t, z;

    d = sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
        z = sqrt(r*r - d*d);
    } else {           /* On hyperbola */
        t = r / 1.41421356237309504880f;
        z = t*t / d;
    }
    return z;
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
    static int count = 0;
    float t1[4], t2[4], t3[4];
    float tf[4];

    vcopy(q1,t1);
    vscale(t1,q2[3]);

    vcopy(q2,t2);
    vscale(t2,q1[3]);

    vcross(q2,q1,t3);
    vadd(t1,t2,tf);
    vadd(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - vdot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) {
        count = 0;
        normalize_quat(dest);
    }
}

void normalize_quat(float q[4])
{
    int i;
    float mag;

    mag = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
    m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
    m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
    m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
    m[0][3] = 0.0f;

    m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
    m[1][1]= 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
    m[1][3] = 0.0f;

    m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
    m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
    m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

int main(int argc, char* argv[])
{
	Initialize(argc, argv);
	glutMainLoop();
	Deallocate();
	cout << "finished" << endl;
    return 0;
}