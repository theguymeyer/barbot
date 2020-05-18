// user.cpp
// Author: Guy Meyer
// Date of creation: May 9th 2020
// Last change date: May 9th 2020
// Objective: This files defines the status of the user as they interact with the buttons

#include "user.h"

using namespace std;

#define bottleCount 5

// method definitions - User

void User :: init_user () {
    bottle_position = 0;
}

int User :: get_user_position () {
    return bottle_position;
}

void User :: inc_bottle_position () {
    bottle_position = (bottle_position + 1) % bottleCount;
}

void User :: dec_bottle_position () {
    if (bottle_position == 0) { bottle_position = 4; }
    else { bottle_position = (bottle_position - 1); }
}