#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\_keipcpostbox.h>

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	_KePrint("Terminal v1.0 running \n");
	int postbox_fd = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	uint16_t thr_id = _KeGetThreadID();
	PostEvent e;
	e.type = 50;
	e.dword = 200;
	e.dword2 = 200;
	e.dword3 = 400;
	e.dword4 = 400;
	e.dword5 = (1 << 0);
	e.to_id = POSTBOX_ROOT_ID;
	e.from_id = thr_id;
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
	while (1) {
		_KePauseThread();
	}
}