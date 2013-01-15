#include <iostream>
#include <math.h>
#include <assert.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "tga.h"
 
GLuint window;
GLint wwidth = 600;
GLint wHeight = 600;
gliGenericImage *imagesource = NULL;
GLint mytexturefilter = GL_NEAREST;
bool mipmap = true;
enum Mode{ CAMERA, LIGHTING, MATERIAL_DIFFUSE , MATERIAL_SPECULAR}; enum Object{CILINDRO, TEAPOT, BOLA }; int curr_mode = CAMERA; int currObj = CILINDRO; bool isPushLeftBtnMouse = false; bool shftActive = false;
 
//set up initial position of the center of the screen 
GLdouble centrx = 0.0; GLdouble ctery = 8.0; GLdouble ctrez = 0.0; 
// set up initial position of the eye 
GLdouble eyex = 0.0; GLdouble eyey = 11.4; GLdouble eyez = 32.0; 
// define the up vector (what is up for my camera?) 
GLdouble upx = 0.0; GLdouble upy = 1.0; GLdouble upz = 0.0;
 
// set up measures of the box for RGBA
GLdouble rxmn= 0.0, rymn= 0.0, rzmn= 0.0, rxmx= 0.0, rymx= 0.0; GLdouble gxmn= 0.0, gymn= 0.0, gzmn= 0.0, gxmx= 0.0, gymx= 0.0; GLdouble bxmn= 0.0, bymn= 0.0, bzmn= 0.0, bxmx= 0.0, bymx= 0.0; GLdouble sxmn= 0.0, symn= 0.0, szmn= 0.0, sxmx= 0.0, symx= 0.0; GLdouble ctrwnx = 0.0, ctrwny = 0.0, ctrwnz = 0.0;
 
// coordinates for the light window
GLdouble light0wnx = 0.0, light0wny = 0.0, light0wnz = 0.0,
    light1wnx = 0.0, light1wny = 0.0, light1wnz = 0.0,
    light2wnx = 0.0, light2wny = 0.0, light2wnz = 0.0,
    light3wnx = 0.0, light3wny = 0.0, light3wnz = 0.0;
 
GLdouble mdlmtrx[16] = {0.0}, prjctmtrx[16] = {0.0}; GLint vwprt[4] = {0};
GLdouble objx = 0.0, objy = 0.0, objz = 0.0;   
GLdouble yprev = 0.0; 
 
// variables to declare which lights are on and which one is // active at any given time
bool lght0enabled = false; bool lght1enabled = false; bool lght2enabled = true; bool lght3enabled = false; short actlight = 0;
 
// Position of the four different lights
float lght0pos[] = {-12.0, 17, 12.0, 0.0}; float lght1pos[] = {-17.0, 19, -17.0, 0.0}; float lght2pos[] = { 17.0, 19, -17.0, 0.0}; float lght3pos[] = { 12.0, 17, 12.0, 0};
 
// original intensity of the four different lights
float lght0inten[] = {0.55, 0.55, 0.55, 1.0}; float lght1inten[] = {0.55, 0.55, 0.55, 1.0}; float lght2inten[] = {0.55, 0.55, 0.55, 1.0}; float lght3inten[] = {0.55, 0.55, 0.55, 1.0};
// Ambience coefficients at the beginning
float teapot_ambientic[] = {0.42, 0.42, 0.42}; float sphere_ambientic[] = {0.42, 0.42, 0.42}; float cylinder_ambientic[] = {0.42, 0.42, 0.42};
// Shininnes coefficients
float teapotTheShinning = 0.35; float sphereTheShinning = 0.67; float cylinderTheShinning = 0.3;
// Diffuse Material coefficients at starting point
float teapotDiffuse[] = {0.8, 0.8, 0.8}; float sphereDiffuse[] = {0.8, 0.8, 0.8}; float cylinderDiffuse[] = {0.8, 0.8, 0.8};// Specular coefficients
float teapotSpec[] = {0.8, 0.8, 0.8}; float sphereSpec[] = {0.8, 0.8, 0.8}; float cylinderSpec[] = {0.8, 0.8, 0.8};

bool isinbox(const int& x, const GLint y, const GLdouble& minx, const GLdouble& maxx, 
	     const GLdouble& miny, const GLdouble& maxy){
    //returns true if (x,y) is inside the box with the given coordinates
    bool inbox = false;
    if(minx < x && x < maxx && miny < y && y < maxy)
	inbox = true;
    return (inbox);
}

void standardTGAprocedures(gliGenericImage * imagesource){
    // As soon as I read the file, I adjust the standard properties for every image
    // I do this because I noticed that this is done to all of them... almost
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // toggling with F
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mytexturefilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mytexturefilter);
    // this determines the wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    //
    if(mipmap){ // this part will be toggled
        gluBuild2DMipmaps(GL_TEXTURE_2D, imagesource->components, imagesource->width,
                          imagesource->height, imagesource->format, GL_UNSIGNED_BYTE, imagesource->pixels);
    }else{                     
        glTexImage2D(GL_TEXTURE_2D, 0, imagesource->components,
                     imagesource->width, imagesource->height, 0, imagesource->format,
                     GL_UNSIGNED_BYTE, imagesource->pixels);
    }
    glEnable(GL_TEXTURE_2D); // don't bind just yet (include here?) 
}
 
void getmyTGA(const char * filename){
    // this is the standard function that the professor sent
    gliVerbose(0);
    FILE *file;
    file = fopen(filename, "rb");
    if (file == NULL) {
        std::cerr << "could not open " << filename << std::endl;
        exit(1);
    }
    imagesource = gliReadTGA(file, filename);
    fclose(file);
    if( !imagesource ) {
        std::cerr << "failed reading " << filename << std::endl;
        exit(1);
    }
}
 
void init(void){
     // these are the first things that are done when we open the screen
     // LIGHTS!
    glLightfv(GL_LIGHT0, GL_POSITION, lght0pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lght0inten);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lght0inten);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lght0inten);
 
    glLightfv(GL_LIGHT1, GL_POSITION, lght1pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lght1inten);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lght1inten);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lght1inten);
 
    glLightfv(GL_LIGHT2, GL_POSITION, lght2pos);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lght2inten);
    glLightfv(GL_LIGHT2, GL_AMBIENT, lght2inten);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lght2inten);
 
    glLightfv(GL_LIGHT3, GL_POSITION, lght3pos);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, lght3inten);
    glLightfv(GL_LIGHT3, GL_AMBIENT, lght3inten);
    glLightfv(GL_LIGHT3, GL_SPECULAR, lght3inten);
 
    glEnable(GL_LIGHTING);
    //glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
    glFrontFace(GL_CW);
    // enable the first light behind so that it's not completely dark
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
}
 
void normalfrom3(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2,
                     GLdouble z2, GLdouble x3, GLdouble y3, GLdouble z3, GLdouble * myv){
 
    // create two vectors in the plane and a wrong version of the cross product
    // (notice that the order is different to the points)
    // this function supports the sphere and gets the messy effect on it
    GLdouble v1 = 0, v2 = 0, v3 = 0, w1 = 0, w2 = 0, w3 = 0, len = 0;
 
    v1 = x2 - x1; v2 = y2 - y1; v3 = z3 - z1;
    w1 = x3- x1; w2 = y3 - y1; w3 = z3 - z1;
    myv[0] = -(v2 * z3 - v3 * z2);
    myv[1] = (v3 * z1 - v1 * z3);
    myv[2] = -(v1 * z2 - v2 * z1);
    len = sqrt(myv[0] * myv[0] + myv[1] * myv[1] + myv[2] * myv[2]);
    myv[0] = myv[0] / len;
    myv[1] = myv[1] / len;
    myv[2] = myv[2] / len;
}
 
void createBola(GLdouble radius, GLint divs) {
    // this function achieves the same results as glSolidSphere
    GLdouble theta, phi;
    GLdouble twoPi = M_PI * 2;
    GLdouble dTheta = twoPi / divs;
    GLdouble dPhi = M_PI / divs;
    GLdouble x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
    //GLdouble (*mypointerv) [3];
    GLdouble * myv;
    myv = (GLdouble *) malloc (3);
    /** texturePos is the texture coord position of the start of the disc */
    float texPosx = 0, texPosy = 0;
    /** TextureStep is tex coord size to match angular steps */
    float texStep = 1.0f / divs;
    // finding slices of the sphere in spheric coordinates
    for (theta = 0; theta < twoPi; theta += dTheta) {
        texPosy = 0;
        for (phi = 0; phi < M_PI; phi += dPhi) // phi is the latitude, as angle from North pole axis
        {
            y1 = radius * cos(phi);
            x1 = radius * sin(phi) * cos(theta);
            z1 = radius * sin(phi) * sin(theta);
 
            y2 = radius * cos(phi);
            x2 = radius * sin(phi) * cos(theta + dTheta);
            z2 = radius * sin(phi) * sin(theta + dTheta);
 
            y3 = radius * cos(phi + dPhi);
            x3 = radius * sin(phi + dPhi) * cos(theta + dTheta);
            z3 = radius * sin(phi + dPhi) * sin(theta + dTheta);
 
            y4 = radius * cos(phi + dPhi);
            x4 = radius * sin(phi + dPhi) * cos(theta);
            z4 = radius * sin(phi + dPhi) * sin(theta);
            glBegin(GL_QUAD_STRIP);
           
            normalfrom3(x2, y2, z2, x1, y1, z1, x4, y4, z4, myv);
            glNormal3dv(myv);
            glTexCoord2f(texPosx, texPosy);
            glVertex3d(x1, y1, z1);
 
            normalfrom3(x3, y3, z3, x2, y2, z2, x1, y1, z1, myv);
            glTexCoord2f(texPosx + texStep, texPosy);
            glNormal3dv(myv);
            glVertex3d(x2, y2, z2);
 
            normalfrom3(x4, y4, z4, x3, y3, z3, x2, y2, z2, myv);
            glNormal3dv(myv);
            glTexCoord2f(texPosx + texStep, texPosy + texStep);
            glVertex3d(x4, y4, z4);
 
            normalfrom3(x1, y1, z1, x4, y4, z4, x3, y3, z3, myv);
            glNormal3dv(myv);
            glTexCoord2f(texPosx, texPosy + texStep);
            glVertex3d(x3, y3, z3);
 
            glEnd();
            texPosy += texStep;
        }
        texPosx += texStep;
    }
}
 
void createCilindro(double radius, double height, double n){
    double alpha = 0.0, dx = 0.0, dy = 0.0, dz = 0.0;
   // top
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i < n; ++i){
        alpha = 2 * M_PI * i / n;
        glNormal3f(0,1,0);
        glTexCoord2f(i / n, 1);
        glVertex3f(0.0, height, 0.0);
        dx = cos(alpha); dy = 0; dz = sin(alpha);
        glNormal3f(dx, dy, dz);
        glTexCoord2f(i / n, (radius+height)/(height+2*radius) );
        glVertex3f(radius*dx, height, radius * dz);
    }
    glEnd();
    // side
    glBegin(GL_QUAD_STRIP);
    for(int i = 0 ; i < n; ++i){
        alpha = 2 * M_PI * i/ n;
        dx = cos(alpha); dy = 0; dz = sin(alpha);
        glNormal3f(dx, dy, dz);
        glTexCoord2f( i / n, radius/(height+2*radius) );
        glVertex3f(radius*dx, 0.0, radius*dz);
        glNormal3f(dx, dy, dz);
        glTexCoord2f(i / n, (radius+height) / (height+2*radius) );
        glVertex3f(radius*dx, radius*dy + height, radius * dz);
    }
    glEnd();
}
 
// Draw the walls
static GLfloat wallVertices[24][3] = {
    // Wall West
    {-35.0, 0.0, 35.0},
    {-35.0, 0.0, -35.0},
    {-35.0, 70.0, -35.0},
    {-35.0, 70.0, 35.0},
    // Wall North
    { -35.0, 0.0, -35.0},
    { 35.0, 0.0, -35.0 },
    { 35.0, 70.0, -35.0 },
    { -35.0, 70.0, -35.0},
    // Wall East
    {35.0, 0.0, -35.0},
    {35.0, 0.0, 35.0},
    {35.0, 70.0, 35.0},
    {35.0, 70.0, -35.0},
    // Wall South
    {35.0, 0.0, 35.0},
    {-35.0, 0.0, 35.0},
    {-35.0, 70.0, 35.0},
    {35.0, 70.0, 35.0},
    // Wall Up
    {35.0, 70.0, 35.0},
    {-35.0, 70.0, 35.0},
    {-35.0, 70.0, -35.0},
    {35.0, 70.0, -35.0},
    // Wall Bottom
    {-35.0, 0.0, -35.0},
    {-35.0, 0.0, 35.0},
    {35.0, 0.0, 35.0},
    {35.0, 00.0, -35.0},
};
 
// create all the walls around
void createPared(void){
    const char* skyname[] = {"nightsky_west.tga", "nightsky_north.tga", "nightsky_east.tga",
                             "nightsky_south.tga", "nightsky_up.tga", "nightsky_down.tga"};
    for(int i = 0; i < 6; i++){
        getmyTGA(skyname[i]);
        standardTGAprocedures(imagesource);
 
        glBegin(GL_QUADS);
        glNormal3i(0,1,0);
        glTexCoord2f(0,0);
        glVertex3fv(wallVertices[4 * i]);
 
        glNormal3i(0,1,0);
        glTexCoord2f(1,0);
        glVertex3fv(wallVertices[4 * i + 1]);
       
        glNormal3i(0,1,0);
        glTexCoord2f(1,1);
        glVertex3fv(wallVertices[4 * i + 2]);
       
        glNormal3i(0,1,0);
        glTexCoord2f(0,1);
        glVertex3fv(wallVertices[4 * i + 3]);
       
        glEnd();
    }
}
 
// place light bulb
void createAllLightbulbs(void){
    glDisable(GL_TEXTURE_2D); // we just want our light to be white    
    switch(actlight){
    case 0:
        glPushMatrix();
        glTranslatef(lght0pos[0], lght0pos[1], lght0pos[2]); //place the ball in its place
        createBola(1.0, 10);
        glPopMatrix();
        break;
    case 1:
        glPushMatrix();
        glTranslatef(lght1pos[0], lght1pos[1], lght1pos[2]); //place the ball in its place
        createBola(1.0, 10);
        glPopMatrix();
        break;
    case 2:
        glPushMatrix();
        glTranslatef(lght2pos[0], lght2pos[1], lght2pos[2]); //place the ball in its place
        createBola(1.0, 10);
        glPopMatrix();
        break;
    case 3:
        glPushMatrix();
        glTranslatef(lght3pos[0], lght3pos[1], lght3pos[2]); //place the ball in its place
        createBola(1.0, 10);
        glPopMatrix();
        break;
    default:
        break;
    }
}
 
void redraw_screen(void){
    GLfloat inten[] = {1, 1, 1}; 
    glViewport(0, 0, wwidth, wHeight);
    float b_c[] = {0.5, 0.5, 0.5};
    float b_zero[] = {0, 0, 0};

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();          
    glFrustum(-1, 1, -1, 1, 1, 100);
 
    gluLookAt(eyex,eyey,eyez,
              centrx,ctery,ctrez,
              upx,upy,upz);
 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    glGetIntegerv(GL_VIEWPORT,vwprt);
    glGetDoublev(GL_PROJECTION_MATRIX, prjctmtrx);
    glGetDoublev(GL_MODELVIEW_MATRIX, mdlmtrx);
 
    // get center of cam
    gluProject(centrx, ctery, ctrez,
               mdlmtrx, prjctmtrx, vwprt,
               &ctrwnx , &ctrwny, &ctrwnz);
       
    glClearColor(0, 0, 0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
    // material teapot
    glMaterialfv(GL_FRONT, GL_AMBIENT, teapot_ambientic);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, teapotDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, teapotSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 128*teapotTheShinning);        
 
    // texture for teapot
    getmyTGA("teapot.tga");
    standardTGAprocedures(imagesource);
 
    glAlphaFunc(GL_GREATER,0);
    glEnable(GL_ALPHA_TEST);
 
    // draw the teapot
    glPushMatrix();            
    glTranslatef(-10, 2.0, 3.0);
    glutSolidTeapot(4);
    glDisable(GL_ALPHA_TEST);
 
    //material setting for cylinder
    glMaterialfv(GL_FRONT, GL_AMBIENT, cylinder_ambientic);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cylinderDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, cylinderSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 128*cylinderTheShinning);
 
    getmyTGA("barbershop.tga");
    standardTGAprocedures(imagesource);
 
    glTranslatef(17.0, 2.0, 6.0);  // place the ball in the right place
    createCilindro(3, 8, 20);
 
    //material setting for the ball            
    glMaterialfv(GL_FRONT, GL_AMBIENT, sphere_ambientic);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphereDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sphereSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 128*sphereTheShinning);
 
    getmyTGA("fractalcolor.tga");
    standardTGAprocedures(imagesource);
 
    glTranslatef(-7.0, 20.0, -18.0); 
    createBola(7.0, 30);
    glPopMatrix();
    glutPostRedisplay();
 
    // materials for light bulbs
    glMaterialfv(GL_FRONT, GL_AMBIENT, inten);
    glMaterialf(GL_FRONT, GL_SHININESS, 128);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, inten);
    glMaterialfv(GL_FRONT, GL_SPECULAR, inten);
    createAllLightbulbs();
 
    // Draw the walls
    createPared();     
 
    // Transform light locations to the coordinate system in the screen 
    //(this is for later capture with the mouse)
    gluProject(lght0pos[0], lght0pos[1], lght0pos[2],
               mdlmtrx, prjctmtrx, vwprt,
               &light0wnx , &light0wny, &light0wnz);
 
    gluProject(lght1pos[0], lght1pos[1], lght1pos[2],
               mdlmtrx, prjctmtrx, vwprt,
               &light1wnx , &light1wny, &light1wnz);
 
    gluProject(lght2pos[0], lght2pos[1], lght2pos[2],
               mdlmtrx, prjctmtrx, vwprt,
               &light2wnx , &light2wny, &light2wnz);
 
    gluProject(lght3pos[0], lght3pos[1], lght3pos[2],
               mdlmtrx, prjctmtrx, vwprt,
               &light3wnx , &light3wny, &light3wnz);
     // Set up the ortographic camera for bars
    // I got this function pretty much verbatim online
    glClear(GL_DEPTH_BUFFER_BIT);      
 
    glViewport(0, 0, wwidth, wHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();          
    glOrtho(-16, 16, -10, 30, -30,30);
 
    gluLookAt(0,0,1,
              0,0,0,
              0,1,0);
       
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
       
    glGetIntegerv(GL_VIEWPORT,vwprt);  
    glGetDoublev(GL_PROJECTION_MATRIX,prjctmtrx);
    glGetDoublev(GL_MODELVIEW_MATRIX,mdlmtrx);
 
    glMaterialfv(GL_FRONT, GL_AMBIENT, b_zero);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, b_c);
    glMaterialfv(GL_FRONT, GL_SPECULAR, b_zero);
    glMaterialf(GL_FRONT, GL_SHININESS, 0);
 
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
       
    // Red bar
    glBegin(GL_POLYGON);
    glNormal3i(0,0,1);                         
    glColor3f(0,0,0);
    glVertex3i(-15, -9, 0);
    // screen coordinates
    gluProject(-15, -9, 0, mdlmtrx, prjctmtrx, vwprt, &rxmn, &rymn, &rzmn);
    glNormal3i(0,0,1);         
    glColor3f(0,0,0);
    glVertex3i(-13, -9, 0);
    gluProject(-13,-9, 0, mdlmtrx, prjctmtrx, vwprt, &rxmx, &rymn, &rzmn);
 
    glNormal3i(0,0,1);                         
    glColor3f(1,0,0);
    glVertex3i(-13, 0, 0);
    gluProject(-13, 0, 0, mdlmtrx, prjctmtrx, vwprt, &rxmx, &rymx, &rzmn);
 
    glNormal3i(0,0,1);         
    glColor3f(1,0,0);
    glVertex3i(-15, 0, 0);             
    glEnd();
 
    //Green bar
    glBegin(GL_POLYGON);
    glNormal3i(0,0,1);                         
    glColor3f(0,0,0);
    glVertex3i(-12, -9, 0);
 
    gluProject(-12, -9, 0, mdlmtrx, prjctmtrx, vwprt, &gxmn, &gymn, &gzmn);
 
    glNormal3i(0,0,1);         
    glColor3f(0,0,0);
    glVertex3i(-10, -9, 0);
    gluProject(-10, -9, 0, mdlmtrx, prjctmtrx, vwprt, &gxmx, &gymn, &gzmn);
 
    glNormal3i(0,0,1);                         
    glColor3f(0,1,0);
    glVertex3i(-10, 0, 0);
    gluProject(-10, 0, 0, mdlmtrx, prjctmtrx, vwprt, &gxmx, &gymx, &gzmn);
 
    glNormal3i(0,0,1);         
    glColor3f(0,1,0);
    glVertex3i(-12, 0, 0);
               
    glEnd();
 
    //Blue bar 
    glBegin(GL_POLYGON);
    glNormal3i(0,0,1);                         
    glColor3f(0,0,0);
    glVertex3i(-9, -9, 0);
    gluProject(-9, -9, 0, mdlmtrx, prjctmtrx, vwprt, &bxmn, &bymn, &bzmn);
 
    glNormal3i(0,0,1);         
    glColor3f(0,0,0);
    glVertex3i(-7, -9, 0);
    gluProject(-7, -9, 0, mdlmtrx, prjctmtrx, vwprt, &bxmx, &bymn, &bzmn);
 
    glNormal3i(0,0,1);                         
    glColor3f(0,0,1);
    glVertex3i(-7, 0, 0);
    gluProject(-7, 0, 0, mdlmtrx, prjctmtrx, vwprt, &bxmx, &bymx, &bzmn);
 
    glNormal3i(0,0,1);
    glColor3f(0,0,1);
    glVertex3i(-9, 0, 0);
               
    glEnd();
 
    //Shininess bar    
    glBegin(GL_POLYGON);
    glNormal3i(0,0,1);                         
    glColor3f(.9,.9,.9);
    glVertex3i(-6, -9, 0);
    gluProject(-6, -9, 0, mdlmtrx, prjctmtrx, vwprt, &sxmn, &symn, &szmn);
 
    glNormal3i(0,0,1);         
    glColor3f(0,0,0);
    glVertex3i(-4, -9, 0);
    gluProject(-4, -9, 0, mdlmtrx, prjctmtrx, vwprt, &sxmx, &symn, &szmn);
 
    glNormal3i(0,0,1);         
    glColor3f(0,0,0);
    glVertex3i(-4, 0, 0);
    gluProject(-4, 0, 0, mdlmtrx, prjctmtrx, vwprt, &sxmx, &symx, &szmn);
 
    glNormal3i(0,0,1);
    glColor3f(.9, .9 , .9);
    glVertex3i(-6, 0, 0);
 
    glEnd();
    glDisable(GL_COLOR_MATERIAL);
 

//////////////////////////////////////////////////////////////////

    // going back to the non-ortographic camera
    glViewport(0, 0, wwidth, wHeight);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();          
    glFrustum(-1 , 1, -1, 1, 1, 100);
    gluLookAt(eyex,eyey,eyez,
              centrx,ctery,ctrez,
              upx,upy,upz);
 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    glutSwapBuffers(); 
}
 
void reshapement(int width, int height){
    wwidth = width;
    wHeight = height;
}
 
void keyboard(unsigned char key, int x, int y){
       
    switch(key){
    // assign a light bulb
    case '1':  
	std::cout << "You selected lightbulb #: "<< key  << std::endl;
	actlight = 0;
	glutPostRedisplay();
        break;
    case '2':
	std::cout << "You selected lightbulb #: "<< key  << std::endl;
	actlight = 1;
	glutPostRedisplay();
        break;
    case '3':
	std::cout << "You selected lightbulb #: "<< key  << std::endl;
	actlight = 2;
	glutPostRedisplay();
        break;
    case '4':
	std::cout << "You selected lightbulb #: "<< key  << std::endl;
	actlight = 3;
	glutPostRedisplay();
        break; 
    case 'f':
    case 'F':
	std::cout << "You changed the texture filter" << std::endl;
    if(mipmap){
	if(mytexturefilter == GL_NEAREST_MIPMAP_NEAREST)
	    mytexturefilter = GL_LINEAR_MIPMAP_LINEAR;
	else
	    mytexturefilter = GL_NEAREST_MIPMAP_NEAREST;     
	
    }else{
	if(mytexturefilter == GL_NEAREST)
	    mytexturefilter = GL_LINEAR;
	else
	    mytexturefilter = GL_NEAREST;
    }
    glutPostRedisplay();               
    break;
    case 'm':
    case 'M':
	std::cout << "You changed the mipmap option" << std::endl;
	if(mipmap){
	    mipmap = false;
	    if(mytexturefilter == GL_NEAREST_MIPMAP_NEAREST)
		mytexturefilter = GL_NEAREST;
	    else
		mytexturefilter = GL_LINEAR;
	}else{
	    mipmap = true;
	    if(mytexturefilter == GL_NEAREST)
		mytexturefilter = GL_NEAREST_MIPMAP_NEAREST;
	    else
		mytexturefilter = GL_LINEAR_MIPMAP_LINEAR;
	}
    glutPostRedisplay();
    break;
 
    // turn all lights on or off
    case 's':
    case 'S':
        if(lght0enabled && lght1enabled && lght2enabled && lght3enabled){
	    std::cout << "You turned ALL the lights on" << std::endl;
            lght0enabled = false; glDisable(GL_LIGHT0);
            lght1enabled = false; glDisable(GL_LIGHT1);
            lght2enabled = false; glDisable(GL_LIGHT2);
            lght3enabled = false; glDisable(GL_LIGHT3);
        }else{
	    std::cout << "You turned ALL the lights off" << std::endl;
            lght0enabled = true; glEnable(GL_LIGHT0);
            lght1enabled = true; glEnable(GL_LIGHT1);
            lght2enabled = true; glEnable(GL_LIGHT2);
            lght3enabled = true; glEnable(GL_LIGHT3);
        }                      
    glutPostRedisplay();
    break;
        //space bar look for the activated light and turn on/off
    case 32 :
        if(LIGHTING == curr_mode || CAMERA == curr_mode){
	    std::cout << "You changed the status on/off of the selected light" << std::endl;
            switch(actlight){
            case 0:
                if(lght0enabled){
                    glDisable(GL_LIGHT0);
                    lght0enabled = false;
                }else{
                    glEnable(GL_LIGHT0);
                    lght0enabled = true;
                }
                break;
            case 1:
                if(lght1enabled){
                    glDisable(GL_LIGHT1);
                    lght1enabled = false;                      
                }else{
                    glEnable(GL_LIGHT1);
                    lght1enabled = true;
                }
                break;
            case 2:
                if(lght2enabled){
                    glDisable(GL_LIGHT2);
                    lght2enabled = false;                      
                }else{
                    glEnable(GL_LIGHT2);
                    lght2enabled = true;
                }
                break;
            case 3:
                if(lght3enabled){
                    glDisable(GL_LIGHT3);
                    lght3enabled = false;                      
                }else{
                    glEnable(GL_LIGHT3);
                    lght3enabled = true;
                }
                break;
            default:
                break;
            }          
        } else{
            // or assign a different object to material change
	    	std::cout << "You changed your object for texture analysis" << std::endl;
            currObj = (1 + currObj)%3;
        }
            glutPostRedisplay();
        break;
       
        // Press escape to finish
    case 27:
        exit(0);
        break;
    default:
        break;
    }
       
}
 
void mouse(int button, int state, int x, int y){
    bool iclickedinabox = false;
    // get the different coordinates of the mouse click
    glGetDoublev(GL_PROJECTION_MATRIX,prjctmtrx);
    glGetIntegerv(GL_VIEWPORT,vwprt);  

    glGetDoublev(GL_MODELVIEW_MATRIX,mdlmtrx);
    gluUnProject(x, wHeight-y, ctrwnz,
                mdlmtrx, prjctmtrx, vwprt,
                &objx, &objy, &objz);
 
    if(MATERIAL_DIFFUSE == curr_mode){
        // answer to a click within the red bar
	
        if(isinbox(x, wHeight - y, rxmn, rxmx, rymn, rymx)){
            if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state){
                switch(currObj){
                case CILINDRO:
                    cylinderDiffuse[0] = (x-rymn)/(rymx-rymn);
		    std::cout << "red diffuse set to: "<< cylinderDiffuse[0] << std::endl;
                    break;
                case TEAPOT:
                    teapotDiffuse[0] = (x-rymn)/(rymx-rymn);           
                    std::cout << "red diffuse set to: " <<  teapotDiffuse[0] << std::endl;
                    break;
                case BOLA:
                    sphereDiffuse[0] = (x-rymn)/(rymx-rymn);     
		     std::cout << "red diffuse set to: " <<  sphereDiffuse[0] << std::endl;
                    break;
                default:
                    break;
                }
            }
            // answer to a click within the green bar
        }else if(isinbox(x, wHeight - y, gxmn, gxmx, gymn, gymx)){
	    if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state){
                switch(currObj){
                case CILINDRO:
                    cylinderDiffuse[1] = (x-gymn)/(gymx-gymn);
		    std::cout << "green diffuse set to: "<< cylinderDiffuse[1] << std::endl;
                    break;
                case TEAPOT:
                    teapotDiffuse[1] = (x-gymn)/(gymx-gymn);           
		    std::cout << "green diffuse set to: "<< teapotDiffuse[1] << std::endl;
                    break;
                case BOLA:
                    sphereDiffuse[1] = (x-gymn)/(gymx-gymn);             
		    std::cout << "green diffuse set to: "<< sphereDiffuse[1] << std::endl;              
                    break;
                default:
                    break;
                }
            }
            // answer to a click within the Blue bar
        }else if(isinbox(x, wHeight - y, bxmn, bxmx, bymn, bymx)){
	    if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state){
                switch(currObj){
                case CILINDRO:
                    cylinderDiffuse[2] = (x-bymn)/(bymx-bymn);
		    std::cout << "blue diffuse set to: "<< cylinderDiffuse[2] << std::endl;
                    break;
                case TEAPOT:
                    teapotDiffuse[2] = (x-bymn)/(bymx-bymn);           
		    std::cout << "blue diffuse set to: "<< teapotDiffuse[2] << std::endl;
                    break;
                case BOLA:
                    sphereDiffuse[2] = (x-bymn)/(bymx-bymn);       
		    std::cout << "blue diffuse set to: "<< sphereDiffuse[2] << std::endl;     
                    break;
                default:
                    break;
                }
            }
            // answer to a click within the shinning bar
        }else if(isinbox(x, wHeight - y, sxmn, sxmx, symn, symx)){
	    if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state){
                switch(currObj){
                case CILINDRO:
                    cylinderTheShinning = (x-symn)/(symx-symn);
		    std::cout << "Shinning for cylinder set to: "<< cylinderTheShinning << std::endl;
                    break;
                case TEAPOT:
                    teapotTheShinning = (x-symn)/(symx-symn);    
		    std::cout << "Shinning for teapot set to: "<< teapotTheShinning << std::endl;      
                    break;
                case BOLA:
                    sphereTheShinning = (x-symn)/(symx-symn);      
		    std::cout << "Shinning for sphere set to: "<< sphereTheShinning << std::endl;
                    break;
                default:
                    break;
                }
            }
        }
        //if in lighting mode, move the lights
    }    
    else if(CAMERA == curr_mode){
        if(GLUT_LEFT_BUTTON == button  && GLUT_DOWN == state){
	    std::cout << "You re-centered the screen with the mouse"  << std::endl;
            centrx = objx;
            ctery = objy;
            ctrez = objz;
        }
    }
    else if(LIGHTING == curr_mode){
        if(GLUT_LEFT_BUTTON == button &&  GLUT_UP == state && shftActive )
            shftActive = false;
        //if click left button close to light 1
        else if(isinbox(x, wHeight - y, light0wnx-5, light0wnx+5, light0wny-5, 
			light0wny + 5) &&  actlight == 0 ){
	    std::cout << "You left-clicked near lightbulb #: "<< actlight + 1  << std::endl;
	    iclickedinabox = true;                 
	}                                  
            //if click left button close to light 2
        else if( isinbox(x, wHeight - y, light1wnx-5, light1wnx+5, light1wny-5, 
			light1wny + 5) && actlight == 1 ) {
	    std::cout << "You left-clicked near lightbulb #: "<< actlight + 1  << std::endl;
	    iclickedinabox = true;
            //if click left button close to light 3
        }
	else if( isinbox(x, wHeight - y, light2wnx-5, light2wnx+5, light2wny-5, 
			light2wny + 5) && actlight == 2 ) {
	    std::cout << "You left-clicked near lightbulb #: "<< actlight + 1  << std::endl;
	    iclickedinabox = true;
            // if click left button close to light 4
        }
	else if( isinbox(x, wHeight - y, light3wnx-5, light3wnx+5, light3wny-5, 
			light3wny + 5) && actlight == 3 ) {
	    std::cout << "You left-clicked near lightbulb #: "<< actlight + 1  << std::endl;
	    iclickedinabox = true;
        }
        // in camera mode, move camera around
    }

     ////// now this is independently but still part of the lightning option
    // check that I'm not holding shift
    if(iclickedinabox){
            if(GLUT_ACTIVE_SHIFT != glutGetModifiers() ){
                if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state){
                    isPushLeftBtnMouse = true;                         
                }else if(GLUT_LEFT_BUTTON == button && GLUT_UP == state)
                    isPushLeftBtnMouse = false;
            }else{
                if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state)
                    shftActive = true;
            }
    }

    reshapement(wwidth,wHeight);
    glutPostRedisplay();
}

void keyboard2(int key, int x, int y){
    switch(key){
    case GLUT_KEY_UP:{
        // move camera to the front
        if(CAMERA == curr_mode){       
            GLdouble dx,dy,dz;
            dx = centrx - eyex;
            dy = ctery - eyey;
            dz = ctrez - eyez;
            eyex += dx/50;
            eyey += dy/50;
            eyez += dz/50;         
        }else if(LIGHTING == curr_mode){
	    std::cout << "You increased intensity of lightbulb #: "<< actlight + 1  << std::endl;
            switch(actlight){
            case 0:
                if(lght0enabled){
                    if(lght0inten[0] < 1){
                        lght0inten[0] += 0.1; lght0inten[1] += 0.1; lght0inten[2] += 0.1;
                        glLightfv(GL_LIGHT0, GL_DIFFUSE, lght0inten);
                        glLightfv(GL_LIGHT0, GL_AMBIENT, lght0inten);
                        glLightfv(GL_LIGHT0, GL_SPECULAR, lght0inten);
                    }
                }
		break;
            case 1:
                if(lght1enabled){
                    if(lght1inten[0] < 1){
                        lght1inten[0] += 0.1;     lght1inten[1] += 0.1;   lght1inten[2] += 0.1;
                        glLightfv(GL_LIGHT1, GL_DIFFUSE, lght1inten);
                        glLightfv(GL_LIGHT1, GL_AMBIENT, lght1inten);
			glLightfv(GL_LIGHT1, GL_SPECULAR, lght1inten);
                    }
                }
		break;
            case 2:
                if(lght2enabled){
                    if(lght2inten[0] < 1){
                        lght2inten[0] += 0.1; lght2inten[1] += 0.1;  lght2inten[2] += 0.1;
                        glLightfv(GL_LIGHT2, GL_DIFFUSE, lght2inten);
                        glLightfv(GL_LIGHT2, GL_AMBIENT, lght2inten);
			glLightfv(GL_LIGHT2, GL_SPECULAR, lght2inten);
                    }
                }
		break;
            case 3:
                if(lght3enabled){
                    if(lght3inten[0] < 1){
                        lght3inten[0] += 0.1; lght3inten[1] += 0.1; lght3inten[2] += 0.1;
                        glLightfv(GL_LIGHT3, GL_DIFFUSE, lght3inten);
                        glLightfv(GL_LIGHT3, GL_AMBIENT, lght3inten);
			glLightfv(GL_LIGHT3, GL_SPECULAR, lght3inten);
                    }
                }
		break;
            default:
                break;
            }
        }
        reshapement(wwidth, wHeight);
        glutPostRedisplay();
        break;         
    }
    case GLUT_KEY_DOWN:{               
        if(CAMERA == curr_mode){       
            GLdouble dx,dy,dz;
            dx = centrx - eyex;
            dy = ctery - eyey;
            dz = ctrez - eyez;
            eyex -= dx/50;
            eyey -= dy/50;
            eyez -= dz/50;                       
        }else if(curr_mode == LIGHTING) {
	    std::cout << "You decreased intensity of lightbulb #: "<< actlight + 1  << std::endl;
            switch(actlight){
            case 0:
                if(lght0enabled){
                    if(lght0inten[0] > 0){
                        lght0inten[0] -= 0.1; lght0inten[1] -= 0.1; lght0inten[2] -= 0.1;
                        glLightfv(GL_LIGHT0, GL_DIFFUSE, lght0inten);
                        glLightfv(GL_LIGHT0, GL_AMBIENT, lght0inten);
			glLightfv(GL_LIGHT0, GL_SPECULAR, lght0inten);
                    }
                }
                break;
            case 1:
                if(lght1enabled){
                    if(lght1inten[0] > 0){
                        lght1inten[0] -= 0.1; lght1inten[1] -= 0.1; lght1inten[2] -= 0.1;
                        glLightfv(GL_LIGHT1, GL_DIFFUSE, lght1inten);
                        glLightfv(GL_LIGHT1, GL_AMBIENT, lght1inten);
			glLightfv(GL_LIGHT1, GL_SPECULAR, lght1inten);
                    }
                }
                break;
            case 2:
                if(lght2enabled){
                    if(lght2inten[0] > 0){
                        lght2inten[0] -= 0.1; lght2inten[1] -= 0.1; lght2inten[2] -= 0.1;
                        glLightfv(GL_LIGHT2, GL_DIFFUSE, lght2inten);
                        glLightfv(GL_LIGHT2, GL_AMBIENT, lght2inten);
			glLightfv(GL_LIGHT2, GL_SPECULAR, lght2inten);
                    }
                }
                break;
            case 3:
                if(lght3enabled){
                    if(lght3inten[0] > 0){
                        lght3inten[0] -= 0.1; lght3inten[1] -= 0.1; lght3inten[2] -= 0.1;
                        glLightfv(GL_LIGHT3, GL_DIFFUSE, lght3inten);
                        glLightfv(GL_LIGHT3, GL_AMBIENT, lght3inten);
			glLightfv(GL_LIGHT3, GL_SPECULAR, lght3inten);
                    }
                }
                break;
            default:
                break;
            }                                                                  
        }
        reshapement(wwidth, wHeight);
        glutPostRedisplay();
        break;
    }
    default:
        break;
    }
}

void mouseMotion(int x, int y){
     GLdouble dz;
    glGetIntegerv(GL_VIEWPORT,vwprt);  
    glGetDoublev(GL_PROJECTION_MATRIX,prjctmtrx);
    glGetDoublev(GL_MODELVIEW_MATRIX,mdlmtrx);
    dz = ctrez - eyez;
 
    // if shift is pressed then move the active light in a direction perpendicular to the screen
    if(shftActive){
	// basically they will disappear in the background
        switch(actlight){
        case 0:
            gluUnProject(x, wHeight-y, light0wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
	    lght0pos[2] = objz;
            break;
        case 1:
            gluUnProject(x, wHeight-y, light0wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
	    lght1pos[2] = objz;      
            break;
        case 2:
            gluUnProject(x, wHeight-y, light0wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
	    lght2pos[2] = objz;      
            break;
        case 3:
            gluUnProject(x, wHeight-y, light0wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
	    lght3pos[2] = objz;       
            break;
        default:
            break;             
        }
        glutPostRedisplay();
 
        // if shift is not pressed then just move the lights on the screen hyperplane
    }else if(isPushLeftBtnMouse){
        switch(actlight){
        case 0:
            gluUnProject(x, wHeight-y, light0wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
            lght0pos[0] = objx;
            lght0pos[1] = objy;
            glutPostRedisplay();                       
            break;
        case 1:
            gluUnProject(x, wHeight-y, light1wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
            lght1pos[0] = objx;
            lght1pos[1] = objy;
            glutPostRedisplay();                       
            break;
        case 2:                        
            gluUnProject(x, wHeight-y, light2wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
            lght2pos[0] = objx;
            lght2pos[1] = objy;
            glutPostRedisplay();                       
            break;
        case 3:                        
            gluUnProject(x, wHeight-y, light3wnz,
                        mdlmtrx, prjctmtrx, vwprt,
                        &objx, &objy, &objz);
            lght3pos[0] = objx;
            lght3pos[1] = objy;
            glutPostRedisplay();                       
            break;
        default:
            break;
        }
    }else
        return;
    yprev = y;
}
 
void menuCallback(int choice){
    switch(choice){
    case 1:
        curr_mode = CAMERA;
        break;
    case 2:
        curr_mode = LIGHTING;
        break;
    case 3:
        curr_mode = MATERIAL_DIFFUSE;
        break;
    case 4:
        curr_mode = MATERIAL_SPECULAR;
        break;
    default:
        break;
    }
}
                                          
int main(int argc, char* argv[]) {
    GLfloat param=0.0; 
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(wwidth, wHeight);
    window = glutCreateWindow("Homework # 2: Textures & Lighting");
       
    init();
 
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboard2);  
    glutMouseFunc(mouse);
    glutReshapeFunc(reshapement); 
    glutMotionFunc(mouseMotion);             
    glutDisplayFunc(redraw_screen);

    int menu = glutCreateMenu(menuCallback);
    assert(menu);
    glutAddMenuEntry("Camera", 1);
    glutAddMenuEntry("Light Positions",2);
    glutAddMenuEntry("Material Diffuse",3);
    glutAddMenuEntry("Material Specular",4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
 
    glPixelStoref(GL_PACK_SKIP_PIXELS,param);
 
    glutMainLoop();
    return 0;  
}
