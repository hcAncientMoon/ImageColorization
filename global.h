#ifndef GLOBAL_H
#define GLOBAL_H

enum LoadCase{
    GRAY,SCRIBBLE
};
enum ColorizationMethod{
    LEVIN,GEODESIC,QUADTREEDECOMP
};
enum MediaType{
    IMAGE,VIDEO
};
enum VideoColorizationMethod{
    FRAMESUB,MOTIONESTIMATE,MIXED,KEYFRAME
};
#define DELTA 1e-5
#define ACCURACYCONTROL(para){if(para<DELTA)para = DELTA;}
#define MAXCHANNEL 4
#define BLENDCNT 3
#define MAX_BLEND_COLOR 3
#define MAX_DISTANCE 65535
#define MAXDISTANCE 1E20
typedef unsigned long COLORREF;
typedef unsigned long DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#define MBLOCKSIZE 8
const bool INDEBUG = false;
const bool SHOWINFO = true;
const int cntOfSample = 5000;
#endif // GLOBAL_H
