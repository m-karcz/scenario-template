#ifndef CONTROLLERAPP_H
#define CONTROLLERAPP_H

#include <iostream>
#include <regex>
#include "parametrizedapp.h"

namespace ns3 {

struct ControllerApp : ParametrizedApp
{
    ControllerApp() :
        ParametrizedApp("/home"),
        lightSensorInterestPattern(R"(\/home\/luminocity\/publish\/(.*)\/([-+]?[0-9]*\.[0-9]+|[0-9]+))")
    {}

    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("ControllerApp")
                .SetParent<ParametrizedApp>()
                .AddConstructor<ControllerApp>();
        return tid;
    }
    void OnInterest(std::shared_ptr<const ndn::Interest> p_interest) override
    {
        std::smatch sm;
        //std::cout << p_interest->toUri() << std::endl;

        auto uri = p_interest->toUri();
        std::regex_search(uri, sm, lightSensorInterestPattern);
        if (sm.size() == 3)
        {
            std::cout << "Light data from " << sm[1] << " : " << sm[2] << std::endl;
            lightValue = std::stod(sm[2]);
        }

        ndn::App::OnInterest(p_interest);
    }

private:
    double lightValue = 0.0;
    const std::regex lightSensorInterestPattern;
};

}

#endif // CONTROLLERAPP_H
