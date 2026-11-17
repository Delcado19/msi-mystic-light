LIGHTING STUDIO - PACKAGE README

Two ways to use this package:

1. Portable (no installation)
   Extract the ZIP anywhere and run msi_mystic_light.exe. The editor drives the
   lighting itself while it runs; closing it stops the output. Nothing is
   installed and no administrator rights are needed, except for RAM lighting,
   which needs the PawnIO driver.

2. Installed (lighting runs without the editor)
   Run installer\Install.cmd and confirm the Windows prompt. A Windows service
   then owns the hardware output and keeps the lighting after you close the
   editor or restart Windows. Open the editor from the Start menu.

   Updating: run Install.cmd from the new package. Do not uninstall first; the
   uninstaller can delete your unsaved editor data.

Once the service is installed, msi_mystic_light.exe from any folder uses that
service instead of driving the hardware itself.

Help > About shows the build id and the path of the running executable.
More: README.md and docs/WINDOWS_SERVICE.md in the source repository.
