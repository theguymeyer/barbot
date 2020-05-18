/* Drink.h */
// Author: Guy Meyer
// Date of creation: May 9th 2020
// Last change date: May 9th 2020
// Objective: This files defines the prototype of the drink class implemented in drink.cpp

// Prototype definitions

#ifndef DRINK_H // include guard
#define DRINK_H

//#include <cstring>

#define bottleCount 5

class Drink
{
private:
    // list of ASCII chars representing drink amounts the (bottleCount) least significant byte will contain the drink info. The first char is Bottle_1, the second char is Bottle_2 ...
    char _drink_structure[(int)bottleCount +1];

public:
    void init_drink(char *);
    char *get_drink();                                      // drink structure getter
    void update_drink_structure(char *); // drink structure setter
    unsigned char get_drink_by_bottle(int);
    void update_drink_by_bottle(int, bool); // the liquid amounts are NOT cyclic - true indicates increase / false indicates decrease
};

#endif /* DRINK_H */
