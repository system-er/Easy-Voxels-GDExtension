#include "chunk.hpp"
#include "voxelengine.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <limits> 

using namespace godot;


static const int edgeTable[256] = {
// http://paulbourke.net/geometry/polygonise/)
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

static const int triTable[256][16] = {
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

static const int edgeConnections[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

static const Vector3 cubeVertices[8] = {
    Vector3(0, 0, 0), // 0
    Vector3(1, 0, 0), // 1
    Vector3(1, 0, 1), // 2
    Vector3(0, 0, 1), // 3
    Vector3(0, 1, 0), // 4
    Vector3(1, 1, 0), // 5
    Vector3(1, 1, 1), // 6
    Vector3(0, 1, 1)  // 7
};



//Chunk::Chunk() :
Chunk::Chunk() : voxel_engine(nullptr), needs_mesh_update(false), mesh_instance(nullptr), sm3d(nullptr) {}

Chunk::Chunk(VoxelEngine* engine, const Vector3i& chunk_pos) : voxel_engine(engine), chunk_position(chunk_pos), needs_mesh_update(false) {

    //voxels.resize(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, nullptr);
    //initialize_voxels();
    if (!voxel_engine) {
        UtilityFunctions::printerr("Chunk constructor: voxelEngine is null!");
    }
    
    vLEFT = Vector3(-1, 0, 0);
    vRIGHT = Vector3(1, 0, 0);
    vUP = Vector3(0, 1, 0);
    vDOWN = Vector3(0, -1, 0);
    vFORWARD = Vector3(0, 0, -1);
    vBACK = Vector3(0, 0, 1);
}

Chunk::~Chunk() {

    for (auto& pair : voxels) {
        delete pair.second;
    }
}

void Chunk::initialize_voxels() {

}

void Chunk::init() {
    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    sm3d = memnew(StandardMaterial3D);
    sm3d->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, voxel_engine->get_tilemap());
    sm3d->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST);
    sm3d->set_cull_mode(godot::BaseMaterial3D::CullMode::CULL_DISABLED);
}


void Chunk::set_voxel(const Vector3i& local_pos, Voxel* voxel) {
    if (!is_in_bounds(local_pos)) {
        delete voxel;
        return;
    }

    auto it = voxels.find(local_pos);
    if (it != voxels.end()) {
        delete it->second;
        voxels.erase(it);
    }

    if (voxel && voxel->is_active()) {
        voxels[local_pos] = voxel;
        needs_mesh_update = true;
    } else {
        delete voxel;
    }
}


void Chunk::update_mesh() {
    if (needs_mesh_update) {
        generate_mesh();
        needs_mesh_update = false;
    }
}


Voxel* Chunk::get_voxel(const Vector3i& local_pos) const {
    //UtilityFunctions::print("GetVoxel: Accessing at ", local_pos);
    if (!is_in_bounds(local_pos)) {
        //UtilityFunctions::print("GetVoxel: Out of bounds, returning default voxel");
        return new SingleTextureVoxel(0, 0.0f);
    }
    auto it = voxels.find(local_pos);
    if (it != voxels.end() && it->second) {
        //UtilityFunctions::print("GetVoxel: Found voxel at ", local_pos);
        return it->second;
    }
    //UtilityFunctions::print("GetVoxel: No voxel found, returning default");
    return new SingleTextureVoxel(0, 0.0f);
}



void Chunk::generate_mesh() {
    UtilityFunctions::print("GenerateMesh: Starting for chunk ", chunk_position);
    if (!voxel_engine) {
        UtilityFunctions::printerr("GenerateMesh: Invalid voxel_engine");
        return;
    }
    if (!mesh_instance) {
        UtilityFunctions::printerr("GenerateMesh: Invalid mesh_instance");
        return;
    }

    Ref<SurfaceTool> surface_tool = Ref<SurfaceTool>(memnew(SurfaceTool));
    UtilityFunctions::print("GenerateMesh: SurfaceTool created");
    surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
    int triangle_count = 0;

    if (voxel_engine->get_mesh_mode() == MESH_CUBE) {
        UtilityFunctions::print("GenerateMesh: Using cube mode");
        for (const auto& pair : voxels) {
            Vector3i local_pos = pair.first;
            Voxel* voxel = pair.second;
            if (!voxel || !voxel->is_active()) continue;
            Vector3 pos(local_pos.x, local_pos.y, local_pos.z);
            triangle_count += add_cube(surface_tool, pos, voxel);
        }
    } else {
        UtilityFunctions::print("GenerateMesh: Using marching cubes mode");
        triangle_count = generate_marching_cubes_mesh(surface_tool);
    }

    UtilityFunctions::print("GenerateMesh: Committing mesh with ", triangle_count, " triangles");
    Ref<ArrayMesh> mesh = surface_tool->commit();
    if (!mesh.is_valid()) {
        UtilityFunctions::printerr("GenerateMesh: Failed to commit mesh");
    }
    mesh_instance->set_mesh(mesh);
    mesh_instance->set_material_override(sm3d);
    UtilityFunctions::print("GenerateMesh: Completed with ", triangle_count, " triangles");
}



float Chunk::get_density_at_global_pos(const godot::Vector3i& global_pos) const {
    godot::Vector3i local_pos = global_pos - chunk_position;

    if (local_pos.x >= 0 && local_pos.x < CHUNK_SIZE &&
        local_pos.y >= 0 && local_pos.y < CHUNK_SIZE &&
        local_pos.z >= 0 && local_pos.z < CHUNK_SIZE) {
        
        auto it = voxels.find(local_pos);
        if (it != voxels.end() && it->second) {
            return it->second->get_density();
        }
    }

    if (voxel_engine) {
        return voxel_engine->get_voxel_density(global_pos);
    }
    return 0.0f;
}


int Chunk::generate_marching_cubes_mesh(Ref<SurfaceTool> st) {
    if (!st.is_valid()) {
        UtilityFunctions::printerr("Marching Cubes: Invalid SurfaceTool");
        return 0;
    }
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    if (!voxel_engine) {
        UtilityFunctions::printerr("Marching Cubes: Invalid voxel_engine");
        return 0;
    }
    if (!mesh_instance) {
        UtilityFunctions::printerr("Marching Cubes: Invalid mesh_instance");
        return 0;
    }

    int triangle_count = 0;
    float iso_level = 0.5f;

    float density_grid[CHUNK_SIZE + 1][CHUNK_SIZE + 1][CHUNK_SIZE + 1];
    for (int x = 0; x <= CHUNK_SIZE; ++x) {
        for (int y = 0; y <= CHUNK_SIZE; ++y) {
            for (int z = 0; z <= CHUNK_SIZE; ++z) {
                Vector3i global_pos = chunk_position * CHUNK_SIZE + Vector3i(x, y, z);
                float density = get_density_at_global_pos(global_pos);
                if (!std::isfinite(density)) density = iso_level;
                density_grid[x][y][z] = density;
            }
        }
    }

    Ref<Texture2D> tilemap = voxel_engine->get_tilemap();
    int tiles_per_row = tilemap->get_width() / 34;
    float tile_size_u = 32.0f / tilemap->get_width();
    float tile_size_v = 32.0f / tilemap->get_height();
    float padding_u = 1.0f / tilemap->get_width();
    float padding_v = 1.0f / tilemap->get_height();
    float uv_offset = 0.5f / tilemap->get_width();

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                int cube_index = 0;
                Vector3 vert_normals[8];
                for (int i = 0; i < 8; ++i) {
                    int offset_x = static_cast<int>(cubeVertices[i].x);
                    int offset_y = static_cast<int>(cubeVertices[i].y);
                    int offset_z = static_cast<int>(cubeVertices[i].z);
                    if (density_grid[x + offset_x][y + offset_y][z + offset_z] > iso_level) {
                        cube_index |= 1 << i;
                    }
                    Vector3i global_pos = chunk_position * CHUNK_SIZE + Vector3i(x, y, z) + Vector3i(offset_x, offset_y, offset_z);
                    float dx = (get_density_at_global_pos(global_pos + Vector3i(1, 0, 0)) - 
                                get_density_at_global_pos(global_pos - Vector3i(1, 0, 0))) / 2.0f;
                    float dy = (get_density_at_global_pos(global_pos + Vector3i(0, 1, 0)) - 
                                get_density_at_global_pos(global_pos - Vector3i(0, 1, 0))) / 2.0f;
                    float dz = (get_density_at_global_pos(global_pos + Vector3i(0, 0, 1)) - 
                                get_density_at_global_pos(global_pos - Vector3i(0, 0, 1))) / 2.0f;
                    vert_normals[i] = Vector3(dx, dy, dz).normalized();
                }

                if (cube_index == 0 || cube_index == 255) continue;

                Vector3 vert_list[12];
                for (int i = 0; i < 12; ++i) {
                    if (edgeTable[cube_index] & (1 << i)) {
                        int v0 = edgeConnections[i][0];
                        int v1 = edgeConnections[i][1];
                        int v0_x = static_cast<int>(cubeVertices[v0].x);
                        int v0_y = static_cast<int>(cubeVertices[v0].y);
                        int v0_z = static_cast<int>(cubeVertices[v0].z);
                        int v1_x = static_cast<int>(cubeVertices[v1].x);
                        int v1_y = static_cast<int>(cubeVertices[v1].y);
                        int v1_z = static_cast<int>(cubeVertices[v1].z);
                        float density0 = density_grid[x + v0_x][y + v0_y][z + v0_z];
                        float density1 = density_grid[x + v1_x][y + v1_y][z + v1_z];
                        float t = (density1 - density0) > 1e-6f ? (iso_level - density0) / (density1 - density0) : 0.5f;
                        t = std::clamp(t, 0.0f, 1.0f);
                        vert_list[i] = cubeVertices[v0] + Vector3(x, y, z) + t * (cubeVertices[v1] - cubeVertices[v0]);
                    }
                }

                Voxel* voxel = get_voxel(Vector3i(x, y, z));
                if (!voxel) continue;

                for (int i = 0; triTable[cube_index][i] != -1; i += 3) {
                    int idx0 = triTable[cube_index][i];
                    int idx1 = triTable[cube_index][i + 1];
                    int idx2 = triTable[cube_index][i + 2];

                    Vector3 vertices[3] = { vert_list[idx0], vert_list[idx1], vert_list[idx2] };
                    Vector3 edge1 = vertices[1] - vertices[0];
                    Vector3 edge2 = vertices[2] - vertices[0];
                    Vector3 face_normal = edge1.cross(edge2).normalized();

                    if (edge1.cross(edge2).length() / 2.0f < 1e-4f) continue;


                    int texture_index = 0;
                    if (face_normal.dot(Vector3(1, 0, 0)) > 0.7f) texture_index = 0; // Right
                    else if (face_normal.dot(Vector3(-1, 0, 0)) > 0.7f) texture_index = 1; // Left
                    else if (face_normal.dot(Vector3(0, 1, 0)) > 0.7f) texture_index = 2; // Up
                    else if (face_normal.dot(Vector3(0, -1, 0)) > 0.7f) texture_index = 3; // Down
                    else if (face_normal.dot(Vector3(0, 0, 1)) > 0.7f) texture_index = 4; // Forward
                    else if (face_normal.dot(Vector3(0, 0, -1)) > 0.7f) texture_index = 5; // Back

                    uint8_t texture_id = 1;
                    if (SingleTextureVoxel* simple_voxel = dynamic_cast<SingleTextureVoxel*>(voxel)) {
                        texture_id = simple_voxel->texture_id - 1;
                    } else if (MultiTextureVoxel* multi_voxel = dynamic_cast<MultiTextureVoxel*>(voxel)) {
                        texture_id = multi_voxel->texture_ids[texture_index] - 1;
                    }

                    float tile_u = (texture_id % tiles_per_row) * 34.0f / tilemap->get_width() + padding_u;
                    float tile_v = (texture_id / tiles_per_row) * 34.0f / tilemap->get_height() + padding_v;


                    Vector2 uvs[3];
                    if (texture_index == 2) { // Up
                        uvs[0] = Vector2(tile_u, tile_v + tile_size_v - uv_offset); // Bottom-left
                        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-right
                        uvs[2] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-right
                    } else if (texture_index == 3) { // Down
                        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Bottom-left
                        uvs[1] = Vector2(tile_u, tile_v + uv_offset); // Bottom-right
                        uvs[2] = Vector2(tile_u, tile_v + tile_size_v - uv_offset); // Top-right
                    } else if (texture_index == 0) { // Right
                        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-left
                        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-left
                        uvs[2] = Vector2(tile_u, tile_v + uv_offset); // Top-right
                    } else if (texture_index == 1) { // Left
                        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-right
                        uvs[1] = Vector2(tile_u, tile_v + tile_size_v - uv_offset); // Bottom-left
                        uvs[2] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-right
                    } else if (texture_index == 4) { // Forward
                        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-right
                        uvs[1] = Vector2(tile_u, tile_v + tile_size_v - uv_offset); // Bottom-left
                        uvs[2] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-right
                    } else { // Back
                        uvs[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset); // Bottom-left
                        uvs[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-left
                        uvs[2] = Vector2(tile_u, tile_v + uv_offset); // Top-right
                    }

   
                    Vector3 tri_center = (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
                    Vector3i grid_pos = Vector3i(x, y, z) + Vector3i(
                        static_cast<int>(std::round(tri_center.x)),
                        static_cast<int>(std::round(tri_center.y)),
                        static_cast<int>(std::round(tri_center.z))
                    );
                    Vector3i global_pos = chunk_position * CHUNK_SIZE + grid_pos;
                    float dx = (get_density_at_global_pos(global_pos + Vector3i(1, 0, 0)) - 
                                get_density_at_global_pos(global_pos - Vector3i(1, 0, 0))) / 2.0f;
                    float dy = (get_density_at_global_pos(global_pos + Vector3i(0, 1, 0)) - 
                                get_density_at_global_pos(global_pos - Vector3i(0, 1, 0))) / 2.0f;
                    float dz = (get_density_at_global_pos(global_pos + Vector3i(0, 0, 1)) - 
                                get_density_at_global_pos(global_pos - Vector3i(0, 0, 1))) / 2.0f;
                    Vector3 grad_normal = Vector3(dx, dy, dz).normalized();

                    if (face_normal.dot(grad_normal) < 0) {
                        std::swap(idx1, idx2);
                        vertices[1] = vert_list[idx1];
                        vertices[2] = vert_list[idx2];
                        face_normal = -face_normal;
                        std::swap(uvs[1], uvs[2]);
                    }

                    Vector3 normals[3];
                    for (int j = 0; j < 3; ++j) {
                        int edge_idx = triTable[cube_index][i + j];
                        int v0 = edgeConnections[edge_idx][0];
                        int v1 = edgeConnections[edge_idx][1];
                        int v0_x = static_cast<int>(cubeVertices[v0].x);
                        int v0_y = static_cast<int>(cubeVertices[v0].y);
                        int v0_z = static_cast<int>(cubeVertices[v0].z);
                        int v1_x = static_cast<int>(cubeVertices[v1].x);
                        int v1_y = static_cast<int>(cubeVertices[v1].y);
                        int v1_z = static_cast<int>(cubeVertices[v1].z);
                        float density0 = density_grid[x + v0_x][y + v0_y][z + v0_z];
                        float density1 = density_grid[x + v1_x][y + v1_y][z + v1_z];
                        float t = (density1 - density0) > 1e-6f ? (iso_level - density0) / (density1 - density0) : 0.5f;
                        t = std::clamp(t, 0.0f, 1.0f);
                        normals[j] = (vert_normals[v0] + t * (vert_normals[v1] - vert_normals[v0])).normalized();
                    }

                    for (int j = 0; j < 3; ++j) {
                        st->set_normal(normals[j]);
                        st->set_uv(uvs[j]);
                        st->add_vertex(vertices[j]);
                    }
                    triangle_count++;

   
                    if (i + 3 < 16 && triTable[cube_index][i + 3] != -1) {
                        int idx3 = triTable[cube_index][i + 3];
                        int idx4 = triTable[cube_index][i + 4];
                        int idx5 = triTable[cube_index][i + 5];

                        Vector3 vertices2[3] = { vert_list[idx3], vert_list[idx4], vert_list[idx5] };
                        Vector3 edge1_2 = vertices2[1] - vertices2[0];
                        Vector3 edge2_2 = vertices2[2] - vertices2[0];
                        Vector3 face_normal2 = edge1_2.cross(edge2_2).normalized();

                        if (edge1_2.cross(edge2_2).length() / 2.0f < 1e-4f) {
                            i += 3;
                            continue;
                        }


                        if (face_normal2.dot(face_normal) > 0.9f) {
                            Vector2 uvs2[3];
                            if (texture_index == 2) { // Up
                                uvs2[0] = Vector2(tile_u, tile_v + tile_size_v - uv_offset); // Bottom-left
                                uvs2[1] = Vector2(tile_u, tile_v + uv_offset); // Top-left
                                uvs2[2] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset); // Top-right
                            } else if (texture_index == 3) { // Down
                                uvs2[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);
                                uvs2[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
                                uvs2[2] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);
                            } else if (texture_index == 0) { // Right
                                uvs2[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
                                uvs2[1] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);
                                uvs2[2] = Vector2(tile_u, tile_v + uv_offset);
                            } else if (texture_index == 1) { // Left
                                uvs2[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
                                uvs2[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);
                                uvs2[2] = Vector2(tile_u, tile_v + uv_offset);
                            } else if (texture_index == 4) { // Forward
                                uvs2[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
                                uvs2[1] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + uv_offset);
                                uvs2[2] = Vector2(tile_u, tile_v + uv_offset);
                            } else { // Back
                                uvs2[0] = Vector2(tile_u + tile_size_u - uv_offset, tile_v + tile_size_v - uv_offset);
                                uvs2[1] = Vector2(tile_u, tile_v + tile_size_v - uv_offset);
                                uvs2[2] = Vector2(tile_u, tile_v + uv_offset);
                            }

                            if (face_normal2.dot(grad_normal) < 0) {
                                std::swap(idx4, idx5);
                                vertices2[1] = vert_list[idx4];
                                vertices2[2] = vert_list[idx5];
                                face_normal2 = -face_normal2;
                                std::swap(uvs2[1], uvs2[2]);
                            }

                            Vector3 normals2[3];
                            for (int j = 0; j < 3; ++j) {
                                int edge_idx = triTable[cube_index][i + 3 + j];
                                int v0 = edgeConnections[edge_idx][0];
                                int v1 = edgeConnections[edge_idx][1];
                                int v0_x = static_cast<int>(cubeVertices[v0].x);
                                int v0_y = static_cast<int>(cubeVertices[v0].y);
                                int v0_z = static_cast<int>(cubeVertices[v0].z);
                                int v1_x = static_cast<int>(cubeVertices[v1].x);
                                int v1_y = static_cast<int>(cubeVertices[v1].y);
                                int v1_z = static_cast<int>(cubeVertices[v1].z);
                                float density0 = density_grid[x + v0_x][y + v0_y][z + v0_z];
                                float density1 = density_grid[x + v1_x][y + v1_y][z + v1_z];
                                float t = (density1 - density0) > 1e-6f ? (iso_level - density0) / (density1 - density0) : 0.5f;
                                t = std::clamp(t, 0.0f, 1.0f);
                                normals2[j] = (vert_normals[v0] + t * (vert_normals[v1] - vert_normals[v0])).normalized();
                            }

                            for (int j = 0; j < 3; ++j) {
                                st->set_normal(normals2[j]);
                                st->set_uv(uvs2[j]);
                                st->add_vertex(vertices2[j]);
                            }
                            triangle_count++;
                            i += 3;
                        }
                    }
                }
            }
        }
    }

    UtilityFunctions::print("Marching Cubes: Completed, triangles: ", triangle_count);
    return triangle_count;
}

    
bool Chunk::is_in_bounds(const Vector3i& pos) const {
    return pos.x >= 0 && pos.x < CHUNK_SIZE &&
           pos.y >= 0 && pos.y < CHUNK_SIZE &&
           pos.z >= 0 && pos.z < CHUNK_SIZE;
}


bool Chunk::is_voxel_active(const Vector3i& pos) const {
    if (!is_in_bounds(pos)) {
        return false;
    }
    auto it = voxels.find(pos);
    return it != voxels.end() && it->second->is_active();
}


bool Chunk::is_neighbor_active(const Vector3i& neighbor_pos) const {
    if (is_in_bounds(neighbor_pos)) {
        auto it = voxels.find(neighbor_pos);
        return it != voxels.end() && it->second->is_active();
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