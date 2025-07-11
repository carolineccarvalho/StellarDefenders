//
// Created by Lucas N. Ferreira on 29/05/25.
//

#pragma once

#include <vector>
#include <unordered_map>
#include "Math.h"
#include "Actors/Actor.h"
#include "Camera.hpp"

class SpatialHashing
{
public:
    SpatialHashing(int cellSize, int width, int height);
    ~SpatialHashing();

    void Insert(Actor *actor);
    void Remove(Actor *actor);
    void Reinsert(Actor *actor);

    std::vector<AABBColliderComponent *> QueryColliders(const glm::vec2& position, const int range = 1) const;

    std::vector<Actor*> Query(const glm::vec2& position, const int range = 1) const;
    std::vector<Actor*> QueryOnCamera(const Camera& camera,
                                                      const float screenWidth,
                                                      const float screenHeight,
                                                      const float extraRadius = 0.0f) const;
private:
    int mCellSize;
    int mWidth;
    int mHeight;

    std::vector<std::vector<std::vector<Actor*> >> mGrid; // 2D grid of colliders
    std::unordered_map<Actor*, glm::vec2> mPositions; // Maps collider to its position
    std::unordered_map<Actor*, std::pair<int, int>> mCellIndices; // Maps collider to its grid cell indices

};
