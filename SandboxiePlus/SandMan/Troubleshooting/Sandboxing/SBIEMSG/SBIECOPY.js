/*
*	group: library
*	class: sandboxing
* name: SBIE 2113/2114/2115 and 2102
* description: handle migration error messages
*
*/

//if (typeof sbieMsg === 'undefined')
//  sbieMsg = sbie.formatMessage(msgCode);

let message = tr("The message %1 is caused by the file migration limit being reached.", msgCode)

invoke("SBIECONF");
if(typeof boxName === 'undefined') boxName = "";
openOptions(message, boxName, 'Migration');