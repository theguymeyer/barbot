// drink.cpp
// Author: Guy Meyer
// Date of creation: May 9th 2020
// Last change date: May 10th 2020
// Objective: This files defines the status of the drink structure as the user interacts with the interface

#include "drink.h"
#include <string.h>

// method definitions - Drink

// TODO make dynamic
void Drink ::init_drink(char * drink_structure)
{
    strcpy(_drink_structure, drink_structure); // the five drink options
}

char *Drink ::get_drink()
{
    return &_drink_structure[0];
}

void Drink::update_drink_structure(char *new_drink_structure)
{
    *_drink_structure = *new_drink_structure;
}

unsigned char Drink ::get_drink_by_bottle(int bottle_num)
{

    //assert(bottle_num <= bottleCount);
    //assert(bottle_num >= 0);

    return _drink_structure[bottle_num];
}

// the liquid amounts are NOT cyclic, with a max of 2 shots - true indicates increase / false indicates decrease (TODO make into enum)
void Drink ::update_drink_by_bottle(int bottle_num, bool inc_dec_selection)
{

    //assert(bottle_num <= bottleCount);
    //assert(bottle_num >= 0);

    char my_bottle = this->get_drink_by_bottle(bottle_num);

    if (inc_dec_selection)
    { // increase shot count by 1
        if (my_bottle >= '2')
        {
        } // max of 2 shots
        else
        {
            my_bottle = my_bottle + 0x1;
        }
    }
    else
    { // decrease shot count by 1
        if (my_bottle == '0')
        {
        }
        else
        {
            my_bottle = my_bottle - 0x1;
        }
    }

    // generate tmp drink to modify the drink structure
    char *tmp_drink = this->get_drink();
    tmp_drink[bottle_num] = my_bottle;

    this->update_drink_structure(tmp_drink);
}