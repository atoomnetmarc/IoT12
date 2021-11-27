/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef Heater_H
#define Heater_H

class HeaterClass
{
public:
    HeaterClass();
    void init(void);
    void loop(void);

private:
};

extern HeaterClass Heater;

#endif