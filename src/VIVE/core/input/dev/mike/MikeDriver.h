/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/*
 * inputhdw.h
 *
 *  Created on: Dec 9, 2014
 *      Author: svnvlad
 */

#pragma once

#include <OpenThreads/Thread>
#include <osg/Matrix>

#include <core/input/inputdevice.h>

using namespace vive;

/**
 * @brief The InputHdw class interacts with input hardware
 *
 * This class interacts with input hardware and stores the data
 * about all configured input hardware e.g. tracking systems,
 * button devices etc.
 *
 * Main interaction loop runs in its own thread
 */
class MikeDriver : public InputDevice
{
    //---------------------Mouse related stuff
    unsigned int btnstatus, oldbtnstatus; /// Mouse Button status bit masks

    //=========End of hardware related stuff======================================
    bool init();

public:
    MikeDriver(const std::string &configPath);
    virtual ~MikeDriver();
    bool poll();
};

