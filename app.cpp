
#include <tiffio.h>
#include <windows.h>
#include <assert.h>
#include <vector>

void MyTRACE(char const *acFormat,...)	//for debug
{
			
	va_list l;
	char acStr[500];
	va_start(l,acFormat);
	::_vsnprintf(acStr,500,acFormat,l);
	OutputDebugStringA(acStr);
	
}

enum BufType
{
	TYPE_32F = 1,
	TYPE_16S = 2,
	TYPE_16U = 3,
};

struct MyBuf
{
	void *p;
	BufType type;
	int w, h, linestep;
};

int typeSize(BufType t)
{
	if (t == TYPE_16S || t == TYPE_16U)
		return 2;
	return 4;
}
int tiffSize_(TIFF *image, int *W, int *H, int *linestep, BufType *Type)
{
  uint16 bps, spp, sfm;
  uint32 width;
  tsize_t stripSize;
  unsigned long imageOffset;
  int stripMax;
  
  //unsigned long bufferSize, count;
  
  // Check that it is of a type that we support
  if((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bps) == 0) ){
    return(42);
  }
  
  MyTRACE( "bits per sample(%d)\n", bps);

  //float type
   if((TIFFGetField(image, TIFFTAG_SAMPLEFORMAT, &sfm) == 0) ){
    
    return(42);
  }

   MyTRACE( "TIFFTAG_SAMPLEFORMAT(%d)\n", sfm);

   if (bps == 32 && sfm == SAMPLEFORMAT_IEEEFP)
	   *Type = TYPE_32F;
   else if (bps == 16 && (sfm == SAMPLEFORMAT_INT))
	   *Type = TYPE_16S;
   else if (bps == 16 && (sfm == SAMPLEFORMAT_UINT))
	   *Type = TYPE_16U;
   else
	   return 43;

  if((TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &spp) == 0) || (spp != 1)){
    MyTRACE( "Either undefined or unsupported number of samples per pixel\n");
    return(42);
  }
  // Read in the possibly multiple strips
  stripSize = TIFFStripSize (image);
  *linestep = stripSize;
  stripMax = TIFFNumberOfStrips (image);
  imageOffset = 0;
  long height = stripMax;
  *H = height;
  // Do whatever it is we do with the buffer -- we dump it in hex
  if(TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width) == 0){
    MyTRACE( "Image does not define its width\n");
    return(42);
  }
  *W = width;

  assert(typeSize(*Type)*width <= stripSize);
  return 0;
}
 


int logBuf(MyBuf const &b, const char*file)
{
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	fprintf(fp, "%d %d %d\n", b.w, b.h, b.type);
	int nt = typeSize(b.type);
	for (int y = 0; y < b.h; y++)
	{
		for (int x = 0; x < b.w; x++)
		{
			const char *p = (char*)b.p +y*b.linestep + nt*x;
			if (b.type == TYPE_16S)
				fprintf(fp, "%hd ", (short&)*p);
			else if (b.type == TYPE_16U)
				fprintf(fp, "%hu ", (unsigned short&)*p);
			else if (b.type == TYPE_32F)
				fprintf(fp, "%f ", (float&)*p);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}
int freeBuf(MyBuf *p)
{
	free(p->p);
	p->p = 0;
	return 0;
}

int readTiff(const char*file, MyBuf *buf){
  TIFF *image;
  
 
  // Open the TIFF image
  if((image = TIFFOpen(file, "r")) == NULL){
    MyTRACE( "Could not open incoming image\n");
    return(42);
  }

  int sts = tiffSize_(image, &buf->w, &buf->h, &buf->linestep, &buf->type);
   if(sts!=0) goto Exit;

   assert(buf->type == TYPE_32F || buf->type == TYPE_16S || buf->type == TYPE_16U);
   
  unsigned long imageOffset = 0;
 	
  unsigned long  bufferSize = buf->h * buf->linestep;
  buf->p = malloc(bufferSize);

  for (int stripCount = 0; stripCount < buf->h; stripCount++)
  {	
	  int result;
    if((result = TIFFReadEncodedStrip (image, stripCount,
		(char*)buf->p + imageOffset,
		buf->linestep)) == -1){
      MyTRACE( "Read error on input strip number %d\n", stripCount);
      return(42);
    }
    imageOffset += result;
  }
  // Deal with photometric interpretations
  uint16 photo;
  if(TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photo) == 0){
    MyTRACE( "Image has an undefined photometric interpretation\n");
    return(42);
  }
  
  MyTRACE( "TIFFTAG_PHOTOMETRIC %d\n", photo);

  //if(photo != PHOTOMETRIC_MINISWHITE){
	
  //  // Flip bits
  //  // MyTRACE("Fixing the photometric interpretation\n");
  //  // for(count = 0; count < bufferSize; count++)
  //    // buffer[count] = ~buffer[count];
  //}
  // Deal with fillorder
 // if(TIFFGetField(image, TIFFTAG_FILLORDER, &fillorder) == 0){
 //   MyTRACE( "Image has an undefined fillorder\n");
 //   return(42);
 // }
 // 
 // if(fillorder != FILLORDER_MSB2LSB){
 //   // We need to swap bits -- ABCDEFGH becomes HGFEDCBA
	//MyTRACE( "Image has an undefined fillorder2\n");
 //   // MyTRACE("Fixing the fillorder\n");
 //   // for(count = 0; count < bufferSize; count++){
 //     // tempbyte = 0;
 //     // if(buffer[count] & 128) tempbyte += 1;
 //     // if(buffer[count] & 64) tempbyte += 2;
 //     // if(buffer[count] & 32) tempbyte += 4;
 //     // if(buffer[count] & 16) tempbyte += 8;
 //     // if(buffer[count] & 8) tempbyte += 16;
 //     // if(buffer[count] & 4) tempbyte += 32;
 //     // if(buffer[count] & 2) tempbyte += 64;
 //     // if(buffer[count] & 1) tempbyte += 128;
 //     // buffer[count] = tempbyte;
 //   // }
 // }
     
  
  // FILE *fp = fopen("c:\\temp\\tiff.log","w");

  // for(count = 0; count < bufferSize; count++){
    // fprintf(fp, "%03f ",  buffer[count]);
    // if((count + 1) % (width / 8) == 0) fprintf(fp, "\n");
    // else fprintf(fp, " ");
  // }
  // fclose(fp);
Exit:
  TIFFClose(image);
  return 0;
}

int main(int c, char **v)
{
	system("pause");
	assert(c > 1);
	
	int wSts = 0;
	char *file = v[1];

	MyBuf buf;
	wSts = readTiff(file, &buf);
	if (wSts != 0) goto Exit;

	char desFile[300];
	_snprintf(desFile, 300, "%s.dat", file);
	logBuf(buf, desFile);

	freeBuf(&buf);
Exit:
	return wSts;
}