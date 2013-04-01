#include <stdio.h>
typedef struct tagBITMAPFILEHEADER {
  unsigned short bfType;
  unsigned int   bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int   bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER{
  unsigned int   biSize;
  int            biWidth;
  int            biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int   biCompress;
  unsigned int   biSizeImg;
  int            biXppm;
  int            biYppm;
  unsigned int   biClrUsed;
  unsigned int   biClrImpt;
} BITMAPINFOHEADER;
typedef struct tagRGBQUAD{
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved;
} RGBQUAD;

int main(int argc, char* argv[])
{
  if( argc < 2 ) return 0;
  FILE* fp = fopen(argv[1], "r");
  BITMAPFILEHEADER bmpFileHeader;
  BITMAPINFOHEADER bmpInfoHeader;
  RGBQUAD          pallet[2];

  fread(&bmpFileHeader.bfType,      sizeof(bmpFileHeader.bfType), 1, fp);
  fread(&bmpFileHeader.bfSize,      sizeof(bmpFileHeader.bfSize), 1, fp);
  fread(&bmpFileHeader.bfReserved1, sizeof(bmpFileHeader.bfReserved1), 1, fp);
  fread(&bmpFileHeader.bfReserved2, sizeof(bmpFileHeader.bfReserved2), 1, fp);
  fread(&bmpFileHeader.bfOffBits,   sizeof(bmpFileHeader.bfOffBits), 1, fp);
  fread(&bmpInfoHeader.biSize,     sizeof(bmpInfoHeader.biSize), 1, fp);
  fread(&bmpInfoHeader.biWidth,    sizeof(bmpInfoHeader.biWidth), 1, fp);
  fread(&bmpInfoHeader.biHeight,   sizeof(bmpInfoHeader.biHeight), 1, fp);
  fread(&bmpInfoHeader.biPlanes,   sizeof(bmpInfoHeader.biPlanes), 1, fp);
  fread(&bmpInfoHeader.biBitCount, sizeof(bmpInfoHeader.biBitCount), 1, fp);
  fread(&bmpInfoHeader.biCompress, sizeof(bmpInfoHeader.biCompress), 1, fp);
  fread(&bmpInfoHeader.biSizeImg,  sizeof(bmpInfoHeader.biSizeImg), 1, fp);
  fread(&bmpInfoHeader.biXppm,     sizeof(bmpInfoHeader.biXppm), 1, fp);
  fread(&bmpInfoHeader.biYppm,     sizeof(bmpInfoHeader.biYppm), 1, fp);
  fread(&bmpInfoHeader.biClrUsed,  sizeof(bmpInfoHeader.biClrUsed), 1, fp);
  fread(&bmpInfoHeader.biClrImpt,  sizeof(bmpInfoHeader.biClrImpt), 1, fp);
  fread(pallet, sizeof(pallet[0]), 2, fp);
  int byteWidth = bmpInfoHeader.biWidth / 8;
  int height    = bmpInfoHeader.biHeight;
  unsigned char line[height][byteWidth];
  for( int i = height - 1 ; i >= 0 ; --i ) fread(line[i], 1, byteWidth, fp);

#define Z '1'
#define O '0'
  for( int i = 0 ; i < height ; ++i ) {
    for( int j = 0 ; j < byteWidth ; ++j ) {
      unsigned char c = line[i][j];
      printf(" %c %c %c %c %c %c %c %c",
	     (c & 0x80)?Z:O, (c & 0x40)?Z:O, (c & 0x20)?Z:O, (c & 0x10)?Z:O,
	     (c & 0x08)?Z:O, (c & 0x04)?Z:O, (c & 0x02)?Z:O, (c & 0x01)?Z:O);
    }
    puts("");
  }
#undef Z
#undef O
  fclose(fp);

  fprintf(stderr, "BITMAPFILEHEADER\n");
  fprintf(stderr, " bfType      = %d\n", bmpFileHeader.bfType);
  fprintf(stderr, " bfSize      = %d\n", bmpFileHeader.bfSize);
  fprintf(stderr, " bfReserved1 = %d\n", bmpFileHeader.bfReserved1);
  fprintf(stderr, " bfReserved2 = %d\n", bmpFileHeader.bfReserved2);
  fprintf(stderr, " bfOffBits   = %d\n", bmpFileHeader.bfOffBits);
  fputs("\n", stderr);
  fprintf(stderr, "BITMAPINFOHEADER\n");
  fprintf(stderr, " biSize     = %d\n", bmpInfoHeader.biSize);
  fprintf(stderr, " biWidth    = %d\n", bmpInfoHeader.biWidth);
  fprintf(stderr, " biHeight   = %d\n", bmpInfoHeader.biHeight);
  fprintf(stderr, " biPlanes   = %d\n", bmpInfoHeader.biPlanes);
  fprintf(stderr, " biBitCount = %d\n", bmpInfoHeader.biBitCount);
  fprintf(stderr, " biCompress = %d\n", bmpInfoHeader.biCompress);
  fprintf(stderr, " biSizeImg  = %d\n", bmpInfoHeader.biSizeImg);
  fprintf(stderr, " biXppm     = %d\n", bmpInfoHeader.biXppm);
  fprintf(stderr, " biYppm     = %d\n", bmpInfoHeader.biYppm);
  fprintf(stderr, " biClrUsed  = %d\n", bmpInfoHeader.biClrUsed);
  fprintf(stderr, " biClrImpt  = %d\n", bmpInfoHeader.biClrImpt);

  return 0;
}
