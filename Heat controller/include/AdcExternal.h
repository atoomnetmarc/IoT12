/*

Copyright 2021 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#ifndef AdcExternal_H
#define AdcExternal_H

class ADCExternalClass
{
    public:
        ADCExternalClass();
        void init(void);
        void loop(void);

    private:
};

extern ADCExternalClass ADCExternal;

#endif