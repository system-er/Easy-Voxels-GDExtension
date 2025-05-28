#ifndef VOXEL_HPP
#define VOXEL_HPP

#include <godot_cpp/classes/node.hpp>
#include <cstdint>

class Voxel {
public:
    virtual ~Voxel() = default;
    virtual bool is_active() const = 0;
    virtual uint8_t get_texture_id(int nr) const { return 0; }
    virtual float get_density() const { return 0.0f; }
};

class SingleTextureVoxel : public Voxel {
public:
    uint8_t texture_id;
    float density;

    SingleTextureVoxel(uint8_t texture_id = 0, float density = 0.0f);
    bool is_active() const override;
    uint8_t get_texture_id(int nr) const override { return texture_id; }
    float get_density() const override { return density; }
};

class MultiTextureVoxel : public Voxel {
public:
    uint8_t* texture_ids;
    float density;
    
    MultiTextureVoxel(uint8_t right, uint8_t left, uint8_t up, uint8_t down, uint8_t forward, uint8_t back, float density = 0.0f);
    ~MultiTextureVoxel();
    bool is_active() const override;
    uint8_t get_texture_id(int nr) const override { return texture_ids[nr]; }
    float get_density() const override { return density; }
};

#endif // VOXEL_HPP