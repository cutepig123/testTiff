#include "mytiffutil.h"
#include <windows.h>
#include <assert.h>
#include <stdio.h>

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
