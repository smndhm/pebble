# World Cup 2026

A Pebble/Rebble watchface for the 2026 World Cup, with digits inspired by the tournament logo.

## Features

- **Custom typeface** — digits drawn as polygon paths at runtime, faithful to the WC26 typographic style
- **5 display modes** — from a minimal logo to a full clock with overlapping pairs
- **48 team themes** — preset colors for every qualified nation (superposition mode)
- **Full color customization** — background, two digit colors, overlap zone
- **Pixel style** — retro pixel-grid overlay in logo mode
- **Geolocation** — auto-detects your country and pre-selects your team at first setup
- **Multilingual** — settings UI in French, English, Spanish, Portuguese, German
- **All platforms** — aplite, basalt, chalk, diorite, emery, flint

## Display modes

| Logo | Logo Pixel | Superposition |
|---|---|---|
| ![](screenshots/basalt/france/logo.png) | ![](screenshots/basalt/france/logo_pixel.png) | ![](screenshots/basalt/france/sup_minutes.png) |

| Superposition H+M | Classique | B&W |
|---|---|---|
| ![](screenshots/basalt/france/sup_hm.png) | ![](screenshots/basalt/france/classic.png) | ![](screenshots/aplite/logo.png) |

### Logo
Two large minute digits stacked vertically. Clean, minimal.

### Logo Pixel
Same as Logo with a pixel-grid effect rendered in the digit colors. Not available on B&W platforms (aplite, diorite, flint).

### Superposition — Minutes
Two minute digits arranged diagonally with a large overlap, in the style of Panini WC26 sticker cards.

### Superposition — Heure + Minutes
Two overlapping digit pairs — hours on top, minutes below. Shows full HH MM.

### Classique
Standard 2×2 grid: hours on the top row, minutes on the bottom.

## Settings

| Setting | Modes | Description |
|---|---|---|
| Mode | All | Logo / Superposition / Classique |
| Style | Logo only | Normal or Pixel |
| Affichage | Superposition only | Minutes only or Heure + Minutes |
| Équipe | Superposition only | 48 qualified nations with preset colors |
| Fond | All | Background color |
| Chiffre 1 | Custom only | Left digit color |
| Chiffre 2 | Custom only | Right digit color |
| Chevauchement | Superposition + custom | Overlap zone color |

Choosing a team preset fills all four colors automatically. Select **— Personnalisé —** to set colors freely.

On B&W platforms (aplite, diorite, flint), color and team settings are hidden — the watchface always renders in white digits on a black background. The overlap zone uses a checkerboard dither pattern to simulate gray.

## Teams

48 qualified nations, sorted alphabetically:

Afrique du Sud · Algérie · Allemagne · Angleterre · Arabie Saoudite · Argentine · Australie · Autriche · Belgique · Bosnie-Herzégovine · Brésil · Canada · Cap-Vert · Colombie · Corée du Sud · Côte d'Ivoire · Croatie · Curaçao · Écosse · Égypte · Équateur · Espagne · États-Unis · France · Ghana · Haïti · Irak · Iran · Japon · Jordanie · Maroc · Mexique · Norvège · Nouvelle-Zélande · Ouzbékistan · Panama · Paraguay · Pays-Bas · Portugal · Qatar · RD Congo · Sénégal · Suède · Suisse · Tchéquie · Tunisie · Türkiye · Uruguay

## Build

```bash
cd watchfaces/fwc26
pebble build
pebble install --emulator basalt
```

Digit paths are generated from the SVG sources in `resources/svg/`:

```bash
python3 tools/font/svg_to_c.py watchfaces/fwc26       # from resources/svg/*.svg
python3 tools/font/fontra_to_c.py watchfaces/fwc26    # from Fontra glyph editor
```

## Credits

The digit typeface was hand-crafted from the WC26 logo letterforms — each glyph redrawn as polygon paths faithful to the official visual identity.  
Built with the [Rebble SDK](https://rebble.io) and [Clay](https://github.com/pebble/clay).  
Author: [smndhm](https://github.com/smndhm)
