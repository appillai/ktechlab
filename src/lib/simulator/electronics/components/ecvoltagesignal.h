/***************************************************************************
 *   Copyright (C) 2003-2004 by David Saxton                               *
 *   david@bluehaze.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ECVOLTAGESIGNAL_H
#define ECVOLTAGESIGNAL_H

#include "component.h"
#include "simulatorexport.h"

class VoltageSignal;
class Circuit;

/**
@short Provides an alternating voltage source
@author David Saxton
*/
class SIMULATOR_EXPORT ECVoltageSignal : public Component
{
public:
	ECVoltageSignal(Circuit &ownerCircuit);
	~ECVoltageSignal();


protected:
    virtual void propertyChanged(Property& theProperty, QVariant newValue,
                                 QVariant oldValue);
	
	VoltageSignal *m_voltageSignal;
};

#endif
