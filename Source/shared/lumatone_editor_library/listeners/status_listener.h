/*
  ==============================================================================

    status_listener.h
    Created: 22 Mar 2022 10:11:10pm
    Author:  Vincenzo Sicurella
 
  ==============================================================================
*/

#pragma once

#include "../data/application_state.h"

namespace LumatoneEditor
{
    //============================================================================
    // Public interface for Lumatone connection status

    class StatusListener
    {
    public:

        virtual ~StatusListener() {}

        virtual void connectionFailed() {}
        virtual void connectionStateChanged(ConnectionState state) {}
        // virtual void connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId) {}
        // virtual void connectionLost() {}
    };

    //============================================================================
    // Interface for components that declare Lumatone connection changes

    class StatusEmitter
    {
    protected:
        juce::ListenerList<StatusListener> statusListeners;

    public:

        virtual ~StatusEmitter() {}

        void addStatusListener(StatusListener* listener) { statusListeners.add(listener); }

        void removeStatusListener(StatusListener* listener) { statusListeners.remove(listener); }
    };

}
