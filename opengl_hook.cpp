#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>


#include "globals.h"
#include "opengl_hook.h"
#include "transform.h"

#define TITLE "781 Lab1"
#define X_RES 600
#define Y_RES 400
#define X_POS 600
#define Y_POS 400
#define BUFFER_OFFSET(n) ((GLubyte *)NULL + n)
#define VERTS 0
#define SHADOWS 1
#define light_x 1.0f
#define light_y 5
#define light_z 0.5f
#define camera_x 0
#define camera_y 0
#define camera_z 5
/*
 *This factor can be changed to control the amount of 
 *scaling required on the bunny/world
 */
#define SCALE_FACT 0.02f
/*
 *Number of bufferes (for vbo)
 */
#define NUM_BUFFERS 2
#define glError() { \
        GLenum err = glGetError(); \
        while (err != GL_NO_ERROR) { \
                        fprintf(stdout, \
                                "glError: %s caught at %s:%u\n",\
                                (char *)gluErrorString(err),\
                                __FILE__, __LINE__); \
                        err = glGetError(); \
                } \
}

using namespace std;

/* Function declarations */


/*
 *OpenGL related startup functions 
 */

/*
 *This function takes care of all glut initialization calls. 
 *It also calls EnableGL which in turn takes care of enabling the 
 *OpenGL features required by the program
 */
void InitGlut(int *argc, char** argv);
void EnableGL();
/*
 *Following are the hooks given to OpenGL
 */
void DisplayFunc();
void ReshapeFunc(int w, int h);
void KeyboardFunc(unsigned char key, int x, int y);
void SpecialKeys(int key, int x, int y);
void MouseFunc(int button, int state, int x, int y);
void MotionFunc(int x, int y);
/*
 *This function is called from the display callback function. It 
 *handles drawing the ply model using vbo
 */
void DrawPly(int flag);
/*
 *This function is called from the display callback function. It 
 *handles drawing the environment around the model.
 */
void DrawArena();

/*
 *This function just copies the vertices and normals populated by the 
 *plyread function into single arrays.
 */
void CopyVertNorm();
void CopyShadow();
/*
 *This is used to calculate the amount by which the model needs to be 
 *scaled in order to make the largest dimension = 1.0
 */
void CalcScale();
/*
 *This takes a point in window space and returns the corresponding 
 *point in local space (model space)
 *x, y, z = Window space co-ordinates
 *fx, fy, fz = pointers to local space co-ordinates which is the 
 *required value
 */

inline void ConvertToLocal(int x, int y, double z, GLdouble* fx, 
        GLdouble* fy, GLdouble* fz);
/*
 *This projects a point on the XY plane to a sphere of radius 'r'
 *x, y = Co-ordinates in the XY plane
 *r = radius of the sphere to be projected to 
 */
inline GLdouble ProjToSphere(GLdouble x, GLdouble y, int r);

/*
 *Calculates the transpose of the 3x3 part of the matrix mat
 *Value modified in mat itself.
 */
void Transpose(GLdouble *mat);
/*
 *Calculates the length of vector specified by x, y and z
 */
inline double VectorLength(GLdouble x, GLdouble y, GLdouble z);
/*
 *Calculates the crossproduct between vectors specified by 
 *x1, y1, z1 and x2, y2, z2. 
 *Return value in array ret
 */
void  CrossProduct(GLdouble x1, GLdouble y1, GLdouble z1,
         GLdouble x2, GLdouble y2, GLdouble z2, GLdouble* ret);
/*todo*/
GLdouble DotProduct(GLdouble x1, GLdouble y1, GLdouble z1,
         GLdouble x2, GLdouble y2, GLdouble z2);

inline void GetVector(GLdouble x1, GLdouble y1, GLdouble z1, 
                      GLdouble x2, GLdouble y2, GLdouble z2,
                      GLdouble *ret);
/*
 *This function multiplies sx, sy, sz by modelviewmat[1-2] depending 
 *on if it is in global or local mode
 */
void MultModel(double sx, double sy, double sz, double *tx, 
        double *ty, double *tz);
/*
 *Sets global mode on if true is passed as 'value'
 */
void GlobalMode(bool value);

/*
 *Globals
 */

/*
 *the code below was taken from codesampler.com
 */
/*******************************************************************/
typedef struct sVertex {
  float r, g, b;
  float x, y, z;
} sVertex;
float g_shadowCasterNormal[] = { 0.0f, 1.0f, 0.0f };
struct ShadowCaster
{
    sVertex *verts;
    float  *normal;
    int     numVerts;
    int     shadowVolume;
};
ShadowCaster g_shadowCaster;
/*the values below are modified to suit the current context*/
sVertex g_shadowCasterVerts[] = 
{
    { 1.0, 1.0, 1.0,  -.3f, 1.5f, -.5f },
    { 1.0, 1.0, 1.0,  -.3f, 1.5f,  .5f },
    { 1.0, 1.0, 1.0,   .3f, 1.5f,  .5f },
    { 1.0, 1.0, 1.0,   .3f, 1.5f, -.5f },
};
/*******************************************************************/
/*
 *end of copied code
 */

GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0f}; 
GLfloat light_dif[] = {1, 1, 1, 1};
GLfloat light_shad[] = {0.2, 0.2, 0.2, 1};
GLfloat light_pos1[] = {light_x, light_y, light_z, 0.0f};
GLfloat light_color1[] = {1.2f, 1.2f, 1.2f, 1.0f}; 
//GLfloat light_pos[] = {1.0f, 6, -5, 1.0f};
GLfloat light_pos[] = {light_x, light_y, light_z, 0.0f};
/*light 0 is ambient and 1 is diffuse*/
double angle = 0.0;
int  debug = 0;
float max_diff = 0;
float init_max_diff;
GLdouble rot_axis[]={0, 0, 0};
GLuint buffers[NUM_BUFFERS];
GLuint shadowMapTexture;
const int shadowMapSize=512;
int windowWidth, windowHeight;
GLfloat* vert;
GLfloat* norm;
GLuint* ind;
double startx, starty, startz, endx, endy, endz;
double stransx, stransy, stransz;
double etransx, etransy, etransz;
double transx, transy, transz;
bool non_axis = false;
bool mouse_left = false;
bool light_move = false;
bool shad_vol = false;
bool planar = false;
bool mouse_right = false;
bool wire = false;
bool track_ball = false;
bool gtrack_ball = false;
bool ltrack_ball = true;
bool global_rot = false;
float scale = 1.0f;
float scaleg = 1.0f;

/*
 *Ply models modelview matrix
 */
GLdouble modelviewmat1[]={1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1};
/*
 *Global modelview matrix
 */
GLdouble modelviewmat2[]={1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1};
GLdouble modelviewmat3[]={1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1};
GLdouble light_viewmat[]={1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1};
GLdouble modelmat[]={1,0,0,0,
                     0,1,0,0,
                     0,0,1,0,
                     0,0,0,1};
GLdouble biasmat[]={.5,0,0,0,
                    0,.5,0,0,
                    0,0,.5,0,
                    .5,.5,.5,1};
GLdouble texturemat[16];
GLdouble lightProjectionMatrix[16], lightViewMatrix[16];
GLdouble cameraProjectionMatrix[16], cameraViewMatrix[16];
/*
 *temperory modelview, projection and viewport 
 */
GLdouble modelviewmat[16];
GLdouble projmat[16];
GLint viewport[4];

/*
 *Function definitions
 */
void OpenGLInit(int* pargc, char** argv)
{
    CalcScale();
    InitGlut(pargc, argv);
    glutMainLoop();
}

void CalcScale()
{
    max_diff = (max_diff < (x_max - cx))? (x_max - cx):max_diff;
    max_diff = (max_diff < (y_max - cy))? (y_max - cy):max_diff;
    max_diff = (max_diff < (z_max - cz))? (z_max - cz):max_diff;
    init_max_diff = max_diff;
}

void InitGlut(int *pargc, char* argv[])
{
    glutInit(pargc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_RGB|GLUT_DOUBLE|GLUT_STENCIL);
    glutInitWindowPosition(X_POS, Y_POS);
    glutInitWindowSize(X_RES, Y_RES);
    glutCreateWindow(TITLE);
    glewInit();
    glutDisplayFunc(DisplayFunc);
    glutReshapeFunc(ReshapeFunc);
    glutKeyboardFunc(KeyboardFunc);
    glutSpecialFunc(SpecialKeys);    
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    EnableGL();
}

void EnableGL()
{
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1); 
    glEnable(GL_COLOR_MATERIAL);
    glClearStencil(0x0);
    glEnable(GL_STENCIL_TEST);
    glShadeModel(GL_SMOOTH);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_amb);
    vert = new GLfloat[vertexcount*3];
    norm = new GLfloat[vertexcount*3];
    ind = new GLuint[facecount*faces[0]->count];
    CopyVertNorm();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGenBuffers(NUM_BUFFERS, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert)*6*vertexcount, NULL,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert)*3*vertexcount,
                    vert);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vert)*3*vertexcount, 
            sizeof(norm)*3*vertexcount, norm);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[SHADOWS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert)*6*vertexcount, NULL,
                 GL_STATIC_DRAW);
    glError();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_NORMALIZE);
    float lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};
    float lightSpecular[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

    g_shadowCaster.verts        = g_shadowCasterVerts;
    g_shadowCaster.normal       = g_shadowCasterNormal;
    g_shadowCaster.numVerts     = (sizeof(g_shadowCasterVerts) 
                                  / sizeof(sVertex));
    g_shadowCaster.shadowVolume = -1;
}

void VectAdd(GLfloat *vect1, GLfloat *vect2, GLfloat *ret,
             short ind, bool add)
{
    short i;
    if(add)
        for(i=0;i<ind;i++)
            ret[i] = vect1[i] + vect2[i];
    else
        for(i=0;i<ind;i++)
            ret[i] = vect1[i] - vect2[i];
    return;
}

void ScalarVectMult(GLdouble scalar, GLfloat* vect, GLfloat* ret)
{
    for(short i=0;i<3;i++)
        ret[i] = scalar*vect[i];
    return;
}
void MultMatrix(double sx, double sy, double sz, GLfloat *tx, 
        GLfloat *ty, GLfloat *tz, GLdouble *mat)
{
    *tx = sx*mat[0]+ sy*mat[4]+ sz*mat[8]+ mat[12];
    *ty = sx*mat[1]+ sy*mat[5]+ sz*mat[9]+ mat[13];
    *tz = sx*mat[2]+ sy*mat[6]+ sz*mat[10]+ mat[14];
    //cout<<sx<<" "<<sy<<" "<<sz<<endl;
    //cout<<*tx<<" "<<*ty<<" "<<*tz<<endl;
    //cin>>debug;
}
void CalcPlanarShadow(Vertex* vert, GLfloat* ret, GLdouble *n)
{
    GLfloat vminusl[3], v[3], temp_vect[3], temp_light[3];
    GLfloat light_temp[3];
    GLdouble temp;
    //cout<<n[0]<<" "<<n[1]<<" "<<n[2]<<endl;cin>>debug;
    temp_vect[0] = vert->x;
    temp_vect[1] = vert->y;
    temp_vect[2] = vert->z;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(transx, transy, transz);
    glMultMatrixd(modelviewmat1);
    glTranslatef(-transx, -transy, -transz);
    glTranslatef(transx, transy, transz);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat);
    glPopMatrix();
    temp_light[0] = light_pos[0];
    temp_light[1] = light_pos[1];
    temp_light[2] = light_pos[2];
    MultMatrix(temp_vect[0], temp_vect[1], temp_vect[2], 
            &v[0], &v[1], &v[2], modelviewmat);
    VectAdd(v, temp_light, vminusl, 3, 0);
    temp = DotProduct(n[0],n[1],n[2], temp_light[0], temp_light[1],
            temp_light[2])/DotProduct(n[0],n[1],n[2], vminusl[0],
                vminusl[1], vminusl[2]);
    ScalarVectMult(temp, vminusl, temp_vect);
    VectAdd(temp_light, temp_vect, ret, 3, 0);
}
void CopyShadow(GLdouble *n)
{
    unsigned int i, j, k;
    unsigned int v;
    GLfloat ret[3];
    debug = 0;
    for(i=2,j=0;i<(vertexcount*3);i+=3,j++)
    {
        CalcPlanarShadow(vertices[j], ret, n);
        vert[i-2] = ret[0];
        vert[i-1] = ret[1];
        vert[i] = ret[2];
        norm[i-2] = n[0];
        norm[i-1] = n[1];
        norm[i] = n[2];
    }
    k = 0;

    for(j=0;j<facecount;j++)
    {
        for(i=0;i < faces[j]->count;i++,k++)
        {
            ind[k] = faces[j]->vertices[i];
        }
    }
}

void CopyVertNorm()
{
    unsigned int i, j, k;
    unsigned int v;
    for(i=2,j=0;i<(vertexcount*3);i+=3,j++)
    {
        vert[i-2] = vertices[j]->x;
        vert[i-1] = vertices[j]->y;
        vert[i] = vertices[j]->z;
        norm[i-2] = vertices[j]->nx;
        norm[i-1] = vertices[j]->ny;
        norm[i] = vertices[j]->nz;
    }
    k = 0;

    for(j=0;j<facecount;j++)
    {
        for(i=0;i < faces[j]->count;i++,k++)
        {
            ind[k] = faces[j]->vertices[i];
        }
    }
}
void myShadowMatrix(float ground[4], float light[4])
{
    float  dot;
    float  shadowMat[4][4];

    dot = ground[0] * light[0] +
          ground[1] * light[1] +
          ground[2] * light[2] +
          ground[3] * light[3];
    
    shadowMat[0][0] = dot - light[0] * ground[0];
    shadowMat[1][0] = 0.0 - light[0] * ground[1];
    shadowMat[2][0] = 0.0 - light[0] * ground[2];
    shadowMat[3][0] = 0.0 - light[0] * ground[3];
    
    shadowMat[0][1] = 0.0 - light[1] * ground[0];
    shadowMat[1][1] = dot - light[1] * ground[1];
    shadowMat[2][1] = 0.0 - light[1] * ground[2];
    shadowMat[3][1] = 0.0 - light[1] * ground[3];
    
    shadowMat[0][2] = 0.0 - light[2] * ground[0];
    shadowMat[1][2] = 0.0 - light[2] * ground[1];
    shadowMat[2][2] = dot - light[2] * ground[2];
    shadowMat[3][2] = 0.0 - light[2] * ground[3];
    
    shadowMat[0][3] = 0.0 - light[3] * ground[0];
    shadowMat[1][3] = 0.0 - light[3] * ground[1];
    shadowMat[2][3] = 0.0 - light[3] * ground[2];
    shadowMat[3][3] = dot - light[3] * ground[3];

    glMultMatrixf((const GLfloat*)shadowMat);
}


void DrawShadow(GLdouble* n)
{
    glDisable( GL_LIGHT0 );
    CopyShadow(n);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(camera_x, camera_y, camera_z, 0, 0, 0, 0, 1, 0);
    glScalef(1/max_diff, 1/max_diff, 1/max_diff);
    glTranslatef(0, -(cy-y_min), 0);
    if(non_axis)
        glTranslatef(cx,0,0);
    glMultMatrixd(modelviewmat2);
    GLfloat tempx, tempy, tempz;
    glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat3);
    MultMatrix(vert[0], vert[1], vert[2], &tempx, &tempy, &tempz,
               modelviewmat3);
    glError();
    glBindBuffer(GL_ARRAY_BUFFER, buffers[SHADOWS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert)*3*vertexcount ,
            vert);
    glError();
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vert)*3*vertexcount, 
            sizeof(norm)*3*vertexcount, norm);
    glError();
    GLenum i;
    glBindBuffer(GL_ARRAY_BUFFER, buffers[SHADOWS]);
    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 3*vertexcount*sizeof(vert)+
                    BUFFER_OFFSET(0));
    glEnableClientState(GL_NORMAL_ARRAY);
    if(faces[0]->count == 3)
        i = GL_TRIANGLES;
    else if (faces[0]->count == 4)
        i = GL_QUADS;
    glDrawElements(i, facecount*faces[0]->count, GL_UNSIGNED_INT, 
            ind);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPopMatrix();
    glEnable( GL_LIGHT0 );
}

void DrawPly(int flag)
{
    GLenum i;
    GLfloat tempx, tempy, tempz;
    glPushMatrix();
    glScalef(scale, scale, scale);
    glColor3f(flag, flag, flag);
    //transx = etransx - stransx;
    //transy = etransy - stransy;
    glTranslatef(transx, transy, transz);
    glMultMatrixd(modelviewmat1);
    glTranslatef(-transx, -transy, -transz);
    glTranslatef(transx, transy, transz);
    if(ltrack_ball)
    {
        if(track_ball)
            glutWireSphere(init_max_diff, 15, 15);
    }
    glTranslatef(-cx, -cy, -cz);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTS]);
    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 3*vertexcount*sizeof(vert)+
                    BUFFER_OFFSET(0));
    glEnableClientState(GL_NORMAL_ARRAY);
    glError();
    if(faces[0]->count == 3)
        i = GL_TRIANGLES;
    else if (faces[0]->count == 4)
        i = GL_QUADS;
    glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat3);
    glDrawElements(i, facecount*faces[0]->count, GL_UNSIGNED_INT, 
            ind);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPopMatrix();
}

void DrawFloor()
{
    glPushMatrix();
    glColor3f(1., 2., 1.);
    glTranslatef(0, -(cy-y_min), 0);
    glBegin(GL_QUADS);
    glVertex3f(.3, 0, -.3);
    glVertex3f(-.3, -.0, -.3);
    glVertex3f(-.3, -.0, .3);
    glVertex3f(.3, 0, .3);
    glEnd();
    if(non_axis)
    {
        glBegin(GL_QUADS);
        glVertex3f(cx, -.0, -.1);
        glVertex3f(cx, -.0, .1);
        glVertex3f(-.2, .1, .1);
        glVertex3f(-.2, .1, -.1);
        glEnd();
    }
    glBegin( GL_POLYGON );
    {
        glNormal3fv( g_shadowCaster.normal );

        for( int i = 0; i < g_shadowCaster.numVerts; ++i )
        {
            glVertex3f( g_shadowCaster.verts[i].x,
                        g_shadowCaster.verts[i].y,
                        g_shadowCaster.verts[i].z );
        }
    }
    glEnd();
    glPopMatrix();
}

void DrawArena()
{
    glPushMatrix();
    glScalef(scaleg, scaleg, scaleg);
    if(gtrack_ball)
    {
        if(track_ball)
            glutWireSphere(VectorLength(.3,0,.3), 15, 15);
    }
    glPushMatrix();
    glColor3f(1.5f, 0.5f, 1.0);
    glTranslatef(-0.1, 0, -.25f);
    glTranslatef(0, -(cy-y_min)/2, 0);
    glutSolidTeapot(.05);
    glPopMatrix();
    glColor3f(2.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-.2,0,.0);
    glTranslatef(0, -(cy-y_min)/2, 0);
    glutSolidCube(0.05);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 2.0f);
    glPushMatrix();
    glTranslatef(.2,0,.0);
    glTranslatef(0, -(cy-y_min)/2, 0);
    glutSolidSphere(0.05, 15, 15);
    glPopMatrix();
    if(!shad_vol)
    {
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_ALWAYS, 1, 0xFFFFFFFF );
        glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE ); 
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        glDepthMask( GL_FALSE );
        DrawFloor();
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        glDepthMask( GL_TRUE );
        glDisable( GL_STENCIL_TEST );
    }
    DrawFloor();
    glPopMatrix();
}

void DrawAll()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    gluLookAt(camera_x, camera_y, camera_z, 0, 0, 0, 0, 1, 0);
    glPushMatrix();
    glScalef(1/max_diff, 1/max_diff, 1/max_diff);
    if(wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glMultMatrixd(modelviewmat2);
    DrawArena();
    DrawPly(1);
    if(planar)
    {
        glPolygonOffset( -1.0, -1.0 );
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_EQUAL, 1, 0xFFFFFFFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        glEnable( GL_BLEND );
        GLdouble vect1[3], vect2[3], n[3];
        if(non_axis)
        {
            GetVector(cx, -.0, -.1, cx, -.0, .1, vect1);
            GetVector(cx, -.0, -.1, -.2, .1, -.1, vect2);
            CrossProduct(vect2[0], vect2[1], vect2[2], 
                         vect1[0], vect1[1], vect1[2], 
                         n);
        }
        else
        {
            GetVector(.3, 0, .3, .3, 0, -.3, vect1);
            GetVector(-.3, -.0, -.3, .3, -.0, -.3, vect2);
            CrossProduct(vect2[0], vect2[1], vect2[2], 
                         vect1[0], vect1[1], vect1[2], 
                         n);
        }
        DrawShadow(n);
        glDisable( GL_BLEND );
        glDisable( GL_STENCIL_TEST );
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPopMatrix();
    //cout<<cy<<" "<<y_min<<endl;cin>>debug;
}
/*
 *this function was taken from codesampler.com
 */
void extendVertex(float newVert[3], float lightPosit[3],
                  sVertex vert, float ext )
{
    float lightDir[3];
    lightDir[0] = vert.x - lightPosit[0];
    lightDir[1] = vert.y - lightPosit[1];
    lightDir[2] = vert.z - lightPosit[2];
    newVert[0] = lightPosit[0] + lightDir[0] * ext;
    newVert[1] = lightPosit[1] + lightDir[1] * ext;
    newVert[2] = lightPosit[2] + lightDir[2] * ext;
}
/*
 *this function was taken from codesampler.com
 */
void buildShadowVolume(ShadowCaster *caster, float lightPosit[3],
                       float ext )
{
    if( caster->shadowVolume != -1 )
        glDeleteLists( caster->shadowVolume, 0 );
    caster->shadowVolume = glGenLists(1);
    glNewList( caster->shadowVolume, GL_COMPILE );
    {
        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        {
            glColor3f( 0.2f, 0.8f, 0.4f );
            float vExtended[3];
            for( int i = 0; i < caster->numVerts; ++i )
            {
                int e0 = i;
                int e1 = i+1;
                if( e1 >= caster->numVerts )
                    e1 = 0;
                glVertex3f( caster->verts[e0].x, 
                            caster->verts[e0].y, 
                            caster->verts[e0].z );
                extendVertex(vExtended, lightPosit,
                             caster->verts[e0], ext );
                glVertex3f( vExtended[0], vExtended[1], vExtended[2] );
                extendVertex(vExtended, lightPosit, 
                             caster->verts[e1], ext );
                glVertex3f( vExtended[0], vExtended[1], vExtended[2] );
                glVertex3f( caster->verts[e1].x, 
                            caster->verts[e1].y,
                            caster->verts[e1].z );
            }
        }
        glEnd();
        glEnable( GL_LIGHTING );
    }
    glEndList();
}

void DisplayFunc()
{
    if(shad_vol)
    {
        buildShadowVolume( &g_shadowCaster, light_pos, 5.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT );
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        DrawAll();
        glEnable( GL_CULL_FACE );
        glEnable( GL_STENCIL_TEST );
        glDepthMask( GL_FALSE );
        glStencilFunc( GL_ALWAYS, 0, 0 );
        glStencilOp( GL_KEEP, GL_INCR, GL_KEEP);
        glCullFace( GL_FRONT);
        glMultMatrixd(modelviewmat2);
        glCallList( g_shadowCaster.shadowVolume );
        glStencilOp( GL_KEEP, GL_DECR, GL_KEEP);
        glCullFace( GL_BACK);
        glCallList( g_shadowCaster.shadowVolume );
        glDepthMask( GL_TRUE );
        glDepthFunc( GL_LEQUAL );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        glCullFace( GL_BACK );
        glDisable( GL_CULL_FACE );
        glStencilFunc( GL_EQUAL, 1, 1 );
        glDisable( GL_LIGHT0 );
        DrawAll();
        glStencilFunc( GL_EQUAL, 0, 1 );
    }
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT );
    glEnable( GL_LIGHT0 );
    DrawAll();
    glDepthFunc( GL_LESS );
    glDisable( GL_STENCIL_TEST);
    if(wire and shad_vol)
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glCallList( g_shadowCaster.shadowVolume );
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    glutSwapBuffers();
}

void swap(GLdouble* x, GLdouble* y)
{
    GLdouble temp;
    temp = *y;
    *y = *x;
    *x = temp;
}
void Transpose(GLdouble *mat)
{
    swap(mat+1, mat+4);
    swap(mat+2, mat+8);
    swap(mat+6, mat+9);
    swap(mat+3, mat+12);
    swap(mat+7, mat+13);
    swap(mat+11, mat+14);
}

void ReshapeFunc(int w, int h)
{
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1.5, 20);
    //gluPerspective(45.0f, (float)windowWidth/windowHeight, 1.0f,
                   //100.0f);
    glGetDoublev(GL_MODELVIEW_MATRIX, cameraProjectionMatrix);
    //glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void MouseFunc(int button, int state, int x, int y)
{
    switch(button)
    {
        case GLUT_LEFT_BUTTON:
            if(state == GLUT_DOWN)
            {
                mouse_left = true;
                double tstartx, tstarty, tstartz;
                ConvertToLocal(x, y, 0.01, &tstartx, &tstarty, 
                        &tstartz);
                tstartz = ProjToSphere(tstartx, tstarty, 1);
                MultModel(tstartx, tstarty, tstartz, &startx, &starty,
                        &startz);
            }
            else
            {
                mouse_left = false;
            }
            break;

        case GLUT_RIGHT_BUTTON:
            if(state == GLUT_DOWN)
            {
                if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
                {
                    if(!global_rot)
                    scale -= SCALE_FACT;
                    else
                    scaleg -= SCALE_FACT;
                }
                else if(glutGetModifiers() == GLUT_ACTIVE_SHIFT)
                {
                    if(!global_rot)
                    scale += SCALE_FACT;
                    else
                    scaleg += SCALE_FACT;
                }
                else
                {
                    mouse_right = true;
                    ConvertToLocal(x, y, 0.01, &stransx, &stransy, 
                                   &stransz);
                }
                glutPostRedisplay();
            }
            else
            {
                mouse_right = false;
            }
            break;

        default:
            break;
    }
}

void MotionFunc(int x, int y)
{
    if (mouse_left == true)
    {
        double tendx, tendy, tendz;
        ConvertToLocal(x, y, 0.01, &tendx, &tendy, &tendz);
        tendz = ProjToSphere(tendx, tendy, 1);
        MultModel(tendx, tendy, tendz, &endx, &endy,
                &endz);
        CrossProduct(startx, starty, startz, 
                endx, endy, endz, rot_axis);
        angle = (VectorLength(rot_axis[0], rot_axis[1], rot_axis[2])/
                (VectorLength(startx, starty, startz)*
                 VectorLength(endx, endy, endz)));
        glutPostRedisplay();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(camera_x, camera_y, camera_z, 0, 0, 0, 0, 1, 0);
        if(!global_rot)
            glLoadMatrixd(modelviewmat1);
        else
            glLoadMatrixd(modelviewmat2);
        glRotated((angle)*(180/3.14), rot_axis[0], rot_axis[1], 
                  rot_axis[2]);
        if(!global_rot)
            glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat1);
        else
            glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat2);
        glLoadIdentity();
    }
    if((mouse_right == true))
    {
        ConvertToLocal(x, y, 0.01, &etransx, &etransy, &etransz);
        glutPostRedisplay();
    }
}

void MultModel(double sx, double sy, double sz, double *tx, 
        double *ty, double *tz)
{
    if(!global_rot)
    {
        *tx = sx*modelviewmat1[0]+
                  sy*modelviewmat1[1]+
                  sz*modelviewmat1[2]+
                  modelviewmat1[3];
        *ty = sx*modelviewmat1[4]+
                  sy*modelviewmat1[5]+
                  sz*modelviewmat1[6]+
                  modelviewmat1[7];
        *tz = sx*modelviewmat1[8]+
                  sy*modelviewmat1[9]+
                  sz*modelviewmat1[10]+
                  modelviewmat1[11];
    }
    else
    {
        *tx = sx*modelviewmat2[0]+
                  sy*modelviewmat2[1]+
                  sz*modelviewmat2[2]+
                  modelviewmat2[3];
        *ty = sx*modelviewmat2[4]+
                  sy*modelviewmat2[5]+
                  sz*modelviewmat2[6]+
                  modelviewmat2[7];
        *tz = sx*modelviewmat2[8]+
                  sy*modelviewmat2[9]+
                  sz*modelviewmat2[10]+
                  modelviewmat2[11];
    }
}

inline double VectorLength(GLdouble x, GLdouble y, GLdouble z)
{
    return(sqrt((double)(x*x+y*y+z*z)));
}

inline void GetVector(GLdouble x1, GLdouble y1, GLdouble z1, 
                      GLdouble x2, GLdouble y2, GLdouble z2,
                      GLdouble *ret)
{
    ret[0] = x2 - x1;
    ret[1] = y2 - y1;
    ret[2] = z2 - z1;
}

void SpecialKeys(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            if(light_move)
                light_pos[1] += 0.1;
            else
                transy += 0.01;
            break;
        case GLUT_KEY_DOWN:
            if(light_move)
                light_pos[1] -= 0.1;
            else
                transy -= 0.01;
            break;
        case GLUT_KEY_RIGHT:
            if(light_move)
                light_pos[0] += 0.1;
            else
                transx += 0.01;
            break;
        case GLUT_KEY_LEFT:
            if(light_move)
                light_pos[0] -= 0.1;
            else
                transx -= 0.01;
            break;
        case GLUT_KEY_PAGE_DOWN:
            if(light_move)
                light_pos[2] -= 0.1;
            else
                transz += 0.01;
            break;
        case GLUT_KEY_PAGE_UP:
            if(light_move)
                light_pos[2] += 0.1;
            else
                transz -= 0.01;
            break;
    }
    glutPostRedisplay();
}
void KeyboardFunc(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 27:
            delete[] vert;
            delete[] norm;
            delete[] ind;
            exit(0);
        case 'w':
            wire = !wire;
            glutPostRedisplay();
            break;
        case 'n':
            non_axis = !non_axis;
            glutPostRedisplay();
            break;
        case 'v':
            shad_vol = !shad_vol;
            planar = non_axis = false;
            glutPostRedisplay();
            break;
        case 't':
            track_ball = !track_ball;
            glutPostRedisplay();
            break;
        case 'g':
            GlobalMode(!global_rot);
            glutPostRedisplay();
            break;
        case 'p':
            planar = !planar;
            shad_vol = false;
            glutPostRedisplay();
            break;
        case 'l':
            light_move = !light_move;
            glutPostRedisplay();
            break;
        case 'r':
            /*Reset all values to get the model in initial position.*/
            global_rot = mouse_left = mouse_right = 
            track_ball = false;
            modelviewmat1={1,0,0,0,
                           0,1,0,0, 
                           0,0,1,0, 
                           0,0,0,1};
            modelviewmat2={1,0,0,0, 
                           0,1,0,0, 
                           0,0,1,0, 
                           0,0,0,1};
            etransx = etransy = etransz = 0;
            stransx = stransy = stransz = 0;
            scale = scaleg = 1.0;
            light_pos[0] = light_x;
            light_pos[1] = light_y;
            light_pos[2] = light_z;
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

inline GLdouble ProjToSphere(GLdouble x, GLdouble y, int r)
{
    return sqrt(fabs((double)(r*r - x*x - y*y)));
}

void CrossProduct(GLdouble x1, GLdouble y1, GLdouble z1,
         GLdouble x2, GLdouble y2, GLdouble z2, GLdouble* ret)
{
    ret[0] = y1*z2 - z1*y2;
    ret[1] = z1*x2 - x1*z2;
    ret[2] = x1*y2 - x2*y1;
}
GLdouble DotProduct(GLdouble x1, GLdouble y1, GLdouble z1,
         GLdouble x2, GLdouble y2, GLdouble z2)
{
    return (x1*x2+y1*y2+z1*z2);
}


inline void ConvertToLocal(int x, int y, double z, GLdouble *finalx,
        GLdouble *finaly, GLdouble *finalz)
{
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat);
    glGetDoublev(GL_PROJECTION_MATRIX, projmat);
    y = viewport[3] - (GLint)y - 1;
    if(gluUnProject((GLdouble)x, (GLdouble)y, (GLdouble)z, 
            modelviewmat, projmat, viewport, finalx,
            finaly, finalz) == GL_FALSE)
    {
        cout<<"Cannot project points "<<finalx<<" "<<finaly<<" "<<
            finalz<<endl;
        exit(-1);
    }
    /*
     *This is to ensure that points clicked outside the sphere are 
     *mapped to points on the sphere.
     */
    (*finalx > 1.0)? 1.0:*finalx;
    (*finaly > 1.0)? 1.0:*finaly;
    (*finalz > 1.0)? 1.0:*finalz;
}

void GlobalMode(bool value)
{
    global_rot = gtrack_ball = value;
    ltrack_ball = !value;
}
