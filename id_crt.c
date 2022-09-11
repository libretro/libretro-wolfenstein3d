// File: id_crt.c
// Project: Wolf4SDL
// Author: André Guilherme
// Creation date: 2022-07-11 
// Description: 
// This file fixes the crt bug
// using 4:3 and Makes support of screenshot.
// Note: This file is heavyly modified of the following repo: 
// https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D/blob/master/crt.h
// Credits for the following pepole: Fabien sanglard and zZeck
// and the original file creation date: 2014-08-26.

#ifndef id_crt_c
#define id_crt_c

#include "id_crt.h"

#ifdef CRT
#include "id_vl.h"
#include "id_in.h"
static int width;
static int height;

u8 coloredFrameBuffer[320 * 200 * 3];

#if SDL_MAJOR_VERSION == 1
GLuint crtTexture;
#endif
void CRT_Init(int _width) 
{
    width = _width;
    height = _width * 3.0 / 4.0;

#if SDL_MAJOR_VERSION == 1  
    //Alloc the OpenGL texture where the screen will be uploaded each frame.
    glGenTextures(1, &crtTexture);
    glBindTexture(GL_TEXTURE_2D, crtTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,         // target
        0,                     // level, 0 = base, no minimap,
        GL_RGB,                // internalformat
        320,                   // width
        200,                   // height
        0,                     // border, always 0 in OpenGL ES
        GL_RGB,                // format
        GL_UNSIGNED_BYTE,      // type
        0
    );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
 
    SDL_GL_SwapBuffers();   
#elif SDL_MAJOR_VERSION == 2
    SDL_Texture* texture = SDL_CreateTexture(renderer, NULL, 0, width, height);
    SDL_GetTextureColorMod(texture, 0xFF, 0xFF, 0xFF);
    SDL_UpdateTexture(texture, NULL, screen->pixels, screenWidth * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);
#endif
}

void CRT_DAC(void) 
{
#if SDL_MAJOR_VERSION == 1
    // Grab the screen framebuffer from SDL
    SDL_Surface* screen = screenBuffer;

    //Convert palette based framebuffer to RGB for OpenGL
    byte* pixelPointer = coloredFrameBuffer;
    for (int i = 0; i < 320 * 200; i++) {
        u8 paletteIndex;
        paletteIndex = ((byte*)screen->pixels)[i];
        *pixelPointer++ = curpal[paletteIndex].r;
        *pixelPointer++ = curpal[paletteIndex].g;
        *pixelPointer++ = curpal[paletteIndex].b;
    }


    //Upload texture
    glBindTexture(GL_TEXTURE_2D, crtTexture);
    glTexSubImage2D(GL_TEXTURE_2D,
        0,
        0,
        0,
        320,
        200,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        coloredFrameBuffer);

    //Draw a quad with the texture
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3i(0, 0, 0);
    glTexCoord2f(0, 0); glVertex3i(0, height, 0);
    glTexCoord2f(1, 0); glVertex3i(width, height, 0);
    glTexCoord2f(1, 1); glVertex3i(width, 0, 0);
    glEnd();


    //Flip buffer
    SDL_GL_SwapBuffers();
#elif SDL_MAJOR_VERSION == 2
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screenBuffer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_GetKeyboardState(NULL);
#endif

    static int wasPressed = 0;
    if(Keyboard(sc_I))
    {
        if(!wasPressed) 
        {
            wasPressed = 1;
            CRT_Screenshot();
        }
        else
        {
            wasPressed = 0;
        }
    }

}
void CRT_FreeScreenshot(SDL_Surface* surface1, SDL_Surface *surface2)
{
    SDL_FreeSurface(surface1);
    SDL_FreeSurface(surface2);
}

void BlitImage(SDL_Surface* img1src, SDL_Surface* img1dst, SDL_Surface* img2src, SDL_Surface* img2dst)
{
    SDL_BlitSurface(img1src, NULL, img1dst, NULL);
    SDL_BlitSurface(img2src, NULL, img2dst, NULL);
}

void CRT_Screenshot(void) 
{
    const char* filename = "screenshot.bmp";
    int aspectWidth = 640;
    int aspectHeight = 440;

    printf("Screenshot.\n");

    SDL_Surface* correctAspect = SDL_CreateRGBSurface(0, aspectWidth, aspectHeight, 32, 0, 0, 0, 0);
    SDL_Surface* incorrectAspect = SDL_CreateRGBSurface(0, screenBuffer->w, screenBuffer->h, 32, 0, 0, 0, 0);
    //I Know This is wrong but it works...
    BlitImage(screenBuffer, incorrectAspect, incorrectAspect, correctAspect);
    SDL_SaveBMP(correctAspect, filename);
    CRT_FreeScreenshot(correctAspect, incorrectAspect);
}

#endif
#endif