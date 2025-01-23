#include <iostream>
#include "cesspit_map.hpp"
#include "world_system.hpp"
#include "enemy.hpp"

Cesspit::Cesspit() {
}

Cesspit::~Cesspit() {
}

void Cesspit::init() {

    // if you want to change the room, set start_room to a differnt currentRoom->exectue()

    setRoomStrategy(std::make_unique<CPEntranceRoomStrategy>());
    m_entrance_room = currentRoom->execute();
    //start_room = m_entrance_room;
    // start coords = (0.1f, 0.4f)

    setRoomStrategy(std::make_unique<CPRoom1Strategy>());
    m_room1 = currentRoom->execute();
    //start_room = m_room1;

    setRoomStrategy(std::make_unique<CPRoom2Strategy>());
    m_room2 = currentRoom->execute();
    // start_room = m_room2;

    setRoomStrategy(std::make_unique<CPRoom3Strategy>());
    m_room3 = currentRoom->execute();
    //start_room = m_room3;

    setRoomStrategy(std::make_unique<CPRoom4Strategy>());
    m_room4 = currentRoom->execute();
    //start_room = m_room4;

    setRoomStrategy(std::make_unique<CPBossRoomStrategy>());
    m_boss_room = currentRoom->execute();
    //start_room = m_boss_room;

    setRoomStrategy(std::make_unique<CPExitRoomStrategy>());
    m_exit_room = currentRoom->execute();
    //start_room = m_exit_room;

    if (start_from_checkpoint) {
        start_room = m_exit_room;
    }
    else {
        start_room = m_entrance_room;
    }
    
    
    // set up all doors
    // entrance to npc coords = (0.03f, 0.42f, 1.f, 0.21f), entrance spawn = (0.9f, 0.3f)
    // entrance to room1
    ConnectionList list_en;
    Connection door_en_to_1 = SetDoor(0.39f, 0.04f, 0.672f, 1.f, m_room1, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.23f));
    list_en.doors.push_back(door_en_to_1);
    
    // room1 to entrance
    ConnectionList list_1;
    Connection door_1_to_en = SetDoor(0.2f, 0.04f, 0.05f, 0.f, m_entrance_room, vec2(renderSystem.getWindowWidth() * 0.675f, renderSystem.getWindowHeight() * 0.9f));
    list_1.doors.push_back(door_1_to_en);
    
    // room1 to room2
    Connection door_1_to_2 = SetDoor(0.03f, 0.8f, 1.f, 0.4f, m_room2, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.25f));
    list_1.doors.push_back(door_1_to_2);

    // room2 to room1
    ConnectionList list_2;
    Connection door_2_to_1 = SetDoor(0.03f, 0.35f, 0.f, 0.18f, m_room1, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.7f));
    list_2.doors.push_back(door_2_to_1);
    
    // room2 to room3
    Connection door_2_to_3 = SetDoor(0.72f, 0.04f, 0.517f, 1.0f, m_room3, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.2f));
    list_2.doors.push_back(door_2_to_3);

    // room3 to room2
    ConnectionList list_3;
    Connection door_3_to_2 = SetDoor(0.3f, 0.04f, 0.92f, 0.f, m_room2, vec2(renderSystem.getWindowWidth() * 0.68f, renderSystem.getWindowHeight() * 0.9f));
    list_3.doors.push_back(door_3_to_2);

    // room2 to room4
    Connection door_2_to_4 = SetDoor(0.03f, 0.35f, 1.f, 0.18f, m_room4, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.7f));
    list_2.doors.push_back(door_2_to_4);

    // room4 to room2
    ConnectionList list_4;
    Connection door_4_to_2 = SetDoor(0.03f, 0.8f, 0.f, 0.4f, m_room2, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.25f));
    list_4.doors.push_back(door_4_to_2);

    // room4 to bossRoom
    Connection door_4_to_boss = SetDoor(0.03f, 0.8f, 1.f, 0.4f, m_boss_room, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.7f));
    list_4.doors.push_back(door_4_to_boss);

    // bossRoom to room4
    ConnectionList list_boss;
    Connection door_boss_to_4 = SetDoor(0.03f, 0.8f, 0.f, 0.4f, m_room4, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.7f));
    list_boss.doors.push_back(door_boss_to_4);

    // bossRoom to exitRoom
    Connection door_boss_to_ex = SetDoor(0.03f, 0.8f, 1.f, 0.4f, m_exit_room, vec2(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.82f));
    list_boss.doors.push_back(door_boss_to_ex);

    // exitRoom to bossRoom
    ConnectionList list_ex;
    Connection door_ex_to_boss = SetDoor(0.03f, 0.18f, 0.f, 0.815f, m_boss_room, vec2(renderSystem.getWindowWidth() * 0.92f, renderSystem.getWindowHeight() * 0.7f));
    list_ex.doors.push_back(door_ex_to_boss);

    // cp to bmt
    // door coords = (0.3f, 0.03f, 0.5f, 0.f), spawn = (0.25f, 0.82f)
    Connection door_cp_to_bmt = SetDoor(0.3f, 0.03f, 0.5f, 0.f, m_exit_room, vec2(renderSystem.getWindowWidth() * 0.13f, renderSystem.getWindowHeight() * 0.82f));
    door_cp_to_bmt.switchMap = true;
    list_ex.doors.push_back(door_cp_to_bmt);

    registry.doorList.emplace(m_entrance_room, std::move(list_en));
    registry.doorList.emplace(m_room1, std::move(list_1));
    registry.doorList.emplace(m_room2, std::move(list_2));
    registry.doorList.emplace(m_room3, std::move(list_3));
    registry.doorList.emplace(m_room4, std::move(list_4));
    registry.doorList.emplace(m_boss_room, std::move(list_boss));
    registry.doorList.emplace(m_exit_room, std::move(list_ex));
}

Connection Cesspit::SetDoor(float width, float height, float xPos, float yPos, Entity connectNextRoom, vec2 connectNextSpawn) {
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
