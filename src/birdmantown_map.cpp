#include <iostream>
#include "world_system.hpp"
#include "enemy.hpp"
#include "birdmantown_map.hpp"

Birdmantown::Birdmantown() {
}

Birdmantown::~Birdmantown() {
}

void Birdmantown::init() {
    setRoomStrategy(std::make_unique<BMTEntranceRoomStrategy>());
    m_entrance_room = currentRoom->execute();
    start_room = m_entrance_room;

    setRoomStrategy(std::make_unique<BMTRoom1Strategy>());
    m_room1 = currentRoom->execute();
    //start_room = m_room1;

    setRoomStrategy(std::make_unique<BMTRoom2Strategy>());
    m_room2 = currentRoom->execute();
    //start_room = m_room2;

    setRoomStrategy(std::make_unique<BMTRoom3Strategy>());
    m_room3 = currentRoom->execute();
    //start_room = m_room3;

    setRoomStrategy(std::make_unique<BMTRoom4Strategy>());
    m_room4 = currentRoom->execute();
    //start_room = m_room4;

    setRoomStrategy(std::make_unique<BMTRoom5Strategy>());
    m_room5 = currentRoom->execute();
    //start_room = m_room5;

    setRoomStrategy(std::make_unique<NPCRoom1Strategy>());
    m_npc_room1 = currentRoom->execute();
    //start_room = m_npc_room1;

    setRoomStrategy(std::make_unique<NPCRoom2Strategy>());
    m_npc_room2 = currentRoom->execute();
    //start_room = m_npc_room2;
    

    setRoomStrategy(std::make_unique<LNRoom1Strategy>());
    m_ln_room1 = currentRoom->execute();
    //start_room = m_ln_room1;

    setRoomStrategy(std::make_unique<LNRoom2Strategy>());
    m_ln_room2 = currentRoom->execute();
    //start_room = m_ln_room2;

    setRoomStrategy(std::make_unique<LNBossRoomStrategy>());
    m_ln_boss_room = currentRoom->execute();
    //start_room = m_ln_boss_room;

    // bmt to cp = (0.32f, 0.04f, 0.09f, 1.f), spawn = (0.08f, 0.17f)
    ConnectionList list_en;
    Connection door_bmt_to_cp = SetDoor(0.32f, 0.04f, 0.09f, 1.f, m_entrance_room, vec2(renderSystem.getWindowWidth() * 0.5f, renderSystem.getWindowHeight() * 0.17f));
    door_bmt_to_cp.switchMap = true;
    list_en.doors.push_back(door_bmt_to_cp);

    // entrance to 1: (0.22f, 0.04f, 0.935f, 0.f), spawn = (0.77f, 0.8f)
    Connection door_en_to_1 = SetDoor(0.22f, 0.04f, 0.935f, 0.f, m_room1, vec2(renderSystem.getWindowWidth() * 0.9f, renderSystem.getWindowHeight() * 0.8f));
    list_en.doors.push_back(door_en_to_1);

    // entrance to npc1: (0.03f, 0.5f, 1.f, 0.65f), spawn = (0.08f, 0.8f)
    Connection door_en_to_npc1 = SetDoor(0.03f, 0.5f, 1.f, 0.65f, m_npc_room1, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.8f));
    list_en.doors.push_back(door_en_to_npc1);

    // npc1 to entrance: (0.03f, 0.8f, 0.f, 0.5f), spwan = (0.92f, 0.8f)
    ConnectionList list_npc1;
    Connection door_npc1_to_en = SetDoor(0.03f, 0.8f, 0.f, 0.5f, m_entrance_room, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_npc1.doors.push_back(door_npc1_to_en);

    // 1 to entrance: (0.28f, 0.04f, 0.91f, 1.f), spawn = (0.92f, 0.18f)
    ConnectionList list_1;
    Connection door_1_to_en = SetDoor(0.28f, 0.04f, 0.91f, 1.f, m_entrance_room, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.18f));
    list_1.doors.push_back(door_1_to_en);

    // 1 to 2: (0.03f, 0.28f, 0.f, 0.77f), spawn = (0.92f, 0.8f)
    Connection door_1_to_2 = SetDoor(0.03f, 0.28f, 0.f, 0.77f, m_room2, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_1.doors.push_back(door_1_to_2);

    // 1 to 3: (0.03f, 0.28f, 0.f, 0.3f), spawn = (0.92f, 0.8f)
    Connection door_1_to_3 = SetDoor(0.03f, 0.28f, 0.f, 0.3f, m_room3, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_1.doors.push_back(door_1_to_3);

    // 1 to 4: (0.42f, 0.04f, 0.9f, 0.f), spawn = (0.2f, 0.78f)
    Connection door_1_to_4 = SetDoor(0.42f, 0.04f, 0.9f, 0.f, m_room4, vec2(renderSystem.getWindowWidth() * 0.125f, renderSystem.getWindowHeight() * 0.9f));
    list_1.doors.push_back(door_1_to_4);

    // 2 to 1: (0.03f, 0.8f, 1.0f, 0.5f), spawn = (0.08f, 0.8f)
    ConnectionList list_2;
    Connection door_2_to_1 = SetDoor(0.03f, 0.8f, 1.0f, 0.5f, m_room1, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.8f));
    list_2.doors.push_back(door_2_to_1);

    /*// 2 to npc2: (0.03f, 0.27f, 0.f, 0.25f), spawn = (0.92f, 0.8f)
    Connection door_2_to_npc2 = SetDoor(0.03f, 0.35f, 0.0f, 0.2f, m_npc_room2, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f), false);
    list_2.doors.push_back(door_2_to_npc2);

    // npc2 to 2: (0.1f, 0.8f, 1.f, 0.5f), spawn = (0.08f, 0.28f)
    ConnectionList list_npc2;
    Connection door_npc2_to_2 = SetDoor(0.03f, 0.8f, 1.f, 0.5f, m_room2, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.28f), false);
    list_npc2.doors.push_back(door_npc2_to_2);*/

    // 3 to 1: (0.03f, 0.8f, 1.0f, 0.5f), spawn = (0.08f, 0.35f)
    ConnectionList list_3;
    Connection door_3_to_1 = SetDoor(0.03f, 0.8f, 1.0f, 0.5f, m_room1, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.35f));
    list_3.doors.push_back(door_3_to_1);

    // 4 to 1: (1.0f, 0.04f, 0.3f, 1.0f). spawn = (0.9f, 0.18f)
    ConnectionList list_4;
    Connection door_4_to_1 = SetDoor(1.0f, 0.04f, 0.3f, 1.0f, m_room1, vec2(renderSystem.getWindowWidth() * 0.9f, renderSystem.getWindowHeight() * 0.18f));
    list_4.doors.push_back(door_4_to_1);

    // 4 to LN1: (0.4f, 0.04f, 0.88f, 0.f), spawn = (0.24f, 0.8f)
    Connection door_4_to_ln1 = SetDoor(0.4f, 0.04f, 0.88f, 0.f, m_ln_room1, vec2(renderSystem.getWindowWidth() * 0.13f, renderSystem.getWindowHeight() * 0.8f));
    list_4.doors.push_back(door_4_to_ln1);

    // 4 to 5: (0.03f, 0.4f, 1.0f, 0.72f), spawn = (0.08f, 0.3f)
    Connection door_4_to_5 = SetDoor(0.03f, 0.4f, 1.0f, 0.72f, m_room5, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.3f));
    list_4.doors.push_back(door_4_to_5);

    // 5 to 4: (0.03f, 0.3f, 0.f, 0.22f), spawn = (0.92f, 0.8f)
    ConnectionList list_5;
    Connection door_5_to_4 = SetDoor(0.03f, 0.3f, 0.f, 0.22f, m_room4, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_5.doors.push_back(door_5_to_4);
    
    // 5 to npc2: (0.03f, 0.4f, 1.0f, 0.72f), spawn = (0.08f, 0.8f)
    Connection door_5_to_npc2 = SetDoor(0.03f, 0.4f, 1.0f, 0.72f, m_npc_room2, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.8f));
    list_5.doors.push_back(door_5_to_npc2);

    // npc2 to 5: (0.03f, 0.8f, 0.f, 0.5f), spawn = (0.92f, 0.8f) 
    ConnectionList list_npc2;
    Connection door_npc2_to_5 = SetDoor(0.03f, 0.8f, 0.f, 0.5f, m_room5, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_npc2.doors.push_back(door_npc2_to_5);

    // LN1 to 4: (0.35f, 0.04f, 0.1f, 1.f), spawn = (0.92f, 0.18f)
    ConnectionList list_ln1;
    Connection door_ln1_to_4 = SetDoor(0.35f, 0.04f, 0.1f, 1.f, m_room4, vec2(renderSystem.getWindowWidth() * 0.9f, renderSystem.getWindowHeight() * 0.18f));
    list_ln1.doors.push_back(door_ln1_to_4);

    // LN1 to LN2: (0.03f, 0.4f, 1.f, 0.7f), spawn = (0.08f, 0.65f)
    Connection door_ln1_to_boss = SetDoor(0.03f, 0.4f, 1.f, 0.7f, m_ln_room2, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.65f));
    list_ln1.doors.push_back(door_ln1_to_boss);

    // LN2 to LN1: (0.1f, 0.4f, 0.f, 0.55f), spawn = (0.92f, 0.8f)
    ConnectionList list_ln2;
    Connection door_npc3_to_ln1 = SetDoor(0.03f, 0.4f, 0.f, 0.7f, m_ln_room1, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.8f));
    list_ln2.doors.push_back(door_npc3_to_ln1);

    // LN2 to boss: (0.03f, 0.4f, 1.f, 0.55f), spawn = (0.08f, 0.8f)
    Connection door_npc3_to_boss = SetDoor(0.03f, 0.4f, 1.f, 0.7f, m_ln_boss_room, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.8f));
    list_ln2.doors.push_back(door_npc3_to_boss);

    /*// boss to LN1: (0.1f, 0.4f, 0.f, 0.7f), spawn = (0.92f, 0.8f)
    ConnectionList list_boss;
    Connection door_boss_to_ln1 = SetDoor(0.03f, 0.4f, 0.f, 0.7f, m_ln_room1, vec2(renderSystem.getWindowWidth() * 0.9f, renderSystem.getWindowHeight() * 0.8f), false);
    list_boss.doors.push_back(door_boss_to_ln1);*/
    

    // boss to exit: (0.03f, 0.4f, 1.f, 0.7f)


    registry.doorList.emplace(m_entrance_room, std::move(list_en));
    registry.doorList.emplace(m_room1, std::move(list_1));
    registry.doorList.emplace(m_room2, std::move(list_2));
    registry.doorList.emplace(m_room3, std::move(list_3));
    registry.doorList.emplace(m_room4, std::move(list_4));
    registry.doorList.emplace(m_room5, std::move(list_5));
    registry.doorList.emplace(m_npc_room1, std::move(list_npc1));
    registry.doorList.emplace(m_npc_room2, std::move(list_npc2));
    registry.doorList.emplace(m_ln_room2, std::move(list_ln2));
    registry.doorList.emplace(m_ln_room1, std::move(list_ln1));
    //registry.doorList.emplace(m_ln_boss_room, std::move(list_boss));
}

Connection Birdmantown::SetDoor(float width, float height, float xPos, float yPos, Entity connectNextRoom, vec2 connectNextSpawn) {
    //Connection connectingDoor;
    Connection doorConnection;
    Entity m_door = Entity();
    Sprite doorSprite(g_texture_paths->at(TEXTURE_ASSET_ID::DOOR));
    width *= doorSprite.width;
    height *= doorSprite.height;
    registry.sprites.emplace(m_door, doorSprite);

    // Create and initialize a Motion component for the platform
    Motion doorMotion;
    if (yPos == 0.f) {
        doorMotion.position = glm::vec2(renderSystem.getWindowWidth() * xPos, renderSystem.getWindowHeight() * yPos + 10.f);
    } else {
        doorMotion.position = glm::vec2(renderSystem.getWindowWidth() * xPos, renderSystem.getWindowHeight() * yPos);
    }
    doorMotion.velocity = glm::vec2(0, 0);
    doorMotion.scale = { width, height };
    registry.motions.emplace(m_door, std::move(doorMotion));

    // add platform to environment to render out later
    Environment doorObj;
    registry.envObject.emplace(m_door, std::move(doorObj));

    registry.bounding_box.emplace(m_door);
    BoundingBox bb = registry.bounding_box.get(m_door);
    bb.height = doorSprite.height;
    bb.width = doorSprite.width;

    // set up doors
    doorConnection.door = m_door;
    doorConnection.nextRoom = connectNextRoom;
    doorConnection.nextSpawn = connectNextSpawn;
    doorConnection.switchMap = false;

    registry.doors.emplace(m_door, doorConnection);

    return doorConnection;
}
