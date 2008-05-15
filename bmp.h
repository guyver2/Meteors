#ifndef BMP_H
#define BMP_H
#include <psptypes.h>


typedef struct imageBMP{
 u32 w,h;
 u32* tab __attribute__((aligned(64)));;
} imageBMP;


void blitBMP(u32* vram, int x, int y, imageBMP img);
void blitPerso(u32* vram, int x, int y);

void saveScreenPsp(u32* tab);
imageBMP loadBMPfromFile(char* path);
#endif
