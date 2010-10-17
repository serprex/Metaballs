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
int*H;
unsigned ms=3;
void GLFWCALL mcb(int b,int a){
	if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_LEFT){
		memcpy(H+ms,H+ms-3,sizeof(int)*3);
		ms+=3;
	}else if(a==GLFW_PRESS&&b==GLFW_MOUSE_BUTTON_RIGHT&&ms>3){
		int c=0;
		unsigned long long d=-1ULL;
		for(int i=0;i<ms-3;i+=3){
			int h=(H[i]-H[ms-3])*(H[i]-H[ms-3])+(H[i+1]-H[ms-2])*(H[i+1]-H[ms-2]);
			if(h<d){
				c=i;
				d=h;
			}
		}
		memmove(H+c,H+c+3,((ms-=3)-c)*sizeof(int));
		memset(H+ms,0,sizeof(int)*3);
	}
}
void GLFWCALL kcb(int b,int a){
}
int main(int argc,char**argv){
	glfwInit();
	if(!glfwOpenWindow(WID,HEI,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	glfwSetMouseButtonCallback(mcb);
	glOrtho(0,WID,HEI,0,1,-1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	#ifdef stdalloc
	H=calloc(1,4000);
	#else
	H=mmap(0,sysconf(_SC_PAGE_SIZE),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	#endif
	unsigned char col[256][3];
	for(int i=0;i<256;i++){
		col[i][0]=i*i*i>>16;
		col[i][1]=i*i>>8;
		col[i][2]=i;
	}
	for(;;){
		#ifdef bench
		double t=glfwGetTime();
		#endif
		glfwGetMousePos(H+ms-3,H+ms-2);
		#pragma omp parallel for
		for(int y=0;y<HEI;y++)
			for(int x=0;x<WID;x++){
				float d=0;
				int i=0;
				#ifdef __SSE2__
				__m128 D=_mm_setzero_ps();
				float f[4]__attribute__((aligned(16)));
				do{
					float h[4]__attribute__((aligned(16)));
					for(int j=0;j<4;j++){
						f[j]=(H[i+j*3]-x)*(H[i+j*3]-x)+(H[i+j*3+1]-y)*(H[i+j*3+1]-y);
						h[j]=H[i+j*3+2];
					}
					D=_mm_add_ps(D,_mm_mul_ps(_mm_load_ps(h),_mm_rsqrt_ps(_mm_load_ps(f))));
				}while((i+=12)<ms);
				_mm_store_ps(f,D);
				d=f[0]+f[1]+f[2]+f[3];
				#else
				do d+=H[i+2]*rsqrt((H[i]-x)*(H[i]-x)+(H[i+1]-y)*(H[i+1]-y)); while((i+=3)<ms);
				#endif
				memcpy(manor[HEI-y-1][x],col[d>255?255:d<0?0:(unsigned char)d],3);
			}
		glDrawPixels(WID,HEI,GL_RGB,GL_UNSIGNED_BYTE,manor);
		#ifdef bench
		printf("%d %f\n",ms/3,glfwGetTime()-t);
		#endif
		glfwSwapBuffers();
		H[ms-1]+=glfwGetMouseWheel()<<8;
		glfwSetMouseWheel(0);
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED))return 0;
	}
}
