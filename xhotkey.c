#include <sys/wait.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
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
#define ARGS(...)				(const char*[]){__VA_ARGS__, NULL}
#define SHIFT (0x1)
#define CTRL (0x4)
#define ALT (0x8)
#define CMD (0x40)

#define NUMLOCK (0x10)

void dmenu_run(void *param);
void spawn(void *program);
void spawna(void *args);
void shell(void *command);
void test(void *param);

const char terminal[] = "st";
const char editor[] = "nvim";

struct hotkey {
	unsigned int mods;
	KeySym keysym;
	void (*action)(void *param);
	void *param;

	KeyCode keycode; // filled in during run
};

struct hotkey hotkeys[] = {
	{0, XF86XK_AudioLowerVolume, spawna, ARGS("amixer", "set", "Master", "2%-"), 0},
	{0, XF86XK_AudioRaiseVolume, spawna, ARGS("amixer", "set", "Master", "2%+"), 0},
	{0, XF86XK_AudioMute, spawna, ARGS("amixer", "set", "Master", "toggle"), 0},
	{CMD, XF86XK_AudioMute, spawna, ARGS(terminal, "alsamixer"), 0},

	{CMD, XF86XK_AudioLowerVolume, spawna, ARGS("mpc", "volume", "-1"), 0},
	{CMD, XF86XK_AudioRaiseVolume, spawna, ARGS("mpc", "volume", "+1"), 0},
	{SHIFT, XF86XK_AudioLowerVolume, spawna, ARGS("mpc", "volume", "-5"), 0},
	{SHIFT, XF86XK_AudioRaiseVolume, spawna, ARGS("mpc", "volume", "+5"), 0},
	{CTRL, XF86XK_AudioLowerVolume, spawna, ARGS("mpc", "volume", "-10"), 0},
	{CTRL, XF86XK_AudioRaiseVolume, spawna, ARGS("mpc", "volume", "+10"), 0},

	{CMD, XK_F1, spawn, "firefox", 0},
	{CMD, XK_F2, spawn, "/home/matt/bin/claws-mail", 0},
	{CMD, XK_F3, spawn, "chromium", 0},
	{CMD, XK_F4, spawn, "thunar", 0},
	{CMD, XK_F5, test, "hello", 0},

	{CMD, XK_F6, spawn, "mumble", 0},
	{CMD, XK_F7, spawn, "switch-headphones-alsa", 0},
	{CMD, XK_F8, spawn, "switch-lineout-alsa", 0},

	{CMD, XK_E, spawn, "emacs", 0},
	{CMD|SHIFT, XK_E, spawna, ARGS("emacs", "."), 0},
	{CMD, XK_V, spawna, ARGS(terminal, editor), 0},
	{CMD|SHIFT, XK_V, spawna, ARGS(terminal, editor, "."), 0},

	{CMD|SHIFT|CTRL, XK_Return, spawna, ARGS("st", "-f", "spleen:pixelsize=24:antialias=false:autohint=false"), 0},
	{CMD, XK_G, spawn, "dmenu-surf.sh", 0},
	{CMD|SHIFT, XK_H, spawn, "dmenu-man", 0},
	{CMD, XK_R, dmenu_run, 0, 0},
	{CMD, XK_W, spawn, "pass_chooser", 0},
	{CMD|SHIFT, XK_W, spawn, "pass_qr_chooser", 0},
	{CMD|SHIFT, XK_L, spawna, ARGS("xscreensaver-command", "-lock"), 0},

	{CMD, XK_minus, spawn, "psi", 0},
	{CMD, XK_slash, spawn, "telegram-desktop", 0},
	{CMD, XK_equal, spawn, "discord", 0},
	{CMD|SHIFT, XK_minus, spawn, "slack", 0},
	{CMD|SHIFT, XK_slash, spawn, "signal-desktop", 0},
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

	signal(SIGCHLD, SIG_IGN);

	for (int i=0; i < LENGTH(hotkeys); i++) {
		KeyCode keycode = XKeysymToKeycode(dpy, hotkeys[i].keysym);
		hotkeys[i].keycode = keycode;
		XGrabKey(dpy, keycode, hotkeys[i].mods, root, owner_events, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keycode, hotkeys[i].mods|NUMLOCK, root, owner_events, GrabModeAsync, GrabModeAsync);
	}

	XSelectInput(dpy, root, KeyPressMask);
	while (true) {
		unsigned int mods;
		XNextEvent(dpy, &ev);
		switch (ev.type) {
			case KeyPress:
				mods = ev.xkey.state;
				mods = (mods & ~NUMLOCK);
				for (int i=0; i < LENGTH(hotkeys); i++) {
					if (hotkeys[i].keycode == ev.xkey.keycode && hotkeys[i].mods == mods) {
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

	}

	char buffer[4096];
	close(filedes[1]);

	int rcount = read(filedes[0], buffer, sizeof(buffer));
	if (rcount == -1) err(-1, "read");
	close(filedes[0]);

	buffer[rcount] = 0;
	if (buffer[rcount-1] == '\n') buffer[rcount-1] = 0;

	printf("running command: %s\n", buffer);

	signal(SIGCHLD, SIG_DFL); // Python will fail its Popen calls without this
	execlp("/bin/sh", "sh", "-c", buffer, NULL);
	err(-1, "exec dmenu command");
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
