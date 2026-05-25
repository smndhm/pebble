const Clay = require('@rebble/clay');

new Clay([
  {
    type: 'heading',
    defaultValue: 'FWC26',
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Display',
      },
      {
        type: 'toggle',
        messageKey: 'MODE_LOGO',
        label: 'Mode Logo (MM seulement)',
        defaultValue: true,
      },
      {
        type: 'toggle',
        messageKey: 'TAP_ENABLED',
        label: 'Tap pour basculer',
        defaultValue: true,
      },
      {
        type: 'submit',
        defaultValue: 'Save',
      },
    ],
  },
  {
    type: 'section',
    capabilities: ['COLOR'],
    items: [
      {
        type: 'heading',
        defaultValue: 'Colors',
      },
      {
        type: 'color',
        messageKey: 'FGCOLOR',
        label: 'Chiffres',
        allowGray: false,
        defaultValue: 'C81428',
      },
      {
        type: 'color',
        messageKey: 'BGCOLOR',
        label: 'Fond',
        allowGray: false,
        defaultValue: '000000',
      },
      {
        type: 'color',
        messageKey: 'TRANSCOLOR',
        label: 'Transition (tap)',
        allowGray: false,
        defaultValue: 'FFFF00',
      },
      {
        type: 'submit',
        defaultValue: 'Save',
      },
    ],
  },
]);
