/*
*	group: other
*	class: sandboxing
* name: SBIE2214: Request to start service name was denied due to dropped rights
* description: Request to start service name was denied due to dropped rights
*/

if (typeof msgCode === 'undefined') {
  msgCode = 2214;
}

let message = tr("The message %1 is caused by you selecting to drop rights from administrators/Power Users groups.\nYou may turn off Drop Rights setting to resolve this issue.", msgCode)

invoke("SBIECONF");
if(typeof boxName === 'undefined') boxName = "";
openOptions(message, boxName, 'Security');