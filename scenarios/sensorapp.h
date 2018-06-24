#pragma once

#include <fstream>

#include "parametrizedapp.h"

namespace ns3 {

struct SensorApp : ParametrizedApp
{
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("SensorApp")
                .SetParent<ParametrizedApp>()
                .AddConstructor<SensorApp>()
                .AddAttribute("DataFile", "DataFile", StringValue(""),
                              MakeStringAccessor(&SensorApp::m_dataFileName), MakeStringChecker())
                .AddAttribute("DataPrefix", "DataPrefix", StringValue("/unknown"),
                              MakeStringAccessor(&SensorApp::m_dataPrefix), MakeStringChecker());
        return tid;
    }
    void StartApplication() override
    {
        std::ifstream is(m_dataFileName.c_str());
        std::string valueToPut;
        while (std::getline(is, valueToPut))
        {
            m_valuesToSend.push_back(valueToPut);
        }
        ParametrizedApp::StartApplication();
        Simulator::Schedule(Seconds(1.0), &SensorApp::SendInterests, this);
    }
    virtual void SendInterests()
    {
        auto l_dataToSend = m_dataPrefix;
        try
        {
            l_dataToSend.append("/" + m_valuesToSend.at(simulationStep));
            SendInterestImpl(l_dataToSend);
            simulationStep++;
            Simulator::Schedule(Seconds(1.0), &SensorApp::SendInterests, this);
        }
        catch(const std::out_of_range& ex)
        {
            std::cerr << "Missing value in " << m_dataFileName << std::endl;
        }
    }

private:
    int simulationStep = 0;
    std::string m_dataPrefix;
    std::string m_dataFileName;
    std::vector<std::string> m_valuesToSend;
};

}
