// user.h - header file for user.cpp
// Author: Guy Meyer
// Date of creation: May 9th 2020
// Last change date: May 9th 2020

// Prototype definitions

class User {
    private:
        int bottle_position;
    public:
        void init_user();
        int get_user_position();
        void inc_bottle_position(); // increments bottle position by 1 and is cyclic
        void dec_bottle_position(); // decreases bottle position by 1 and is cyclic
};