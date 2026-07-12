# Sandboxie-Plus Troubleshooting System

The Troubleshooting System is a JavaScript-based wizard framework that helps users diagnose and resolve common sandboxing issues. Scripts are executed by a QJSEngine-based engine and interact with users through the Box Assistant wizard UI.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        BoxAssistant (UI)                        │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌──────────┐  ┌──────┐ │
│  │BeginPage│→ │GroupPage│→ │ListPage │→ │ RunPage  │→ │Submit│ │
│  └─────────┘  └─────────┘  └─────────┘  └──────────┘  └──────┘ │
└─────────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────────┐
│                      CWizardEngine (Thread)                     │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    QJSEngine (V4)                         │  │
│  │  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────────────┐  │  │
│  │  │ system │  │  sbie  │  │ wizard │  │ Global funcs   │  │  │
│  │  │ object │  │ object │  │ object │  │ tr, invoke,... │  │  │
│  │  └────────┘  └────────┘  └────────┘  └────────────────┘  │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Script Structure

Each troubleshooting script is a JavaScript file with a metadata header:

```javascript
/*
* group: sandboxing          // Parent group ID (root, sandboxing, browser, ui, apps, etc.)
* class: sandboxing          // Script class for filtering (sandboxing, ui)
* name: Script display name  // Shown in wizard UI
* description: Brief description of what this script does
* os_builds: 22000-99000     // Optional: OS build range (e.g., Windows 11 only)
* versions: 1.0.0-2.0.0      // Optional: Sandboxie version range
* bold: true                 // Optional: Display name in bold
*/

// Script code here
```

### Special Groups

| Group | Description |
|-------|-------------|
| `root` | Top-level categories shown on wizard start page |
| `system` | Internal scripts, not shown in UI |
| `library` | Utility scripts that can be invoked by other scripts |
| `other` | SBIE message handlers |

## Global Functions

### `tr(text, ...args)` - Translation
Translates text and substitutes placeholders.

```javascript
tr('Select which box to fix.')
tr('The mitigation has been applied. Please test in %1.', boxName)
```

### `invoke(scriptName)` - Invoke Another Script
Executes another troubleshooting script by name (without .js extension).

```javascript
invoke("SBIECONF");  // Loads SBIECONF.js
invoke("SBIE" + msgCode);  // Dynamic script loading
```

### `wildCompare(pattern, string)` - Wildcard Matching
Compares a string against a wildcard pattern.

```javascript
if (wildCompare("chrome*", processName)) { ... }
```

---

## The `system` Object

Provides system-level operations for file/registry access, process execution, and compatibility checking.

### Logging

```javascript
system.log(message)   // Log message to debug output
```

### File System Operations

```javascript
// List directory contents
let entries = system.listDir(path, filter, bSubDirs);
// Returns: [{isDir: 0|1, name: "file.txt", path: "C:\\...\\file.txt"}, ...]

// Create directory
system.mkDir(path);

// Check file/folder exists
if (system.exists(path)) { ... }

// Read file
let data = system.readFile(path, pos, length);

// Write file (pos=-1 truncates first)
system.writeFile(path, data, pos);

// Get file info
let info = system.getFileInfo(path);
// Returns: {path, isDir, size}

// Delete file
system.remove(path);

// Check if file exists (with path expansion)
if (system.checkFile(path)) { ... }
```

### Registry Operations

```javascript
// List registry key contents
let entries = system.listRegKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\...");
// Returns: [{type: "REG_SZ|REG_DWORD|...", name: "ValueName", value: ...}, ...]

// Read registry value
let value = system.getRegValue(key, name);

// Write registry value
system.setRegValue(key, name, value, type);  // type: REG_SZ, REG_DWORD, etc.

// Delete registry key/value
system.removeRegKey(key);
system.removeRegValue(key, name);

// Check if registry key exists
if (system.checkRegKey(keyPath)) { ... }
```

### Process Execution

```javascript
let result = system.execute(path, arguments, options);
// Options:
//   workingDirectory: string
//   hide: bool           - Hide window
//   elevate: bool        - Run as administrator (UAC prompt)
// Returns: {exitCode, output, error}
```

**Example:**
```javascript
let ret = system.execute('cmd.exe', '/c net stop AppIdSvc', {elevate: true});
if (ret.error)
    wizard.setResult(false, 'Failed: ' + ret.error);
```

### Environment & System Info

```javascript
// Expand environment variable
let appData = system.expand("LocalAppData");

// Expand Sandboxie-specific paths
let path = system.expandPath("%SbieHome%\\SandMan.exe");
// Supports: %SbieHome%, %PlusData%, and standard env vars

// Get OS version
let ver = system.version();
// Returns: {major, minor, build, platform}

// Get current language
let lang = system.language();  // e.g., "en_US"
```

### Compatibility Checking

These functions check for installed software/services:

```javascript
// Enumerate all
let classes = system.enumClasses();    // Window classes
let services = system.enumServices();  // Windows services
let products = system.enumProducts();  // Installed products
let objects = system.enumObjects();    // Named objects
let updates = system.enumUpdates();    // Windows updates

// Check specific patterns (wildcard supported)
if (system.checkClasses("Chrome_*")) { ... }
if (system.checkServices("ChromeElevation*")) { ... }
if (system.checkProducts("Google Chrome*")) { ... }
if (system.checkObjects("\\BaseNamedObjects\\*Chrome*")) { ... }

// Reset cached enumeration data
system.resetData();
```

### Sleep

```javascript
system.sleep(1000);  // Pause for 1000ms
```

---

## The `sbie` Object

Provides access to Sandboxie configuration, boxes, and templates.

### Version & Features

```javascript
let version = sbie.getVersion();  // e.g., "1.14.0"

// Check if feature is available (requires supporter certificate)
if (sbie.testFeature('AppC')) { ... }  // App Compartment
if (sbie.testFeature('SMod')) { ... }  // Security Mode
```

### Sandbox Management

```javascript
// Get sandbox by name
let box = sbie.getBox(boxName);

// List all sandboxes
let boxes = sbie.listBoxes();  // Returns: ["DefaultBox", "TestBox", ...]

// Create new sandbox
let newBox = sbie.newBox("NewBoxName");
```

### Template Management

```javascript
// Get template
let template = sbie.getTemplate("Chromium_Elevation");

// List all templates
let templates = sbie.listTemplates();

// Create new local template
let newTmpl = sbie.newTemplate("Local_MyTemplate");
```

### Global Settings

```javascript
let global = sbie.getGlobal();  // Global Sandboxie settings
let conf = sbie.getConf();      // Plus configuration settings
```

### Tracing & Logging

```javascript
// Start resource access trace
sbie.setupTrace();

// Read trace entries
let entries = sbie.readTrace({
    start: 0,
    count: 100,
    pid: 1234,        // Filter by process ID
    box: "DefaultBox" // Filter by sandbox
});
// Returns: [{timeStamp, process, pid, tid, type, status, name, message}, ...]

// Log message to SandMan
sbie.logMessage("Message text", notify);  // notify: show notification
```

### Shell Integration

```javascript
// Reinstall shell extensions
sbie.shellInstall({
    runUnBoxed: true,   // Add "Run Unsandboxed" menu
    legacy: false       // Use legacy shell extensions
});

// Remove shell extensions
sbie.shellRemove();
```

### Cleanup

```javascript
sbie.cleanUp();  // Refresh process list
```

---

## The `box` Object (Sandbox)

Returned by `sbie.getBox()` or `sbie.newBox()`. Inherits from INI object.

### Properties

```javascript
let name = box.getName();
let fileRoot = box.getFileRoot();  // e.g., "C:\\Sandbox\\User\\BoxName"
let regRoot = box.getRegRoot();    // Registry root path
```

### Process Management

```javascript
// Start a program in the sandbox
let pid = box.startTask(command, {
    elevated: false,
    directory: "C:\\WorkDir"
});

// List running processes
let tasks = box.listTasks();
// Returns: [{pid, parentId, name, commandline, fileName, timeStamp, flags, type, isRunning}, ...]

// Get info about specific process
let info = box.taskInfo(pid);

// Stop specific process
box.stopTask(pid);

// Terminate all processes
box.terminate();
```

### Special Commands

```javascript
// Start with special commands
box.start();                    // Open run dialog
box.start("default_browser");   // Start default browser
box.start("explorer.exe");      // Start explorer
```

### Content Management

```javascript
// Delete sandbox contents
box.deleteContent();

// Remove entire sandbox
box.removeSandbox();
```

### Shortcuts

```javascript
box.makeShortcut(target, {
    location: "desktop",     // "desktop", "startmenu", "documents"
    name: "Shortcut Name",
    iconPath: "path/to/icon.ico",
    iconIndex: 0,
    workDir: "C:\\WorkDir"
});

// Special targets
box.makeShortcut("default_browser", {location: "desktop"});
```

### Diagnostic Log

```javascript
let events = box.getDiagLog();
// Returns: [{processName, eventId, eventData: [...]}, ...]
```

### Configuration (INI Methods)

```javascript
// Get all settings as map
let settings = box.getIniSection({withTemplate: true});
// Returns: {Setting1: [value1, value2], Setting2: [value], ...}

// Get single value
let value = box.getIniValue("DropAdminRights", {
    type: "bool",        // "bool", "number", or string (default)
    default: false,
    withGlobal: false,
    withTemplate: false
});

// Get all values for multi-value setting
let values = box.getIniValues("Template", {withTemplate: true});

// Set value (replaces)
box.setIniValue("NoSecurityIsolation", "y");

// Append value (for multi-value settings)
box.appendIniValue("Template", "OpenCOM");

// Insert value at beginning
box.insertIniValue("Template", "BlockNetwork");

// Delete value
box.delIniValue("DropAdminRights");           // Delete setting
box.delIniValue("Template", "BlockNetwork");  // Delete specific value

// Remove entire section
box.removeSection();
```

### Shadow Mode (Wizard Only)

In wizard context, changes are made to a shadow copy:

```javascript
// Apply shadow changes to original box
box.applyChanges(true);   // true = apply, false = discard
```

---

## The `wizard` Object

Controls the wizard UI flow. Only available in wizard scripts.

### Display Forms

```javascript
let result = wizard.showForm(formDefinition, promptText);
```

**Form Field Types:**

| Type | Properties | Returns |
|------|------------|---------|
| `label` | `name` | - |
| `edit` | `id`, `name`, `value`, `hint` | string |
| `check` | `id`, `name`, `value` | boolean |
| `radio` | `id`, `name`, `value` | boolean |
| `file` | `id`, `name` | string (path) |
| `folder` | `id`, `name` | string (path) |
| `box` | `id`, `name` | string (box name) |
| `combo` | `id`, `name`, `items` | selected value |

**Example:**
```javascript
let form = [
    {type: 'combo', id: 'browser', items: found, name: tr('Select Browser')},
    {type: 'box', id: 'box', name: tr('Select Sandbox')},
    {type: 'check', id: 'legacy', name: tr('Use legacy mode')}
];
let data = wizard.showForm(form, tr('Select options'));

// Access results
let selectedBrowser = data['browser'];
let selectedBox = data['box'];
let useLegacy = data['legacy'];
```

**Radio Button Groups:**
```javascript
let form = [
    {type: 'radio', id: 'yes', name: tr('Yes')},
    {type: 'radio', id: 'no', name: tr('No')}
];
let ret = wizard.showForm(form, tr('Was the issue resolved?'));
if (ret['yes'] == true) { ... }
```

**Combo with Items:**
```javascript
let items = [
    {name: tr('Option A'), value: 1},
    {name: tr('Option B'), value: 2}
];
let form = [{type: 'combo', id: 'choice', items: items, name: tr('Select')}];
```

### Show Status

```javascript
// Show status message (non-blocking)
wizard.showStatus(tr('Processing...'));

// Show status and wait for user to click Next
wizard.showStatus(tr('Press NEXT to continue'), true);
```

### Set Result

```javascript
// Mark script as successful
wizard.setResult(true);

// Mark script as failed with message
wizard.setResult(false, tr('Mitigation not successful'));
```

### Report Data

Add data to the issue report:

```javascript
wizard.reportAdd('key', 'value');
wizard.reportAdd('installer', installerPath);
wizard.reportAdd('ProcessName', processName);
```

### Open Dialogs

```javascript
// Open sandbox options dialog
wizard.openOptions(boxName, 'General');     // Tab: General, Migration, etc.

// Open global settings dialog
wizard.openSettings('General');
```

---

## Script Variables

Scripts may receive predefined variables depending on how they were invoked:

| Variable | Description |
|----------|-------------|
| `boxName` | Name of the affected sandbox |
| `processName` | Name of the affected process |
| `msgCode` | SBIE message code (number) |
| `msgData` | SBIE message data (array) |
| `sbieMsg` | Formatted SBIE message text |
| `docLink` | Link to online documentation |

**Example check:**
```javascript
if (typeof boxName === 'undefined' || !boxName) {
    let form = [
        {id: 'box', name: tr('Select affected sandbox'), type: 'box'}
    ];
    let ret = wizard.showForm(form);
    boxName = ret.box;
}
```

---

## Complete Examples

### Simple Fix Script

```javascript
/*
* group: browser
* class: sandboxing
* name: Extensions disappearing
* description: Fix Chrome extensions being reset
*/

let form = [{type: 'box', id: 'box'}];
let data = wizard.showForm(form, tr('Select which box to fix.'));

let box = sbie.getBox(data['box']);
box.appendIniValue('Template', 'Chromium_Elevation');
box.applyChanges();
box.start("default_browser");

let confirmForm = [
    {id: 'yes', name: tr('Yes'), type: 'radio'},
    {id: 'no', name: tr('No'), type: 'radio'}
];
let ret = wizard.showForm(confirmForm,
    tr('The fix has been applied. Please test and confirm.'));

if (ret['yes'] == true) {
    wizard.setResult(true);
} else {
    wizard.setResult(false, tr('Fix not successful'));
}
```

### SBIE Message Handler

```javascript
/*
* group: other
* class: sandboxing
* name: SBIE1307: Internet access denied
* description: Program cannot access the Internet
*/

if (typeof boxName === 'undefined' || !boxName) {
    let form = [
        {id: 'box', name: tr('Select affected sandbox'), type: 'box'},
        {id: 'process', name: tr('Enter Process Name'), type: 'edit'}
    ];
    let ret = wizard.showForm(form);
    boxName = ret.box;
    processName = ret.process;
}

let message = tr('Internet access is restricted for this sandbox.');

invoke("SBIECONF");
openOptions(message, boxName, 'Internet');
wizard.setResult(true);
```

### Library Script (Reusable)

```javascript
/*
* group: library
* class: sandboxing
* name: open config
* description: Opens box config on a given page
*/

function openOptions(message, box, page) {
    if (!box) {
        let form = [{id: 'box', name: tr('Select sandbox'), type: 'box'}];
        let ret = wizard.showForm(form);
        boxName = ret.box;
    }

    message += '\n\n' + tr('Open box settings to change this?');

    let form = [
        {id: 'yes', name: tr('Yes'), type: 'radio'},
        {id: 'no', name: tr('No'), type: 'radio'}
    ];

    let ret = wizard.showForm(form, message);
    if (ret['yes'] == true) {
        wizard.openOptions(box || boxName, page);
    }
    if (ret['no'] == true) {
        wizard.setResult(true);
    }
}
```

### System Check Script

```javascript
/*
* group: system
* name: App compatibility checker
* description: Checks which templates need to be enabled
*/

let Classes = system.enumClasses();
let Services = system.enumServices();
let Products = system.enumProducts();

let templates = sbie.listTemplates();
let needed = [];

for (let i = 0; i < templates.length; i++) {
    let template = sbie.getTemplate(templates[i]);
    let scan = template.getIniValue('Tmpl.Scan');

    if (scan.indexOf('s') != -1) {  // Scan services
        let services = template.getIniValues('Tmpl.ScanService');
        for (let j = 0; j < services.length; j++) {
            if (system.checkServices(services[j])) {
                needed.push(templates[i]);
                break;
            }
        }
    }
}

result = needed;  // Return value
```

---

## Localization

Translation files are JSON maps in `lang_XX.json`:

```json
{
    "Select Browser": "Seleccionar navegador",
    "The fix has been applied.": "La corrección ha sido aplicada."
}
```

Use the `tr()` function in scripts:
```javascript
tr('Select Browser')  // Returns translated string
```

---

## File Organization

```
Troubleshooting/
├── layout.json              # UI structure and groups
├── AppCompatibility.js      # Template scanner
├── DiagnosticMonitor.js     # Event monitor
├── lang_*.json              # Translation files
├── UI/
│   ├── desktop.js           # Desktop shortcut creator
│   └── shell.js             # Shell integration fixer
├── KnownApps/
│   ├── unknown.js           # Generic app crash handler
│   └── viber.js             # Viber-specific fix
└── Sandboxing/
    ├── webcam.js            # Windows 11 webcam fix
    ├── installer.js         # Installer troubleshooter
    ├── game_fps.js          # Gaming optimization
    ├── chrome_svc.js        # Chrome elevation fix
    └── SBIEMSG/
        ├── SBIEMSG.js       # Generic message handler
        ├── SBIECONF.js      # Config helper library
        ├── SBIECOPY.js      # File copy error handler
        ├── SBIEFIXPC.js     # Parental control fix
        ├── SBIE1307.js      # Internet restriction
        ├── SBIE1308.js      # Start restriction
        ├── SBIE2102.js      # File too large
        └── ...
```

---

## Best Practices

1. **Always check variables exist** before using them
2. **Use `tr()` for all user-visible text** to support localization
3. **Call `box.applyChanges()`** after modifying settings in wizard mode
4. **Provide feedback** using `wizard.showForm()` with radio buttons
5. **Set result** at the end: `wizard.setResult(true/false)`
6. **Keep messages concise** - wizard has limited space
7. **Use library scripts** for common operations via `invoke()`
