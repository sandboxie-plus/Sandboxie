/*
*	group: ui
*	class: ui
* name: Browser shortcut is missing from the desktop
* description: This procedure will add a browser shortcut to the desktop
*
*/

let appData = system.expand("LocalAppData");
let knownBrowsers = [
    // Firefox based
  { path: "C:\\Program Files\\Mozilla Firefox\\firefox.exe", name: tr('Firefox')},
  { path: "C:\\Program Files\\Firefox Developer Edition\\firefox.exe", name: tr('Firefox (Dev)')},
  { path: "C:\\Program Files\\Waterfox\\waterfox.exe", name: tr('Waterfox')},
  { path: "C:\\Program Files\\LibreWolf\\librewolf.exe", name: tr('Libre Wolf')},
  { path: "C:\\Program Files\\Pale Moon\\palemoon.exe", name: tr('Pale Moon')},
  { path: "C:\\Program Files\\SeaMonkey\\seamonkey.exe", name: tr('Sea Monkey')},

    // Chromium based
  { path: "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe", name: tr('Google Chrome')},
  { path: "C:\\Program Files\\Google\\Chrome Dev\\Application\\chrome.exe", name: tr('Google Chrome (Dev)')},
  { path: "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe", name: tr('Microsoft Edge')},
  { path: "C:\\Program Files (x86)\\Microsoft\\Edge Dev\\Application\\msedge.exe", name: tr('Microsoft Edge (Dev)')},
  { path: "C:\\Program Files\\BraveSoftware\\Brave-Browser\\Application\\brave.exe", name: tr('Brave Browser')},
  { path: "C:\\Program Files\\SRWare Iron (64-Bit)\\iron.exe", name: tr('Iron Browser')},
  { path: "C:\\Program Files\\Comodo\\Dragon\\dragon.exe", name: tr('Dragon Browser')},
  { path: appData + "\\Chromium\\Application\\chrome.exe", name: tr('Chromium Browser')},
  { path: appData + "\\Programs\\Opera\\launcher.exe", name: tr('Opera Browser')},
  { path: appData + "\\Vivaldi\\Application\\vivaldi.exe", name: tr('Vivaldi Browser')},
  { path: appData + "\\Opera Software\\Opera Neon\\Application\\neon.exe", name: tr('Neon Browser')},
  { path: appData + "\\Maxthon\\Application\\Maxthon.exe", name: tr('Maxthon Browser')}
];

let found = [];

found.push({name: tr('Default Browser'), value: -1});
for(let i=0; i < knownBrowsers.length; i++){
  if(system.checkFile(knownBrowsers[i].path))
    found.push({name: knownBrowsers[i].name, value: i});
}
found.push({name: tr('Another'), value: -2});

let form1 = [
  {type: 'combo', id: 'browser', items: found, name: tr('Select Browser')},
  {type: 'box', id: 'box', name: tr('Select Sandbox')}
];

let data1 = wizard.showForm(form1, tr('Select which browser you want to create a shortcut for and in what box'));

let box = sbie.getBox(data1['box']);

if(data1['browser'] == -2){
  let form2 = [
    {type: 'file', id: 'browser'},
  ];

  let data2 = wizard.showForm(form2, tr('Select Browser Path'));
  
  box.makeShortcut(data2['browser'], {location: 'desktop'});
}
else if(data1['browser'] == -1){
  box.makeShortcut('default_browser', {location: 'desktop'});
} else {
  let path = knownBrowsers[data1['browser']].path;
  box.makeShortcut(path, {location: 'desktop'});
}
wizard.setResult(true);