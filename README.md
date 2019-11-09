# xhotkey
hotkeys for X.org, inspired by the suckless style

## Usage
There is no config file. At the top of the source code is a struct that defines all the hotkeys. You edit the config,
then recompile and restart xhotkeys.

If you define an alias, you can do this quickly and easily:
`alias vihk='cd ~/projects/xhotkey && $EDITOR xhotkey.c && make && killall xhotkey; cd && ~/projects/xhotkey/xhotkey &!'`

There are KeyDown hotkeys and KeyUp hotkeys. KeyUp is useful for hotkeys that work better when no keyboard keys
are pressed (like pasting text).

There are 3 ways to run an external program.
- `shell`
- `spawn`
- `spawna`

#### `shell`
Run a string in the `sh` interpreter. (Easiest)

#### `spawn`
Run a command without shell interpreter.

#### `spawna`
Same as `spawn`, but pass arguments to the command.

## Why?
Because I like the suckless config-in-code style. Why import a config-parsing library, parse a config file every time
the program starts, and have to dynamically allocate a datastructure to store the config in the program's memory? For
anyone who likes C, the config file format is pretty simple. Actually, I much prefer this to the xbindkeys style config.

### Potential future improvements
- Put config into a config.h file to make it easier to track changes without merge conflicts
- Move non-generic functions into a separate file so there can be a sort of "plugin" architecture
- Make a hotkey for editing the hotkey config and recompiling+restarting (So META!)

These depend on if anyone else likes the idea and wants to use it, though. Open an issue and I'll probably do it =)
