# Easy-Voxels-GdExtension

an very easy voxelengine c++ gdextension for godot 4.4.1    
it uses 16x16x16 chunks and face culling.    
the tileset is 32x32 + 1 pixel padding = 34x34.    
the gdextension creates a new Node VoxelEngine.    
you can use is for example in gdscript:    
```
  # instantiate class VoxelEngine
  var ve = VoxelEngine.new()
	
  # initialize VoxelEngine sizex, sizey, sizez, tilemap with padding, parentnode, camera3D
  ve.InitVE(64, 32, 64, ResourceLoader.load("res://resources/textures/tilemap32.png"), self, get_node("Camera3D"))

  # set some voxels
  ve.set_voxel_singletexture(Vector3i(14, 2, 16), 4)
  ve.set_voxel_multitexture(Vector3i(16, 2, 16), 2, 3, 4, 5, 6, 7)

  ve.update_world()
```
