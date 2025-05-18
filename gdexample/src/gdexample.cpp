#include "gdexample.h"
#include <godot_cpp/core/class_db.hpp>
#include <time.h>

using namespace godot;

void GDExample::_bind_methods() {
}

GDExample::GDExample() {
	// Initialize any variables here.
	time_passed = 0.0;
}

GDExample::~GDExample() {
	// Add your cleanup here.
}

void GDExample::_ready() {
	clock_t t1,t2;

    UtilityFunctions::print("gdexample _Ready");
	tilemap = ResourceLoader::get_singleton()->load("res://resources/textures/tilemap32.png");
	//ve = memnew(VoxelEngine(WORLD_SIZE_X, WORLD_SIZE_Y, WORLD_SIZE_Z, tilemap));
	ve = memnew(VoxelEngine);
	//ve->InitVE(WORLD_SIZE_X, WORLD_SIZE_Y, WORLD_SIZE_Z, tilemap, Object::cast_to<Node3D>(this->get_parent()));
	//add_child(ve);
    //    ResourceLoader::get_singleton()->load("res://resources/textures/tilemap32.png"));
    //ve->set_parentnode(Object::cast_to<Node3D>(this->get_parent()));
	//ve->set_voxel(Vector3i(0, 0, 0), new SingleTextureVoxel(2));
    //ve->set_voxel(Vector3i(0, 1, 0), new SingleTextureVoxel(4));
    //ve->set_voxel(Vector3i(16, 0, 0), new SingleTextureVoxel(2));
	//ve->set_voxel(Vector3i(1, 1, 0), new SingleTextureVoxel(5));

	initialize_world();
	t1=clock();
	ve->update_world();
	t2=clock();
    float diff = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
	UtilityFunctions::print("update_world time:", diff);
}

void GDExample::_process(double delta) {
	//time_passed += delta;

	//Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));

	//set_position(new_position);
}


void GDExample::initialize_world() {
    UtilityFunctions::print("main InitializeWorld");
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            ve->set_voxel_singletexture(Vector3i(x, 0, z), 2);
        }
    }
    ve->set_voxel_singletexture(Vector3i(10, 1, 10), 3);
    ve->set_voxel_singletexture(Vector3i(11, 1, 10), 4);

}
	


