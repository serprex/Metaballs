#define _GNU_SOURCE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef __SSE__
#include <xmmintrin.h>
#else
#ifndef x5f3759df
#include <math.h>
#endif
float rsqrt(float x){
#ifdef x5f3759df
	int i=0x5f3759df-(*(int*)&x>>1);
	return*(float*)&i;
#else
	return 1f/sqrtf(x);
#endif
}
#endif
uint8_t*manor;
float*F,*X,*Y;
uint_fast16_t ms=1,WID,HEI;
int psz;
void mcb(GLFWwindow*wnd,int b,int a,int m){
	if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_LEFT){
		if(ms*sizeof(float)>=psz){
			X=realloc(X,psz<<=1);
			Y=realloc(Y,psz);
			F=realloc(F,psz);
		}
		X[ms]=X[ms-1];
		Y[ms]=Y[ms-1];
		F[ms]=F[ms-1];
		ms++;
	}else if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_RIGHT&&ms>1){
		int c=0;
		float d=WID*WID+HEI*HEI;
		for(int i=0;i<ms-1;i++){
			float h=(X[i]-X[ms-1])*(X[i]-X[ms-1])+(Y[i]-Y[ms-1])*(Y[i]-Y[ms-1]);
			if(h<d){
				c=i;
				d=h;
			}
		}
		memmove(X+c,X+c+1,(--ms-c)*sizeof(float));
		memmove(Y+c,Y+c+1,(ms-c)*sizeof(float));
		memmove(F+c,F+c+1,(ms-c)*sizeof(float));
		X[ms]=Y[ms]=F[ms]=0;
	}
}
void scb(GLFWwindow*wnd,double x,double y)
{
	F[ms-1]+=(x+y)*512;
}
int main(int argc,char**argv){
	glfwInit();
	const GLFWvidmode*vm=glfwGetVideoMode(glfwGetPrimaryMonitor());
	WID=vm->width;
	HEI=vm->height;
	GLFWwindow*wnd=glfwCreateWindow(WID,HEI,0,0,0);
	glfwMakeContextCurrent(wnd);
	glfwSetMouseButtonCallback(wnd,mcb);
	glfwSetScrollCallback(wnd,scb);
	glOrtho(0,WID,HEI,0,1,-1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	manor=malloc(HEI*WID*3);
	X=calloc(1,psz=2000);
	Y=calloc(1,psz);
	F=calloc(1,psz);
	F[0]=psz;
	unsigned char col[256][3];
	for(int i=0;i<256;i++){
		col[i][0]=i*i*i>>16;
		col[i][1]=i*i>>8;
		col[i][2]=i;
	}
	for(;;){
		double x,y;
		glfwGetCursorPos(wnd,&x,&y);
		X[ms-1]=x;
		Y[ms-1]=y;
		#pragma omp parallel for schedule(static)
		for(int y=0;y<HEI;y++)
			for(int x=0;x<WID;x++){
				float d=0;
				int i=0;
				#ifdef __SSE__
				__m128 D=_mm_setzero_ps();
				do{
					__m128 xx=_mm_sub_ps(_mm_load_ps(X+i),_mm_set1_ps(x)),yy=_mm_sub_ps(_mm_load_ps(Y+i),_mm_set1_ps(y));
					D=_mm_add_ps(D,_mm_mul_ps(_mm_load_ps(F+i),_mm_rsqrt_ps(_mm_add_ps(_mm_mul_ps(xx,xx),_mm_mul_ps(yy,yy)))));
				}while((i+=4)<ms);
				float f[4]__attribute__((aligned(16)));
				_mm_store_ps(f,D);
				d=f[0]+f[1]+f[2]+f[3];
				#else
				do d+=F[i]*rsqrt((X[i]-x)*(X[i]-x)+(Y[i]-y)*(Y[i]-y)); while(++i<ms);
				#endif
				memcpy(manor+((HEI-1-y)*WID+x)*3,col[d>255?255:d<0?0:(unsigned char)d],3);
			}
		glDrawPixels(WID,HEI,GL_RGB,GL_UNSIGNED_BYTE,manor);
		glfwSwapBuffers(wnd);
		glfwPollEvents();
		if(glfwGetKey(wnd,GLFW_KEY_ESCAPE)||glfwWindowShouldClose(wnd))return 0;
	}
}
