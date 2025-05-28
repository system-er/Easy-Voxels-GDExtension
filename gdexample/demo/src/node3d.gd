extends Node3D


var ve: VoxelEngine
var cam: Camera3D
var mtimer: float = 0.0
var voxeloldpos: Vector3i = Vector3i(-1, -1, -1)
const MINTERVAL: float = 0.3


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	cam = get_node("Camera3D")
	
	# instantiate class VoxelEngine
	ve = VoxelEngine.new()
	
	# initialize VoxelEngine sizex, sizey, sizez, tilemap with padding, parentnode, camera3D
	ve.InitVE(64, 32, 64, ResourceLoader.load("res://resources/textures/tilemap32.png"), self, cam)
	ve.set_mesh_mode(0)
	# set some voxels
	ve.fill_voxel_region(Vector3i(0, 1, 0), Vector3i(32, 1, 32), 1, 2, PackedByteArray(), 1.0) 
	ve.fill_voxel_region(Vector3i(12, 2, 12), Vector3i(20, 2, 20), 1, 6, PackedByteArray(), 1.0) 
	#ve.set_voxel_singletexture(Vector3i(14, 2, 16), 4, 1.0)
	#ve.set_voxel_singletexture(Vector3i(14, 2, 17), 5, 1.0)
	#ve.set_voxel_singletexture(Vector3i(14, 3, 16), 6, 1.0)
	#ve.set_voxel_singletexture(Vector3i(15, 2, 16), 7, 1.0)
	#ve.set_voxel_multitexture(Vector3i(16, 2, 16), 2, 3, 4, 5, 6, 7, 1.0)
	#ve.set_voxel_multitexture(Vector3i(18, 2, 16), 7, 8, 10, 7, 8, 13, 1.0)
	ve.set_voxel_multitexture(Vector3i(18, 5, 16), 3, 4, 5, 6, 7, 8, 1.0)
	ve.fill_voxel_region(Vector3i(14, 3, 14), Vector3i(16, 7, 16), 1, 4, PackedByteArray(), 1.0) 
	
	var start_timestamp = Time.get_unix_time_from_system()
	# create the meshes of the chunks and see the world
	ve.update_world()
	var stop_timestamp = Time.get_unix_time_from_system()
	var time = str((stop_timestamp-start_timestamp)*1000, 0.1).pad_decimals(2)
	print("update_world milliseconds:", time)
	


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta: float) -> void:
	#pass
#	var voxelresult : Vector3i
#	mtimer += delta
#	if mtimer >= MINTERVAL:
#		voxelresult = ve.identify_voxel()
#		if voxelresult != voxeloldpos:
#			print("voxelresult:", voxelresult)
#			#ve.delete_voxel(voxelresult)
#			voxeloldpos = voxelresult
#			mtimer = 0.0
