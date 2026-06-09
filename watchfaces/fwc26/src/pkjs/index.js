const Clay = require('@rebble/clay');

// ---------------------------------------------------------------------------
// Translations
// ---------------------------------------------------------------------------

var i18n = {
  fr: {
    display: 'Affichage',
    mode: 'Mode',
    save: 'Enregistrer',
    team: 'Équipe',
    yourTeam: 'Votre équipe',
    colors: 'Couleurs',
    bg: 'Fond',
    left: 'Chiffre 1',
    right: 'Chiffre 2',
    collision: 'Chevauchement',
    custom: '— Personnalisé —',
    pixel: 'Style',
    pixelModes: ['Normal', 'Pixel'],
    subMode: 'Affichage',
    subModes: ['Minutes', 'Heure + Minutes'],
    modes: ['Logo', 'Superposition', 'Classique']
  },
  en: {
    display: 'Display',
    mode: 'Mode',
    save: 'Save',
    team: 'Team',
    yourTeam: 'Your team',
    colors: 'Colors',
    bg: 'Background',
    left: 'Digit 1',
    right: 'Digit 2',
    collision: 'Overlap zone',
    custom: '— Custom —',
    pixel: 'Style',
    pixelModes: ['Normal', 'Pixel'],
    subMode: 'Display',
    subModes: ['Minutes', 'Hour + Minutes'],
    modes: ['Logo', 'Overlap', 'Classic']
  },
  es: {
    display: 'Pantalla',
    mode: 'Modo',
    save: 'Guardar',
    team: 'Selección',
    yourTeam: 'Tu selección',
    colors: 'Colores',
    bg: 'Fondo',
    left: 'Dígito 1',
    right: 'Dígito 2',
    collision: 'Zona de superposición',
    custom: '— Personalizado —',
    pixel: 'Estilo',
    pixelModes: ['Normal', 'Pixel'],
    subMode: 'Visualización',
    subModes: ['Minutos', 'Hora + Minutos'],
    modes: ['Logo', 'Superposición', 'Clásico']
  },
  pt: {
    display: 'Visualização',
    mode: 'Modo',
    save: 'Salvar',
    team: 'Seleção',
    yourTeam: 'Sua seleção',
    colors: 'Cores',
    bg: 'Fundo',
    left: 'Dígito 1',
    right: 'Dígito 2',
    collision: 'Zona de sobreposição',
    custom: '— Personalizado —',
    pixel: 'Estilo',
    pixelModes: ['Normal', 'Pixel'],
    subMode: 'Exibição',
    subModes: ['Minutos', 'Hora + Minutos'],
    modes: ['Logo', 'Sobreposição', 'Clássico']
  },
  de: {
    display: 'Anzeige',
    mode: 'Modus',
    save: 'Speichern',
    team: 'Mannschaft',
    yourTeam: 'Deine Mannschaft',
    colors: 'Farben',
    bg: 'Hintergrund',
    left: 'Ziffer 1',
    right: 'Ziffer 2',
    collision: 'Überlappungszone',
    custom: '— Benutzerdefiniert —',
    pixel: 'Stil',
    pixelModes: ['Normal', 'Pixel'],
    subMode: 'Anzeige',
    subModes: ['Minuten', 'Stunde + Minuten'],
    modes: ['Logo', 'Überlappung', 'Klassisch']
  }
};

// Team names sorted alphabetically in French (index = kTeams[] index in C)
var teamNames = {
  fr: [
    'Afrique du Sud',
    'Algérie',
    'Allemagne',
    'Angleterre',
    'Arabie Saoudite',
    'Argentine',
    'Australie',
    'Autriche',
    'Belgique',
    'Bosnie-Herzégovine',
    'Brésil',
    'Canada',
    'Cap-Vert',
    'Colombie',
    'Corée du Sud',
    "Côte d'Ivoire",
    'Croatie',
    'Curaçao',
    'Écosse',
    'Égypte',
    'Équateur',
    'Espagne',
    'États-Unis',
    'France',
    'Ghana',
    'Haïti',
    'Irak',
    'Iran',
    'Japon',
    'Jordanie',
    'Maroc',
    'Mexique',
    'Norvège',
    'Nouvelle-Zélande',
    'Ouzbékistan',
    'Panama',
    'Paraguay',
    'Pays-Bas',
    'Portugal',
    'Qatar',
    'RD Congo',
    'Sénégal',
    'Suède',
    'Suisse',
    'Tchéquie',
    'Tunisie',
    'Türkiye',
    'Uruguay'
  ],
  en: [
    'South Africa',
    'Algeria',
    'Germany',
    'England',
    'Saudi Arabia',
    'Argentina',
    'Australia',
    'Austria',
    'Belgium',
    'Bosnia and Herzegovina',
    'Brazil',
    'Canada',
    'Cape Verde',
    'Colombia',
    'South Korea',
    "Côte d'Ivoire",
    'Croatia',
    'Curaçao',
    'Scotland',
    'Egypt',
    'Ecuador',
    'Spain',
    'United States',
    'France',
    'Ghana',
    'Haiti',
    'Iraq',
    'Iran',
    'Japan',
    'Jordan',
    'Morocco',
    'Mexico',
    'Norway',
    'New Zealand',
    'Uzbekistan',
    'Panama',
    'Paraguay',
    'Netherlands',
    'Portugal',
    'Qatar',
    'DR Congo',
    'Senegal',
    'Sweden',
    'Switzerland',
    'Czechia',
    'Tunisia',
    'Türkiye',
    'Uruguay'
  ]
};
// Spanish, Portuguese, German fall back to English team names
teamNames.es = teamNames.en;
teamNames.pt = teamNames.en;
teamNames.de = teamNames.en;

// ---------------------------------------------------------------------------
// Language detection
// ---------------------------------------------------------------------------

var langCode = (
  typeof navigator !== 'undefined' && navigator.language
    ? navigator.language
    : 'en'
)
  .split('-')[0]
  .toLowerCase();
var t = i18n[langCode] || i18n.en;
var names = teamNames[langCode] || teamNames.en;

// ---------------------------------------------------------------------------
// Country code → team index (ISO 3166-1 alpha-2, same sort order as kTeams[])
// ---------------------------------------------------------------------------

var countryToTeam = {
  ZA: 0,
  DZ: 1,
  DE: 2,
  GB: 3,
  SA: 4,
  AR: 5,
  AU: 6,
  AT: 7,
  BE: 8,
  BA: 9,
  BR: 10,
  CA: 11,
  CV: 12,
  CO: 13,
  KR: 14,
  CI: 15,
  HR: 16,
  CW: 17,
  EG: 19,
  EC: 20,
  ES: 21,
  US: 22,
  FR: 23,
  GH: 24,
  HT: 25,
  IQ: 26,
  IR: 27,
  JP: 28,
  JO: 29,
  MA: 30,
  MX: 31,
  NO: 32,
  NZ: 33,
  UZ: 34,
  PA: 35,
  PY: 36,
  NL: 37,
  PT: 38,
  QA: 39,
  CD: 40,
  SN: 41,
  SE: 42,
  CH: 43,
  CZ: 44,
  TN: 45,
  TR: 46,
  UY: 47
};
// Scotland (index 18) detected separately via ISO3166-2-lvl4 = GB-SCT

// ---------------------------------------------------------------------------
// Team options for Clay select
// ---------------------------------------------------------------------------

var teamOptions = [{ label: t.custom, value: 48 }];
for (var i = 0; i < names.length; i++) {
  teamOptions.push({ label: names[i], value: i });
}

// ---------------------------------------------------------------------------
// Clay config
// ---------------------------------------------------------------------------

var clayConfig = [
  { type: 'heading', defaultValue: 'FWC26' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: t.display },
      {
        type: 'select',
        messageKey: 'DISPLAY_MODE',
        label: t.mode,
        defaultValue: 0,
        options: [
          { label: t.modes[0], value: 0 }, // Logo
          { label: t.modes[1], value: 1 }, // Superposition
          { label: t.modes[2], value: 2 } // Classique
        ]
      },
      {
        type: 'select',
        messageKey: 'LOGO_PIXEL',
        label: t.pixel,
        defaultValue: 0,
        options: [
          { label: t.pixelModes[0], value: 0 },
          { label: t.pixelModes[1], value: 1 }
        ]
      },
      {
        type: 'select',
        messageKey: 'SUB_MODE',
        label: t.subMode,
        defaultValue: 0,
        options: [
          { label: t.subModes[0], value: 0 },
          { label: t.subModes[1], value: 1 }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: t.team },
      {
        type: 'select',
        messageKey: 'TEAM_INDEX',
        label: t.yourTeam,
        defaultValue: 48,
        options: teamOptions
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: t.colors },
      {
        type: 'color',
        messageKey: 'BGCOLOR',
        label: t.bg,
        allowGray: false,
        defaultValue: '000000'
      },
      {
        type: 'color',
        messageKey: 'FGCOLOR',
        label: t.left,
        allowGray: false,
        defaultValue: 'FFFFFF'
      },
      {
        type: 'color',
        messageKey: 'COLOR_RIGHT',
        label: t.right,
        allowGray: false,
        defaultValue: 'FFFFFF'
      },
      {
        type: 'color',
        messageKey: 'COL_COLOR',
        label: t.collision,
        allowGray: false,
        defaultValue: '000000'
      }
    ]
  },
  { type: 'submit', defaultValue: t.save }
];

function clayCustomFn() {
  var clay = this;

  clay.on(clay.EVENTS.AFTER_BUILD, function () {
    var watchInfo =
      typeof Pebble !== 'undefined' && Pebble.getActiveWatchInfo
        ? Pebble.getActiveWatchInfo()
        : {};
    var isBW =
      watchInfo.platform === 'aplite' ||
      watchInfo.platform === 'diorite' ||
      watchInfo.platform === 'flint';

    var modeItem = clay.getItemByMessageKey('DISPLAY_MODE');
    var pixelItem = clay.getItemByMessageKey('LOGO_PIXEL');
    var subModeItem = clay.getItemByMessageKey('SUB_MODE');
    var teamItem = clay.getItemByMessageKey('TEAM_INDEX');
    var bgItem = clay.getItemByMessageKey('BGCOLOR');
    var fgItem = clay.getItemByMessageKey('FGCOLOR');
    var rightItem = clay.getItemByMessageKey('COLOR_RIGHT');
    var colItem = clay.getItemByMessageKey('COL_COLOR');

    var teamHeading = null;
    if (teamItem) {
      var all = clay.getAllItems();
      var tIdx = all.indexOf(teamItem);
      if (
        tIdx > 0 &&
        all[tIdx - 1].config &&
        all[tIdx - 1].config.type === 'heading'
      ) {
        teamHeading = all[tIdx - 1];
      }
    }

    function getMode() {
      return modeItem ? parseInt(modeItem.get(), 10) : 0;
    }
    function getTeamIdx() {
      return teamItem ? parseInt(teamItem.get(), 10) : 48;
    }

    function update() {
      var mode = getMode();
      var isLogo = mode === 0;
      var isSup = mode === 1;
      var teamIdx = getTeamIdx();
      var isCustom = !isSup || teamIdx === 48;

      if (isBW) {
        // B&W platforms: no colors, no team, no pixel style
        if (pixelItem) pixelItem.hide();
        [teamItem, teamHeading].forEach(function (it) {
          if (it) it.hide();
        });
        [bgItem, fgItem, rightItem, colItem].forEach(function (it) {
          if (it) it.hide();
        });
        if (subModeItem) isSup ? subModeItem.show() : subModeItem.hide();
        return;
      }

      // Pixel style selector: logo only
      if (pixelItem) isLogo ? pixelItem.show() : pixelItem.hide();
      // Sub-mode (Minutes/HM): superposition only
      if (subModeItem) isSup ? subModeItem.show() : subModeItem.hide();
      // Team section: superposition only
      [teamItem, teamHeading].forEach(function (it) {
        if (it) isSup ? it.show() : it.hide();
      });
      // Background: always
      if (bgItem) bgItem.show();
      // Digit colors (Chiffre 1 & 2): custom only (shown in both normal and pixel modes)
      [fgItem, rightItem].forEach(function (it) {
        if (it) isCustom ? it.show() : it.hide();
      });
      // Collision: superposition + custom only
      if (colItem) isSup && isCustom ? colItem.show() : colItem.hide();
    }

    if (modeItem) modeItem.on('change', update);
    if (pixelItem) pixelItem.on('change', update);
    if (teamItem) teamItem.on('change', update);

    // Sync Clay fields from watch state
    s_applyWatchConfig = function () {
      if (!s_watchConfig) return;
      [
        [bgItem, s_watchConfig.BGCOLOR],
        [fgItem, s_watchConfig.FGCOLOR],
        [rightItem, s_watchConfig.COLOR_RIGHT],
        [colItem, s_watchConfig.COL_COLOR]
      ].forEach(function (p) {
        if (p[0] && p[1] !== undefined) p[0].set(argbToColorInt(p[1]));
      });
      if (teamItem && s_watchConfig.TEAM_INDEX !== undefined)
        teamItem.set(s_watchConfig.TEAM_INDEX);
      if (pixelItem && s_watchConfig.LOGO_PIXEL !== undefined)
        pixelItem.set(s_watchConfig.LOGO_PIXEL);
      if (subModeItem && s_watchConfig.SUB_MODE !== undefined)
        subModeItem.set(s_watchConfig.SUB_MODE);
      if (modeItem && s_watchConfig.DISPLAY_MODE !== undefined)
        modeItem.set(s_watchConfig.DISPLAY_MODE);
      update();
    };

    // Use localStorage values as-is (Clay populates them from the last save).
    // Request watch config to handle first-install and out-of-sync edge cases.
    update();
    s_watchConfig = null;
    requestConfig();
    setTimeout(function () {
      if (!s_watchConfig) update();
    }, 3000);

    // Geolocation: pre-fill team if still at default (superposition mode + team=custom)
    if (teamItem && getTeamIdx() === 48 && navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(
        function (pos) {
          var xhr = new XMLHttpRequest();
          xhr.onreadystatechange = function () {
            if (xhr.readyState !== 4 || xhr.status !== 200) return;
            try {
              var data = JSON.parse(xhr.responseText);
              var addr = data.address || {};
              var cc = (addr.country_code || '').toUpperCase();
              var sub = (addr['ISO3166-2-lvl4'] || '').toUpperCase();
              var idx =
                cc === 'GB' ? (sub === 'GB-SCT' ? 18 : 3) : countryToTeam[cc];
              if (idx !== undefined) {
                teamItem.set(idx);
                update();
              }
            } catch (_e) {}
          };
          xhr.open(
            'GET',
            'https://nominatim.openstreetmap.org/reverse?format=json&lat=' +
              pos.coords.latitude +
              '&lon=' +
              pos.coords.longitude
          );
          xhr.send();
        },
        function () {}
      );
    }
  });
}

// ---------------------------------------------------------------------------
// Watch → Clay state sync
// ---------------------------------------------------------------------------

// Convert Pebble 8-bit argb (0b11RRGGBB) to a decimal integer for Clay color items.
// Clay's color manipulator stores/retrieves values as decimal integers (parseInt base 10),
// so we must pass a number, not a hex string.
function argbToColorInt(argb) {
  var r = ((argb >> 4) & 0x3) * 85;
  var g = ((argb >> 2) & 0x3) * 85;
  var b = (argb & 0x3) * 85;
  return (r << 16) | (g << 8) | b;
}

var s_watchConfig = null;
var s_applyWatchConfig = null; // set by AFTER_BUILD, called when config arrives

function requestConfig() {
  Pebble.sendAppMessage(
    { REQUEST_CONFIG: 1 },
    function () {},
    function () {}
  );
}

// On startup, ask the watch for its current config
Pebble.addEventListener('ready', function () {
  requestConfig();
});

// When config arrives: cache it and apply to Clay if already built
Pebble.addEventListener('appmessage', function (e) {
  var p = e.payload;
  if (p.DISPLAY_MODE !== undefined || p.BGCOLOR !== undefined) {
    s_watchConfig = p;
    if (s_applyWatchConfig) s_applyWatchConfig();
  }
});

new Clay(clayConfig, clayCustomFn);
