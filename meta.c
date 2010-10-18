#define _GNU_SOURCE
#include <GL/glfw.h>
#ifdef stdalloc
#include <stdlib.h>
#else
#include <sys/unistd.h>
#include <sys/mman.h>
#endif
#include <string.h>
#ifdef bench
#include <stdio.h>
#endif
#ifdef __SSE2__
#include <xmmintrin.h>
#else
#ifndef x5f3759df
#include <math.h>
#endif
static float rsqrt(float x){
#ifdef x5f3759df
	int i=0x5f3759df-(*(int*)&x>>1);
	return*(float*)&i;
#else
	return 1./sqrtf(x);
#endif
}
#endif
#ifndef WID
#define WID 768
#endif
#ifndef HEI
#define HEI 768
#endif
unsigned char manor[HEI][WID][3];
float*F,*X,*Y;
unsigned ms=1,psz;
void GLFWCALL mcb(int b,int a){
	if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_LEFT){
		if(ms*sizeof(float)>=psz){
		#ifdef stdalloc
			X=realloc(X,psz+=psz);
			Y=realloc(Y,psz+=psz);
			F=realloc(F,psz);
		#else
			X=mremap(X,psz,psz<<=1,0);
			Y=mremap(Y,psz,psz<<=1,0);
			F=mremap(F,psz>>1,psz,0);
		#endif
		}
		X[ms]=X[ms-1];
		Y[ms]=Y[ms-1];
		F[ms]=F[ms-1];
		ms++;
	}else if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_RIGHT&&ms>1){
		int c=0;
		unsigned long long d=-1ULL;
		for(int i=0;i<ms-1;i++){
			int h=(X[i]-X[ms-1])*(X[i]-X[ms-1])+(Y[i]-Y[ms-1])*(Y[i]-Y[ms-1]);
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
int main(int argc,char**argv){
	glfwInit();
	if(!glfwOpenWindow(WID,HEI,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	glfwSetMouseButtonCallback(mcb);
	glOrtho(0,WID,HEI,0,1,-1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	#ifdef stdalloc
	X=calloc(1,psz=1000);
	Y=calloc(1,psz);
	F=calloc(1,psz);
	#else
	X=mmap(0,psz=sysconf(_SC_PAGE_SIZE),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	Y=mmap(0,psz,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	F=mmap(0,psz,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	#endif
	unsigned char col[256][3];
	for(int i=0;i<256;i++){
		col[i][0]=i*i*i>>16;
		col[i][1]=i*i>>8;
		col[i][2]=i;
	}
	for(;;){
		int x,y;
		glfwGetMousePos(&x,&y);
		X[ms-1]=x;
		Y[ms-1]=y;
		#ifdef bench
		double t=glfwGetTime();
		#endif
		#pragma omp parallel for schedule(dynamic)
		for(int y=0;y<HEI;y++)
			for(int x=0;x<WID;x++){
				float d=0;
				int i=0;
				#ifdef __SSE2__
				__m128 D=_mm_setzero_ps();
				do{
					__m128 xx=_mm_sub_ps(_mm_load_ps(X+i),_mm_set1_ps(x));
					__m128 yy=_mm_sub_ps(_mm_load_ps(Y+i),_mm_set1_ps(y));
					D=_mm_add_ps(D,_mm_mul_ps(_mm_load_ps(F+i),_mm_rsqrt_ps(_mm_add_ps(_mm_mul_ps(xx,xx),_mm_mul_ps(yy,yy)))));
				}while((i+=4)<ms);
				float f[4]__attribute__((aligned(16)));
				_mm_store_ps(f,D);
				d=f[0]+f[1]+f[2]+f[3];
				#else
				do d+=F[i]*rsqrt((X[i]-x)*(X[i]-x)+(Y[i]-y)*(Y[i]-y)); while(++i<ms);
				#endif
				memcpy(manor[HEI-1-y][x],col[d>255?255:d<0?0:(unsigned char)d],3);
			}
		#ifdef bench
		printf("%d %f\n",ms,glfwGetTime()-t);
		#endif
		glDrawPixels(WID,HEI,GL_RGB,GL_UNSIGNED_BYTE,manor);
		glfwSwapBuffers();
		F[ms-1]+=glfwGetMouseWheel()<<8;
		glfwSetMouseWheel(0);
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED))return 0;
	}
}
