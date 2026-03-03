#include "voxel.hpp"
SingleTextureVoxel::SingleTextureVoxel(uint8_t texture_id) : texture_id(texture_id) {}

bool SingleTextureVoxel::is_active() const {
    return texture_id != 0;
}

MultiTextureVoxel::MultiTextureVoxel(uint8_t right, uint8_t left, uint8_t up, uint8_t down, uint8_t forward, uint8_t back) {
    texture_ids = new uint8_t[6];
    texture_ids[0] = right;
    texture_ids[1] = left;
    texture_ids[2] = up;
    texture_ids[3] = down;
    texture_ids[4] = forward;
    texture_ids[5] = back;
}

MultiTextureVoxel::~MultiTextureVoxel() {
    delete[] texture_ids;
}

bool MultiTextureVoxel::is_active() const {
    return texture_ids != nullptr && texture_ids[0] != 0;
}