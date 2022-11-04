# unreal-engine-3dft-plugin


Building and testing the plugin on a Mac:

- Go to the Epic Games page and download the Epic Games Installer, you might need to create an account with Epic.
- Run the Epic Games Installer on your Mac and go to the Unreal Engine section. Install the Unreal Engine from there (5.0.3 at the time of writing this documentation).
- While the engine is being downloaded and installed, make sure to also install XCode. Unreal Engine only supports XCode up to version 13.x so you cannot install version 14 released during summer 2022.
- After XCode is installed, launch it to make sure it installs the command line tools and you accept the necessary terms and conditions.
- You can then close XCode and try to open the .uproject file at the root of this repository. Unreal Engine should offer you to build the plugin from source and load the editor.

If this does not work or you want to debug the plugin, you can generate the XCode project files from a terminal using a script provided by the Unreal Engine. This will generate a XCode Workspace which will allow you to build the plugin and run it from the debugger.

- Open a terminal and run the following command :
/Users/Shared/Epic\ Games/UE_5.0/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh /path/to/your/iModel.uproject
- Locate the file named iModel.xcworkspace which should have been created in the same location than the .uproject file and open it. This should open XCode
- Click the run button in XCode, this should build and start the Unreal Engine Editor