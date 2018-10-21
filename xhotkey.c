#include <sys/wait.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XF86keysym.h>

// More info: https://stackoverflow.com/questions/4037230/global-hotkey-with-x11-xlib

// LENGTH macro borrowed from dwm
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define SHIFT (0x1)
#define CTRL (0x4)
#define ALT (0x8)
#define CMD (0x40)

void dmenu_run(void *param);
void spawn(void *program);
void spawna(void *args);
void shell(void *command);
void test(void *param);

struct hotkey {
	unsigned int mods;
	KeySym keysym;
	void (*action)(void *param);
	void *param;

	KeyCode keycode; // filled in during run
};

struct hotkey hotkeys[] = {
	{0, XF86XK_AudioLowerVolume, spawna, (const char*[]){"amixer", "set", "Master", "2%-", NULL}, 0},
	{0, XF86XK_AudioRaiseVolume, spawna, (const char*[]){"amixer", "set", "Master", "2%+", NULL}, 0},
	{0, XF86XK_AudioMute, spawna, (const char*[]){"amixer", "set", "Master", "toggle", NULL}, 0},

	{0, XK_F1, spawn, "firefox", 0},
	{0, XK_F2, spawn, "/home/matt/bin/claws-mail", 0},
	{0, XK_F3, spawn, "surf", 0},
	{0, XK_F4, spawn, "thunar", 0},
	{CMD, XK_F5, test, "hello", 0},

	{0, XK_F6, spawn, "mumble", 0},
	{0, XK_F7, spawn, "switch-headphones-alsa", 0},
	{0, XK_F8, spawn, "switch-lineout-alsa", 0},

	{CMD, XK_G, spawn, "dmenu-surf.sh", 0},
	//{CMD, XK_R, shell, "`dmenu </dev/null`", 0},
	{CMD, XK_R, dmenu_run, 0, 0},
	{CMD, XK_W, spawn, "pass_chooser", 0},
	{CMD|SHIFT, XK_W, spawn, "pass_qr_chooser", 0},

	{CMD, XK_minus, spawn, "psi", 0},
	{CMD, XK_slash, spawn, "telegram-desktop", 0},
	{CMD, XK_equal, spawn, "discord", 0},
};

int main()
{
	Display *dpy = XOpenDisplay(0);
	if (dpy == NULL) errx(1, "Failed to open X display");

	Window root = DefaultRootWindow(dpy);
	XEvent ev;

	Bool owner_events = False;
	
	// close stdin to keep the forks from reading console input
	close(0);

	for (int i=0; i < LENGTH(hotkeys); i++) {
		KeyCode keycode = XKeysymToKeycode(dpy, hotkeys[i].keysym);
		hotkeys[i].keycode = keycode;
		XGrabKey(dpy, keycode, hotkeys[i].mods, root, owner_events, GrabModeAsync, GrabModeAsync);
	}

	XSelectInput(dpy, root, KeyPressMask);
	while (true) {
		XNextEvent(dpy, &ev);
		switch (ev.type) {
			case KeyPress:
				for (int i=0; i < LENGTH(hotkeys); i++) {
					if (hotkeys[i].keycode == ev.xkey.keycode && hotkeys[i].mods == ev.xkey.state) {
						printf("Keypress: %s mods: 0x%x\n", XKeysymToString(hotkeys[i].keysym), ev.xkey.state);
						hotkeys[i].action(hotkeys[i].param);
					}
				}
				break;
			default:
				break;
		}
	}

	XCloseDisplay(dpy);
	return 0;
}

void dmenu_run(void *param)
{
	pid_t pid = fork();
	if (pid != 0) return;
	
	int filedes[2];
	if (pipe(filedes) == -1) err(-1, "pipe");

	pid = fork();
	if (pid == 0) {
		close(filedes[0]);

		if (dup2(filedes[1], 1) == -1) err(1, "dup2 Error");
		close(filedes[1]);

		execlp("dmenu", "dmenu", NULL);
		err(1, "exec dmenu");

	} else {
		//printf("waiting\n");
		//int wstatus;
		char buffer[4096];
		close(filedes[1]);

		// no need to wait since we have a pipe
		//if (waitpid(pid, &wstatus, 0) == -1) err(-1, "waitpid");


		int rcount = read(filedes[0], buffer, sizeof(buffer));
		if (rcount == -1) err(-1, "read");
		close(filedes[0]);

		buffer[rcount] = 0;
		if (buffer[rcount-1] == '\n') buffer[rcount-1] = 0;

		printf("running command: %s\n", buffer);

		pid = fork();
		if (pid == 0) execlp("/bin/sh", "sh", "-c", buffer, NULL);
		err(-1, "exec dmenu command");
	}
}

void shell(void *command)
{
	pid_t pid = fork();
	if (pid == 0) {
		execlp("/bin/sh", "sh", "-c", command, NULL);
		err(-1, "exec shell");
	}
}

void spawn(void *program)
{
	pid_t pid = fork();
	if (pid == 0) {
		execlp(program, program, NULL);
		err(-1, "exec");
	}
}

void spawna(void *args)
{
	pid_t pid = fork();
	if (pid == 0) {
		execvp(((const char* const*)args)[0], (char* const*)args);
		err(-1, "exec");
	}
} 

void test(void *param)
{
	printf("test function: %s\n", (const char *)param);
}
