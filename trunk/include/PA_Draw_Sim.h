#ifndef __PA_DRAW_SIM_H__
#define __PA_DRAW_SIM_H__ 1

#pragma pack(1)

typedef struct{
	u16 Id; // ?
	u32 Length;
	u16 Nothing1, Nothing2; // ?
	u16 ImageStart1, ImageStart2; // Offset of start of image, start at position 0x0A, which can only be 2-byte aligined
} BMPHeader0;

typedef struct{
	u32 SizeofHeader; // 40
	u32 Width, Height;
	u16 Colorplanes; // Usually 1
	u16 BitsperPixel; //1, 2, 4, 8, 16, 24, 32
	u32 Compression;  // 0 for none, 1...
	u32 SizeofData; // Not reliable
	u32 WidthperMeter, HeightperMeter; // Don't care
	u32 NColors, ImportantColors; // Number of colors used, important colors ?
} BMP_Headers;

void PA_LoadBmpToBuffer(u16 *Buffer, s16 x, s16 y, void *bmp, s16 SWidth);

#endif // __PA_DRAW_SIM_H__
