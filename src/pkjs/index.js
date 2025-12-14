var Clay = require("pebble-clay");
var clay = new Clay([
  {
    type: "heading",
    defaultValue: "LSLT",
  },
  {
    type: "section",
    items: [
      {
        type: "heading",
        defaultValue: "Colors",
      },
      {
        type: "color",
        messageKey: "BGCOLOR",
        label: "Background",
        allowGray: false,
        defaultValue: "000000",
      },
      {
        type: "color",
        messageKey: "FGCOLOR",
        label: "Foreground",
        allowGray: false,
        defaultValue: "ffffff",
      },
      {
        type: "submit",
        defaultValue: "Save",
      },
    ],
  },
]);
