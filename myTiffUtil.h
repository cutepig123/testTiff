#pragma once


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

#ifdef myTiffUtil_EXPORTS
#define	APP_XXPORT	_declspec(dllexport)
#else
#define	APP_XXPORT	_declspec(dllimport)
#endif

APP_XXPORT int logBuf(MyBuf const &b, const char*file);

APP_XXPORT int freeBuf(MyBuf *p);

APP_XXPORT int readTiff(const char*file, MyBuf *buf);

APP_XXPORT const void *getp(MyBuf const &b, int x, int y);
