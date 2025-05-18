#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include "voxelengine.hpp"

namespace godot {

class GDExample : public Sprite2D {
	//GDCLASS(GDExample, Sprite2D)

private:
	double time_passed;
	static const int WORLD_SIZE_X = 64;
    static const int WORLD_SIZE_Y = 32;
    static const int WORLD_SIZE_Z = 64;
	godot::Ref<godot::Texture2D> tilemap;
	VoxelEngine* ve;

protected:
	static void _bind_methods();

public:
	GDExample();
	~GDExample();
	void _ready() override;
    //void _input(const godot:
	void _process(double delta) override;
	void initialize_world();
};

}

#endif