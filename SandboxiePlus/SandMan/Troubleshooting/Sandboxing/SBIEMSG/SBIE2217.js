/*
*	group: other
*	class: sandboxing
* name: SBIE2217: Request to run as Administrator was denied due to dropped rights
* description: Request to run as Administrator was denied due to dropped rights
*
*/

if (typeof msgCode === 'undefined') {
  msgCode = 2217;
}

let message = tr("The message %1 is caused by you selecting to drop rights from administrators/Power Users groups.\nYou may solve the issue by either disabling this setting or make applications think they are running elevated.", msgCode)

invoke("SBIECONF");
if(typeof boxName === 'undefined') boxName = "";
openOptions(message, boxName, 'Security');