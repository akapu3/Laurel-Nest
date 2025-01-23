// internal
#include <iostream>
#include "physics_system.hpp"
#include "ecs.hpp"
#include <limits>
#include <thread>
#include <mutex>

#include "world_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity

void PhysicsSystem::setRoom(Entity newRoom) {
    currentRoom = newRoom;
}

void PhysicsSystem::setPlayer(const Entity& newPlayer) {
    player = newPlayer;
}

vec2 get_bounding_box(const Motion& motion)
{
    // abs is to avoid negative scale due to the facing direction.
    if (motion.boundingBox) {
        return *motion.boundingBox;
    }
    return { abs(motion.scale.x), abs(motion.scale.y) };
}

bool PhysicsSystem::checkForCollision(Entity e1, Entity e2, vec2& direction, vec2& overlap) {
    Motion motion1 = registry.motions.get(e1);
    Motion motion2 = registry.motions.get(e2);
    vec2 box1 = get_bounding_box(motion1);
    vec2 box2 = get_bounding_box(motion2);

    vec2 half_size1;
    vec2 half_size2;

    if(registry.bosses.has(e1)){
        if(registry.healths.get(e1).current_health > 0){
            half_size1 = (box1/2.f);
            half_size1.y += 200.f;
            half_size2 = box2 / 2.f;
        }else{
            half_size1 = box1 / 2.f;
            half_size2 = box2 / 2.f;
        }
    }else if(registry.bosses.has(e2)){
        if(registry.healths.get(e2).current_health > 0) {
            half_size1 = (box1 / 2.f);
            half_size2 = box2 / 2.f;
            half_size2.y += 200.f;
        }else{
            half_size1 = box1 / 2.f;
            half_size2 = box2 / 2.f;
        }
    }else{
        half_size1 = box1 / 2.f;
        half_size2 = box2 / 2.f;
    }

    vec2 dp = motion1.position - motion2.position;

    float overlapX = half_size1.x + half_size2.x - abs(dp.x);
    float overlapY = half_size1.y + half_size2.y - abs(dp.y);

    if (overlapX > 0 && overlapY > 0) {
        vec2 collisionDirection;
        if (overlapX < overlapY) {
            collisionDirection = vec2((dp.x > 0) ? 1 : -1, 0);
        }
        else {
            collisionDirection = vec2(0, (dp.y > 0) ? 1 : -1);
        }

        direction = collisionDirection;
        overlap = vec2(overlapX, overlapY);

        return true;
    }

    return false;
}

void projectOntoAxis(const std::vector<vec2>& points, const vec2& axis, float& min, float& max) {
    min = max = dot(points[0], axis);
    for (const auto& p : points) {
        float projection = dot(p, axis);
        if (projection < min) {
            min = projection;
        }
        else if (projection > max) {
            max = projection;
        }
    }
}

// uses separating axis theorem
bool playerMeshCollide(Entity player, Entity other, vec2& direction, vec2& overlap) {
    auto& motion = registry.motions.get(other);
    auto& motion1 = registry.motions.get(player);
    const Mesh& mesh = registry.playerMeshes.get(player).stateMeshes[PlayerState::WALKING];

    // Transform player's mesh vertices to world space
    Transform trans;
    trans.translate(motion1.position).rotate(motion1.angle).scale({WALKING_BB_WIDTH, WALKING_BB_HEIGHT});

    std::vector<vec2> transformedVertices;
    transformedVertices.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices) {
        vec3 transformed = trans.mat * vec3(v.position.x, v.position.y, 1.0f);
        transformedVertices.emplace_back(transformed.x, transformed.y);
    }

    vec2 half_size = get_bounding_box(motion) / 2.0f;
    vec2 minbb = motion.position - half_size;
    vec2 maxbb = motion.position + half_size;

    std::vector<vec2> axes;

    for (const auto& edge : mesh.edges) {
        vec2 p1 = transformedVertices[edge.first];
        vec2 p2 = transformedVertices[edge.second];

        // edge vector and its normal
        vec2 edgeVec = p2 - p1;
        vec2 normal = vec2(-edgeVec.y, edgeVec.x);
        if (glm::length(normal) > 0.0001f) {
            axes.push_back(normalize(normal));
        }
    }

    axes.emplace_back(1.0f, 0.0f);
    axes.emplace_back(0.0f, 1.0f);

    std::vector<vec2> aabbCorners = {
        {minbb.x, minbb.y},
        {maxbb.x, minbb.y},
        {maxbb.x, maxbb.y},
        {minbb.x, maxbb.y}
    };

    // SAT test
    float minOverlap = FLT_MAX;
    vec2 minOverlapAxis(0.0f, 0.0f);
    bool collisionDetected = true;

    for (const auto& axis : axes) {
        float minA, maxA;
        float minB, maxB;

        projectOntoAxis(transformedVertices, axis, minA, maxA);
        projectOntoAxis(aabbCorners, axis, minB, maxB);

        float axisOverlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (axisOverlap <= 0) {
            collisionDetected = false;
            break; // Separating axis found, no collision
        }
        else {
            // Keep the smallest overlap
            if (axisOverlap < minOverlap) {
                minOverlap = axisOverlap;
                minOverlapAxis = axis;
                vec2 d = motion.position - motion1.position;
                if (dot(d, axis) < 0) {
                    minOverlapAxis = -axis;
                }
            }
        }
    }

    if (collisionDetected) {
        if (fabs(minOverlapAxis.x) > fabs(minOverlapAxis.y)) {
            direction = vec2((minOverlapAxis.x > 0) ? 1.0f : -1.0f, 0.0f);
        }
        else {
            if (motion1.position.y < motion.position.y) {
                direction = vec2(0.0f, 1.0f);
            }
            else {
                direction = vec2(0.0f, -1.0f);
            }
        }
        overlap = direction * minOverlap;
        return true;
    }

    return false;
}

void PhysicsSystem::step(float elapsed_ms) {
    float step_seconds = elapsed_ms / 1000.f;

    // Only load entities that are already in this room.
    if (!registry.rooms.has(currentRoom)) {
        return;
    }
    Room& room = registry.rooms.get(currentRoom);
    std::vector<Entity> roomEntities{ player };
    for (const auto& entity : registry.motions.entities) {
        if (room.has(entity)) {
            roomEntities.push_back(entity);
        }
    }
    size_t numEntities = roomEntities.size();
    size_t numThreads = threadPool.getNumThreads();
    size_t batchSize = (numEntities + numThreads - 1) / numThreads;
    std::mutex mutex;
    std::vector<std::tuple<Entity, Entity, vec2, vec2>> collisions;

    for (size_t batch = 0; batch < numThreads; batch++) {
        size_t start = batch * batchSize;
        size_t end = std::min(start + batchSize, numEntities);

        threadPool.enqueue([&, start, end]() {
            std::vector<std::tuple<Entity, Entity, vec2, vec2>> localCollisions;
            for (size_t i = start; i < end; i++) {

                Entity entity_i = roomEntities[i];

                // Compare each entity with all other entities (i, j) pairs only once
                for (size_t j = i + 1; j < numEntities; j++) {
                    Entity entity_j = roomEntities[j];

                    vec2 direction;
                    vec2 overlap;

                    if (checkForCollision(entity_i, entity_j, direction, overlap)) {
                        // TODO for Kuter: there is an even better optimization, only loop the room entity list

                            // Mesh Collision for player
                            if (registry.players.has(entity_i)) {
                                vec2 direction2;
                                vec2 overlap2;
                                if (playerMeshCollide(entity_i, entity_j, direction2, overlap2)) {
                                    localCollisions.push_back({ entity_i, entity_j, direction2, overlap2 });
                                    localCollisions.push_back({ entity_j, entity_i, -direction2, overlap2 });
                                }
                            }
                            else {
                                // Non-player collision events directly with direction and overlap
                                localCollisions.push_back({ entity_i, entity_j, -direction, overlap });
                                localCollisions.push_back({ entity_j, entity_i, direction, overlap });
                            }
                        
                    }
                }
            }
            {
                // Lock the thread while we push everything from local collision into our global collisions list.
                std::lock_guard<std::mutex> lock(mutex);
                collisions.insert(collisions.end(), localCollisions.begin(), localCollisions.end());
            }
        });
    }
    threadPool.waitForCompletion();

    for (auto& c : collisions) {
        registry.collisions.emplace_with_duplicates(std::get<0>(c), std::get<1>(c), std::get<2>(c), std::get<3>(c));
    }
}
