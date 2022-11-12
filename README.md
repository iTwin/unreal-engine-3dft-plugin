# Unreal Engine 3DFT Plugin

## Table of contents

1. [Pre-requisites](#pre-requisites)
2. [Compile the project manually and debug it](#compile-the-project-manually-and-debug-it)
3. [Technology overview](#technology-overview)
4. [Plugin](#plugin)
5. [Tutorials](#tutorials)

## Pre-requisites

### Mac

Please, follow the url links to get detailed instructions on how to install the described applications.

1. Install [Unreal Engine](https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/) (we recommend **5.0.3** at the time of writing this documentation)

2. Install[XCode 13.x](https://developer.apple.com/support/xcode/) (Unreal Engine only supports XCode up to version 13.x at the time of writing this documentation, so you cannot install version 14 released during summer 2022.)

3. After XCode is installed, launch it to make sure it installs the command line tools and you accept the necessary terms and conditions. You can then close XCode.

4. Open the .uproject file at the root of this repository. Unreal Engine should offer you to build the plugin from source and proceed to load the editor.

5. If Unreal Engine shows errors when loading the .uproject, [compile it manually](#compile-the-project-manually-and-debug-it).

### Windows

Please, follow the url links to get detailed instructions on how to install the described applications.

1. Install [Unreal Engine](https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/) (we recommend **5.0.3** at the time of writing this documentation)

2. Install [Visual Studio](https://docs.unrealengine.com/5.0/en-US/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine/) or [Visual Code](https://docs.unrealengine.com/5.0/en-US/setting-up-visual-studio-code-for-unreal-engine/).

3. Open the .uproject file at the root of this repository. Unreal Engine should offer you to build the plugin from source and proceed to load the editor.

4. If Unreal Engine shows errors when loading the .uproject, [compile it manually](#compile-the-project-manually-and-debug-it).

</br>

## Compile the project manually and debug it

### Mac

You can generate the XCode project files from a terminal using a script provided by the Unreal Engine. This will generate a XCode Workspace which will allow you to build the plugin and run it from the debugger.

1. Open a terminal and run the following command :
/Users/Shared/Epic\ Games/UE_5.0/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh /path/to/your/iModel.uproject

2. Locate the file named iModel.xcworkspace which should have been created in the same location than the .uproject file and open it. This should open XCode.

3. Click the run button in XCode, this should build and start the Unreal Engine Editor.

4. Find more information about compiling game projects [here](https://docs.unrealengine.com/5.0/en-US/compiling-game-projects-in-unreal-engine-using-cplusplus/).

### Windows

1. Generate the Visual Studio project file by right-clicking on the .uproject file and selecting *Generate Visual Studio project files*.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![Generate VS project files](docs/windows_generate_vs_project_files.png)

2. You can also generate them manually following these [instructions](https://docs.unrealengine.com/5.0/en-US/how-to-generate-unreal-engine-project-files-for-your-ide/).

3. Use the **Debug** command to run and debug the project. It will automatically run the Unreal Editor with your project loaded.

4. Find more information about compiling game projects [here](https://docs.unrealengine.com/5.0/en-US/compiling-game-projects-in-unreal-engine-using-cplusplus/).

<br/>

## Technology Overview

Engineers are sitting on some incredible models that are too complicated to render in real-time using traditional methods.

3DFT (**3D** **F**ast **T**ransmission) is a new rendering codec that rapidly optimizes/compresses large 3D models for high Unreal Engine framerates, without data loss.

Aimed to achieve the ”4 F’s” (fast encoding, fast transmission, fast decoding, fast rendering) 3DFT defines encoding and decoding methods, a persistence format and a rendering architecture.

3DFT is a fundamental part of [The Bentley iTwin platform](https://www.bentley.com/software/itwin-platform/), a collection of APIs and services designed to help you build digital twin applications and bring them to market quickly. It provides the foundation for building SaaS solutions to design, build, and operate infrastructure assets. Accelerate application development by letting the iTwin platform handle data integration, visualization, change tracking, security, and other complex challenges. Whether you are building SaaS solutions for your clients advancing their digital twin initiatives, or implementing bespoke solutions in your organization, this is the platform for you.

3DFT streams the 3d models stored in the iTwin platform to Unreal Engine directly, without the need to export/import them. The data streaming optimizes the transmission of data so only a fraction of the size of the model is transmitted, instead of exporting/importing the whole model.

![dani](docs/3dft_dataflow_overview.png)

<br/>
<br/>

If you want to learn more, click of the image to watch the session about 3DFT that we did at the Unreal Fest 2022.

<br/>

[![3DFT at Unreal Fest](https://img.youtube.com/vi/Iag2lJj82m4/0.jpg)](https://www.youtube.com/watch?v=Iag2lJj82m4)

## Plugin


## Tutorials
