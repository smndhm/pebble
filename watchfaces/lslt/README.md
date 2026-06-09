# LSLT

A minimalist Pebble/Rebble watchface that animates a custom variable font — digit weight grows smoothly over time.

| Thin | Color | Bold |
|---|---|---|
| ![](screenshots/basalt/thin.png) | ![](screenshots/basalt/mid.png) | ![](screenshots/basalt/bold.png) |

## How it works

Each digit transitions from thin to bold as time progresses:

- **Hour digits** — weight grows from 00 to 59 minutes, resets on the hour
- **Minute digits** — weight grows from 0 to 59 seconds, resets on the minute

## Settings

- **Background** — any Pebble color
- **Foreground** — any Pebble color

## Platforms

All six Pebble platforms: aplite, basalt, chalk, diorite, emery, flint.

## Build

```bash
cd watchfaces/lslt
pebble build
pebble install --emulator basalt
```

## Credits

Built with the [Rebble SDK](https://rebble.io) and [Clay](https://github.com/pebble/clay).  
Author: [smndhm](https://github.com/smndhm)
