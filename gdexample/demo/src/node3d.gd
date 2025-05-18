extends Node3D


var ve: VoxelEngine
var cam: Camera3D
var mtimer: float = 0.0
const MINTERVAL: float = 0.5

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	cam = get_node("Camera3D")
	
	# instantiate class VoxelEngine
	ve = VoxelEngine.new()
	
	# initialize VoxelEngine sizex, sizey, sizez, tilemap with padding, parentnode, camera3D
	ve.InitVE(64, 32, 64, ResourceLoader.load("res://resources/textures/tilemap32.png"), self, cam)
	
	# set some voxels
	for x in range(32):
		for z in range(32):
			ve.set_voxel_singletexture(Vector3i(x, 0, z), 2)
	for x in range(8):
		for z in range(8):
			ve.set_voxel_singletexture(Vector3i(12+x, 1, 12+z), 6)
	ve.set_voxel_singletexture(Vector3i(14, 2, 16), 4)
	ve.set_voxel_multitexture(Vector3i(16, 2, 16), 2, 3, 4, 5, 6, 7)
	ve.set_voxel_multitexture(Vector3i(18, 2, 16), 7, 8, 10, 7, 8, 13)
	ve.set_voxel_multitexture(Vector3i(18, 5, 16), 3, 4, 5, 6, 7, 8)
	
	var start_timestamp = Time.get_unix_time_from_system()
	# create the meshes of the chunks and see the world
	ve.update_world()
	var stop_timestamp = Time.get_unix_time_from_system()
	print("update_world milliseconds:", (stop_timestamp-start_timestamp)*1000)
	
	
	ve.set_voxel_singletexture(Vector3i(1, 1, 1), 4)
	# test get_voxel_type
	var voxeltype = ve.get_voxel_type(Vector3i(1, 1, 1))
	print("voxeltype:", voxeltype)
	var voxeltexture = ve.get_voxel_texture(Vector3i(1, 1, 1), 0)
	print("voxeltexture:", voxeltexture)
	# test delete_voxel
	ve.delete_voxel(Vector3i(1, 1, 1));
	ve.update_world()
	


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	#pass
	var voxelresult : Vector3i
	mtimer += delta
	if mtimer >= MINTERVAL:
		voxelresult = ve.identify_voxel()
		print("voxel under mouse:", voxelresult)
		mtimer = 0.0

	
