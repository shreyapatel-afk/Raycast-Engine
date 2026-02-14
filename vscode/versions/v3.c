//collision
#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <math.h>

#define mapX  8      
#define mapY  8      
#define mapS 64      
int map[mapX*mapY]=          
{
 1,1,1,1,1,1,1,1,
 1,0,1,0,0,0,0,1,
 1,0,1,0,0,0,0,1,
 1,0,1,0,0,0,0,1,
 1,0,0,0,0,1,0,1,
 1,0,0,0,0,1,1,1,
 1,0,0,0,0,0,0,1,
 1,1,1,1,1,1,1,1,	
};

void saveLevel(const char* filename){
    FILE* f = fopen(filename,"w");
    if(!f) return;
    int x,y;
    for(y=0;y<mapY;y++){
        for(x=0;x<mapX;x++){
            fprintf(f,"%d ", map[y*mapX+x]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
    printf("Level saved to %s\n", filename);
}

void loadLevel(const char* filename){
    FILE* f = fopen(filename,"r");
    if(!f) return;
    int x,y;
    for(y=0;y<mapY;y++){
        for(x=0;x<mapX;x++){
            fscanf(f,"%d", &map[y*mapX+x]);
        }
    }
    fclose(f);
    printf("Level loaded from %s\n", filename);
}

//PLAYER
float degToRad(int a) { return a*M_PI/180.0;}
int FixAng(int a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}

float px,py,pdx,pdy,pa;

void drawMap2D()
{
 int x,y,xo,yo;
 for(y=0;y<mapY;y++)
 {
  for(x=0;x<mapX;x++)
  {
   if(map[y*mapX+x]==1){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);} 
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
    return map[my*mapX+mx];
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
 if(key=='l'){ loadLevel("level.txt"); }
 if(key=='k'){ saveLevel("level.txt"); }
 glutPostRedisplay();
}

//Draw Rays and Walls
float distance(ax,ay,bx,by,ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}

void drawRays3D()
{
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);
 
 for(r=0;r<60;r++)
 {
  //Vertical
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}
  else { rx=px; ry=py; dof=8;}

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && map[mp]==1){ dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}       
   else{ rx+=xo; ry+=yo; dof+=1;}
  } 
  vx=rx; vy=ry;

  //Horizontal
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}
  else{ rx=px; ry=py; dof=8;}
 
  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && map[mp]==1){ dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}       
   else{ rx+=xo; ry+=yo; dof+=1;}
  } 
  
  float shade;
  if(disV<disH){ rx=vx; ry=vy; disH=disV; shade = 0.6;} // vertical walls darker
  else shade = 1.0; // horizontal walls brighter

  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            
  int lineH = (mapS*320)/(disH); if(lineH>320){ lineH=320;}                     
  int lineOff = 160 - (lineH>>1);                                               

  // draw ray in 2D map
  glColor3f(0,1,0);
  glLineWidth(1);
  glBegin(GL_LINES);
  glVertex2i(px,py);
  glVertex2i(rx,ry);
  glEnd();

  // sky
  glColor3f(0.3,0.6,1.0);
  glBegin(GL_QUADS);
  glVertex2i(r*8+530,0);
  glVertex2i(r*8+538,0);
  glVertex2i(r*8+538,160);
  glVertex2i(r*8+530,160);
  glEnd();

  // floor
  glColor3f(0.3,0.3,0.3);
  glBegin(GL_QUADS);
  glVertex2i(r*8+530,160);
  glVertex2i(r*8+538,160);
  glVertex2i(r*8+538,320);
  glVertex2i(r*8+530,320);
  glEnd();

  // wall (filled rectangle instead of line)
  glColor3f(0,shade,0);
  glBegin(GL_QUADS);
  glVertex2i(r*8+530,lineOff);
  glVertex2i(r*8+538,lineOff);
  glVertex2i(r*8+538,lineOff+lineH);
  glVertex2i(r*8+530,lineOff+lineH);
  glEnd();

  ra=FixAng(ra-1);                                                              
 }
}

void mouse(int button, int state, int x, int y)
{
    if(state==GLUT_DOWN){
        int mx = x/mapS;
        int my = y/mapS;
        if(mx>=0 && mx<mapX && my>=0 && my<mapY){
            int idx = my*mapX+mx;
            map[idx] = !map[idx];
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
}

void display()
{   
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
 drawMap2D();
 drawPlayer2D();
 drawRays3D();
 glutSwapBuffers();  
}

int main(int argc, char* argv[])
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(1024,510);
 glutCreateWindow("cockroch engine");
 init();
 glutDisplayFunc(display);
 glutKeyboardFunc(Buttons);
 glutMouseFunc(mouse);
 glutMainLoop();
}

