#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <math.h>

#include "Textures/All_Textures.ppm"
#include "Textures/sky.ppm"
#include "Textures/title.ppm"
#include "Textures/won.ppm"
#include "Textures/lost.ppm"
#include "Textures/sprites.ppm"

#define mapX  8      
#define mapY  8      
#define mapS 64      

int mapW[]=          
{
 1,1,1,1,2,2,2,2,
 6,0,0,1,0,0,0,2,
 1,0,0,4,0,2,0,2,
 1,5,4,5,0,0,0,2,
 2,0,0,0,0,0,0,1,
 2,0,0,0,0,1,0,1,
 2,0,0,0,0,0,0,1,
 1,1,1,1,1,1,1,1,	
};

int mapF[]=          //floors
{
 0,0,0,0,0,0,0,0,
 0,0,0,0,2,2,2,0,
 0,0,0,0,6,0,2,0,
 0,0,8,0,2,7,6,0,
 0,0,2,0,0,0,0,0,
 0,0,2,0,8,0,0,0,
 0,1,1,1,1,0,8,0,
 0,0,0,0,0,0,0,0,	
};

int mapC[]=          //ceiling
{
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,4,2,4,0,0,0,0,
 0,0,2,0,0,0,0,0,
 0,0,2,0,0,0,0,0,
 0,0,0,0,0,0,0,0,	
};

typedef struct       //All veriables per sprite
{
 int type;           //static, key, enemy
 int state;          //on off
 int map;            //texture to show
 float x,y,z;        //position
}sprite; sprite sp[4];
int depth[120];      //hold wall line depth to compare for sprite depth


void saveLevel(const char* filename){


    FILE* f = fopen(filename,"w");


    if(!f) return;


    int x,y;


    for(y=0;y<mapY;y++){


        for(x=0;x<mapX;x++) { fprintf(f,"%d ", mapW[y*mapX+x]); }


        fprintf(f,"\n");


    }


    for(y=0;y<mapY;y++){


        for(x=0;x<mapX;x++) { fprintf(f,"%d ", mapF[y*mapX+x]); }


        fprintf(f,"\n");


    }


    for(y=0;y<mapY;y++){


        for(x=0;x<mapX;x++) { fprintf(f,"%d ", mapC[y*mapX+x]); }


        fprintf(f,"\n");


    }


    fclose(f);


    printf("Level saved to %s\n", filename);


}





void loadLevel(const char* filename){


    FILE* f = fopen(filename,"r");


    if(!f) return;


    int x,y;


    for(y=0;y<mapY;y++){ for(x=0;x<mapX;x++) { fscanf(f,"%d", &mapW[y*mapX+x]); }}


    for(y=0;y<mapY;y++){ for(x=0;x<mapX;x++) { fscanf(f,"%d", &mapF[y*mapX+x]); }}


    for(y=0;y<mapY;y++){ for(x=0;x<mapX;x++) { fscanf(f,"%d", &mapC[y*mapX+x]); }}


    fclose(f);


    printf("Level loaded from %s\n", filename);


}





//PLAYER


float degToRad(float a) { return a*M_PI/180.0;}


float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}





float px,py,pdx,pdy,pa;
float frame1,frame2,fps;





void drawMap2D()


{


 int x,y,xo,yo;


 for(y=0;y<mapY;y++)


 {


  for(x=0;x<mapX;x++)


  {


   if(mapW[y*mapX+x]>0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}


   xo=x*mapS; yo=y*mapS;


   glBegin(GL_QUADS); 


   glVertex2i( xo+1, yo+1); 


   glVertex2i( xo+1, mapS+yo-1); 


   glVertex2i( mapS+xo-1, mapS+yo-1);  


   glVertex2i( mapS+xo-1, yo+1); 


   glEnd();


  } 


 } 


}





void drawPlayer2D()


{


 glColor3f(1,1,0);   glPointSize(8);    glLineWidth(4);


 glBegin(GL_POINTS); glVertex2i(px,py); glEnd();


 glBegin(GL_LINES);  glVertex2i(px,py); glVertex2i(px+pdx*20,py+pdy*20); glEnd();


}





int isWall(float x, float y) {


    int mx = (int)x >> 6;


    int my = (int)y >> 6;


    if(mx < 0 || mx >= mapX || my < 0 || my >= mapY) return 1;


    return mapW[my*mapX+mx] > 0;


}





void Buttons(unsigned char key,int x,int y)


{


 float nx, ny;


  if(key=='a'){ pa+=5; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));}	


  if(key=='d'){ pa-=5; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));}  


 if(key=='w'){ 


     nx = px + pdx*5;


     ny = py + pdy*5;


     if(!isWall(nx, ny)) { px = nx; py = ny; }


 }


 if(key=='s'){ 


     nx = px - pdx*5;


     ny = py - pdy*5;


     if(!isWall(nx, ny)) { px = nx; py = ny; }


 }


 if(key=='e') //open doors


 { 


  int xo=0; if(pdx<0){ xo=-25;} else{ xo=25;}


  int yo=0; if(pdy<0){ yo=-25;} else{ yo=25;} 


  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;


  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;


  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}


 }


 if(key=='l'){ loadLevel("level.txt"); }


 if(key=='k'){ saveLevel("level.txt"); }


 glutPostRedisplay();


}

void drawSky()     //draw sky and rotate based on player rotation
{int x,y;
 for(y=0;y<40;y++)
 {
  for(x=0;x<120;x++)
  {
   int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120; //return 0-120 based on player angle
   int pixel=(y*120+xo)*3;
   int red   =sky[pixel+0];
   int green =sky[pixel+1];
   int blue  =sky[pixel+2];
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+530,y*8); glEnd();
  }	
 }
}

void drawSprite()
{
 int x,y,s;
 if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30){ sp[0].state=0;} //pick up key 	
 if(px<sp[3].x+30 && px>sp[3].x-30 && py<sp[3].y+30 && py>sp[3].y-30){ printf("You Lose!\n");} //enemy kills

 //enemy attack
 int spx=(int)sp[3].x>>6,          spy=(int)sp[3].y>>6;          //normal grid position
 int spx_add=((int)sp[3].x+15)>>6, spy_add=((int)sp[3].y+15)>>6; //normal grid position plus     offset
 int spx_sub=((int)sp[3].x-15)>>6, spy_sub=((int)sp[3].y-15)>>6; //normal grid position subtract offset
 if(sp[3].x>px && mapW[spy*8+spx_sub]==0){ sp[3].x-=0.04*fps;}
 if(sp[3].x<px && mapW[spy*8+spx_add]==0){ sp[3].x+=0.04*fps;}
 if(sp[3].y>py && mapW[spy_sub*8+spx]==0){ sp[3].y-=0.04*fps;}
 if(sp[3].y<py && mapW[spy_add*8+spx]==0){ sp[3].y+=0.04*fps;}

 for(s=0;s<4;s++)
 {
  float sx=sp[s].x-px; //temp float variables
  float sy=sp[s].y-py;
  float sz=sp[s].z;

  float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
  float a=sy*CS+sx*SN; 
  float b=sx*CS-sy*SN; 
  sx=a; sy=b;

  sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
  sy=(sz*108.0/sy)+( 80/2);

  int scale=32*80/b;   //scale sprite based on distance
  if(scale<0){ scale=0;} if(scale>120){ scale=120;}  

  //texture
  float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;

  for(x=sx-scale/2;x<sx+scale/2;x++)
  {
   t_y=31;
   for(y=0;y<scale;y++)
   {
    if(sp[s].state==1 && x>0 && x<120 && b<depth[x])
    {
     int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
     int red   =sprites[pixel+0];
     int green =sprites[pixel+1];
     int blue  =sprites[pixel+2];
     if(red!=255, green!=0, blue!=255) //dont draw if purple
     {
      glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+530,sy*8-y*8); glEnd(); //draw point 
     }
     t_y-=t_y_step; if(t_y<0){ t_y=0;}
    }
   }
   t_x+=t_x_step;
  }
 }
}

void drawRays3D()
{
 float mapEditScale = 0.25;
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  

  while(dof<8) 
  {
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  }
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
 
  while(dof<8) 
  {
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  }
  
  float shade=1;
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}
  
  //---Draw 2D Ray--- 
  glLineWidth(1); glColor3f(0,1,0); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 320 - (lineH>>1);                                               //line offset

  depth[r]=disH; //save this line's depth
  //---draw walls---
  int y;
  float ty=ty_off*ty_step;
  float tx;
  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  for(y=0;y<lineH;y++)
  {
   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =All_Textures[pixel+0]*shade;
   int green =All_Textures[pixel+1]*shade;
   int blue  =All_Textures[pixel+2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+530,y+lineOff); glEnd();
   ty+=ty_step;
  }
 
  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   =All_Textures[pixel+0]*0.7;
  int green =All_Textures[pixel+1]*0.7;
  int blue  =All_Textures[pixel+2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+530,y); glEnd();

 //---draw ceiling---
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   =All_Textures[pixel+0];
  green =All_Textures[pixel+1];
  blue  =All_Textures[pixel+2];
  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+530,640-y); glEnd();}
 }
 
 ra=FixAng(ra-0.5);                                                               //go to next ray
 }
}

void mouse(int button, int state, int x, int y)
{

    if(state==GLUT_DOWN){
        int mx = x/mapS;
        int my = y/mapS;
        if(mx>=0 && mx<mapX && my>=0 && my<mapY){
            int idx = my*mapX+mx;
            mapW[idx] = (mapW[idx] + 1) % 9; // cycle through 9 wall types (0-8)
            glutPostRedisplay();
        }
    }
}

void init()
{
 glClearColor(0.3,0.3,0.3,0);
 gluOrtho2D(0,1024,510,0);
 px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));
 
 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=5*64;   sp[0].z=20; //key
 sp[1].type=2; sp[1].state=1; sp[1].map=1; sp[1].x=1.5*64; sp[1].y=4.5*64; sp[1].z= 0; //light 1
 sp[2].type=2; sp[2].state=1; sp[2].map=1; sp[2].x=3.5*64; sp[2].y=4.5*64; sp[2].z= 0; //light 2
 sp[3].type=3; sp[3].state=1; sp[3].map=2; sp[3].x=2.5*64; sp[3].y=2*64;   sp[3].z=20; //enemy
}

void display()
{
 //frames per second
 frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
 drawMap2D();
 drawPlayer2D();
 drawSky();
 drawRays3D();
 drawSprite();
 glutSwapBuffers();
 glutPostRedisplay();
}

int main(int argc, char* argv[])
{
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(1024,510);
 glutCreateWindow("raycast engine with editor");
 init();
 glutDisplayFunc(display);
 glutKeyboardFunc(Buttons);
 glutMouseFunc(mouse);
 glutMainLoop();
}
