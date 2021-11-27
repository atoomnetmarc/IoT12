/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef AnalogComparator_H
#define AnalogComparator_H

class AnalogComparatorClass
{
public:
    AnalogComparatorClass();
    void init(void);
    void loop(void);
    bool hasTriggered(void);
    void trigger(void);

private:
    bool triggered = false;
};

extern AnalogComparatorClass AnalogComparator;

#endif