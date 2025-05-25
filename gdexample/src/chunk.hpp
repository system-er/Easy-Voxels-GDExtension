#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include "voxel.hpp"
#include <vector>

class VoxelEngine;

// Hash-Funktor für Vector3i
namespace godot {
    struct Vector3iHasher {
        std::size_t operator()(const Vector3i& v) const {
            std::size_t h1 = std::hash<int>{}(v.x);
            std::size_t h2 = std::hash<int>{}(v.y);
            std::size_t h3 = std::hash<int>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}


class Chunk : public godot::Node3D {
    //GDCLASS(Chunk, godot::Node3D)

private:
    static const int CHUNK_SIZE = 16;

    //std::vector<Voxel*> voxels; // 1D array of Voxel pointers
    std::unordered_map<godot::Vector3i, Voxel*, godot::Vector3iHasher> voxels;
    godot::Vector3i chunk_position;
    //godot::Ref<godot::MeshInstance3D> mesh_instance;
    godot::MeshInstance3D *mesh_instance; // = nullptr;
    //SurfaceTool *sm3d = nullptr;
    VoxelEngine* voxel_engine;
    bool needs_mesh_update;
    //godot::Ref<godot::StandardMaterial3D> sm3d;
    godot::StandardMaterial3D *sm3d; // = nullptr;

    godot::Vector3 vLEFT;
    godot::Vector3 vRIGHT;
    godot::Vector3 vUP;
    godot::Vector3 vDOWN;
    godot::Vector3 vFORWARD;
    godot::Vector3 vBACK;

    void initialize_voxels();
    bool is_in_bounds(const godot::Vector3i& pos) const;
    bool is_voxel_active(const godot::Vector3i& pos) const;
    bool is_neighbor_active(const godot::Vector3i& neighbor_pos) const;
    int add_cube(godot::Ref<godot::SurfaceTool> st, const godot::Vector3& pos, Voxel* voxel);
    int add_face(godot::Ref<godot::SurfaceTool> st, godot::Vector3 pos, const godot::Vector3& normal, uint8_t texture_id,
                 float tile_u, float tile_v, float tile_size_u, float tile_size_v, float uv_offset);

    // Helper to convert 3D coordinates to 1D index
    inline size_t get_index(int x, int y, int z) const {
        return x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
    }

protected:
    static void _bind_methods();

public:
    Chunk();
    Chunk(VoxelEngine* engine, const godot::Vector3i& chunk_pos);
    ~Chunk();

    void init();
    void set_voxel(const godot::Vector3i& local_pos, Voxel* voxel);
    void update_mesh();
    Voxel* get_voxel(const godot::Vector3i& local_pos) const;
    void generate_mesh();

};

#endif // CHUNK_HPP