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
	ve.set_debug_enabled(false)
	# initialize VoxelEngine sizex, sizey, sizez, tilemap with padding, parentnode, camera3D
	ve.InitVE(64, 32, 64, ResourceLoader.load("res://resources/textures/tilemap32.png"), self, cam)

	# set some voxels
	ve.fill_voxel_region(Vector3i(0, 0, 0), Vector3i(32, 2, 32), 1, 2) 
	ve.fill_voxel_region(Vector3i(0, 0, 0), Vector3i(31, 0, 31), 1, 2)
	ve.fill_voxel_region(Vector3i(12, 1, 12), Vector3i(20, 3, 20), 1, 6) 
	ve.fill_voxel_region(Vector3i(14, 2, 14), Vector3i(17, 5, 17), 1, 4) 
	ve.set_voxel_multitexture(Vector3i(9, 5, 16), 3, 4, 5, 6, 7, 8)
	#ve.fill_voxel_region(Vector3i(20, 2, 20), Vector3i(21, 5, 21), 1, 12, PackedByteArray())
	#ve.sphere_singletexture(Vector3i(14, 12, 14), 8, 5, 0.9)
	
	var start_timestamp = Time.get_unix_time_from_system()
	# create the meshes of the chunks and see the world
	ve.update_world()
	var stop_timestamp = Time.get_unix_time_from_system()
	var time = str((stop_timestamp-start_timestamp)*1000, 0.1).pad_decimals(2)
	print("update_world milliseconds:", time)
	


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	var voxelresult : Vector3i
	mtimer += delta
	if mtimer >= MINTERVAL:
		voxelresult = ve.identify_voxel()
		if voxelresult != voxeloldpos:
			print("identify_voxel result:", voxelresult)
			voxeloldpos = voxelresult
			mtimer = 0.0


#func _input(event):
	# Receives key input			
	#if event.pressed and event.keycode == KEY_ESCAPE:
	#	get_tree().quit()
			
