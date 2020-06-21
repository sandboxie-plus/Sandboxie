Qt Solutions Component: Single Application

The QtSingleApplication component provides support for
applications that can be only started once per user.



Version history:

2.0: - Version 1.3 ported to Qt 4.

2.1: - Fix compilation problem on Mac.

2.2: - Really fix the Mac compilation problem.
     - Mac: fix crash due to wrong object releasing.
     - Mac: Fix memory leak.

2.3: - Windows: Force creation of internal widget to make it work
     with Qt 4.2.

2.4: - Fix the system for automatic window raising on message
     reception. NOTE: minor API change.

2.5: - Mac: Fix isRunning() to work and report correctly.

2.6: - - initialize() is now obsolete, no longer necessary to call
     it
     - - Fixed race condition where multiple instances migth be started
     - - QtSingleCoreApplication variant provided for non-GUI (console)
     usage
     - Complete reimplementation. Visible changes:
     - LGPL release.

