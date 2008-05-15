#include "chotto.h"



PSP_MODULE_INFO("Evite moi ca", 0, 1, 0);

PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

void StopApp();

#define MAX_VIDEO_FRAME_SIZE	(32*1024)
#define MAX_STILL_IMAGE_SIZE	(512*1024)

#define TAILLE 20
#define PERSO 24

typedef struct {
int x, y, decx, decy, actif;
} objet;
int xPerso = 240;
int yPerso = 137;
int pts = 0;

objet tabObjet[10];

static imageBMP perso;
static imageBMP balle;


//static u32 balleBuffer[480*272] __attribute__((aligned(64)));
static u32 shotBuffer[480*272] __attribute__((aligned(64)));

static u8  buffer[MAX_STILL_IMAGE_SIZE] __attribute__((aligned(64)));
static u8  work[68*1024] __attribute__((aligned(64)));
static u32 framebuffer[480*272] __attribute__((aligned(64)));
static u32  tampon[480*272] __attribute__((aligned(64)));

static SceUID waitphoto;
static int takephoto = 0;


void newObj(objet *tmp)
{
 switch (rand()%4){
  case 0: //demarre en haut
          tmp->y = -TAILLE;
          tmp->x = rand()%(480+TAILLE)-TAILLE;
          tmp->decx = 0;
 	  while (tmp->decx == 0) tmp->decx = rand()%12 - 6;
 	  tmp->decy = 1+rand()%3;
          break;
  case 1: //demarre en bas
          tmp->y = 272;
          tmp->x = rand()%(480+TAILLE)-TAILLE;
          tmp->decx = 0;
 	  while (tmp->decx == 0) tmp->decx = rand()%12 - 6;
 	  tmp->decy = -1-rand()%3;
          break;
  case 2: //demarre a droite
          tmp->x = 480;
          tmp->y = rand()%(272+TAILLE)-TAILLE;
          tmp->decy = 0;
 	  while (tmp->decy == 0) tmp->decy = rand()%12 - 6;
 	  tmp->decx = -1-rand()%3;
          break;
  case 3: //demarre a droite
          tmp->x = -TAILLE;
          tmp->y = rand()%(272+TAILLE)-TAILLE;
          tmp->decy = 0;
 	  while (tmp->decy == 0) tmp->decy = rand()%12 - 6;
 	  tmp->decx = 1+rand()%3;
          break;
  default : break;
  }
}


void bouge()
{
 int i;
 for (i=0; i<10; i++)
  {
   objet* tmp = &tabObjet[i];
   if (tmp->actif)
    {
     tmp->x += tmp->decx;
     tmp->y += tmp->decy;
     if (tmp->x > 480 || tmp->x < -(TAILLE+10) || tmp->y >272 || tmp->y < -(TAILLE+10))
      {
        pts++;
	newObj(tmp);
      }
    }
  }
}

void affObj()
{
  int i;
 for (i=0; i<10; i++)
  {
   objet tmp = tabObjet[i];
   if (tmp.actif)
    {
     blitBMP(tampon, tmp.x-TAILLE, tmp.y-TAILLE, balle);
    }
  }
}

#define CARRE(x) (x)*(x)

int colision()
{
  int i;
 for (i=0; i<10; i++)
  {
   objet tmp = tabObjet[i];
   int max = CARRE(TAILLE+PERSO-10);
   if (tmp.actif)
    {
     long dist = CARRE((tmp.x-TAILLE)-(xPerso-PERSO)) + CARRE((tmp.y-TAILLE)-(yPerso-PERSO));
     if (dist < max) return 1;
    }
  }
  return 0;
}

void init()
{
int i;
        for (i=0; i<10; i++)
         {
          objet* tmp = &tabObjet[i];
          tmp->actif = 1;
	  newObj(tmp);
         }
  xPerso = 240;
  yPerso = 137;
  pts = 0;
}



int video_thread(SceSize args, void *argp)
{
	PspUsbCamSetupVideoParam videoparam;
	int result;
	u32 *vram = (u32 *)0x04000000;

	memset(&videoparam, 0, sizeof(videoparam));
	videoparam.size = sizeof(videoparam);
	videoparam.resolution = PSP_USBCAM_RESOLUTION_480_272;
	videoparam.framerate = PSP_USBCAM_FRAMERATE_30_FPS;
	videoparam.wb = PSP_USBCAM_WB_AUTO;
	videoparam.saturation = 125;
	videoparam.brightness = 128;
	videoparam.contrast = 64;
	videoparam.sharpness = 0;
	videoparam.effectmode = PSP_USBCAM_EFFECTMODE_NORMAL;
	videoparam.framesize = MAX_VIDEO_FRAME_SIZE;
	videoparam.evlevel = PSP_USBCAM_EVLEVEL_0_0;	

	result = sceUsbCamSetupVideo(&videoparam, work, sizeof(work));	
	if (result < 0)
	{
		printf("Error 0x%08X in sceUsbCamSetupVideo.\n", result);
		sceKernelExitDeleteThread(result);
	}

	sceUsbCamAutoImageReverseSW(1);

	result = sceUsbCamStartVideo();	
	if (result < 0)
	{
		printf("Error 0x%08X in sceUsbCamStartVideo.\n", result);
		sceKernelExitDeleteThread(result);
	}

	sceDisplaySetMode(0, 480, 272);
	sceDisplaySetFrameBuf((void *)0x04000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
	long int sommeX, sommeY, nbCorres;
	while(1)
	{
	init();
	int continuer = 1;
	while (continuer)
	{
		int i, j, m, n;
		// lit la video
		result = sceUsbCamReadVideoFrameBlocking(buffer, MAX_VIDEO_FRAME_SIZE);
		if (result < 0)
		{
			printf("Error 0x%08X in sceUsbCamReadVideoFrameBlocking,\n", result);
			sceKernelExitDeleteThread(result);
		}

		result = sceJpegDecodeMJpeg(buffer, result, framebuffer, 0);
		if (result < 0)
		{
			printf("Error 0x%08X decoding mjpeg data.\n", result);
			sceKernelExitDeleteThread(result);
		}
		sommeX = sommeY = nbCorres = 0;
		for (i = 0; i < 272; i++)
		{
			m = i*480;
			n = i*512;
			for (j = 0; j < 480; j++)
			{
				couleur tmp = rgba2coul(framebuffer[m+j]);
				if (tmp.r>190 && tmp.g<60 && tmp.b<60) // ~red
				 {
				  nbCorres++;
				  sommeY += i;
				  sommeX += j;
				  //shotBuffer[m+j] = vram[n+j] = 0x00ffffff;
				  //tampon[m+j] = 0x00ffffff;
				 }
				//else shotBuffer[m+j] = vram[n+j] = framebuffer[m+j];
				else tampon[m+j] = framebuffer[m+j];
			}
		}
		if(nbCorres) {
		  xPerso = sommeX / nbCorres;
		  yPerso = sommeY / nbCorres;
		  int b = 5;
		 }
		blitBMP(tampon, xPerso-PERSO, yPerso-PERSO, perso);
		bouge();//bouge les balles
		affObj(); // affiche les balles
		if(colision()) continuer= 0;
		
		//--------------------
		//flip l'affichage
		for (i = 0; i < 272; i++)
		{
		 m = i*480;
		 n = i*512;
		 for (j = 0; j < 480; j++)
		  {
		    shotBuffer[m+j] = vram[n+j] = tampon[m+j];
		  }
		}
		//--------------------


		
		if (takephoto)
		{
		 saveScreenPsp(shotBuffer);
		 takephoto = 0;
		}
	}
	 pspDebugScreenClear();
	 printf("Perdu !\n\n");
	 printf("Score : %d\n\n\n", pts);
	 printf("Merci d'avoir pris le temps de jouer\n\n\n\n");
	 printf("[X] pour recommencer");
	 SceCtrlData pad;
	 while (1)
	 {
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			break;
		}

		sceKernelDelayThread(50000);
	 }
	}
	sceKernelExitDeleteThread(0);
	return 0;	
}

#define TEMPO 0

int oldmain()
{
	SceUID thid;
	SceCtrlData pad, oldpad;
		
	SetupCallbacks();
        srand(time(0));
	pspDebugScreenInit();
	printf("Evite moi ca.\n\n");
	printf("Guyver2, merci Kururin\n\n\n");
	printf("Branche la camera , mets toi en face d'un point rouge\net appuis sur [X]\n");
//----------------------------------------------- modifications begining
        perso = loadBMPfromFile("./datas/perso.bmp");
        balle = loadBMPfromFile("./datas/meteore.bmp");
	int i;
        for (i=0; i<10; i++)
         {
          objet* tmp = &tabObjet[i];
          tmp->actif = 1;
	  newObj(tmp);
         }
        
//------------------------------------------------ end of modifications

	while (1)
	{
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			break;
		}

		sceKernelDelayThread(50000);
	}

	if (LoadModules() < 0)
		sceKernelSleepThread();
	if (InitJpegDecoder() < 0)
		sceKernelSleepThread();
	
	if (StartUsb() < 0)
		sceKernelSleepThread();

	if (sceUsbActivate(PSP_USBCAM_PID) < 0)
	{
		printf("Error activating the camera.\n");
		sceKernelSleepThread();
	}

	//if (InitJpegDecoder() < 0)
	//	sceKernelSleepThread();

	while (1)
	{
		if ((sceUsbGetState() & 0xF) == PSP_USB_CONNECTION_ESTABLISHED)
			break;

		sceKernelDelayThread(50000);
	}

	waitphoto = sceKernelCreateSema("WaitPhotoSema", 0, 0, 1, NULL);
	if (waitphoto < 0)
	{
		printf("Cannot create semaphore.\n");
		sceKernelSleepThread();
	}

	thid = sceKernelCreateThread("video_thread", video_thread, 16, 256*1024, 0, NULL);
	if (thid < 0)
	{
		printf("Cannot create video thread.\n");
		sceKernelSleepThread();
	}

	if (sceKernelStartThread(thid, 0, NULL) < 0)
	{
		printf("Cannot start video thread.\n");
		sceKernelSleepThread();
	}

	oldpad.Buttons = 0xFFFFFFFF;
	long tempo = TEMPO;
	while (1)
	{
	 tempo--;
	 if (tempo<0) tempo = 0;
		sceCtrlPeekBufferPositive(&pad, 1);
		if (pad.Buttons != oldpad.Buttons)
		{
			if (pad.Buttons & PSP_CTRL_RTRIGGER)
			{
				takephoto = 1;
			}
			if (pad.Buttons & PSP_CTRL_LEFT && !tempo)
			{
			 // si left
			}
			else if (pad.Buttons & PSP_CTRL_RIGHT && !tempo)
			{
			 // si right
			}
		}

		oldpad.Buttons = pad.Buttons;
		sceKernelDelayThread(50000);
	}

	return 0;
}








/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	StopApp();
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void *) exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

int LoadModules()
{
	int result = sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC);
	if (result < 0)
	{
		printf("Error 0x%08X loading usbacc.prx.\n", result);
		return result;
	}

	result = sceUtilityLoadUsbModule(PSP_USB_MODULE_CAM);	
	if (result < 0)
	{
		printf("Error 0x%08X loading usbcam.prx.\n", result);
		return result;
	}

	// For jpeg decoding
	result = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if (result < 0)
	{
		printf("Error 0x%08X loading avcodec.prx.\n", result);
	}

	return result;
}

int UnloadModules()
{
	int result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_CAM);
	if (result < 0)
	{
		printf("Error 0x%08X unloading usbcam.prx.\n", result);
		return result;
	}

	result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_ACC);
	if (result < 0)
	{
		printf("Error 0x%08X unloading usbacc.prx.\n", result);
		return result;
	}

	result = sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
	if (result < 0)
	{
		printf("Error 0x%08X unloading avcodec.prx.\n", result);
	}

	return result;
}

int StartUsb()
{
	int result = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbbus driver.\n", result);
		return result;
	}

	result = sceUsbStart(PSP_USBACC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbacc driver.\n", result);
		return result;
	}
	
	result = sceUsbStart(PSP_USBCAM_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbcam driver.\n", result);
		return result;
	}

	result = sceUsbStart(PSP_USBCAMMIC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbcammic driver.\n", result);		
	}

	return result;
}

int StopUsb()
{
	int result = sceUsbStop(PSP_USBCAMMIC_DRIVERNAME, 0, 0);	
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbcammic driver.\n", result);
		return result;
	}
	
	result = sceUsbStop(PSP_USBCAM_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbcam driver.\n", result);
		return result;
	}

	result = sceUsbStop(PSP_USBACC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbacc driver.\n", result);
		return result;
	}

	result = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbbus driver.\n", result);
	}

	return result;
}

int InitJpegDecoder()
{
	int result = sceJpegInitMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X initing MJPEG library.\n", result);
		return result;
	}

	result = sceJpegCreateMJpeg(480, 272);
	if (result < 0)
	{
		printf("Error 0x%08X creating MJPEG decoder context.\n", result);
	}

	return result;
}

int FinishJpegDecoder()
{
	int result = sceJpegDeleteMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X deleting MJPEG decoder context.\n", result);
		return result;
	}

	result = sceJpegFinishMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X finishing MJPEG library.\n", result);
	}

	return result;
}


void StopApp()
{
	sceUsbDeactivate(PSP_USBCAM_PID);
	StopUsb();
	FinishJpegDecoder();
	UnloadModules();
}
