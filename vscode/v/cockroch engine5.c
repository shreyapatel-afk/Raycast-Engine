#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>

/*
#include "Textures/All_Textures.ppm"
#include "Textures/sky.ppm"
#include "Textures/title.ppm"
#include "Textures/won.ppm"
#include "Textures/lost.ppm"
#include "Textures/sprites.ppm"
#include "Textures/All_Buttons.ppm"
*/

void drawText(int x, int y, char *string);

#include <io.h>

char levelFiles[10][20];
int numLevelFiles = 0;
int wallTiles[] = {1, 2, 3, 4, 5, 6};
int floorTiles[] = {1, 3, 5, 7};
int ceilingTiles[] = {1, 3, 5};

void findFiles()
{
    struct _finddata_t file;
    intptr_t hFile;
    numLevelFiles = 0;
    if ((hFile = _findfirst("level_*.h", &file)) != -1L)
    {
        do
        {
            strcpy(levelFiles[numLevelFiles], file.name);
            numLevelFiles++;
        } while (_findnext(hFile, &file) == 0 && numLevelFiles < 10);
        _findclose(hFile);
    }
}

float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}
float distance(float ax,float ay,float bx,float by,float ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}
float px,py,pdx,pdy,pa;
float frame1,frame2,fps;
int gameState=-2, timer=0; //game state. init, start screen, game loop, win/lose
float fade=0;              //the 3 screens can fade up from black

int currentMap=0;      //0=walls, 1=floor, 2=ceiling
int currentTexture=0;  //texture id number, 0=empty    
int numSprite=2;       //total number of sprites
int buttonState=0;     //which UI button is pressed  
int dragItem=0;        //state if we are dragging an item
int currrentLevel=1;   //level that we can save in a file

typedef struct
{
 int w,a,d,s;                     //button state on off
}ButtonKeys; ButtonKeys Keys;

//-----------------------------MAP----------------------------------------------
#include<stdio.h>    //for printf
int mapX=16;         //map width
int mapY=10;         //map height
#define mapS 64      //map cube size

                     //Edit these 3 arrays with values 0-4 to create your own level! 
int mapW[17*13];     //walls
int mapF[17*13];     //floors
int mapC[17*13];     //ceiling


typedef struct       //All veriables per sprite
{
 int type;           //static, key, enemy
 int state;          //on off
 int map;            //texture to show
 float x,y,z;        //position
 int r,g,b;          //color
}sprite; sprite sp[32];
sprite backup_sp[32];
int depth[120];      //hold wall line depth to compare for sprite depth
float backup_px, backup_py, backup_pa;


void addTextures(int x,int y)
{
 //convert mouse to larger pixel screen
 x=x/48;
 y=y/48;
 //can't add outside of map area
 if(x>=mapX || y>=mapY){ return;}
 //add on click or dragging  
 int arrayPosition=x+y*mapX;
 if(currentMap==0){ mapW[arrayPosition]=currentTexture;}
 if(currentMap==1){ mapF[arrayPosition]=currentTexture;}
 if(currentMap==2){ mapC[arrayPosition]=currentTexture;}	
}


void save() 
{int x,y;
 char fileName[16]; 
 snprintf(fileName,sizeof(fileName),"level_%d.h",currrentLevel); 

 FILE *fp = fopen(fileName,"w");
 if(fp == NULL){ return;}

 //map width and height
 fprintf(fp,"%i,%i,",mapX,mapY);

 //player info
 fprintf(fp,"%f,%f,%f,", px, py, pa);

 //number of sprites
 fprintf(fp,"%i, ", numSprite);

 //sprites
 for(x=0;x<numSprite;x++)
 { 
  fprintf(fp,"%i,%i,%i, %f,%f,%f, %i,%i,%i, ",
  sp[x].type,sp[x].state,sp[x].map, 
  sp[x].x,sp[x].y,sp[x].z, 
  sp[x].r,sp[x].g,sp[x].b);
 }

 //walls, floor, ceiling
 for(x=0;x<mapX*mapY;x++)
 { 
  fprintf(fp,"%i,%i,%i,", mapW[x],mapF[x],mapC[x]);
 }

 //close
 fclose(fp);
 //print if saved
 printf("saved level: %i\n",currrentLevel);
}


void load() 
{int x,y;
 char fileName[16];
 snprintf(fileName,sizeof(fileName),"level_%i.h",currrentLevel);
 FILE *fp = fopen(fileName,"r");
 //file doesn't exist
 if(fp == NULL){ printf("no file to load\n"); return;}

 char c;
 //map width and height
 fscanf(fp,"%i",&mapX); c=getc(fp); 
 fscanf(fp,"%i",&mapY); c=getc(fp); 

 //player variables 
 fscanf(fp,"%f",&px); c=getc(fp);   
 fscanf(fp,"%f",&py); c=getc(fp);     
 fscanf(fp,"%f",&pa); c=getc(fp); 
 pdx= cos(degToRad(pa)); 
 pdy=-sin(degToRad(pa));

 //number of sprites
 fscanf(fp,"%i",&numSprite); c=getc(fp); 

 //sprites
 for(x=0;x<numSprite;x++)
 { 
  fscanf(fp,"%i",&sp[x].type);  c=getc(fp);
  fscanf(fp,"%i",&sp[x].state); c=getc(fp);
  fscanf(fp,"%i",&sp[x].map);   c=getc(fp);
  fscanf(fp,"%f",&sp[x].x);     c=getc(fp);
  fscanf(fp,"%f",&sp[x].y);     c=getc(fp);
  fscanf(fp,"%f",&sp[x].z);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].r);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].g);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].b);     c=getc(fp);
 }

 //walls, floor, ceiling
 for(x=0;x<mapX*mapY;x++)
 { 
  fscanf(fp,"%i",&mapW[x]); c=getc(fp);
  fscanf(fp,"%i",&mapF[x]); c=getc(fp);
  fscanf(fp,"%i",&mapC[x]); c=getc(fp);
 }

 //close
 fclose(fp); 
 //print if loaded
 printf("loaded level: %i\n",currrentLevel);
} 

void mouse(int button, int state, int x, int y) 
{
 if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
 {
    if(gameState == -2) // Main Menu
    {
        if(x > 380 && x < 580 && y > 200 && y < 250) // New Level
        {
            gameState = -1;
        }
        if(x > 380 && x < 580 && y > 300 && y < 350) // Load Level
        {
            gameState = -3;
        }
    }
    else if(gameState == -3) // Level Select
    {
        int i;
        for (i = 0; i < numLevelFiles; i++)
        {
            if (x > 380 && x < 580 && y > 200 + i * 60 && y < 250 + i * 60)
            {
                sscanf(levelFiles[i], "level_%d.h", &currrentLevel);
                load();
                gameState = -1;
            }
        }
    }
    else if(gameState == -1) // Map Editor
    {
        // Left Buttons
        if(x > 10 && x < 110)
        {
            if(y > 10 && y < 60){ buttonState = 9; } // New Level
            if(y > 70 && y < 120){ gameState = -3; } // Load Level
            if(y > 130 && y < 180){ save(); } // Save
            if(y > 190 && y < 240){ gameState = 0; } // Play
        }

        // Right Buttons
        if(x > 850 && x < 950)
        {
            if(y > 10 && y < 60){ buttonState = 3; } // Player
            if(y > 70 && y < 120){ buttonState = 4; } // Enemy
            if(y > 130 && y < 180){ buttonState = 10; } // Key
            if(y > 190 && y < 240){ buttonState = 11; } // Remove Enemy
            if(y > 250 && y < 300){ buttonState = 12; } // Remove Key
            if(y > 310 && y < 360){ currentMap = 0; } // Walls
            if(y > 370 && y < 420){ currentMap = 1; } // Floors
            if(y > 430 && y < 480){ currentMap = 2; } // Ceilings
        }

        if(buttonState == 9) // New Level
        {
            int i;
            for(i=0; i<mapX*mapY; i++){ mapW[i]=0; mapF[i]=0; mapC[i]=0; }
            numSprite = 0;
            buttonState = 0;
        }
        if(buttonState == 10) // Add Key
        {
            int s=numSprite;
            sp[s].type=1; sp[s].state=1; sp[s].map=0; sp[s].x=px; sp[s].y=py; sp[s].z=20;
            sp[s].r=255; sp[s].g=255; sp[s].b=0;
            numSprite++;
            buttonState = 0;
        }

        // Bottom Palette
        if(y > 580 && y < 640)
        {
            int* tiles;
            int tileCount;
            if(currentMap == 0) { tiles = wallTiles; tileCount = sizeof(wallTiles)/sizeof(int); }
            else if(currentMap == 1) { tiles = floorTiles; tileCount = sizeof(floorTiles)/sizeof(int); }
            else if(currentMap == 2) { tiles = ceilingTiles; tileCount = sizeof(ceilingTiles)/sizeof(int); }

            int i;
            for(i=0; i<tileCount; i++)
            {
                if(x > 10 + i*70 && x < 10 + i*70 + 64)
                {
                    currentTexture = tiles[i];
                }
            }
        }

        if(buttonState == 11) // Remove Enemy
        {
            int mx = x / 8 / 6 * 64;
            int my = y / 8 / 6 * 64;
            int i;
            for(i=0; i<numSprite; i++)
            {
                if(sp[i].type == 3 && mx+10>sp[i].x && mx-10<sp[i].x && my+10>sp[i].y && my-10<sp[i].y)
                {
                    int j;
                    for(j=i; j<numSprite-1; j++)
                    {
                        sp[j] = sp[j+1];
                    }
                    numSprite--;
                    break;
                }
            }
            buttonState = 0;
        }
        if(buttonState == 12) // Remove Key
        {
            int mx = x / 8 / 6 * 64;
            int my = y / 8 / 6 * 64;
            int i;
            for(i=0; i<numSprite; i++)
            {
                if(sp[i].type == 1 && mx+10>sp[i].x && mx-10<sp[i].x && my+10>sp[i].y && my-10<sp[i].y)
                {
                    int j;
                    for(j=i; j<numSprite-1; j++)
                    {
                        sp[j] = sp[j+1];
                    }
                    numSprite--;
                    break;
                }
            }
            buttonState = 0;
        }

        addTextures(x,y);
    }
 }
 if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
 { 
  //toggle buttons
  if(x/8>=104){ buttonState=0;}
 }

 //right button down
 if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
 {
  //previous texture
  if(x/8>=104 && y/8<16+8*0){ currentTexture-=1; if(currentTexture<0){ currentTexture=7;}} 
 
  //convert mouse to grid
  int mx=x*64/6/8;
  int my=y*64/6/8;

  //select player
  if(mx+10>px && mx-10<px && my+10>py && my-10<py){ dragItem=1;}

  //check all sprites if selected
  for(x=0;x<numSprite;x++)
  {
   if(mx+10>sp[x].x && mx-10<sp[x].x && my+10>sp[x].y && my-10<sp[x].y){ dragItem=x+10; break;}
  }
 }

 //right button up
 if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP){ dragItem=0;}

}


void MouseMove(int x,int y)
{
 //player 
 if(dragItem==1)
 {
  px=x*64/6/8; 
  py=y*64/6/8;
 }
 //move sprites
 if(dragItem>1)
 {
  sp[dragItem-10].x=x*64/6/8; 
  sp[dragItem-10].y=y*64/6/8;
 }

 //drag to draw walls
 if(dragItem==0)
 {
  addTextures(x,y);
 }

 glutPostRedisplay();
}


void drawCurrentTexture(int v,int posX, int posY)
{int x,y;
 int colors[8][3] = {
    {0,0,0},       // 0: Eraser (Black) - Preserved
    {128,0,0},     // 1: Maroon
    {0,0,128},     // 2: Navy (Perimeter Wall) - Preserved
    {0,255,255},   // 3: Aqua
    {255,0,255},   // 4: Fuchsia (Door) - Preserved
    {255,255,0},   // 5: Yellow
    {255,165,0},   // 6: Orange
    {0,255,0},     // 7: Lime (Win Tile) - Remapped from 20
};
 for(y=0;y<16;y++)
 {
  for(x=0;x<16;x++)
  {
   //int pixel=(x*2)+(y*2)*32+v*32*32; 
   int red   =colors[v][0]; 
   int green =colors[v][1];
   int blue  =colors[v][2];
   glColor3ub(red,green,blue); 
   glBegin(GL_POINTS); 
   glVertex2i(x*8+posX+4,y*8+posY+4); 
   glEnd();
  }	
 }	
}

void drawSquareFromArray(int v,int posX, int posY, int *array,int black)
{int x,y;
 //skip if not a wall 
 if(array[v]==0){ return;}

 int colors[8][3] = {
    {0,0,0},       // 0: Eraser (Black) - Preserved
    {128,0,0},     // 1: Maroon
    {0,0,128},     // 2: Navy (Perimeter Wall) - Preserved
    {0,255,255},   // 3: Aqua
    {255,0,255},   // 4: Fuchsia (Door) - Preserved
    {255,255,0},   // 5: Yellow
    {255,165,0},   // 6: Orange
    {0,255,0},     // 7: Lime (Win Tile) - Remapped from 20
};

 for(y=0;y<6;y++)
 {
  for(x=0;x<6;x++)
  {
   //int pixel=((x*2)*32+(y*2))+array[v]*32*32; 
   int red   =colors[array[v]][0]; 
   int green =colors[array[v]][1];
   int blue  =colors[array[v]][2];
   if(black==1 && array[v]>0){ red=0; green=0; blue=0;}
   glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+(8*6*posX)+4,y*8+(8*6*posY)+4); glEnd();
  }	
 }	
}

void drawMainMenu()
{
    // Draw "New Level" button
    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_QUADS);
    glVertex2i(380, 200);
    glVertex2i(580, 200);
    glVertex2i(580, 250);
    glVertex2i(380, 250);
    glEnd();

    glColor3f(0, 0, 0);
    drawText(430, 230, "New Level");

    // Draw "Load Level" button
    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_QUADS);
    glVertex2i(380, 300);
    glVertex2i(580, 300);
    glVertex2i(580, 350);
    glVertex2i(380, 350);
    glEnd();

    glColor3f(0, 0, 0);
    drawText(430, 330, "Load Level");
}

void drawLevelSelect()
{
    findFiles();
    int i;
    for (i = 0; i < numLevelFiles; i++)
    {
        glColor3f(0.8, 0.8, 0.8);
        glBegin(GL_QUADS);
        glVertex2i(380, 200 + i * 60);
        glVertex2i(580, 200 + i * 60);
        glVertex2i(580, 250 + i * 60);
        glVertex2i(380, 250 + i * 60);
        glEnd();

        glColor3f(0, 0, 0);
        drawText(430, 230 + i * 60, levelFiles[i]);
    }
}

void mapEditor()
{
    int x,y,i,j;
    //clear background 
    for(y=0;y<80;y++)
    {
        for(x=0;x<120;x++)
        {
            glColor3ub(200,220,240);
            glBegin(GL_POINTS); 
            glVertex2i(x*8+4,y*8+4); 
            glEnd();
        }	
    }

    //grid
    for(y=0;y<mapY;y++)
    {
        for(x=0;x<mapX;x++)
        {
            if(currentMap==0){ drawSquareFromArray(x+y*mapX,x,y,mapW,0);}
            if(currentMap==1){ drawSquareFromArray(x+y*mapX,x,y,mapF,0);}
            if(currentMap==2){ drawSquareFromArray(x+y*mapX,x,y,mapC,0);}
            if(currentMap> 0){ drawSquareFromArray(x+y*mapX,x,y,mapW,1);} //draw black walls
        }
    }

    //--- Left Buttons ---
    // New Level
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(10, 10); glVertex2i(110, 10); glVertex2i(110, 60); glVertex2i(10, 60); glEnd();
    glColor3f(0, 0, 0); drawText(20, 40, "New Level");
    // Load
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(10, 70); glVertex2i(110, 70); glVertex2i(110, 120); glVertex2i(10, 120); glEnd();
    glColor3f(0, 0, 0); drawText(40, 100, "Load");
    // Save
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(10, 130); glVertex2i(110, 130); glVertex2i(110, 180); glVertex2i(10, 180); glEnd();
    glColor3f(0, 0, 0); drawText(40, 160, "Save");
    // Play
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(10, 190); glVertex2i(110, 190); glVertex2i(110, 240); glVertex2i(10, 240); glEnd();
    glColor3f(0, 0, 0); drawText(40, 220, "Play");

    //--- Right Buttons ---
    // Player
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 10); glVertex2i(950, 10); glVertex2i(950, 60); glVertex2i(850, 60); glEnd();
    glColor3f(0, 0, 0); drawText(870, 40, "Player");
    // Enemy
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 70); glVertex2i(950, 70); glVertex2i(950, 120); glVertex2i(850, 120); glEnd();
    glColor3f(0, 0, 0); drawText(875, 100, "Enemy");
    // Key
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 130); glVertex2i(950, 130); glVertex2i(950, 180); glVertex2i(850, 180); glEnd();
    glColor3f(0, 0, 0); drawText(885, 160, "Key");
    // Remove Enemy
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 190); glVertex2i(950, 190); glVertex2i(950, 240); glVertex2i(850, 240); glEnd();
    glColor3f(0, 0, 0); drawText(860, 220, "Rem Enemy");
    // Remove Key
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 250); glVertex2i(950, 250); glVertex2i(950, 300); glVertex2i(850, 300); glEnd();
    glColor3f(0, 0, 0); drawText(865, 280, "Rem Key");
    // Walls
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 310); glVertex2i(950, 310); glVertex2i(950, 360); glVertex2i(850, 360); glEnd();
    glColor3f(0, 0, 0); drawText(880, 340, "Walls");
    // Floors
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 370); glVertex2i(950, 370); glVertex2i(950, 420); glVertex2i(850, 420); glEnd();
    glColor3f(0, 0, 0); drawText(875, 400, "Floors");
    // Ceilings
    glColor3f(0.8, 0.8, 0.8); glBegin(GL_QUADS); glVertex2i(850, 430); glVertex2i(950, 430); glVertex2i(950, 480); glVertex2i(850, 480); glEnd();
    glColor3f(0, 0, 0); drawText(870, 460, "Ceilings");


    //--- Bottom Palette ---
    int* tiles;
    int tileCount;
    if(currentMap == 0) { tiles = wallTiles; tileCount = sizeof(wallTiles)/sizeof(int); }
    else if(currentMap == 1) { tiles = floorTiles; tileCount = sizeof(floorTiles)/sizeof(int); }
    else if(currentMap == 2) { tiles = ceilingTiles; tileCount = sizeof(ceilingTiles)/sizeof(int); }

    for(i=0; i<tileCount; i++)
    {
        drawCurrentTexture(tiles[i], 10 + i*70, 580);
        if(tiles[i] == currentTexture)
        {
            glColor3f(1,1,1);
            glBegin(GL_LINE_LOOP);
            glVertex2i(10+i*70, 580);
            glVertex2i(10+i*70+64, 580);
            glVertex2i(10+i*70+64, 580+64);
            glVertex2i(10+i*70, 580+64);
            glEnd();
        }
    }


    //player
    glColor3ub(0,255,0); glBegin(GL_POINTS); glVertex2i(px/64*6*8,py/64*6*8); glEnd();
    glColor3ub(0,155,0); glBegin(GL_POINTS); glVertex2i((px+pdx*16)/64*6*8,(py+pdy*16)/64*6*8); glEnd();
    //draw all sprites, key,lights,enemy
    for(x=0;x<numSprite;x++)
    {
        glColor3ub(sp[x].r,sp[x].g,sp[x].b); 
        glBegin(GL_POINTS); 
        glVertex2i(sp[x].x/64*6*8,sp[x].y/64*6*8); 
        glEnd();
    }
}

void drawSprite()
{
 int x,y,s;
 if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30){ sp[0].state=0;} //pick up key 	

 //enemy attack
 for(x=0;x<numSprite;x++)
 {
  //skip if not an enemy 
  if(sp[x].type!=3){ continue;}
  if(px<sp[x].x+30 && px>sp[x].x-30 && py<sp[x].y+30 && py>sp[x].y-30){ gameState=4; return;} //enemy kills

  //add variation to player position and speed
  float ran = (float)rand()/(float)(RAND_MAX/0.1); 

  int spx=(int)sp[x].x>>6,          spy=(int)sp[x].y>>6;          //normal grid position
  int spx_add=((int)sp[x].x+15)>>6, spy_add=((int)sp[x].y+15)>>6; //normal grid position plus     offset
  int spx_sub=((int)sp[x].x-15)>>6, spy_sub=((int)sp[x].y-15)>>6; //normal grid position subtract offset
  if(sp[x].x>px && mapW[spy*mapX+spx_sub]==0){ sp[x].x-=ran*fps;}
  if(sp[x].x<px && mapW[spy*mapX+spx_add]==0){ sp[x].x+=ran*fps;}
  if(sp[x].y>py && mapW[spy_sub*mapX+spx]==0){ sp[x].y-=ran*fps;}
  if(sp[x].y<py && mapW[spy_add*mapX+spx]==0){ sp[x].y+=ran*fps;}
 }

 for(s=0;s<numSprite;s++)
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
   //t_y=31;
   for(y=0;y<scale;y++)
   {
    if(sp[s].state==1 && x>0 && x<120 && b<depth[x])
    {
     //int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
     int red   =sp[s].r;
     int green =sp[s].g;
     int blue  =sp[s].b;
     //if(red!=255, green!=0, blue!=255) //dont draw if purple
     //{
      glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd(); //draw point 
     //}
     //t_y-=t_y_step; if(t_y<0){ t_y=0;}
    }
   }
   //t_x+=t_x_step;
  }
 }
}

//---------------------------Draw Rays and Walls--------------------------------
void drawRays2D()
{	
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
  else { rx=px; ry=py; dof=20;}                                                  //looking up or down. no hit  

  while(dof<20) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]; dof=20; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=20;}                                                   //looking straight left or right
 
  while(dof<20) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]; dof=20; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  
  float shade=1;
  glColor3f(0,0.8,0);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 320 - (lineH>>1);                                               //line offset

  depth[r]=disH; //save this line's depth

  //---draw walls---
  int y;
  float tx, ty;
  //float ty_off=0; 
  //float tx;
  //if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  //else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  
  int colors[8][3] = {
    {0,0,0},       // 0: Eraser (Black) - Preserved
    {128,0,0},     // 1: Maroon
    {0,0,128},     // 2: Navy (Perimeter Wall) - Preserved
    {0,255,255},   // 3: Aqua
    {255,0,255},   // 4: Fuchsia (Door) - Preserved
    {255,255,0},   // 5: Yellow
    {255,165,0},   // 6: Orange
    {0,255,0},     // 7: Lime (Win Tile) - Remapped from 20
};

  for(y=0;y<lineH;y++)
  {
   //int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =colors[hmt][0]*shade;
   int green =colors[hmt][1]*shade;
   int blue  =colors[hmt][2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,y+lineOff); glEnd();
   //ty+=ty_step;
  }
 
  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)];
  //int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   =colors[mp][0]*0.7;
  int green =colors[mp][1]*0.7;
  int blue  =colors[mp][2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,y); glEnd();

 //---draw ceiling---
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)];
  //pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   =colors[mp][0];
  green =colors[mp][1];
  blue  =colors[mp][2];
  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,640-y); glEnd();}
 }
 
 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 }
}//-----------------------------------------------------------------------------


void drawSky()     //draw sky and rotate based on player rotation
{int x,y;
 for(y=0;y<40;y++)
 {
  for(x=0;x<120;x++)
  {
   //int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120; //return 0-120 based on player angle
   //int pixel=(y*120+xo)*3;
   int red   =135;
   int green =206;
   int blue  =235;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+4,y*8+4); glEnd();
  }	
 }
}

void drawText(int x, int y, char *string)
{
    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
    }
}

void screen(int v) //draw any full screen image. 120x80 pixels
{
 int x,y;
 //int *T;
 int r,g,b;
 if(v==1){ r=0; g=0; b=255; drawText(450, 300, "Loading...");} //title screen - blue
 if(v==2){ r=0; g=255; b=0;} //won screen - green
 if(v==3){ r=255; g=0; b=0;} //lost screen - red
 for(y=0;y<80;y++)
 {
  for(x=0;x<120;x++)
  {
   //int pixel=(y*120+x)*3;
   int red   =r*fade;
   int green =g*fade;
   int blue  =b*fade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+4,y*8+4); glEnd();
  }	
 }	
 glColor3f(1,1,1); //set text color to white
 if(v==2){ drawText(450, 300, "You Win!");} //won screen - green
 if(v==3){ drawText(450, 300, "You Lose");} //lost screen - red
 if(fade<1){ fade+=0.001*fps;} 
 if(fade>1){ fade=1;}
}

void init()//init all variables when game starts
{
 glClearColor(0.3,0.3,0.3,0);
 glPointSize(8);

 px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));                                 //init player

 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=5*64; sp[0].z=20; //key
 sp[1].type=3; sp[1].state=1; sp[1].map=2; sp[1].x=2.5*64; sp[1].y=2*64; sp[1].z=20; //enemy
 //add color just for 2D edit view
 sp[0].r=255; sp[0].g=  0; sp[0].b=0; //key
 sp[1].r=255; sp[1].g=255; sp[1].b=0; //enemy

 //create wall perimeter
 int x,y;
 for(y=0;y<mapY;y++)
 {
  for(x=0;x<mapX;x++)
  { 
   if(x==0 || x==mapX-1 || y==0 || y==mapY-1){ mapW[y*mapX+x]=2;} 
   mapF[y*mapX+x]=1;
   mapC[y*mapX+x]=0;
  }
 }
}

void display()
{  
 //frames per second
 frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

 if(gameState == -2)
 {
    drawMainMenu();
 }
 else if(gameState == -3)
 {
    drawLevelSelect();
 }
 //2D edit 
 else if(gameState<0){ mapEditor();}

 //3D game
 else
 {
  if(gameState==0) //init game
  {
    fade=0; timer=0;
    backup_px = px; backup_py = py; backup_pa = pa;
    int i;
    for(i=0; i<numSprite; i++){ backup_sp[i] = sp[i]; }
    gameState=1;
  }
  if(gameState==1){ screen(1); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=2;}} //start screen
  if(gameState==2) //The main game loop
  {
   //buttons
   if(Keys.a==1){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
   if(Keys.d==1){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

   int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
   int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
   int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
   int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
   if(Keys.w==1)                                                                  //move forward
   {  
    if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.2*fps;}
    if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.2*fps;}
   }
   if(Keys.s==1)                                                                  //move backward
   { 
    if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.2*fps;}
    if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.2*fps;}
   } 
   drawSky();
   drawRays2D();
   drawSprite();
   if(mapF[((int)px>>6)+((int)py>>6)*mapX]==7){ fade=0; timer=0; gameState=3;} //Entered block 7, Win game!!
  }
  //won screen
  if(gameState==3)
  { 
   screen(2); timer+=1*fps; 
   if(timer>2000)
   { 
    fade=0; timer=0; gameState=-1;
    px = backup_px; py = backup_py; pa = backup_pa;
    int i;
    for(i=0; i<numSprite; i++){ sp[i] = backup_sp[i]; }
   }
  } 
  if(gameState==4){ screen(3); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=-1;
    px = backup_px; py = backup_py; pa = backup_pa;
    int i;
    for(i=0; i<numSprite; i++){ sp[i] = backup_sp[i]; }
  }} //lost screen
 }

 glutPostRedisplay();
 glutSwapBuffers();  
}

void ButtonDown(unsigned char key,int x,int y)                                  //keyboard button pressed down
{
 if(key=='a'){ Keys.a=1;} 	
 if(key=='d'){ Keys.d=1;} 
 if(key=='w'){ Keys.w=1;}
 if(key=='s'){ Keys.s=1;}
 if(key=='e' && sp[0].state==0)             //open doors
 { 
  int xo=0; if(pdx<0){ xo=-25;} else{ xo=25;}
  int yo=0; if(pdy<0){ yo=-25;} else{ yo=25;} 
  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
 }
 //quickly set current texture to empty
 if(key=='q'){ currentTexture=0;} 
 //spacebar return to 2D edit view
 if(key==32){ gameState=-1;} 

 glutPostRedisplay();
}

void ButtonUp(unsigned char key,int x,int y)                                    //keyboard button pressed up
{
 if(key=='a'){ Keys.a=0;} 	
 if(key=='d'){ Keys.d=0;} 
 if(key=='w'){ Keys.w=0;}
 if(key=='s'){ Keys.s=0;}
 glutPostRedisplay();
}

void resize(int w,int h)                                                        //screen window rescaled, snap back
{
 glutReshapeWindow(960,640);
}

int main(int argc, char* argv[])
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(960,640);
 glutInitWindowPosition( glutGet(GLUT_SCREEN_WIDTH)/2-960/2 ,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2 );
 glutCreateWindow("cockroch engine");
 gluOrtho2D(0,960,640,0);
 init();
 glutMouseFunc(mouse);
 glutMotionFunc(MouseMove);
 glutDisplayFunc(display);
 glutReshapeFunc(resize);
 glutKeyboardFunc(ButtonDown);
 glutKeyboardUpFunc(ButtonUp);
 glutMainLoop();
}

