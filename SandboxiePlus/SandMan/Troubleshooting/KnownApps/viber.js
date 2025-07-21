/*
*	group: apps
*	class: sandboxing
* name: Viber crashes on startup
* description: The Viber messenger crashes on startup
*
*/

  message = tr('Viber is not compatible with the standard isolation yellow type Sandbox. However is known to work in a Green (App Compartment) type sandbox.');

  invoke("SBIECONF");
  if(typeof boxName === 'undefined') boxName = "";
  openOptions(message, boxName, 'General');