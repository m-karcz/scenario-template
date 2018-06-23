#ifndef OCCUPATIONSENSORAPP_H
#define OCCUPATIONSENSORAPP_H

#include <iostream>
#include <vector>

#include "parametrizedapp.h"

namespace ns3 {

struct OccupationSensorApp : ParametrizedApp
{
    OccupationSensorApp() : ParametrizedApp("/occupationsensor",
                        {"/home/occupation/publish/occupation-sensor-lvl0"})
    {
        std::ifstream is("scenarios/occupation_data.txt");
        int occupationStatus;
        while (is >> occupationStatus)
            occupationStatuses.push_back(occupationStatus);
    }
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("OccupationSensorApp")
                .SetParent<ndn::App>()
                .AddConstructor<OccupationSensorApp>();
        return tid;
    }
    void StartApplication() override
    {
        ParametrizedApp::StartApplication();
        Simulator::Schedule(Seconds(1.0), &OccupationSensorApp::SendInterests, this);
    }
    void SendInterests()
    {
        auto prefix = m_interestedPrefixes.front();
        prefix.append("/" + std::to_string(occupationStatuses.at(simulationStep)));
        SendInterestImpl(prefix);
        simulationStep++;
        Simulator::Schedule(Seconds(1.0), &OccupationSensorApp::SendInterests, this);
    }

private:
    int simulationStep = 0;
    std::vector<int> occupationStatuses;
};

}


#endif // OCCUPATIONSENSORAPP_H
