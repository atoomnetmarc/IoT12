/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

/*
 * Composition root: owns the module wiring and the order in which the modules
 * are set up and serviced. main.cpp only calls these two functions.
 */
void AppSetup(void);
void AppLoop(void);
