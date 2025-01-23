#pragma once
#include <vector>

#include "ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
    // Callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface*> registry_list;

public:
    // Manually created list of all components this game has
    ComponentContainer<SavePoint> savePoints;
    ComponentContainer<Health> healths;
    ComponentContainer<HealthFlask> healthFlasks;
    ComponentContainer<Damage> damages;
    ComponentContainer<RecentlyDamagedTimer> recentDamageTimers;
    ComponentContainer<InvincibilityTimer> invinciblityTimers;
    ComponentContainer<PlusHeartTimer> plusHeartTimers;
    ComponentContainer<HealTimer> healTimers;
    ComponentContainer<ProjectileTimer> projectileTimers;
    ComponentContainer<Projectile> projectiles;
    ComponentContainer<Weapon> weapons;
    ComponentContainer<Motion> motions;
    ComponentContainer<Collision> collisions;
    ComponentContainer<DeathTimer> deathTimers;
    ComponentContainer<Player> players;
    ComponentContainer<Hostile> hostiles;
    ComponentContainer<Boss> bosses;
    ComponentContainer<TransformComponent> transforms;
    ComponentContainer<Sprite> sprites;
    ComponentContainer<PlayerMeshes> playerMeshes;
    ComponentContainer<Mesh*> meshPtrs;
    ComponentContainer<std::vector<Sprite>> heartSprites;
    ComponentContainer<RenderRequest> renderRequests; 
    ComponentContainer<ScreenState> screenStates;
    ComponentContainer<GoombaFlyingState> goombaFlyingStates;
    ComponentContainer<Animation<PlayerState>> playerAnimations;
    ComponentContainer<Animation<ChickenState>> chickenAnimations;
    ComponentContainer<Animation<GBState>> gbAnimations;
    ComponentContainer<Animation<FlyingGoombaState>> flyingGoombaAnimations;
    ComponentContainer<Gravity> gravity;
    ComponentContainer<Patrol_AI> patrol;
    ComponentContainer<BoundingBox> bounding_box;
    ComponentContainer<Environment> envObject;
    ComponentContainer<Combat> combat;
    ComponentContainer<Ground> grounds;
    ComponentContainer<MovingPlatform> movingPlatform;
    ComponentContainer<Patrol_AI> patrol_ais;

    ComponentContainer<Pelican> pelican;
    ComponentContainer<Elder> elder;
    ComponentContainer<Kat> kat;

    ComponentContainer<BadObj> badObjs;
    ComponentContainer<BadObjTimer> badObjTimers;

    ComponentContainer<Room> rooms;
    ComponentContainer<Connection> doors;
    ComponentContainer<ConnectionList> doorList;
    ComponentContainer<HeartPowerUp> heartPowerUp;
    ComponentContainer<SwordPowerUp> swordPowerUp;
    ComponentContainer<MenuItem> menuItems;
    ComponentContainer<Wall> walls;

    ComponentContainer<EndingTriggers> endingTriggers;

    // constructor that adds all containers for looping over them
    // IMPORTANT: Don't forget to add any newly added containers!
    ECSRegistry()
    {
        registry_list.push_back(&plusHeartTimers);
        registry_list.push_back(&healTimers);
        registry_list.push_back(&goombaFlyingStates);
        registry_list.push_back(&savePoints);
        registry_list.push_back(&healths);
        registry_list.push_back(&healthFlasks);
        registry_list.push_back(&damages);
        registry_list.push_back(&recentDamageTimers);
        registry_list.push_back(&invinciblityTimers);
        registry_list.push_back(&projectileTimers);
        registry_list.push_back(&projectiles);
        registry_list.push_back(&weapons);
        registry_list.push_back(&deathTimers);
        registry_list.push_back(&motions);
        registry_list.push_back(&collisions);
        registry_list.push_back(&players);
        registry_list.push_back(&hostiles);
        registry_list.push_back(&bosses);
        registry_list.push_back(&transforms);
        registry_list.push_back(&sprites);
        registry_list.push_back(&playerMeshes);
        registry_list.push_back(&meshPtrs);
        registry_list.push_back(&heartSprites);
        registry_list.push_back(&renderRequests);
        registry_list.push_back(&screenStates);
        registry_list.push_back(&playerAnimations);
        registry_list.push_back(&chickenAnimations);
        registry_list.push_back(&flyingGoombaAnimations);
        registry_list.push_back(&gravity);
        registry_list.push_back(&patrol);
        registry_list.push_back(&bounding_box);
        registry_list.push_back(&envObject);
        registry_list.push_back(&combat);
        registry_list.push_back(&grounds);
        registry_list.push_back(&movingPlatform);
        registry_list.push_back(&patrol_ais);
        registry_list.push_back(&doors);
        registry_list.push_back(&doorList);
        registry_list.push_back(&heartPowerUp);
        registry_list.push_back(&swordPowerUp);
        registry_list.push_back(&menuItems);
        registry_list.push_back(&pelican);
        registry_list.push_back(&elder);
        registry_list.push_back(&kat);
        registry_list.push_back(&walls);
        registry_list.push_back(&badObjs);
        registry_list.push_back(&badObjTimers);
        registry_list.push_back(&gbAnimations);
        registry_list.push_back(&endingTriggers);
    }

    void clear_all_components() {
        for (ContainerInterface* reg : registry_list)
            reg->clear();
    }

    void list_all_components() {
        printf("Debug info on all registry entries:\n");
        for (ContainerInterface* reg : registry_list)
            if (reg->size() > 0)
                printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
    }

    void list_all_components_of(Entity e) {
        printf("Debug info on components of entity %u:\n", (unsigned int)e);
        for (ContainerInterface* reg : registry_list)
            if (reg->has(e))
                printf("type %s\n", typeid(*reg).name());
    }

    void remove_all_components_of(Entity e) {
        for (ContainerInterface* reg : registry_list)
            reg->remove(e);
    }
};

extern ECSRegistry registry; // The global ECS registry
