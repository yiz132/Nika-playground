#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <iostream>

#pragma once
struct Random{
    ConfigLoader* cl;
    MyDisplay* display;
    Level* map;
    LocalPlayer* lp;
    std::vector<Player*>* players;
    int tmpSpectator;

    Random(ConfigLoader* configLoada, MyDisplay* myDisplay, Level* level, LocalPlayer* localPlayer, std::vector<Player*>* all_players){
        cl = configLoada;
        display = myDisplay;
        map = level;
        lp = localPlayer;
        players = all_players;
    }

    void autoGrapple() {
        if(!map->playable) return;
        if(lp->dead) return;

        KeySym qKeySym = XK_Q;

        if (display->keyDown(qKeySym)) {
            if (!lp->inJump) {
                int n = mem::Read<int>(lp->base + OFF_GRAPPLE + OFF_GRAPPLE_ATTACHED);
    
                std::cout << n << std::endl;
    
                printf("n: %d\n", n);  // Use %d to print an integer in printf.
                if (n == 1) {
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
                }
            }
	    }
    }

    void AutoTapStrafe() {
        if(!map->playable) return;
        if(lp->dead) return;

        bool ts_start = true;
        bool longclimb = false;

        float wallrun_start = mem::Read<float>(lp->base + OFF_WALL_RUN_START_TIME);
        float wallrun_clear = mem::Read<float>(lp->base + OFF_WALL_RUN_CLEAR_TIME);
        float world_time = mem::Read<float>(lp->base + OFFSET_TIME_BASE);

        if (wallrun_start > wallrun_clear) {
        float climbTime = world_time - wallrun_start;
        if (climbTime > 0.8) {
            longclimb = true;
            ts_start = false;
        }
        else {
            ts_start = true;
        }
        }
        if (ts_start) {
        if (longclimb) {
            if (world_time > wallrun_clear + 0.1)
            longclimb = false;
        }

        int flags = mem::Read<int>(lp->base + OFF_CENTITY_FLAGS);
        int backward_state = mem::Read<int>(OFF_REGION + OFF_IN_BACKWARD);
        int forward_state = mem::Read<int>(OFF_REGION + OFF_IN_FORWARD);
        int force_forward = mem::Read<int>(OFF_REGION + OFF_IN_FORWARD + 0x8);
        int skydrive_state = mem::Read<int>(lp->base + OFF_SKY_DIVE_STATUS);
        int duck_state = mem::Read<int>(lp->base + OFF_DUCK_STATUS);

        if (((flags & 0x1) == 0) && !(skydrive_state > 0) && !longclimb &&
            !(backward_state > 0)) {
            if (((duck_state > 0) && (forward_state == 33))) { // Previously 33
            if (force_forward == 0) {
                mem::Write<int>(OFF_REGION + OFF_IN_FORWARD + 0x8, 1);
            }
            else {
                mem::Write<int>(OFF_REGION + OFF_IN_FORWARD + 0x8, 0);
            }
            }
        }
        else if ((flags & 0x1) != 0) {
            if (forward_state == 0) {
            mem::Write<int>(OFF_REGION + OFF_IN_FORWARD + 0x8, 0);
            }
            else if (forward_state == 33) {
            mem::Write<int>(OFF_REGION + OFF_IN_FORWARD + 0x8, 1);
            }
        }
        }
    }

    void BHop() {
        if(!map->playable) return;
        if(lp->dead) return;

        KeySym leftControlKeySym = XK_Control_L;
        KeySym spaceKeySym = XK_space;

        if (display->keyDown(leftControlKeySym) && display->keyDown(spaceKeySym)) {
            std::this_thread::sleep_for(
            std::chrono::milliseconds(25));
            mem::Write(OFF_REGION + OFF_IN_JUMP + 0x08, 5);
            std::this_thread::sleep_for(
            std::chrono::milliseconds(25));
            mem::Write(OFF_REGION + OFF_IN_JUMP + 0x08, 4);
        }
    }

    void superGlide(){
        if(cl->FEATURE_SUPER_GLIDE_ON){
            static int sgState = 0;
            static int sgFrameTime = 0;

            float traversalStartTime = lp->traversalStartTime;

            float traversalProgress = lp->traversalProgress;

            float traversalReleaseTime = lp->traversalReleaseTime;

            float time = lp->worldtime;
            float hangOnWall = time - traversalStartTime;

            switch (sgState){
            case 0:
                if (traversalProgress > 0.88f && hangOnWall > 0.0f && hangOnWall < 1.5f && traversalReleaseTime == 0.0f)
                {
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                    sgState = 1;
                    sgFrameTime = lp->frameCount;
                }
                else if (hangOnWall > 0.1f && hangOnWall < 0.12f && traversalProgress < 0.85f) {
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
                }
                break;
            case 1:
                if(sgFrameTime + 1 <= lp->frameCount)
                {
                    mem::Write<int>(OFF_REGION + OFF_IN_DUCK + 0x8, 5);
                    sgState = 2;
                    sgFrameTime = lp->frameCount;
                }
                break;
            case 2:
                if (time - traversalReleaseTime > 0.1f && sgFrameTime + 1 <= lp->frameCount)
                {
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
                    mem::Write<int>(OFF_REGION + OFF_IN_DUCK + 0x8, 4);
                    sgState = 0;
                }
                break;
            }
        }
    }
    
    void quickTurn(){
        if(!map->playable) return;
        if(!lp->isValid()) return;
        if(lp->dead) return;
        Vector2D localYawtoClamp = lp->viewAngles;
        localYawtoClamp.Clamp();
        float localYaw = localYawtoClamp.y;
        // quickTurn
        if(cl->FEATURE_QUICKTURN_ON){
            if(display->keyDown(cl->FEATURE_QUICKTURN_BUTTON)){
                lp->setYaw((localYaw + 180));
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    }   
    void mapRadar(){
        if (display->keyDown(cl->FEATURE_MAP_RADAR_BUTTON) && cl->FEATURE_MAP_RADAR_ON) {
            uint64_t pLocal = mem::Read<uint64_t>(OFF_REGION + OFF_LOCAL_PLAYER, "LocalPlayer");

            int currentTEAM = mem::Read<int>(pLocal + OFF_TEAM_NUMBER, "Curren TeamID");
            
            for (uintptr_t i = 0; i <= 80000; i++)
            {
            mem::Write<int>(pLocal + OFF_TEAM_NUMBER, 1);
            }
            for (uintptr_t i = 0; i <= 80000; i++)
            {
            mem::Write<int>(pLocal + OFF_TEAM_NUMBER, currentTEAM);
            } 
        }
    }
    void printLevels(){
        if(cl->FEATURE_PRINT_LEVELS_ON){
            if(display->keyDown(cl->FEATURE_PRINT_LEVELS_BUTTON)){
                printf("[N]=[NAME]-[LEVEL]-[LEGEND]\n\n");
                for (auto i = 0; i < players->size(); i++)
                {
                    Player *p = players->at(i);
                    if(!p->dead && p->isPlayer()){
                        int playerLvl = p->GetPlayerLevel();
                        std::string namePlayer = p->getPlayerName();
                        std::string modelName = p->getPlayerModelName();
                        
                        if(p->friendly){
                            printf("\033[91m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());  
                        } else if (playerLvl > 900 &&  playerLvl < 1900){
                            printf("\033[94m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        } else if (playerLvl > 1900){
                            printf("\033[92m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        } else {
                            printf("[%i]=[%s]-[%i]-[%s]\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        }
                    }           
                }   
            }            
        }        
    }
    void spectatorView(){
        if(!map->playable && map->trainingArea) return;
        if(lp->dead) return;
        int spectatorcount = 0;   
        std::vector<std::string> spectatorList;
        std::vector<std::string> modelName;
        if(cl->FEATURE_SPECTATOR_ON){
            for (int i = 0; i < players->size(); i++)
            { 
                Player *p = players->at(i);         
                if (p->IsSpectating()){
                    spectatorcount++;
                    tmpSpectator = spectatorcount;
                    
                    std::string playerName = p->getPlayerName();
                    std::string modelNameIndex = p->getPlayerModelName();    
                    spectatorList.push_back(p->getPlayerName());
                    //modelName.push_back(modelNameIndex);
                }            
            }
            const auto spectatorlist_size = static_cast<int>(spectatorList.size());
           
            if (spectatorcount > 0){
                printf("\n-[%d]-- SPECTATORS -- \n", spectatorcount);
                for (int i = 0; i < spectatorlist_size; i++) 
                {   
                    printf("---[%s]---\n", spectatorList.at(i).c_str());
                }
            }              
        }      
    }
    void skinChanger(){
        if(!map->playable) return;
        if(lp->dead) return;
        float curTime = lp->worldtime;
        float endTime = curTime +5.5;
        std::map<int, std::vector<int>> weaponSkinMap;
        //Light ammo weapons
        weaponSkinMap[105] = { 6 };   //WEAPON_P2020 
        weaponSkinMap[81] = { 6 };   //WEAPON_RE45 
        weaponSkinMap[80] = { 11 };   //WEAPON_ALTERNATOR 
        weaponSkinMap[104] = { 2 };   //WEAPON_R99  
        weaponSkinMap[0] = { 10 };     //WEAPON_R301   
        weaponSkinMap[106] = { 2 };    //WEAPON_SPITFIRE 
        weaponSkinMap[89] = { 5 };    //WEAPON_G7 
        //Heavy ammo weapons
        weaponSkinMap[112] = { 10};   // Car-SMG 
        weaponSkinMap[21] = { 6 };    // Rampage 
        weaponSkinMap[111] = { 9 };      //3030 
        weaponSkinMap[90] = {10 };   //WEAPON_HEMLOCK  
        weaponSkinMap[88] = { 8 };    //FlatLine  
        //Energy ammo weapons
        weaponSkinMap[113] = { 8 };    //WEAPON_NEMESIS  
        weaponSkinMap[110] = { 9 };    //WEAPON_VOLT 
        weaponSkinMap[107] = { 7 };    //WEAPON_TRIPLE_TAKE 
        weaponSkinMap[93] = { 3 };    //WEAPON_LSTAR 
        weaponSkinMap[84] = { 5 };    //WEAPON_DEVOTION 
        weaponSkinMap[86] = { 8 };    //WEAPON_HAVOC 
        //Sniper ammo weapons
        weaponSkinMap[1] = { 5 };    //WEAPON_SENTINEL 
        weaponSkinMap[83] = { 8 };    //WEAPON_CHARGE_RIFLE 
        weaponSkinMap[85] = { 7 };    //WEAPON_LONGBOW 
        //Shotgun ammo weapons
        weaponSkinMap[96] = { 5 };    //WEAPON_MOZAMBIQUE 
        weaponSkinMap[87] = { 8 };    //WEAPON_EVA8 
        weaponSkinMap[103] = { 7 };    //WEAPON_PEACEKEEPER 
        weaponSkinMap[95] = { 5 };    //WEAPON_MASTIFF 
        //Legendary ammo weapons
        weaponSkinMap[109] = { 5 };    //WEAPON_WINGMAN 
        weaponSkinMap[102] = { 7 };    //WEAPON_PROWLER
        weaponSkinMap[2] = { 3 };    //WEAPON_BOCEK
        weaponSkinMap[92] = { 6 };    //WEAPON_KRABER
        weaponSkinMap[163] = { 3 };    //WEAPON_THROWING_KNIFE
        weaponSkinMap[164] = { 2 };    //WEAPON_THERMITE_GRENADE 
        weaponSkinMap[3] = { 2 };    //WEAPON_BUSTER_SWORD_R25 

        if (cl->FEATURE_SKINCHANGER_ON){
            int waponIndex = lp->weaponIndex;
            if (weaponSkinMap.count(waponIndex) == 0) return;
            int skinID = weaponSkinMap[waponIndex][0];
            //printf("Weapon: %s Activated Skin ID: %d \n", WeaponName(waponIndex).c_str(), skinID);  
            mem::Write<int>(lp->base + OFF_SKIN, skinID+1);
            mem::Write<int>(lp->weaponEntity + OFF_SKIN, skinID);
        }                    
    }

    void antiRecoil() {
        int recoil_range = 1.2;

        if (!display || !display->display) {
            std::cerr << "Unable to open X display\n";
            return;
        }


        Display* xDisplay = display->display;

        Window root = DefaultRootWindow(xDisplay);
        XEvent event;

        // Variables to store mouse button states
        int left_button_pressed = 0;
        int right_button_pressed = 0;

        // Check the state of mouse buttons
        XQueryPointer(xDisplay, root, &event.xbutton.root, &event.xbutton.window,
                        &event.xbutton.x_root, &event.xbutton.y_root,
                        &event.xbutton.x, &event.xbutton.y,
                        &event.xbutton.state);

        // Bitwise AND to check if left and right buttons are pressed
        left_button_pressed = (event.xbutton.state & Button1Mask);  // Left button
        right_button_pressed = (event.xbutton.state & Button3Mask); // Right button

        if (left_button_pressed && right_button_pressed) {
            // Simulate anti-recoil by moving the mouse in jitter motion
            XTestFakeRelativeMotionEvent(xDisplay, -recoil_range, recoil_range, CurrentTime); // Move up-right
            XFlush(xDisplay);
            std::this_thread::sleep_for(std::chrono::milliseconds(7)); // Sleep for 7 ms

            XTestFakeRelativeMotionEvent(xDisplay, recoil_range, -recoil_range, CurrentTime); // Move down-left
            XFlush(xDisplay);
            std::this_thread::sleep_for(std::chrono::milliseconds(7)); // Sleep for 7 ms
        }

        // Add a small delay to avoid CPU overconsumption
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

     
    void runAll(int counter){
        superGlide();
        BHop();
        AutoTapStrafe();
        antiRecoil();
        autoGrapple();
    }
};
