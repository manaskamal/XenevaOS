#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <chitralekha.h>
#include <widgets\window.h>


void TerminalHandleMessage(PostEvent *e) {
	switch (e->type) {
	case DEODHAI_REPLY_MOUSE_EVENT:
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	ChitralekhaApp *app = ChitralekhaStartApp(argc, arv);
	ChWindow* win = ChCreateWindow(app, (1 << 0), "Xeneva Terminal", 300, 300, CHITRALEKHA_DEFAULT_WIN_WIDTH, CHITRALEKHA_DEFAULT_WIN_HEIGHT);
	win->color = BLACK;
	ChWindowPaint(win);

	ChFontDrawText(win->canv, app->baseFont, "Shubho Mahaashtami !!", 10, 40, 10, WHITE);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1);
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	/* open the sound device-file, it is in /dev directory */
	int snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);

	/* allocate an ioctl structure where required information
	* will be putted */
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));

	/* uint_1 holds the millisecond to sleep after
	* one frame playback */
	ioctl.uint_1 = 10;

	/* most important aurora_syscall_magic number, without
	* this, iocontrol system call will not work */
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	/* register the app to sound layer, as it will create
	* a private dsp-box for this app */
	_KeFileIoControl(snd, SOUND_REGISTER_SNDPLR, &ioctl);

	/* now open your sound file, note that here demo is playing
	* a raw wave file with 48kHZ-16bit format, to play mp3 or
	* other format, one needs another conversion layer of samples */

	int song = _KeOpenFile("/song.wav", FILE_OPEN_READ_ONLY);
	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);
	XEFileStatus fs;
	_KeFileStat(song, &fs);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		TerminalHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();

		/* with each frame read the sound, write it
		* to sound device, the sound device will automatically
		* put the app to sleep for smooth playback for some
		* milli-seconds
		*/
		_KeWriteFile(snd, songbuf, 4096);
		_KeReadFile(song, songbuf, 4096);
		_KeFileStat(song, &fs);

		/* after we finish playing the sound, we exit */
		if (fs.eof) {
			_KeCloseFile(song);
			_KeProcessExit();
		}

	}
}