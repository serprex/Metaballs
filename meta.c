#define _GNU_SOURCE
#include <GL/glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef xmmintrin
#include <xmmintrin.h>
#else
#ifdef x5f3759df
#include <math.h>
#endif
static float rsqrt(float x){
#if defined x5f3759df
	int i=0x5f3759df-(*(int*)&x>>1);
	return*(float*)&i;
#else
	return 1./sqrtf(x);
#endif
}
#endif
unsigned char manor[512][512][3];
int*H;
unsigned ms=3;
int main(int argc,char**argv){
	glfwInit();
	if(!glfwOpenWindow(511,511,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	glOrtho(0,511,511,0,1,-1);
	H=malloc(sizeof(int)*3);
	unsigned char col[256][3];
	for(int i=0;i<256;i++){
		col[i][0]=i*i*i>>16;
		col[i][1]=i*i>>8;
		col[i][2]=i;
	}
	for(;;){
		double t=glfwGetTime();
		glfwGetMousePos(H+ms-3,H+ms-2);
		#pragma omp parallel for
		for(int x=0;x<512;x++)
			for(int y=0;y<512;y++){
				float d=0;
				#ifdef xmmintrin
				int i=0;
				for(;(ms-i)%12&&i<ms;i+=3){
					float f __attribute__((aligned(16)));
					_mm_store_ss(&f,_mm_rsqrt_ss(_mm_set1_ps((H[i]-x)*(H[i]-x)+(H[i+1]-y)*(H[i+1]-y))));
					d+=H[i+2]*f;
				}
				for(;i<ms;i+=12){
					float f[4]__attribute__((aligned(16)))={};
					for(int j=0;j<4;j++)f[j]=(H[i+j*3]-x)*(H[i+j*3]-x)+(H[i+j*3+1]-y)*(H[i+j*3+1]-y);
					_mm_store_ps(f,_mm_rsqrt_ps(_mm_load_ps(f)));
					d+=H[i+2]*f[0]+H[i+5]*f[1]+H[i+8]*f[2]+H[i+11]*f[3];
				}
				#else
				for(int i=0;i<ms;i+=3)d+=H[i+2]*rsqrt((H[i]-x)*(H[i]-x)+(H[i+1]-y)*(H[i+1]-y));
				#endif
				memcpy(manor[511-y][x],col[d>255?255:(unsigned char)d],3);
			}
		glDrawPixels(511,511,GL_RGB,GL_UNSIGNED_BYTE,manor);
		printf("%f\n",glfwGetTime()-t);
		int pl=glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT),pr=glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT);
		glfwSwapBuffers();
		H[ms-1]+=250*glfwGetMouseWheel();
		glfwSetMouseWheel(0);
		if(!pr&&glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)&&ms>2)H=realloc(H,sizeof(int)*(ms-=3));
		if(!pl&&glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)){
			H=realloc(H,sizeof(int)*(ms+=3));
			memcpy(H+ms-3,H+ms-6,sizeof(int)*3);
		}
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED))return 0;
	}
}
