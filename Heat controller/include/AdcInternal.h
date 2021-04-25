/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef AdcInternal_H
#define AdcInternal_H

class ADCInternalClass
{
    public:
        ADCInternalClass();
        void init(void);
        void loop(void);

    private:
};

extern ADCInternalClass ADCInternal;

#endif