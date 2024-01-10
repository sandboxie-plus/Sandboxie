/*
* group: other
* class: sandboxing
* name: SBIE2313 Could not execute specific process
* description: Sandboxie failed to start a process in the sandbox
*
*/

if(msgData[0] == "SandboxieDcomLaunch.exe (346)")
{
  invoke("SBIEFIXPC");
  try_fix_PC_SRP();
}
else
	wizard.showStatus(tr('Sandboxie was not able to execute one of its own programs. Check access permissions to the Sandboxie installation folder and/or reinstall Sandboxie.\n\nPossible Causes:\n1. Sandboxie was configured to block access to the folder containing its program files.\n2. A third-party (HIPS) security software was configured to block the execution of the program mentioned in the message.\n\nYou can submit an issue report on the next page to help us to analyze the issue.'), true);