#include <stdio.h>
#include <stdlib.h>
#include <psptypes.h>
#include "color.h"
#include <pspsdk.h>
#include <pspuser.h>
#include <pspdisplay.h>
#include "bmp.h"

#define printf pspDebugScreenPrintf


int compteur = 0;


void saveBMP(imageBMP img)
{
compteur = rand() % 10000;
  char path[128];
  sprintf(path, "ms0:/PICTURE/BMP%03d.bmp", compteur);
  compteur++;
  u8 c;
  u32 i;
  u16 si;
  
  FILE *fd = fopen(path, "wb");
  c = 0x42;
  fwrite(&c, 1,1, fd); //-- B
  c = 0x4d;
  fwrite(&c, 1,1, fd); //-- M
  i = 54 + img.h*img.w*3; // en tete + 480*272*3
  fwrite(&i, 4, 1, fd);
  i = 0; // reserve
  fwrite(&i, 4, 1, fd);
  i = 54; // offset
  fwrite(&i, 4, 1, fd);
  i = 40; // entete image
  fwrite(&i, 4, 1, fd);
  // largeur
  fwrite(&img.w, 4, 1, fd);
  //hauteur
  fwrite(&img.h, 4, 1, fd);
  si = 1; // plan (?)
  fwrite(&si, 2, 1, fd);
  si = 24; // profondeur
  fwrite(&si, 2, 1, fd);
  i = 0; // compression
  fwrite(&i, 4, 1, fd);
  i = 16; //taille ---------- # FIXME
  fwrite(&i, 4, 1, fd);
  i = 2834; //resol H
  fwrite(&i, 4, 1, fd);
  i = 2834; //resol V
  fwrite(&i, 4, 1, fd);
  i = 0; // palette
  fwrite(&i, 4, 1, fd);
  i = 0; // palette bis
  fwrite(&i, 4, 1, fd);
  
  //--------------------- fin de l'en-tete
  // parcours de l'image
  //  3 -> -> 4
  //  |       |
  //  ^       ^
  //  |       |
  //  1 -> -> 2

  int k, l, m;
  for (k = img.h-1; k >= 0; k--)
   {
    m = k*img.w;
    for (l = 0; l < img.w; l++)
      {
	couleur tmp = rgba2coul(img.tab[m+l]);
	//tmp.g /= 2;
	fwrite(&tmp.b, 1, 1, fd);
	fwrite(&tmp.g, 1, 1, fd);
	fwrite(&tmp.r, 1, 1, fd);
      }
    }
  fclose(fd);
}


//-------------------------------
// sauve l'ecran de la psp dans un fichier bmp
// donne le nom a partir du num
//-------------------------------
void saveScreenPsp(u32* tab)
 {
  compteur = rand() % 10000;
  char path[128];
  sprintf(path, "ms0:/PICTURE/%03d.bmp", compteur);
  compteur++;
  u8 c;
  u32 i;
  u16 si;
  
  FILE *fd = fopen(path, "wb");
  c = 0x42;
  fwrite(&c, 1,1, fd); //-- B
  c = 0x4d;
  fwrite(&c, 1,1, fd); //-- M
  i = 54 + 391680; // en tete + 480*272*3
  fwrite(&i, 4, 1, fd);
  i = 0; // reserve
  fwrite(&i, 4, 1, fd);
  i = 54; // offset
  fwrite(&i, 4, 1, fd);
  i = 40; // entete image
  fwrite(&i, 4, 1, fd);
  i = 480; // largeur
  fwrite(&i, 4, 1, fd);
  i = 272; // hauteur
  fwrite(&i, 4, 1, fd);
  si = 1; // plan (?)
  fwrite(&si, 2, 1, fd);
  si = 24; // profondeur
  fwrite(&si, 2, 1, fd);
  i = 0; // compression
  fwrite(&i, 4, 1, fd);
  i = 16; //taille ---------- # FIXME
  fwrite(&i, 4, 1, fd);
  i = 2834; //resol H
  fwrite(&i, 4, 1, fd);
  i = 2834; //resol V
  fwrite(&i, 4, 1, fd);
  i = 0; // palette
  fwrite(&i, 4, 1, fd);
  i = 0; // palette bis
  fwrite(&i, 4, 1, fd);
  
  //--------------------- fin de l'en-tete
  // parcours de l'image
  //  3 -> -> 4
  //  |       |
  //  ^       ^
  //  |       |
  //  1 -> -> 2

  int k, l, m;
  for (k = 271; k >= 0; k--)
   {
    m = k*480;
    for (l = 0; l < 480; l++)
      {
	couleur tmp = rgba2coul(tab[m+l]);
	//tmp.g /= 2;
	fwrite(&tmp.b, 1, 1, fd);
	fwrite(&tmp.g, 1, 1, fd);
	fwrite(&tmp.r, 1, 1, fd);
      }
    }
  fclose(fd);
 }




//-------------------------------
// charge un fichier bmp a partir d'un nom de fichier
//-------------------------------
imageBMP loadBMPfromFile(char* path)
{
 imageBMP res;
 u8 c;
 u32 i;
 int offset;

  FILE *fd = fopen(path, "r");
  if (fd == NULL) {
  printf("erreur d'ouverture de %s\n", path);
  res.tab = NULL;
  res.w = res.h = 0;
  return res;
  }
  fread(&c, 1,1, fd);//-- B
  fread(&c, 1,1, fd);//-- M
  fread(&i, 4,1, fd);//-- taille totale
  fread(&i, 4,1, fd);// reserve
  fread(&offset, 4,1, fd);// -- offset
  fread(&offset, 4,1, fd);// -- en tete image
  fread(&res.w, 4,1, fd);// largeur
  fread(&res.h, 4,1, fd);// hauteur
  fseek(fd, offset+14, SEEK_SET);// on va au morceau interessant
  int k,l,cpt;
  cpt = 1;
  res.tab = (u32*) malloc(res.w*res.h*sizeof(u32));
  for(k=0; k<res.h; k++)
   for(l=0; l<res.w; l++)
    {
     couleur tmp;
     fread(&tmp.b, 1, 1, fd);
     fread(&tmp.g, 1, 1, fd);
     fread(&tmp.r, 1, 1, fd);
     tmp.a = 0;
     res.tab[res.w*(res.h-1-k)+l] = coul2rgba(tmp);
     //printf("%06x ", tmp.r<<16 | tmp.g<<8 | tmp.b);
     //if (!(cpt%10)) printf("\n");
     //cpt++;
    }
   fclose(fd);
   //saveBMP(res);
   return res;
}




void blitBMP(u32* vram, int x, int y, imageBMP img)
{
 int i,j;
 for(i=0; i<img.h; i++)
  if (y+i > 0 && y+i < 272)
    for(j=0; j<img.w; j++)
     {
      if((img.tab[i*img.w+j]|0xff000000) != 0xffff00ff) // si ce n'est pas du rose (trasnparent)
       {
        if(x+j>0 && x+j<480) vram[(y+i)*480+x+j] = img.tab[i*img.w+j];
       }
     }

}

void blitPerso(u32* vram, int x, int y)
{
}
