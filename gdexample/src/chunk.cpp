#include "chunk.hpp"
#include "voxelengine.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

//Chunk::Chunk() :
Chunk::Chunk() : voxel_engine(nullptr), needs_mesh_update(false), mesh_instance(nullptr), sm3d(nullptr) {}

Chunk::Chunk(VoxelEngine* engine, const Vector3i& chunk_pos) : voxel_engine(engine), chunk_position(chunk_pos), needs_mesh_update(false) {

    voxels.resize(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, nullptr);
    initialize_voxels();
    if (!voxel_engine) {
        UtilityFunctions::printerr("Chunk constructor: voxelEngine is null!");
    } else {

    }
    
    vLEFT = Vector3(-1, 0, 0);
    vRIGHT = Vector3(1, 0, 0);
    vUP = Vector3(0, 1, 0);
    vDOWN = Vector3(0, -1, 0);
    vFORWARD = Vector3(0, 0, -1);
    vBACK = Vector3(0, 0, 1);
}

Chunk::~Chunk() {
    for (Voxel* voxel : voxels) {
        delete voxel;
    }
}

void Chunk::initialize_voxels() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                voxels[get_index(x, y, z)] = new SingleTextureVoxel(0);
            }
        }
    }
}

void Chunk::init() {
    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    sm3d = memnew(StandardMaterial3D);
    sm3d->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, voxel_engine->get_tilemap());
    sm3d->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST);

}

void Chunk::set_voxel(const Vector3i& local_pos, Voxel* voxel) {
    if (is_in_bounds(local_pos)) {
        size_t index = get_index(local_pos.x, local_pos.y, local_pos.z);
        delete voxels[index];
        voxels[index] = voxel;
        needs_mesh_update = true;
    }
}

void Chunk::update_mesh() {
    if (needs_mesh_update) {
        generate_mesh();
        needs_mesh_update = false;
    }
}

Voxel* Chunk::get_voxel(const Vector3i& local_pos) const {
    if (is_in_bounds(local_pos)) {
        return voxels[get_index(local_pos.x, local_pos.y, local_pos.z)];
    }
    return new SingleTextureVoxel(0);
}

void Chunk::generate_mesh() {
    Ref<SurfaceTool> surface_tool = Ref<SurfaceTool>(memnew(SurfaceTool));
    surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
    int triangle_count = 0;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Voxel* voxel = voxels[get_index(x, y, z)];
                if (!voxel->is_active()) continue;

                Vector3 pos(x, y, z);
                triangle_count += add_cube(surface_tool, pos, voxel);
            }
        }
    }

    Ref<ArrayMesh> mesh = surface_tool->commit();
    mesh_instance->set_mesh(mesh);
    mesh_instance->set_material_override(sm3d);
    UtilityFunctions::print("GenerateMesh trianglecount:", triangle_count);
}

bool Chunk::is_in_bounds(const Vector3i& pos) const {
    return pos.x >= 0 && pos.x < CHUNK_SIZE &&
           pos.y >= 0 && pos.y < CHUNK_SIZE &&
           pos.z >= 0 && pos.z < CHUNK_SIZE;
}

bool Chunk::is_voxel_active(const Vector3i& pos) const {
    Vector3i global_pos = chunk_position * CHUNK_SIZE + pos;
    if (!voxel_engine) {
        UtilityFunctions::printerr("IsVoxelActive (Chunk at ", chunk_position, "): voxelEngine is null! Returning false.");
        return false;
    }
    Voxel* voxel = voxel_engine->get_voxel(global_pos);
    bool active = voxel->is_active();
    //delete voxel;
    return active;
}

bool Chunk::is_neighbor_active(const Vector3i& neighbor_pos) const {
    if (is_in_bounds(neighbor_pos)) {
        return get_voxel(neighbor_pos)->is_active();
    }

    Vector3i global_pos = chunk_position * CHUNK_SIZE + neighbor_pos;
    if (global_pos.x < 0 || global_pos.x >= VoxelEngine::WORLD_SIZE_X ||
        global_pos.y < 0 || global_pos.y >= VoxelEngine::WORLD_SIZE_Y ||
        global_pos.z < 0 || global_pos.z >= VoxelEngine::WORLD_SIZE_Z) {
        return false;
    }

    Voxel* voxel = voxel_engine->get_voxel(global_pos);
    bool active = voxel->is_active();
    delete voxel;
    return active;
}

int Chunk::add_cube(Ref<SurfaceTool> st, const Vector3& pos, Voxel* voxel) {
    int added_triangles = 0;
    Ref<Texture2D> tilemap = voxel_engine->get_tilemap();
    int tiles_per_row = tilemap->get_width() / 34;
    float tile_size_u = 32.0f / tilemap->get_width();
    float tile_size_v = 32.0f / tilemap->get_height();
    float uv_offset = 0.5f / tilemap->get_width();
    float padding_u = 1.0f / tilemap->get_width();
    float padding_v = 1.0f / tilemap->get_height();

    Vector3i local_pos((int)pos.x, (int)pos.y, (int)pos.z);

    if (SingleTextureVoxel* simple_voxel = dynamic_cast<SingleTextureVoxel*>(voxel)) {
        uint8_t texture_id = simple_voxel->texture_id - 1;
        float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
        float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;

        if (!is_voxel_active(local_pos + Vector3i(1, 0, 0)))
            added_triangles += add_face(st, pos, vRIGHT, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        if (!is_voxel_active(local_pos + Vector3i(-1, 0, 0)))
            added_triangles += add_face(st, pos, vLEFT, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        if (!is_voxel_active(local_pos + Vector3i(0, 1, 0)))
            added_triangles += add_face(st, pos, vUP, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        if (!is_voxel_active(local_pos + Vector3i(0, -1, 0)))
            added_triangles += add_face(st, pos, vDOWN, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        if (!is_voxel_active(local_pos + Vector3i(0, 0, 1)))
            added_triangles += add_face(st, pos, vFORWARD, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        if (!is_voxel_active(local_pos + Vector3i(0, 0, -1)))
            added_triangles += add_face(st, pos, vBACK, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
    } else if (MultiTextureVoxel* multi_voxel = dynamic_cast<MultiTextureVoxel*>(voxel)) {
        uint8_t* texture_ids = multi_voxel->texture_ids;

        // Right face
        if (!is_voxel_active(local_pos + Vector3i(1, 0, 0))) {
            uint8_t texture_id = texture_ids[0] - 1; // Right face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vRIGHT, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }

        // Left face
        if (!is_voxel_active(local_pos + Vector3i(-1, 0, 0))) {
            uint8_t texture_id = texture_ids[1] - 1; // Left face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vLEFT, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }

        // Up face
        if (!is_voxel_active(local_pos + Vector3i(0, 1, 0))) {
            uint8_t texture_id = texture_ids[2] - 1; // Up face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vUP, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }

        // Down face
        if (!is_voxel_active(local_pos + Vector3i(0, -1, 0))) {
            uint8_t texture_id = texture_ids[3] - 1; // Down face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vDOWN, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }

        // Forward face
        if (!is_voxel_active(local_pos + Vector3i(0, 0, 1))) {
            uint8_t texture_id = texture_ids[4] - 1; // Forward face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vFORWARD, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }

        // Back face
        if (!is_voxel_active(local_pos + Vector3i(0, 0, -1))) {
            uint8_t texture_id = texture_ids[5] - 1; // Back face
            float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
            float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;
            added_triangles += add_face(st, pos, vBACK, texture_id, tile_u, tile_v, tile_size_u, tile_size_v, uv_offset);
        }
    }

    return added_triangles;
}

int Chunk::add_face(Ref<SurfaceTool> st, godot::Vector3 pos, const Vector3& normal, uint8_t texture_id,
                    float tile_u, float tile_v, float tile_size_u, float tile_size_v, float uv_offset) {
    //texture_id++;
    Ref<Texture2D> tilemap = voxel_engine->get_tilemap();
    int tiles_per_row = tilemap->get_width() / 34;
    float padding_u = 1.0f / tilemap->get_width();
    float padding_v = 1.0f / tilemap->get_height();

    tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
    tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;

    Vector3 vertices[4];
    Vector2 uvs[4];

    if (normal == vRIGHT) {
        vertices[0] = pos + Vector3(0.5f, -0.5f, -0.5f); // Bottom-left
        vertices[1] = pos + Vector3(0.5f, 0.5f, -0.5f);  // Top-left
        vertices[2] = pos + Vector3(0.5f, 0.5f, 0.5f);   // Top-right
        vertices[3] = pos + Vector3(0.5f, -0.5f, 0.5f);  // Bottom-right
        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-left
        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);              // Top-left
        uvs[2] = Vector2(tile_u, tile_v + uv_offset);                                        // Top-right
        uvs[3] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);                          // Bottom-right
    } else if (normal == vLEFT) {
        vertices[0] = pos + Vector3(-0.5f, -0.5f, 0.5f);  // Bottom-right
        vertices[1] = pos + Vector3(-0.5f, 0.5f, 0.5f);   // Top-right
        vertices[2] = pos + Vector3(-0.5f, 0.5f, -0.5f);  // Top-left
        vertices[3] = pos + Vector3(-0.5f, -0.5f, -0.5f); // Bottom-left
        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-right
        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);              // Top-right
        uvs[2] = Vector2(tile_u, tile_v + uv_offset);                                        // Top-left
        uvs[3] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);                          // Bottom-left
    } else if (normal == vUP) {
        vertices[0] = pos + Vector3(-0.5f, 0.5f, -0.5f); // Bottom-left
        vertices[1] = pos + Vector3(-0.5f, 0.5f, 0.5f);  // Bottom-right
        vertices[2] = pos + Vector3(0.5f, 0.5f, 0.5f);   // Top-right
        vertices[3] = pos + Vector3(0.5f, 0.5f, -0.5f);  // Top-left
        uvs[0] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);
        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
        uvs[2] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);
        uvs[3] = Vector2(tile_u, tile_v + uv_offset);
    } else if (normal == vDOWN) {
        vertices[0] = pos + Vector3(-0.5f, -0.5f, 0.5f);  // Bottom-left
        vertices[1] = pos + Vector3(-0.5f, -0.5f, -0.5f); // Bottom-right
        vertices[2] = pos + Vector3(0.5f, -0.5f, -0.5f);  // Top-right
        vertices[3] = pos + Vector3(0.5f, -0.5f, 0.5f);   // Top-left
        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);              // Bottom-left
        uvs[1] = Vector2(tile_u, tile_v + uv_offset);                                        // Bottom-right
        uvs[2] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);                          // Top-right
        uvs[3] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Top-left
    } else if (normal == vFORWARD) {
        vertices[0] = pos + Vector3(0.5f, -0.5f, 0.5f);   // Bottom-right
        vertices[1] = pos + Vector3(0.5f, 0.5f, 0.5f);    // Top-right
        vertices[2] = pos + Vector3(-0.5f, 0.5f, 0.5f);   // Top-left
        vertices[3] = pos + Vector3(-0.5f, -0.5f, 0.5f);  // Bottom-left
        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);
        uvs[2] = Vector2(tile_u, tile_v + uv_offset);
        uvs[3] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);
    } else if (normal == vBACK) {
        vertices[0] = pos + Vector3(-0.5f, -0.5f, -0.5f); // Bottom-left
        vertices[1] = pos + Vector3(-0.5f, 0.5f, -0.5f);  // Top-left
        vertices[2] = pos + Vector3(0.5f, 0.5f, -0.5f);   // Top-right
        vertices[3] = pos + Vector3(0.5f, -0.5f, -0.5f);  // Bottom-right
        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-left
        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);              // Top-left
        uvs[2] = Vector2(tile_u, tile_v + uv_offset);                                        // Top-right
        uvs[3] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);                          // Bottom-right
    }

    int indices[6] = {0, 2, 1, 0, 3, 2};
    for (int i : indices) {
        st->set_normal(normal);
        st->set_uv(uvs[i]);
        st->add_vertex(vertices[i]);
    }

    return 2;
}

void Chunk::_bind_methods() {
}