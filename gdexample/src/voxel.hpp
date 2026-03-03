#ifndef VOXEL_HPP
#define VOXEL_HPP

#include <godot_cpp/classes/node.hpp>
#include <cstdint>

class Voxel {
public:
    virtual ~Voxel() = default;
    virtual bool is_active() const = 0;
    virtual uint8_t get_texture_id(int nr) const { return 0; }
};

class SingleTextureVoxel : public Voxel {
public:
    uint8_t texture_id;

    SingleTextureVoxel(uint8_t texture_id = 0);
    bool is_active() const override;
    uint8_t get_texture_id(int nr) const override { return texture_id; }
};

class MultiTextureVoxel : public Voxel {
public:
    uint8_t* texture_ids;
    
    MultiTextureVoxel(uint8_t right, uint8_t left, uint8_t up, uint8_t down, uint8_t forward, uint8_t back);
    ~MultiTextureVoxel();
    bool is_active() const override;
    uint8_t get_texture_id(int nr) const override { return texture_ids[nr]; }
};

#endif // VOXEL_HPP