#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <GL/glx.h>     // Header file fot the glx libraries.
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <unistd.h>     // needed to sleep.
#include <math.h>
#include "/usr/include/alsa/asoundlib.h"
#include <pthread.h>
#include "glui.h"
#include <fstream>

unsigned char midimsg[4]={0,0,0,0};
pthread_t                   midiInThread;
char *device_in = NULL;
snd_rawmidi_t *handle_in = 0;
void* threadFunction(void*); 

struct {
  int use;
  int volume;
  float time;     
  float x,y,z;  //position
  float xi,yi,zi; //direction
  float r,g,b;
  int type;  //on , off
  int object;  //3d object element number
  int rotstate;
  int roty;
}typedef event;
event p[127];

struct {
  float fov;
  float xpos,ypos,zpos;
  float xat,yat,zat;
  float camaron;
  float camrotate;
  int zoom;
  float zoomtime;
}typedef camera;
camera cam;



/* ascii code for the escape key */
#define ESCAPE 27
static float colors[12][3]=
  {
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};


//GLUI declarations
int main_window;
GLUI *glui;

GLUI_Panel *event_panel;
GLUI_Translation *xy;
GLUI_Translation *z;	
GLUI_Listbox *list;
GLUI_Spinner *event_spinner;
GLUI_Button *read_event;
GLUI_Panel *anim_panel;
GLUI_Checkbox *anim1;
GLUI_Checkbox *anim2;
GLUI_Spinner *anim3;

GLUI_Spinner *bground;
GLUI_Spinner *col_spinner;
GLUI_Spinner *cam_spinner;

GLUI_Panel *file_panel;
GLUI_EditText *filename;
GLUI_Button *load;
GLUI_Button *save;


//GLUI live variables;
//event live
int eventnumber=0;
int readevent=0;
float tempz[1];
float tempxy[2];
int Type=0;
char *eventlist[] = {"","Note","Speaker","Snare","Hi_hat"};
int menuanim1=0, menuanim2=0, menuanim3=0;

GLUI_String filename_var;

float tempcolor[3];
int translatestate =0;
float scroll = 0.0f; //mathematical sens and cos temp
float rotateevent=0; //event 
float zoomtemp=0;
int zm=0; //beat_zoom global
float color[3] = {0,0,0};
int background=4;
int bg=0;


#define LOCXY 201
#define LOCZ 202
#define ANIM1 501
#define ANIM2 502
#define ANIM3 503
#define LOAD 601
#define SAVE 602
#define MESH1 301
#define READ 400
#define EVENT 401
#define COLOR 210
#define BACKG 211



/* storage for one texture  */
GLuint texture[8];


/* Image type - contains height, width, and data */
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
int ImageLoad(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
	printf("File Not Found : %s\n",filename);
	return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    printf("Width of %s: %lu\n", filename, image->sizeX);
    
    // read the height 
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    printf("Height of %s: %lu\n", filename, image->sizeY);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }
    
    // we're done.
    return 1;
}
    
// Load Bitmaps And Convert To Textures
void LoadGLTextures() {	
    // Load Texture
    Image *image1;
    
    // allocate space for texture
    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Data/note.bmp", image1)) {
	exit(1);
    }        

     glGenTextures(1, &texture[0]);
     glBindTexture(GL_TEXTURE_2D, texture[0]);  



    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    if (!ImageLoad("Data/bassdrum.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[1]);
    glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
 
    if (!ImageLoad("Data/snaredrum.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[2]);
    glBindTexture(GL_TEXTURE_2D, texture[2]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
  
    if (!ImageLoad("Data/hihats.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[3]);
    glBindTexture(GL_TEXTURE_2D, texture[3]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
  
    if (!ImageLoad("Data/background1.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[4]);
    glBindTexture(GL_TEXTURE_2D, texture[4]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
  if (!ImageLoad("Data/background2.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[5]);
    glBindTexture(GL_TEXTURE_2D, texture[5]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
 
 if (!ImageLoad("Data/background3.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[6]);
    glBindTexture(GL_TEXTURE_2D, texture[6]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
  if (!ImageLoad("Data/background4.bmp", image1)) {
	exit(1);
    }        

    // Create Texture	
    glGenTextures(1, &texture[7]);
    glBindTexture(GL_TEXTURE_2D, texture[7]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
};



void control_CALLBACK(int  control)
{
  if (control == LOCXY)
    {

      p[eventnumber].x = tempxy[0];
      p[eventnumber].y = tempxy[1];
    }
  if(control == LOCZ)
    {

      p[eventnumber].z = tempz[0];
    } 
  if(control == EVENT)
    {
      anim1->set_int_val(p[eventnumber].rotstate);
      anim2->set_int_val(p[eventnumber].roty);
    }
  if (control == MESH1)
    {
      int t_type = list->get_int_val();
      p[eventnumber].object = t_type;
    } 
  if (control == COLOR)
    {
      color[0] = tempcolor[0];
      color[1] = tempcolor[1];
      color[2] = tempcolor[2];
    } 
  
  if (control == BACKG)
    {
      background = bg + 4;
     
    }
  if (control == ANIM1)
    {
      if(p[eventnumber].rotstate)
	{p[eventnumber].rotstate = 0;}
      else p[eventnumber].rotstate = 1;	
    } 
  
  if (control == ANIM2)
    {
      if(p[eventnumber].roty)
	{p[eventnumber].roty = 0;}
      else p[eventnumber].roty = 1;
    }

  if (control == ANIM3)
    {
      cam.zoom = anim3->get_int_val();	
    } 
  
  if(control == READ)
    {
      int temp = midimsg[1];
      event_spinner->set_int_val(temp);
      anim1->set_int_val(p[eventnumber].rotstate);
      anim2->set_int_val(p[eventnumber].roty);
    }
  if(control == SAVE)
    {
      int j;
      ofstream Savefile(filename->get_text());
      for(j=0;j<127;j++)
	{
	  Savefile.write((char*)&p[j],sizeof(event));
	}
      Savefile.write((char*)&cam,sizeof(camera));
      Savefile.write((char*)&translatestate,sizeof(int));
      Savefile.write((char*)&background,sizeof(int));
      for(j=0;j<3;j++)
	{
      Savefile.write((char*)&color[j],sizeof(float));
	}
      Savefile.close();
    }
 
  if(control == LOAD)
    {
      int j;
      ifstream Loadfile(filename->get_text());
      for(j=0;j<127;j++)
	{
	  Loadfile.read((char*)&p[j],sizeof(event));
	} 
      Loadfile.read((char*)&cam,sizeof(camera));
      Loadfile.read((char*)&translatestate,sizeof(int));
      Loadfile.read((char*)&background,sizeof(int));
      for(j=0;j<3;j++)
	{
      Loadfile.read((char*)&color[j],sizeof(float));
	}
      Loadfile.close();
      GLUI_Master.sync_live_all;
    }

}


void initarray()
{  int i = 0;
  for(i=0; i<127; i++)
    {
      
      p[i].use = 0; //All particles inactive

      p[i].time = 0; 

      p[i].type = 0; //
      

      p[i].volume = 0;
      p[i].object = 1; //cambioglobalobjeto
      p[i].rotstate=0;
      p[i].roty=0;


      p[i].x += (0.1574803) *(i-32)/4.5; 
      p[i].z += 0.157480  *(i-32)/4.5; 

      p[i].r=colors[(i+1)/(127/12)][0];
      p[i].g=colors[(i+1)/(127/12)][1];
      p[i].b=colors[(i+1)/(127/12)][2];



      cam.fov=50.0;

      cam.zoom = 0;
      cam.zoomtime = 0;
      cam.xpos=0;
      cam.ypos=0;
      cam.zpos=.1;
      cam.xat=0;
      cam.yat=0;
      cam.zat=0;
      
      cam.camaron=0;
      cam.camrotate=0;
    }
}



void drawevents_events() 
{
  

  int i=0;

 for(i=0;i<127;i++)
    {
      
      if(p[i].type) //if active
	{ 

	  glPushMatrix();
	  
	  glTranslatef(p[i].x,p[i].y,p[i].z-6);

	  if(p[i].object==1)
	    {
	      glColor4f(p[i].r,p[i].g,p[i].b,1);
	    }
	  else
	    {
	      glColor4f(1,1,1,1);
	    }

	  if(p[i].rotstate)
	    {
	      glPushMatrix();
	      p[i].xi = -tan(p[i].time/4);
	      p[i].yi = (i*0.015)*sin(2*scroll);
	      p[i].zi = 1*cos(scroll)-0.5; 
	      
	      glTranslatef(0,p[i].yi,p[i].zi);
	      glTranslatef(p[i].xi,0,0);
	     } 
	  
	  if(p[i].roty)
	    {
	      glPushMatrix();
	      glRotatef(rotateevent*4,0,0,-1);
	    }  

	  if(zm && cam.zoom == i)
	    {
	      glPushMatrix();
	      glTranslatef(0,0,cam.zoomtime*0.1);
	    }

	  float vt = (p[i].volume *.009);
	  glScalef(vt,vt,vt);
	  
	  switch(p[i].object)
	    {
	      
	    case 1: 
	      
	      glBindTexture(GL_TEXTURE_2D,texture[0]);
	     glBegin(GL_QUADS);
	     glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,0.0f); glVertex3f(1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f);
	     glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f,1.0f,1.0f);
	     glEnd();
	     
	     break;
	      
	     
	    case 2:    
	      glBindTexture(GL_TEXTURE_2D,texture[1]);
	     glBegin(GL_QUADS);
	     glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,0.0f); glVertex3f(1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f);
	     glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f,1.0f,1.0f);
	     glEnd();
	      
		 break;
		 
	    case 3: 
	      glBindTexture(GL_TEXTURE_2D,texture[2]);
	     glBegin(GL_QUADS);
	     glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,0.0f); glVertex3f(1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f);
	     glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f,1.0f,1.0f);
	     glEnd();
	     break;
	      
	    case 4:  
	      glBindTexture(GL_TEXTURE_2D,texture[3]);
	     glBegin(GL_QUADS);
	     glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,0.0f); glVertex3f(1.0f,-1.0f,1.0f);
	     glTexCoord2f(1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f);
	     glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f,1.0f,1.0f);
	     glEnd();
	      break;
	      
	    case 0:    break; 
	      
	    DEFAULT: break;
	    }

	   if(p[i].rotstate)
	     {
	       glPopMatrix();
	     }

	   if(p[i].roty)
	     {
	       glPopMatrix();
	     }
	   
	   if(zm && cam.zoom == i)
	     {
	       glPopMatrix();
	     }

       	   glPopMatrix();

	   p[i].time += 0.05;
	   
	   if(p[i].time == 1000)
	     {
	       p[i].time = 0;
	     }
	}
    }
 
}
 

void drawevents()
{ 

  if(glutGetWindow()!=main_window)
    glutSetWindow(main_window);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();                     
  gluPerspective(cam.fov,9/6,0.1f,200.0f);


  glMatrixMode(GL_MODELVIEW); 
  glLoadIdentity(); 
  gluLookAt(cam.xpos,cam.ypos, cam.zpos,

		cam.xat, cam.yat, cam.zat,
		
		0.0, 1.0, 0.0);


 if(p[cam.zoom].type==0 && cam.zoomtime!=0)
    {
      zm = 1;
      glPushMatrix();
      cam.zoomtime -= 0.5;
      glTranslatef(0,0,cam.zoomtime);
    }

 if(p[cam.zoom].type==0 && cam.zoomtime==0)
   { z=0;}

 if(p[cam.zoom].type==1  && cam.zoom!=0)
       {
	 zm = 1;
	 glPushMatrix();
	 glTranslatef(0,0,cam.zoomtime);
	 if(cam.zoomtime > p[cam.zoom].volume * 0.12)
	   {cam.zoomtime -= 0.5;}
	 else cam.zoomtime += 0.5;
	 
       }

 if(translatestate) 
    {
      glPushMatrix();
      glRotatef(cam.camrotate,0,1,0);
     
    }

 glBindTexture(GL_TEXTURE_2D, texture[background]);
 
 glBegin(GL_QUADS);

  //Scenario

 glColor3f(0.0,0,0);	  
 
 glTexCoord2f(0.0f, 0.0f);glVertex3f(50.0f,10,-50.0f);
 glColor3f(color[0],color[1],color[2]); 
 glTexCoord2f(1.0f, 0.0f);glVertex3f(-50.0f,10,-50.0f);
 
 glColor3f(color[0],color[1],color[2]);
 
 glTexCoord2f(1.0f, 1.0f);glVertex3f(-50.0f,-10.0f, -50.0f);
 glTexCoord2f(0.0f, 1.0f);glVertex3f( 50.0f,- 10.0f,-50.0f);
 
 
 glColor3f(color[0],color[1],color[2]);
 glTexCoord2f(0.0f, 0.0f);glVertex3f(50.0f, 10.0f,50.0f);
 glColor3f(.0,.0,.0);
 glTexCoord2f(1.0f, 0.0f);glVertex3f(-50.0f, 10.0f,50.0f);
 
 glColor3f(color[0],color[1],color[2]);
 
 glTexCoord2f(1.0f, 1.0f);glVertex3f( -50.0f,- 10.0f,50.0f);
 glTexCoord2f(0.0f, 1.0f);glVertex3f(50.0f,- 10.0f,50.0f);
 
 glColor3f(.0,.0,.0);
 glTexCoord2f(0.0f, 0.0f);glVertex3f(-50.0f, 10.0f,50.0f);
 glColor3f(color[0],color[1],color[2]);
 glTexCoord2f(1.0f, 0.0f);glVertex3f(-50.0f, 10.0f,-50.0f);
 
 glColor3f(color[0],color[1],color[2]);
 glTexCoord2f(1.0f, 1.0f);glVertex3f(-50.0f,-10.0f,-50.0f);
 glTexCoord2f(0.0f, 1.0f);glVertex3f(-50.0f,- 10.0f,50.0f);
  
  
glColor3f(color[0],color[1],color[2]);
  glTexCoord2f(0.0f, 0.0f);glVertex3f(50.0f, 10.0f,50.0f);
glColor3f(.0,.0,.0);
  glTexCoord2f(1.0f, 0.0f);glVertex3f(50.0f, 10.0f,-50.0f);

  glColor3f(color[0],color[1],color[2]);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f,-10.0f, -50.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f,- 10.0f,50.0f);


 
   
  glColor3f(color[0],color[1],color[2]);
   
   glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f,50.0f);
   glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f,-10.0f,50.0f);
   glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f,-10.0f, -50.0f);
   glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f,- 10.0f,-50.0f);

    glColor3f(color[0],color[1],color[2]);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, 10.0f,50.0f);
    glColor3f(.0,.0,.0);
   glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f,10.0f,50.0f);
   glColor3f(color[0],color[1],color[2]);
   glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f,10.0f, -50.0f);
   glColor3f(.0,.0,.0);
   glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f,10.0f,-50.0f);
 
   glEnd();
  
   if(translatestate) 
     {
      glPopMatrix(); 
     }


   if(zm)
     {
       glPopMatrix();
     }

  drawevents_events(); //midieventsdrawin'
 
  cam.camrotate +=0.15;
  rotateevent += 0.2;
  scroll += 0.02;

  glutSwapBuffers();
}


  

void InitGL(int Width, int Height)	        
{
  LoadGLTextures();				// Load The Texture(s) 
  glEnable(GL_TEXTURE_2D);			// Enable Texture Mapping
    
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0);		
	
  glDisable(GL_DEPTH_TEST);			// Enables Depth Testing

  glShadeModel(GL_SMOOTH);
  
  glEnable(GL_BLEND);               // Enable Blending
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);	
  
  GLfloat LightAmbient[]= { .8f, 0.8f, 0.8f, 1.0f }; 	
  GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };		
  GLfloat LightPosition[]= { 0.0f, 5.0f, 0.0f, 1.0f };

  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		
  glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);		
   glEnable(GL_LIGHT1);			

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();			                 
  
  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,200.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
 
initarray();
}


void ReSizeGLScene(int Width, int Height)
{
    if (Height==0)				
	Height=1;
    glViewport(0, 0, Width, Height);		
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,200.0f);
    glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */


void DrawGLScene()

{ 
  drawevents(); 
  glutSwapBuffers();
}


/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{
  switch(key)
    {
    case 27:
    case 'q':
            exit(0);                   
	    break;
    case '1':
      glui->show();break;
    case '2':
      glui->hide();break;
      
    case 'a':
      cam.yat -= 0.0005;
      break;
      
    case 'z':
      cam.yat += 0.0005;
      break;

  case 's':
      cam.ypos -= 0.0005;
      break;
      
    case 'x':
      cam.ypos += 0.0005;
      break;


    } 
}

int main(int argc, char **argv) 
{  
  int i, status; 
  int err;
  
	if(argc==1)
	  { 
	    fprintf(stderr, "usage: ./midi -hw:x,x\n");
	    exit(0);
	  }
	
	device_in=argv[1];
	fprintf(stderr,"device %s\n",device_in);
	
	if(device_in)
	  {
	  err=snd_rawmidi_open(&handle_in,NULL,device_in,0);
	  if (err) 
	    { 
	      fprintf(stderr,"No pude abrir el dispositive: %d\n", device_in,err);
	    }
	}

	status = pthread_create(&midiInThread, NULL, threadFunction, NULL);
	if (status == -1) {
	  printf("Error: unable to create MIDI input thread.\n");
	  exit(1);}

	//OPENGL initialization
	glutInit(&argc, argv);  
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  
	glutInitWindowSize(800, 600);  
	glutInitWindowPosition(120, 0);  
	main_window = glutCreateWindow("3S.M.I.D.I.");  
	glutDisplayFunc(&DrawGLScene);  
	//	glutFullScreen();

	glutKeyboardFunc(&keyPressed);
	glutReshapeFunc(&ReSizeGLScene);

	InitGL(800, 600);

	//MENU SYS
	
	glui = GLUI_Master.create_glui_subwindow(main_window,GLUI_SUBWINDOW_RIGHT);
	GLUI_Panel *global_panel = glui->add_panel("Global");

	col_spinner =  glui->add_spinner_to_panel
	  (global_panel,"R:",GLUI_SPINNER_FLOAT,&tempcolor[0],210, control_CALLBACK);
	col_spinner->set_int_limits(0,1,GLUI_LIMIT_CLAMP);
	col_spinner->set_speed(.9);

	col_spinner =  glui->add_spinner_to_panel
	  (global_panel,"G:",GLUI_SPINNER_FLOAT,&tempcolor[1],210, control_CALLBACK);
	col_spinner->set_int_limits(0,1,GLUI_LIMIT_CLAMP);
	col_spinner->set_speed(.9);

	col_spinner =  glui->add_spinner_to_panel
	  (global_panel,"B:",GLUI_SPINNER_FLOAT,&tempcolor[2],210, control_CALLBACK);
	col_spinner->set_int_limits(0,1,GLUI_LIMIT_CLAMP);
	col_spinner->set_speed(.9);

	cam_spinner =  glui->add_spinner_to_panel
	  (global_panel,"Cam fov:",GLUI_SPINNER_FLOAT,&cam.fov);
	cam_spinner->set_int_limits(20,200,GLUI_LIMIT_CLAMP);
	cam_spinner->set_speed(10);

	glui->add_separator_to_panel(global_panel);

	bground =  glui->add_spinner_to_panel
	  (global_panel,"Background:",GLUI_SPINNER_INT,&bg,211,control_CALLBACK);
	bground->set_int_limits(0,4,GLUI_LIMIT_WRAP);
	bground->set_speed(.009);

	glui->add_checkbox_to_panel(global_panel,"Translate" , &translatestate);

	glui->add_button_to_panel(global_panel,"Quit",0,(GLUI_Update_CB)exit);
	glui->add_separator();

	///events

	event_panel = glui->add_rollout("Events",false);
	glui->add_separator_to_panel(event_panel);

	event_spinner =  glui->add_spinner_to_panel
	  (event_panel,"Note#:",GLUI_SPINNER_INT, &eventnumber,401,control_CALLBACK);

	event_spinner->set_int_limits(0,128,GLUI_LIMIT_WRAP);
	event_spinner->set_speed(0.1);
	read_event = glui->add_button_to_panel(event_panel,"READ MIDI",400, control_CALLBACK);

	glui->add_separator_to_panel(event_panel);

	anim_panel = glui->add_panel_to_panel(event_panel,"Animation");
	anim1 = glui->add_checkbox_to_panel(anim_panel,"Movement" ,&menuanim1,501, control_CALLBACK);
	anim2 = glui->add_checkbox_to_panel(anim_panel,"Rotate_Spin" , &menuanim2,502, control_CALLBACK);
	anim3 = glui->add_spinner_to_panel(anim_panel,"Beat_Zoom", GLUI_SPINNER_INT, &menuanim3,503,control_CALLBACK);
	event_spinner->set_int_limits(0,128,GLUI_LIMIT_WRAP);
	event_spinner->set_speed(0.1);

	xy =	  glui->add_translation_to_panel
	  (event_panel,"XY", GLUI_TRANSLATION_XY, tempxy, 201, control_CALLBACK);
	z =	  glui->add_translation_to_panel
	  (event_panel,"Z",  GLUI_TRANSLATION_Z, tempz, 202, control_CALLBACK);

	xy->set_speed(0.02);
	z->set_speed(0.02);

	list = glui->add_listbox_to_panel(event_panel,"mesh:",
					  &Type,301,control_CALLBACK);
	
	for(int cnt=0;cnt<5;cnt++)
	
	  {list->add_item(cnt,eventlist[cnt]);
	  }

	glui->add_separator_to_panel(event_panel);
	//SAVE_LOAD_MENU


	glui->add_separator();
	
	file_panel = glui->add_panel("Load / Save");

	filename = glui->add_edittext_to_panel(file_panel,"Name:",GLUI_EDITTEXT_TEXT,&filename_var);
	glui->add_separator_to_panel(file_panel);
	load = glui->add_button_to_panel(file_panel,"Load",601, control_CALLBACK);
	save = glui->add_button_to_panel(file_panel,"Save",602, control_CALLBACK);
	glui->add_separator_to_panel(file_panel);
 

	glui->set_main_gfx_window(main_window);
	GLUI_Master.set_glutIdleFunc(drawevents);

	glutMainLoop();
}



void* threadFunction(void* x) {
  
  int status2;
   int i; 

  while (1) 
      {

      status2 = snd_rawmidi_read(handle_in,&midimsg[0],sizeof(midimsg[0]));
      if (status2 < 0) 
	{
         printf("Error reading %s\n", handle_in);
         exit(1);
	}

      switch(midimsg[0])
	{
	  
	case 153:
	 
	  midimsg[3]=1;
	  fprintf(stderr, "NOTEON_ %d ",midimsg[0]);
	  snd_rawmidi_read(handle_in,&midimsg[1],1);
	  fprintf(stderr, "Note = %d ",midimsg[1]);
	  snd_rawmidi_read(handle_in,&midimsg[2],1);
	  fprintf(stderr, "Volume = %d\n",midimsg[2]);
	  
	 
	  
	  if(midimsg[2]!=0) {
	    p[midimsg[1]].type = 1;
	    p[midimsg[1]].volume = midimsg[2]; 
	    p[midimsg[1]].use = 1;}
	  
	  else
	    {
	      p[midimsg[1]].use = 0; p[midimsg[1]].type = 0; p[midimsg[1]].time = 0;
	    }
	 

	  break;

	  case 137:
	    
	    midimsg[3]=2;
	    fprintf(stderr, "Note OFF %d \n",midimsg[0]); 
	     snd_rawmidi_read(handle_in,&midimsg[1],1);
	     fprintf(stderr, "Noteoff = %d ",midimsg[1]);
	    snd_rawmidi_read(handle_in,&midimsg[2],1);
	    fprintf(stderr, "Volumeoff = %d\n",midimsg[2]);
	    p[midimsg[1]].use = 0; p[midimsg[1]].type = 0; p[midimsg[1]].time = 0;
	    break;

	    
	default: 
	  if( midimsg[0] < 127 && midimsg[3]==1)
	    {
	      
	      midimsg[1] = midimsg[0];
	      fprintf(stderr, "NoteON2 = %d ",midimsg[1]);
	      snd_rawmidi_read(handle_in,&midimsg[2],1);
	      fprintf(stderr, "Volume = %d\n",midimsg[2]);
	      	    
	      p[midimsg[1]].type = 1;
	    }

	  if(midimsg[2]!=0) 
	    {
	      p[midimsg[1]].volume = midimsg[2]; 
	      p[midimsg[1]].use = 1;
	    }
	
	  else
	    {
	      p[midimsg[1]].use = 0; p[midimsg[1]].type = 0; p[midimsg[1]].time = 0;
	    }
	  
	  if( midimsg[0] < 127 && midimsg[3]==2)
	    {
	      
	      midimsg[1] = midimsg[0];
	      fprintf(stderr, "NoteOFF2 = %d ",midimsg[1]);
	      snd_rawmidi_read(handle_in,&midimsg[2],1);
	      fprintf(stderr, "VolumeFF = %d\n",midimsg[2]);
	       p[midimsg[1]].use = 0;	    
	      p[midimsg[1]].type = 0; p[midimsg[1]].time = 0;
	    }
	  break;
	}                                 
      
      }//while 1end
}//function thread end
   
   
	
	
