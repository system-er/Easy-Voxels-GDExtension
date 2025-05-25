#include "voxelengine.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/viewport.hpp> 

using namespace godot;

int VoxelEngine::WORLD_SIZE_X = 0;
int VoxelEngine::WORLD_SIZE_Y = 0;
int VoxelEngine::WORLD_SIZE_Z = 0;


VoxelEngine::VoxelEngine() : parent(nullptr) {}


VoxelEngine::~VoxelEngine() {
    for (Chunk* chunk : chunks) {
        if (chunk) {
            memdelete(chunk); // Use memdelete instead of delete
        }
    }
}

void VoxelEngine::InitVE(int size_x, int size_y, int size_z, 
    Ref<Texture2D> tex, godot::Node3D* parentnode, godot::Camera3D* cameranode){
    WORLD_SIZE_X = size_x;
    WORLD_SIZE_Y = size_y;
    WORLD_SIZE_Z = size_z;
    tilemap = tex;

    UtilityFunctions::print("VoxelEngine ready - tilemap:", tilemap);

    int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;

    chunks.resize(chunks_x * chunks_y * chunks_z, nullptr);

    parent = memnew(Node3D);
    camera = memnew(Camera3D);
    parent = parentnode;
    camera = cameranode;
    UtilityFunctions::print("parent:", parent, " camera:", camera);
}



void VoxelEngine::update_world() {
    UtilityFunctions::print("VoxelEngine UpdateWorld");
    for (Chunk* chunk : chunks) {
        if (chunk) {
            chunk->update_mesh();
        }
    }
}


void VoxelEngine::set_voxel_singletexture(const Vector3i& global_pos, uint8_t textureid) {
    
    Voxel* voxel = new SingleTextureVoxel(textureid);
    
    if (!is_in_world(global_pos)) {
        delete voxel;
        return;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;
    if (chunk_pos.x < 0 || chunk_pos.x >= chunks_x ||
        chunk_pos.y < 0 || chunk_pos.y >= chunks_y ||
        chunk_pos.z < 0 || chunk_pos.z >= chunks_z) {
        delete voxel;
        return;
    }

    size_t index = get_chunk_index(chunk_pos);
    if (!chunks[index]) {
        Chunk* chunk = memnew(Chunk(this, chunk_pos));
        chunks[index] = chunk;
        parent->add_child(chunk);
        chunk->set_position(chunk_pos * CHUNK_SIZE);
        chunk->init();
    }

    chunks[index]->set_voxel(local_pos, voxel);
}

void VoxelEngine::set_voxel_multitexture(const Vector3i& global_pos, 
    uint8_t right, uint8_t left, uint8_t up, uint8_t down, uint8_t forward, uint8_t back) {
    
    Voxel* voxel = new MultiTextureVoxel(right, left, up, down, forward, back);
    
    if (!is_in_world(global_pos)) {
        delete voxel;
        return;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;
    if (chunk_pos.x < 0 || chunk_pos.x >= chunks_x ||
        chunk_pos.y < 0 || chunk_pos.y >= chunks_y ||
        chunk_pos.z < 0 || chunk_pos.z >= chunks_z) {
        delete voxel;
        return;
    }

    size_t index = get_chunk_index(chunk_pos);
    if (!chunks[index]) {
        Chunk* chunk = memnew(Chunk(this, chunk_pos));
        chunks[index] = chunk;
        parent->add_child(chunk);
        chunk->set_position(chunk_pos * CHUNK_SIZE);
        chunk->init();
    }

    chunks[index]->set_voxel(local_pos, voxel);
}

/*
void VoxelEngine::delete_voxel(const Vector3i& global_pos) {
    if (!is_in_world(global_pos)) {
        UtilityFunctions::print("DeleteVoxel: ", global_pos, " is outside world bounds");
        return;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    size_t index = get_chunk_index(chunk_pos);
    if (chunks[index]) {
        chunks[index]->set_voxel(local_pos, new SingleTextureVoxel(0));
        chunks[index]->update_mesh();
        update_neighbor_chunks(chunk_pos, local_pos);
    } else {
        UtilityFunctions::print("DeleteVoxel: No chunk found at chunk position ", chunk_pos);
    }
}
*/
void VoxelEngine::delete_voxel(const Vector3i& global_pos) {
    if (!is_in_world(global_pos)) {
        UtilityFunctions::print("DeleteVoxel: ", global_pos, " is outside world bounds");
        return;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    size_t index = get_chunk_index(chunk_pos);
    if (chunks[index]) {
        // Setze einen inaktiven Voxel (wird in set_voxel entfernt)
        chunks[index]->set_voxel(local_pos, new SingleTextureVoxel(0));
        chunks[index]->update_mesh();
        update_neighbor_chunks(chunk_pos, local_pos);
    } else {
        UtilityFunctions::print("DeleteVoxel: No chunk found at chunk position ", chunk_pos);
    }
}

Voxel* VoxelEngine::get_voxel(const Vector3i& global_pos) const {
    if (!is_in_world(global_pos)) {
        return new SingleTextureVoxel(0);
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
    int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;

    if (chunk_pos.x < 0 || chunk_pos.x >= chunks_x ||
        chunk_pos.y < 0 || chunk_pos.y >= chunks_y ||
        chunk_pos.z < 0 || chunk_pos.z >= chunks_z) {
        return new SingleTextureVoxel(0);
    }

    size_t index = get_chunk_index(chunk_pos);
    if (index >= chunks.size() || !chunks[index]) {
        return new SingleTextureVoxel(0);
    }

    Voxel* voxel = chunks[index]->get_voxel(local_pos);
    if (!voxel) {
        return new SingleTextureVoxel(0);
    }

    return voxel;
}


int VoxelEngine::get_voxel_type(const Vector3i& global_pos) const {
    if (!is_in_world(global_pos)) {
        return 0;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    size_t index = get_chunk_index(chunk_pos);
    if (index < chunks.size() && chunks[index]) {
        
        if (SingleTextureVoxel* simple_voxel = dynamic_cast<SingleTextureVoxel*>(chunks[index]->get_voxel(local_pos))) return 1;
    }   if (MultiTextureVoxel* simple_voxel = dynamic_cast<MultiTextureVoxel*>(chunks[index]->get_voxel(local_pos))) return 2;

    return 0;
}

uint8_t VoxelEngine::get_voxel_texture(const godot::Vector3i& global_pos, int nr) const {

    if (!is_in_world(global_pos)) {
        return 0;
    }

    Vector3i chunk_pos(
        global_pos.x / CHUNK_SIZE,
        global_pos.y / CHUNK_SIZE,
        global_pos.z / CHUNK_SIZE
    );
    Vector3i local_pos(
        global_pos.x % CHUNK_SIZE,
        global_pos.y % CHUNK_SIZE,
        global_pos.z % CHUNK_SIZE
    );

    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    size_t index = get_chunk_index(chunk_pos);
    if (index < chunks.size() && chunks[index]) {

        if (SingleTextureVoxel* simple_voxel = dynamic_cast<SingleTextureVoxel*>(chunks[index]->get_voxel(local_pos))){
            return chunks[index]->get_voxel(local_pos)->get_texture_id(0);
        }
        if (MultiTextureVoxel* simple_voxel = dynamic_cast<MultiTextureVoxel*>(chunks[index]->get_voxel(local_pos))){
            return chunks[index]->get_voxel(local_pos)->get_texture_id(nr);
        }
    }

    return 0;
}


bool VoxelEngine::is_in_world(const Vector3i& pos) const {
    return pos.x >= 0 && pos.x < WORLD_SIZE_X &&
           pos.y >= 0 && pos.y < WORLD_SIZE_Y &&
           pos.z >= 0 && pos.z < WORLD_SIZE_Z;
}

void VoxelEngine::update_neighbor_chunks(const Vector3i& chunk_pos, const Vector3i& local_pos) {
    Vector3i neighbor_pos;
    size_t index;

    // Check neighbor in the negative x direction
    if (local_pos.x == 0) {
        neighbor_pos = chunk_pos + Vector3i(-1, 0, 0);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }

    // Check neighbor in the positive x direction
    if (local_pos.x == CHUNK_SIZE - 1) {
        neighbor_pos = chunk_pos + Vector3i(1, 0, 0);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }

    // Check neighbor in the negative y direction
    if (local_pos.y == 0) {
        neighbor_pos = chunk_pos + Vector3i(0, -1, 0);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }

    // Check neighbor in the positive y direction
    if (local_pos.y == CHUNK_SIZE - 1) {
        neighbor_pos = chunk_pos + Vector3i(0, 1, 0);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }

    // Check neighbor in the negative z direction
    if (local_pos.z == 0) {
        neighbor_pos = chunk_pos + Vector3i(0, 0, -1);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }

    // Check neighbor in the positive z direction
    if (local_pos.z == CHUNK_SIZE - 1) {
        neighbor_pos = chunk_pos + Vector3i(0, 0, 1);
        index = get_chunk_index(neighbor_pos);
        if (index < chunks.size() && chunks[index]) {
            chunks[index]->generate_mesh();
        }
    }
}


Ref<Texture2D> VoxelEngine::get_tilemap() const {
    return tilemap;
}

godot::Vector3i VoxelEngine::identify_voxel() const {
    if(!camera) return Vector3i(-1, -1, -1);

    // Ray Setup
    godot::Viewport* viewport = camera->get_viewport();
    godot::Vector2 mouse_pos = viewport->get_mouse_position();
    godot::Vector3 ray_origin = camera->project_ray_origin(mouse_pos);
    godot::Vector3 ray_normal = camera->project_ray_normal(mouse_pos);

    // Ray Offset
    ray_origin += ray_normal * 0.05f;

    // Initial Voxel Position
    godot::Vector3i voxel_pos(
        static_cast<int>(std::floor(ray_origin.x)),
        static_cast<int>(std::floor(ray_origin.y)),
        static_cast<int>(std::floor(ray_origin.z))
    );

    // Step Direction
    godot::Vector3i step(
        (ray_normal.x > 0) ? 1 : (ray_normal.x < 0 ? -1 : 0),
        (ray_normal.y > 0) ? 1 : (ray_normal.y < 0 ? -1 : 0),
        (ray_normal.z > 0) ? 1 : (ray_normal.z < 0 ? -1 : 0)
    );

    // T Delta Calculations
    godot::Vector3 t_delta(
        (ray_normal.x != 0) ? 1.0f / std::abs(ray_normal.x) : FLT_MAX,
        (ray_normal.y != 0) ? 1.0f / std::abs(ray_normal.y) : FLT_MAX,
        (ray_normal.z != 0) ? 1.0f / std::abs(ray_normal.z) : FLT_MAX
    );

    // T Max Initialization
    godot::Vector3 t_max(
        (ray_normal.x != 0) ? ((voxel_pos.x + (ray_normal.x > 0 ? 1 : 0)) - ray_origin.x) / ray_normal.x : FLT_MAX,
        (ray_normal.y != 0) ? ((voxel_pos.y + (ray_normal.y > 0 ? 1 : 0)) - ray_origin.y) / ray_normal.y : FLT_MAX,
        (ray_normal.z != 0) ? ((voxel_pos.z + (ray_normal.z > 0 ? 1 : 0)) - ray_origin.z) / ray_normal.z : FLT_MAX
    );

    constexpr int MAX_STEPS = 1000;
    for(int i = 0; i < MAX_STEPS; ++i) {
        if(!is_in_world(voxel_pos)) break;

        Voxel* voxel = get_voxel(voxel_pos);
        if(voxel && voxel->is_active()) {
            // Backtrack check
            godot::Vector3i prev_pos = voxel_pos - step;
            if(is_in_world(prev_pos)) {
                Voxel* prev_voxel = get_voxel(prev_pos);
                if(prev_voxel && prev_voxel->is_active()) {
                    return voxel_pos-step;
                }
            }
            return voxel_pos;
        }

        // Traversal logic
        if(t_max.x < t_max.y && t_max.x < t_max.z) {
            voxel_pos.x += step.x;
            t_max.x += t_delta.x;
        } 
        else if(t_max.y < t_max.z) {
            voxel_pos.y += step.y;
            t_max.y += t_delta.y;
        } 
        else {
            voxel_pos.z += step.z;
            t_max.z += t_delta.z;
        }
    }
    return voxel_pos;
}

/*
void VoxelEngine::fill_voxel_region(const godot::Vector3i& start, const godot::Vector3i& end, int voxel_type, uint8_t texture_id, const godot::PackedByteArray& multi_texture_ids) {
    // Validierung der Parameter
    if (voxel_type != 1 && voxel_type != 2) {
        UtilityFunctions::printerr("FillVoxelRegion: Invalid voxel_type! Must be 1 (SingleTextureVoxel) or 2 (MultiTextureVoxel).");
        return;
    }
    if (voxel_type == 2 && multi_texture_ids.size() != 6) {
        UtilityFunctions::printerr("FillVoxelRegion: MultiTextureVoxel requires exactly 6 texture IDs!");
        return;
    }

    // Normalisiere Start- und Endpunkte (start <= end)
    Vector3i min_pos(
        std::min(start.x, end.x),
        std::min(start.y, end.y),
        std::min(start.z, end.z)
    );
    Vector3i max_pos(
        std::max(start.x, end.x),
        std::max(start.y, end.y),
        std::max(start.z, end.z)
    );

    // Prüfe, ob der Bereich innerhalb der Weltgrenzen liegt
    if (!is_in_world(min_pos) || !is_in_world(max_pos)) {
        UtilityFunctions::printerr("FillVoxelRegion: Region is outside world bounds!");
        return;
    }

    // Berechne die Chunk-Positionen für Start- und Endpunkte
    Vector3i start_chunk(
        min_pos.x / CHUNK_SIZE,
        min_pos.y / CHUNK_SIZE,
        min_pos.z / CHUNK_SIZE
    );
    Vector3i end_chunk(
        max_pos.x / CHUNK_SIZE,
        max_pos.y / CHUNK_SIZE,
        max_pos.z / CHUNK_SIZE
    );

    // Prüfe, ob der Bereich vollständig in einem Chunk liegt
    if (start_chunk == end_chunk) {
        // Bereich liegt in einem einzigen Chunk
        size_t chunk_index = get_chunk_index(start_chunk);
        if (!chunks[chunk_index]) {
            chunks[chunk_index] = memnew(Chunk(this, start_chunk));
            parent->add_child(chunks[chunk_index]);
            chunks[chunk_index]->set_position(start_chunk * CHUNK_SIZE);
            chunks[chunk_index]->init();
        }

        // Lokale Koordinaten innerhalb des Chunks
        Vector3i local_start(
            min_pos.x % CHUNK_SIZE,
            min_pos.y % CHUNK_SIZE,
            min_pos.z % CHUNK_SIZE
        );
        Vector3i local_end(
            max_pos.x % CHUNK_SIZE,
            max_pos.y % CHUNK_SIZE,
            max_pos.z % CHUNK_SIZE
        );
        if (local_start.x < 0) local_start.x += CHUNK_SIZE;
        if (local_start.y < 0) local_start.y += CHUNK_SIZE;
        if (local_start.z < 0) local_start.z += CHUNK_SIZE;
        if (local_end.x < 0) local_end.x += CHUNK_SIZE;
        if (local_end.y < 0) local_end.y += CHUNK_SIZE;
        if (local_end.z < 0) local_end.z += CHUNK_SIZE;

        // Fülle den Bereich im Chunk
        for (int x = local_start.x; x <= local_end.x; ++x) {
            for (int y = local_start.y; y <= local_end.y; ++y) {
                for (int z = local_start.z; z <= local_end.z; ++z) {
                    Voxel* voxel = nullptr;
                    if (voxel_type == 1) {
                        voxel = new SingleTextureVoxel(texture_id);
                    } else if (voxel_type == 2) {
                        voxel = new MultiTextureVoxel(
                            multi_texture_ids[0], multi_texture_ids[1], multi_texture_ids[2],
                            multi_texture_ids[3], multi_texture_ids[4], multi_texture_ids[5]
                        );
                    }
                    chunks[chunk_index]->set_voxel(Vector3i(x, y, z), voxel);
                }
            }
        }
        chunks[chunk_index]->update_mesh();
        update_neighbor_chunks(start_chunk, local_start);
        update_neighbor_chunks(start_chunk, local_end);
    } else {
        // Bereich überspannt mehrere Chunks
        int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
        int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
        int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;

        for (int cx = start_chunk.x; cx <= end_chunk.x; ++cx) {
            for (int cy = start_chunk.y; cy <= end_chunk.y; ++cy) {
                for (int cz = start_chunk.z; cz <= end_chunk.z; ++cz) {
                    Vector3i chunk_pos(cx, cy, cz);
                    size_t chunk_index = get_chunk_index(chunk_pos);

                    // Erstelle Chunk, wenn nicht vorhanden
                    if (!chunks[chunk_index]) {
                        chunks[chunk_index] = memnew(Chunk(this, chunk_pos));
                        parent->add_child(chunks[chunk_index]);
                        chunks[chunk_index]->set_position(chunk_pos * CHUNK_SIZE);
                        chunks[chunk_index]->init();
                    }

                    // Bestimme lokale Grenzen für diesen Chunk
                    Vector3i local_start(
                        cx == start_chunk.x ? min_pos.x % CHUNK_SIZE : 0,
                        cy == start_chunk.y ? min_pos.y % CHUNK_SIZE : 0,
                        cz == start_chunk.z ? min_pos.z % CHUNK_SIZE : 0
                    );
                    Vector3i local_end(
                        cx == end_chunk.x ? max_pos.x % CHUNK_SIZE : CHUNK_SIZE - 1,
                        cy == end_chunk.y ? max_pos.y % CHUNK_SIZE : CHUNK_SIZE - 1,
                        cz == end_chunk.z ? max_pos.z % CHUNK_SIZE : CHUNK_SIZE - 1
                    );
                    if (local_start.x < 0) local_start.x += CHUNK_SIZE;
                    if (local_start.y < 0) local_start.y += CHUNK_SIZE;
                    if (local_start.z < 0) local_start.z += CHUNK_SIZE;
                    if (local_end.x < 0) local_end.x += CHUNK_SIZE;
                    if (local_end.y < 0) local_end.y += CHUNK_SIZE;
                    if (local_end.z < 0) local_end.z += CHUNK_SIZE;

                    // Fülle den Bereich im aktuellen Chunk
                    for (int x = local_start.x; x <= local_end.x; ++x) {
                        for (int y = local_start.y; y <= local_end.y; ++y) {
                            for (int z = local_start.z; z <= local_end.z; ++z) {
                                Voxel* voxel = nullptr;
                                if (voxel_type == 1) {
                                    voxel = new SingleTextureVoxel(texture_id);
                                } else if (voxel_type == 2) {
                                    voxel = new MultiTextureVoxel(
                                        multi_texture_ids[0], multi_texture_ids[1], multi_texture_ids[2],
                                        multi_texture_ids[3], multi_texture_ids[4], multi_texture_ids[5]
                                    );
                                }
                                chunks[chunk_index]->set_voxel(Vector3i(x, y, z), voxel);
                            }
                        }
                    }
                    chunks[chunk_index]->update_mesh();
                    update_neighbor_chunks(chunk_pos, local_start);
                    update_neighbor_chunks(chunk_pos, local_end);
                }
            }
        }
    }
}
*/
void VoxelEngine::fill_voxel_region(const godot::Vector3i& start, const godot::Vector3i& end, int voxel_type, uint8_t texture_id, const godot::PackedByteArray& multi_texture_ids) {
    // Validierung der Parameter
    if (voxel_type != 1 && voxel_type != 2) {
        UtilityFunctions::printerr("FillVoxelRegion: Invalid voxel_type! Must be 1 (SingleTextureVoxel) or 2 (MultiTextureVoxel).");
        return;
    }
    if (voxel_type == 2 && multi_texture_ids.size() != 6) {
        UtilityFunctions::printerr("FillVoxelRegion: MultiTextureVoxel requires exactly 6 texture IDs!");
        return;
    }

    // Normalisiere Start- und Endpunkte (start <= end)
    Vector3i min_pos(
        std::min(start.x, end.x),
        std::min(start.y, end.y),
        std::min(start.z, end.z)
    );
    Vector3i max_pos(
        std::max(start.x, end.x),
        std::max(start.y, end.y),
        std::max(start.z, end.z)
    );

    // Prüfe, ob der Bereich innerhalb der Weltgrenzen liegt
    if (!is_in_world(min_pos) || !is_in_world(max_pos)) {
        UtilityFunctions::printerr("FillVoxelRegion: Region is outside world bounds!");
        return;
    }

    // Berechne die Chunk-Positionen für Start- und Endpunkte
    Vector3i start_chunk(
        min_pos.x / CHUNK_SIZE,
        min_pos.y / CHUNK_SIZE,
        min_pos.z / CHUNK_SIZE
    );
    Vector3i end_chunk(
        max_pos.x / CHUNK_SIZE,
        max_pos.y / CHUNK_SIZE,
        max_pos.z / CHUNK_SIZE
    );

    // Prüfe, ob der Bereich vollständig in einem Chunk liegt
    if (start_chunk == end_chunk) {
        // Bereich liegt in einem einzigen Chunk
        size_t chunk_index = get_chunk_index(start_chunk);
        if (!chunks[chunk_index]) {
            chunks[chunk_index] = memnew(Chunk(this, start_chunk));
            parent->add_child(chunks[chunk_index]);
            chunks[chunk_index]->set_position(start_chunk * CHUNK_SIZE);
            chunks[chunk_index]->init();
        }

        // Lokale Koordinaten innerhalb des Chunks
        Vector3i local_start(
            min_pos.x % CHUNK_SIZE,
            min_pos.y % CHUNK_SIZE,
            min_pos.z % CHUNK_SIZE
        );
        Vector3i local_end(
            max_pos.x % CHUNK_SIZE,
            max_pos.y % CHUNK_SIZE,
            max_pos.z % CHUNK_SIZE
        );
        if (local_start.x < 0) local_start.x += CHUNK_SIZE;
        if (local_start.y < 0) local_start.y += CHUNK_SIZE;
        if (local_start.z < 0) local_start.z += CHUNK_SIZE;
        if (local_end.x < 0) local_end.x += CHUNK_SIZE;
        if (local_end.y < 0) local_end.y += CHUNK_SIZE;
        if (local_end.z < 0) local_end.z += CHUNK_SIZE;

        // Fülle den Bereich im Chunk
        for (int x = local_start.x; x <= local_end.x; ++x) {
            for (int y = local_start.y; y <= local_end.y; ++y) {
                for (int z = local_start.z; z <= local_end.z; ++z) {
                    Voxel* voxel = nullptr;
                    if (voxel_type == 1) {
                        voxel = new SingleTextureVoxel(texture_id);
                    } else if (voxel_type == 2) {
                        voxel = new MultiTextureVoxel(
                            multi_texture_ids[0], multi_texture_ids[1], multi_texture_ids[2],
                            multi_texture_ids[3], multi_texture_ids[4], multi_texture_ids[5]
                        );
                    }
                    chunks[chunk_index]->set_voxel(Vector3i(x, y, z), voxel);
                }
            }
        }
        chunks[chunk_index]->update_mesh();
        update_neighbor_chunks(start_chunk, local_start);
        update_neighbor_chunks(start_chunk, local_end);
    } else {
        // Bereich überspannt mehrere Chunks
        int chunks_x = (WORLD_SIZE_X + CHUNK_SIZE - 1) / CHUNK_SIZE;
        int chunks_y = (WORLD_SIZE_Y + CHUNK_SIZE - 1) / CHUNK_SIZE;
        int chunks_z = (WORLD_SIZE_Z + CHUNK_SIZE - 1) / CHUNK_SIZE;

        for (int cx = start_chunk.x; cx <= end_chunk.x; ++cx) {
            for (int cy = start_chunk.y; cy <= end_chunk.y; ++cy) {
                for (int cz = start_chunk.z; cz <= end_chunk.z; ++cz) {
                    Vector3i chunk_pos(cx, cy, cz);
                    size_t chunk_index = get_chunk_index(chunk_pos);

                    // Erstelle Chunk, wenn nicht vorhanden
                    if (!chunks[chunk_index]) {
                        chunks[chunk_index] = memnew(Chunk(this, chunk_pos));
                        parent->add_child(chunks[chunk_index]);
                        chunks[chunk_index]->set_position(chunk_pos * CHUNK_SIZE);
                        chunks[chunk_index]->init();
                    }

                    // Bestimme lokale Grenzen für diesen Chunk
                    Vector3i local_start(
                        cx == start_chunk.x ? min_pos.x % CHUNK_SIZE : 0,
                        cy == start_chunk.y ? min_pos.y % CHUNK_SIZE : 0,
                        cz == start_chunk.z ? min_pos.z % CHUNK_SIZE : 0
                    );
                    Vector3i local_end(
                        cx == end_chunk.x ? max_pos.x % CHUNK_SIZE : CHUNK_SIZE - 1,
                        cy == end_chunk.y ? max_pos.y % CHUNK_SIZE : CHUNK_SIZE - 1,
                        cz == end_chunk.z ? max_pos.z % CHUNK_SIZE : CHUNK_SIZE - 1
                    );
                    if (local_start.x < 0) local_start.x += CHUNK_SIZE;
                    if (local_start.y < 0) local_start.y += CHUNK_SIZE;
                    if (local_start.z < 0) local_start.z += CHUNK_SIZE;
                    if (local_end.x < 0) local_end.x += CHUNK_SIZE;
                    if (local_end.y < 0) local_end.y += CHUNK_SIZE;
                    if (local_end.z < 0) local_end.z += CHUNK_SIZE;

                    // Fülle den Bereich im aktuellen Chunk
                    for (int x = local_start.x; x <= local_end.x; ++x) {
                        for (int y = local_start.y; y <= local_end.y; ++y) {
                            for (int z = local_start.z; z <= local_end.z; ++z) {
                                Voxel* voxel = nullptr;
                                if (voxel_type == 1) {
                                    voxel = new SingleTextureVoxel(texture_id);
                                } else if (voxel_type == 2) {
                                    voxel = new MultiTextureVoxel(
                                        multi_texture_ids[0], multi_texture_ids[1], multi_texture_ids[2],
                                        multi_texture_ids[3], multi_texture_ids[4], multi_texture_ids[5]
                                    );
                                }
                                chunks[chunk_index]->set_voxel(Vector3i(x, y, z), voxel);
                            }
                        }
                    }
                    chunks[chunk_index]->update_mesh();
                    update_neighbor_chunks(chunk_pos, local_start);
                    update_neighbor_chunks(chunk_pos, local_end);
                }
            }
        }
    }
}

void VoxelEngine::_bind_methods() {
    // Bind methods if needed
    ClassDB::bind_method(D_METHOD("InitVE", "size_x", "size_y", "size_z", "tex", "parentnode"), &VoxelEngine::InitVE);
    ClassDB::bind_method(D_METHOD("set_voxel_singletexture", "global_pos", "textureid"), 
        &VoxelEngine::set_voxel_singletexture);
    ClassDB::bind_method(D_METHOD("set_voxel_multitexture", "global_pos", 
        "right", "left", "up", "down", "forward", "back"), 
        &VoxelEngine::set_voxel_multitexture);
    ClassDB::bind_method(D_METHOD("update_world"), &VoxelEngine::update_world);
    ClassDB::bind_method(D_METHOD("delete_voxel", "global_pos"), &VoxelEngine::delete_voxel);
    ClassDB::bind_method(D_METHOD("get_voxel_type", "global_pos"), &VoxelEngine::get_voxel_type);
    ClassDB::bind_method(D_METHOD("get_voxel_texture", "global_pos", "nr"), &VoxelEngine::get_voxel_texture);
    ClassDB::bind_method(D_METHOD("identify_voxel"), &VoxelEngine::identify_voxel);
    ClassDB::bind_method(D_METHOD("fill_voxel_region", "start", "end", "voxel_type", "texture_id", "multi_texture_ids"), 
    &VoxelEngine::fill_voxel_region, DEFVAL(godot::PackedByteArray()));
}

