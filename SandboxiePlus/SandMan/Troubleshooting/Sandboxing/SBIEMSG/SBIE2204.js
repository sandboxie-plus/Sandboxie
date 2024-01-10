/*
* group: other
* class: sandboxing
* name: SBIE2204: Cannot start a specific sandboxed service
* description: Sandboxie failed to start a service in the sandbox
*
*/

if(msgData[0] == "DcomLaunch (346)")
{   
  invoke("SBIEFIXPC");
  try_fix_PC_SRP();
}
else
	wizard.showStatus(tr('The message indicates that Sandboxie was unable to start one of the helper programs SandboxieRpcSs or SandboxieDcomLaunch. The name noted in the message can be rpcss or dcomlaunch.\n\nYou can submit an issue report on the next page to help us to analyze the issue.'), true);
