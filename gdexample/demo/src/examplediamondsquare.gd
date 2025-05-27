extends Node3D


var ve: VoxelEngine
var cam: Camera3D

var size = 65 # 2^n + 1
var height_map = []
var max_height = 45
var roughness = 0.5

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	cam = get_node("Camera3D")
	
	# instantiate class VoxelEngine
	ve = VoxelEngine.new()
	
	# initialize VoxelEngine sizex, sizey, sizez, tilemap with padding, parentnode, camera3D
	ve.InitVE(128, 32, 128, ResourceLoader.load("res://resources/textures/terrain32.png"), self, cam)

	initialize_height_map()
	diamond_square()
	apply_to_voxel_engine()
	
	var start_timestamp = Time.get_unix_time_from_system()
	ve.update_world()
	var stop_timestamp = Time.get_unix_time_from_system()
	var time = str((stop_timestamp-start_timestamp)*1000, 0.1).pad_decimals(2)
	print("update_world milliseconds:", time)
	


func initialize_height_map():
	height_map.clear()
	for x in range(size):
		var row = []
		for z in range(size):
			row.append(0.0)
		height_map.append(row)
		
	height_map[0][0] = 0.0
	height_map[0][size-1] = 0.0
	height_map[size-1][0] = 0.0
	height_map[size-1][size-1] = 0.0

func diamond_square():
	var step = size - 1
	var scale = max_height * roughness

	while step > 1:
		var half_step = step / 2
		
		for x in range(0, size - 1, step):
			for z in range(0, size - 1, step):
				var avg = (height_map[x][z] + height_map[x + step][z] + 
					height_map[x][z + step] + height_map[x + step][z + step]) / 4.0
				height_map[x + half_step][z + half_step] = avg + randf_range(-scale, scale)
		
		for x in range(0, size, step):
			for z in range(0, size, step):
				if x == 0 or x == size - 1 or z == 0 or z == size - 1:
					continue
				var mid_x = x + half_step
				var mid_z = z + half_step
				
				var avg = (height_map[x][z] + height_map[x + step][z] + 
				height_map[mid_x][mid_z] + height_map[mid_x - step][mid_z]) / 4.0
				height_map[mid_x][z] = avg + randf_range(-scale, scale)
				
				avg = (height_map[x + step][z] + height_map[x + step][z + step] + 
					height_map[mid_x][mid_z] + height_map[mid_x][mid_z - step]) / 4.0
				height_map[x + step][mid_z] = avg + randf_range(-scale, scale)
				
				avg = (height_map[x][z + step] + height_map[x + step][z + step] + 
				height_map[mid_x][mid_z] + height_map[mid_x - step][mid_z]) / 4.0
				height_map[mid_x][z + step] = avg + randf_range(-scale, scale)
				
				avg = (height_map[x][z] + height_map[x][z + step] + 
				height_map[mid_x][mid_z] + height_map[mid_x][mid_z - step]) / 4.0
				height_map[x][mid_z] = avg + randf_range(-scale, scale)
		
		step /= 2
		scale *= roughness

func apply_to_voxel_engine():
	for x in range(size):
		for z in range(size):
			var height = int(clamp(height_map[x][z], 0, max_height))
			
			var texture_id = 2
			if height > 0 and height <= 2:
				texture_id = 5
			elif height > 2 and height <= 5:
				texture_id = 3
			elif height > 5 and height <= 8:
				texture_id = 4
			elif height > 8 and height <= 12:	
				texture_id = 7
			elif height > 12:
				texture_id = 6

			for y in range(height + 1):
				ve.set_voxel_singletexture(Vector3i(x, y, z), texture_id)
