#ifndef LIGHTSENSORAPP_H
#define LIGHTSENSORAPP_H

#include <vector>
#include <fstream>

#include "parametrizedapp.h"

namespace ns3 {

struct LightSensorApp : ParametrizedApp
{
    LightSensorApp() : ParametrizedApp("/lightsensor",
                        {"/home/luminocity/publish/light-sensor-0x01"})
    {
        std::ifstream is("scenarios/light_data.txt");
        double lightValue;
        while (is >> lightValue)
            lightValues.push_back(lightValue);
    }
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("LightSensorApp")
                .SetParent<ndn::App>()
                .AddConstructor<LightSensorApp>();
        return tid;
    }
    void StartApplication() override
    {
        ParametrizedApp::StartApplication();
        Simulator::Schedule(Seconds(1.0), &LightSensorApp::SendInterests, this);
    }
    void SendInterests()
    {
        auto prefix = m_interestedPrefixes.front();
        prefix.append("/" + std::to_string(lightValues.at(simulationStep)));
        SendInterestImpl(prefix);
        simulationStep++;
        Simulator::Schedule(Seconds(1.0), &LightSensorApp::SendInterests, this);
    }

private:
    int simulationStep = 0;
    std::vector<double> lightValues;
};

}

#endif // LIGHTSENSORAPP_H
