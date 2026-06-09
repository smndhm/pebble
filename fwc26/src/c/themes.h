#pragma once
#include <pebble.h>

// Team colors packed as Pebble 6-bit argb (0b11RRGGBB).
// Teams sorted alphabetically (French names) — indices match Clay dropdown values.
// bg is always sky blue (0xCB) per Panini FWC26 card design.
typedef struct {
  uint8_t bg;   // background (window fill)
  uint8_t fg;   // left digit
  uint8_t fg2;  // right digit
  uint8_t col;  // overlap/collision zone color
} Team;

#define TEAM_CUSTOM  48
#define TEAM_COUNT   48

static const Team kTeams[TEAM_COUNT] = {
  //                             bg     fg     fg2    col
  /*  0 Afrique du Sud   */ { 0xCB, 0xF0, 0xC2, 0xC4 }, // bg=sky blue,  fg=rouge,    fg2=bleu foncé, col=vert très foncé
  /*  1 Algérie          */ { 0xCB, 0xC8, 0xFF, 0xF0 }, // bg=sky blue,  fg=vert,     fg2=blanc,   col=rouge
  /*  2 Allemagne        */ { 0xCB, 0xC0, 0xFC, 0xF0 }, // bg=sky blue,  fg=noir,     fg2=jaune,   col=rouge
  /*  3 Angleterre       */ { 0xCB, 0xFF, 0xFF, 0xF0 }, // bg=sky blue,  fg=blanc,    fg2=blanc,   col=rouge
  /*  4 Arabie Saoudite  */ { 0xCB, 0xC8, 0xC8, 0xFF }, // bg=sky blue,  fg=vert,     fg2=vert,    col=blanc
  /*  5 Argentine        */ { 0xCB, 0xD7, 0xFF, 0xDB }, // bg=sky blue,  fg=bleu pâle, fg2=blanc,  col=bleu pâle
  /*  6 Australie        */ { 0xCB, 0xC3, 0xFF, 0xF0 }, // bg=sky blue,  fg=bleu,     fg2=blanc,   col=rouge
  /*  7 Autriche         */ { 0xCB, 0xF0, 0xFF, 0xF5 }, // bg=sky blue,  fg=rouge,    fg2=blanc,   col=rouge pâle
  /*  8 Belgique         */ { 0xCB, 0xC0, 0xF0, 0xFC }, // bg=sky blue,  fg=noir,     fg2=rouge,   col=jaune
  /*  9 Bosnie-Herzégovine */ { 0xCB, 0xC3, 0xFC, 0xD7 }, // bg=sky blue, fg=bleu,    fg2=jaune,   col=bleu clair
  /* 10 Brésil           */ { 0xCB, 0xC4, 0xC4, 0xFC }, // bg=sky blue,  fg=vert très foncé, fg2=vert très foncé, col=jaune
  /* 11 Canada           */ { 0xCB, 0xF0, 0xFF, 0xF5 }, // bg=sky blue,  fg=rouge,    fg2=blanc,   col=rouge clair
  /* 12 Cap-Vert         */ { 0xCB, 0xC3, 0xC3, 0xF0 }, // bg=sky blue,  fg=bleu,     fg2=bleu,    col=rouge
  /* 13 Colombie         */ { 0xCB, 0xFC, 0xF0, 0xC3 }, // bg=sky blue,  fg=jaune,    fg2=rouge,   col=bleu
  /* 14 Corée du Sud     */ { 0xCB, 0xFF, 0xC2, 0xF0 }, // bg=sky blue,  fg=blanc,    fg2=bleu foncé, col=rouge
  /* 15 Côte d'Ivoire    */ { 0xCB, 0xF4, 0xC8, 0xFF }, // bg=sky blue,  fg=orange,   fg2=vert foncé, col=blanc
  /* 16 Croatie          */ { 0xCB, 0xF0, 0xC3, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=bleu,    col=blanc
  /* 17 Curaçao          */ { 0xCB, 0xC2, 0xFC, 0xDB }, // bg=sky blue,  fg=bleu foncé, fg2=jaune fluo, col=bleu pâle foncé
  /* 18 Écosse           */ { 0xCB, 0xC7, 0xC7, 0xFF }, // bg=sky blue,  fg=bleu moyen, fg2=bleu moyen, col=blanc
  /* 19 Égypte           */ { 0xCB, 0xF0, 0xC0, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=noir,    col=blanc
  /* 20 Équateur         */ { 0xCB, 0xFC, 0xF0, 0xC3 }, // bg=sky blue,  fg=jaune,    fg2=rouge,   col=bleu
  /* 21 Espagne          */ { 0xCB, 0xF0, 0xFC, 0xF5 }, // bg=sky blue,  fg=rouge,    fg2=jaune,   col=rouge pâle
  /* 22 États-Unis       */ { 0xCB, 0xC3, 0xC2, 0xF0 }, // bg=sky blue,  fg=bleu,     fg2=bleu foncé, col=rouge
  /* 23 France           */ { 0xCB, 0xC3, 0xF0, 0xFF }, // bg=sky blue,  fg=bleu,     fg2=rouge,   col=blanc
  /* 24 Ghana            */ { 0xCB, 0xF0, 0xC8, 0xFC }, // bg=sky blue,  fg=rouge,    fg2=vert,    col=jaune
  /* 25 Haïti            */ { 0xCB, 0xC3, 0xF0, 0xF5 }, // bg=sky blue,  fg=bleu,     fg2=rouge,   col=rouge clair
  /* 26 Irak             */ { 0xCB, 0xF0, 0xC0, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=noir,    col=blanc
  /* 27 Iran             */ { 0xCB, 0xC8, 0xF0, 0xFF }, // bg=sky blue,  fg=vert,     fg2=rouge,   col=blanc
  /* 28 Japon            */ { 0xCB, 0xFF, 0xFF, 0xF0 }, // bg=sky blue,  fg=blanc,    fg2=blanc,   col=rouge
  /* 29 Jordanie         */ { 0xCB, 0xC0, 0xC8, 0xFF }, // bg=sky blue,  fg=noir,     fg2=vert foncé, col=blanc
  /* 30 Maroc            */ { 0xCB, 0xF0, 0xF0, 0xC8 }, // bg=sky blue,  fg=rouge,    fg2=rouge,   col=vert foncé
  /* 31 Mexique          */ { 0xCB, 0xC4, 0xF0, 0xFF }, // bg=sky blue,  fg=vert foncé, fg2=rouge, col=blanc
  /* 32 Norvège          */ { 0xCB, 0xF0, 0xC3, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=bleu,    col=blanc
  /* 33 Nouvelle-Zélande */ { 0xCB, 0xC3, 0xF0, 0xD7 }, // bg=sky blue,  fg=bleu,     fg2=rouge,   col=bleu pâle
  /* 34 Ouzbékistan      */ { 0xCB, 0xD7, 0xC8, 0xFF }, // bg=sky blue,  fg=bleu clair, fg2=vert,    col=blanc
  /* 35 Panama           */ { 0xCB, 0xFF, 0xC3, 0xF0 }, // bg=sky blue,  fg=blanc,    fg2=bleu,    col=rouge
  /* 36 Paraguay         */ { 0xCB, 0xF0, 0xC3, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=bleu,    col=blanc
  /* 37 Pays-Bas         */ { 0xCB, 0xF0, 0xC3, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=bleu,    col=blanc
  /* 38 Portugal         */ { 0xCB, 0xC8, 0xF0, 0xDC }, // bg=sky blue,  fg=vert foncé, fg2=rouge, col=vert pâle
  /* 39 Qatar            */ { 0xCB, 0xFF, 0xD0, 0xE0 }, // bg=sky blue,  fg=blanc,    fg2=bordeaux foncé, col=bordeaux clair
  /* 40 RD Congo         */ { 0xCB, 0xC3, 0xF0, 0xFC }, // bg=sky blue,  fg=bleu,     fg2=rouge,   col=jaune
  /* 41 Sénégal          */ { 0xCB, 0xC8, 0xF0, 0xFC }, // bg=sky blue,  fg=vert foncé, fg2=rouge, col=jaune
  /* 42 Suède            */ { 0xCB, 0xC3, 0xC3, 0xFC }, // bg=sky blue,  fg=bleu,     fg2=bleu,    col=jaune
  /* 43 Suisse           */ { 0xCB, 0xF0, 0xF0, 0xFF }, // bg=sky blue,  fg=rouge vif, fg2=rouge vif, col=blanc
  /* 44 Tchéquie         */ { 0xCB, 0xC3, 0xFF, 0xF0 }, // bg=sky blue,  fg=bleu,     fg2=blanc,   col=rouge
  /* 45 Tunisie          */ { 0xCB, 0xF0, 0xF0, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=rouge,   col=blanc
  /* 46 Türkiye          */ { 0xCB, 0xF0, 0xF0, 0xFF }, // bg=sky blue,  fg=rouge,    fg2=rouge,   col=blanc
  /* 47 Uruguay          */ { 0xCB, 0xFF, 0xC3, 0xD7 }, // bg=sky blue,  fg=blanc,    fg2=bleu,    col=bleu pâle
};
