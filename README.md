# CSG Graph Editor and Visualizer

The purpose of the project is to implement an editor for **constructive solid geometry** (CSG) scenes which supports real-time visualization.

Scenes will be modeled and edited as **CSG tree graphs**, created by a node edtior based on [imgui-node-editor](https://github.com/thedmd/imgui-node-editor).

The visualization will use **ray tracing** based pixel shaders. The **signed distance functions** of the shaders will be generated and updated in real-time, from the graphs constructed in the node editor.

# Dragonfly Framework

The project uses the OpenGL based Dragonfly rendering framework, developed at Eötvös Loránd University.

### More Information

 - [Dragonfly Poster](https://people.inf.elte.hu/csabix/publications/articles/WSPS_2020_Poster_Dragonfly.pdf)
 - [Dragonfly Project](https://github.com/ELTE-IK-CG/Dragonfly)
 - [Dragonfly Template Repository](https://github.com/ELTE-IK-CG/The-Dragon-Flies)
 - Or contact the authors at csabix@inf.elte.hu or bundas@inf.elte.hu
 
### Install

 - Download the [DragonflyPack.zip](https://drive.google.com/file/d/1YHrIyQqoi5fef00poe038AW2M7P9rVfB/view?usp=sharing)
 - Find a location and create a virtual T drive, eg.: `subst T: C:\MyT_Drive\`
 - Unzip the `DragonflyPack` there. There should be 4 folders in `T:\DragonflyPack`
 - Download Dragonfly as a submodule within the cloned The-Dragon-Flies project
 - Use Visual Studio to build the solution

### Credits

Dragonfly is developed by Csaba Bálint and Róbert Bán at Eötvös Loránd University, Budapest, Hungary.

Supported by the ÚNKP19-3 New National Excellence Program of the Ministry for Innovation and Technology. The project has been supported by the European Union, co-financed by the European Social Fund (EFOP-3.6.3-VEKOP-16-2017-00001).

Dragonfly is licensed under the MIT License, see `Dragonfly\LICENSE.txt` for more information.
