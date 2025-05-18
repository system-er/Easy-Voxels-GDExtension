# Easy-Voxels-GdExtension

an easy voxelengine c++ gdextension for godot 4.4.1    
it uses 16x16x16 chunks and face culling. there are voxels with a single texture or multitexture voxels.        
the tileset is 32x32 + 1 pixel padding = 34x34.    
the gdextension creates a new Node VoxelEngine.    

    
![Pic1](pic1.JPG)


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

# the methods with parameters:    
InitVE "size_x", "size_y", "size_z", "tex", "parentnode"    
set_voxel_singletexture "global_pos", "textureid"     
set_voxel_singletexture "global_pos", "textureid"    
set_voxel_multitexture "global_pos",  "right", "left", "up", "down", "forward", "back"     
update_world    
delete_voxel "global_pos"    
get_voxel_type "global_pos"    
get_voxel_texture "global_pos", "nr"    
