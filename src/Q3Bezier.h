//
//  Q3Bezier.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 24.11.24.
//

#pragma once

struct tBSPFace;
struct tBSPVertex;

void tesselateBezierPatches(std::vector<tBSPFace>& faces, std::vector<tBSPVertex>& verts, std::vector<int>& indices, int level);
