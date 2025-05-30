#ifndef VOXEL_ENGINE_HPP
#define VOXEL_ENGINE_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include "chunk.hpp"
#include <unordered_map>


enum MeshMode {
    MESH_CUBE,
    MESH_MARCHING_CUBES
};


class VoxelEngine : public godot::Node3D {
    GDCLASS(VoxelEngine, godot::Node3D)

public:
    static int WORLD_SIZE_X;
    static int WORLD_SIZE_Y;
    static int WORLD_SIZE_Z;
    static const int CHUNK_SIZE = 16;

private:
    // Use the custom hasher for std::unordered_map
    //std::unordered_map<godot::Vector3i, Chunk*, godot::Vector3iHasher> chunks;
    std::vector<Chunk*> chunks;
    godot::Ref<godot::Texture2D> tilemap;
    int mesh_mode;

    inline size_t get_chunk_index(const godot::Vector3i& chunk_pos) const {
        int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
        int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
        return chunk_pos.x + chunk_pos.y * chunks_x + chunk_pos.z * chunks_x * chunks_y;
    }

    bool is_in_world(const godot::Vector3i& pos) const;
    void update_neighbor_chunks(const godot::Vector3i& chunk_pos, const godot::Vector3i& local_pos);

protected:
    static void _bind_methods();

public:
    
    VoxelEngine();
    //VoxelEngine(int size_x, int size_y, int size_z, godot::Ref<godot::Texture2D> tex = nullptr);
    //VoxelEngine();
    ~VoxelEngine();

    godot::Node3D* parent;
    godot::Camera3D* camera;
    void InitVE(int size_x, int size_y, int size_z, 
        godot::Ref<godot::Texture2D> tex, godot::Node3D* parentnode, godot::Camera3D* cameranode);
    void set_parentnode(Node3D* node);
    void update_world();
    //void set_voxel(const godot::Vector3i& global_pos, Voxel* voxel);
    void set_voxel_singletexture(const godot::Vector3i& global_pos, uint8_t textureid,float density = 0.0f);
    void set_voxel_multitexture(const godot::Vector3i& global_pos, 
    uint8_t right, uint8_t left, uint8_t up, uint8_t down, uint8_t forward, uint8_t back, float density = 0.0f);
    void sphere_singletexture(const godot::Vector3i& global_pos, uint8_t textureid, int radius, float density = 0.0f);
    void fill_voxel_region(const godot::Vector3i& start, const godot::Vector3i& end, int voxel_type, 
        uint8_t texture_id, const godot::PackedByteArray& multi_texture_ids = godot::PackedByteArray(), float density = 0.0f);
    void delete_voxel(const godot::Vector3i& global_pos);
    Voxel* get_voxel(const godot::Vector3i& global_pos) const;
    int get_voxel_type(const godot::Vector3i& global_pos) const;
    uint8_t get_voxel_texture(const godot::Vector3i& global_pos, int nr) const;
    godot::Ref<godot::Texture2D> get_tilemap() const;

    godot::Vector3i identify_voxel() const;

    void set_mesh_mode(int mode);
    int get_mesh_mode() const;
    float get_voxel_density(const godot::Vector3i& global_pos) const;
};

#endif // VOXEL_ENGINE_HPP