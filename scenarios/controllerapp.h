#ifndef CONTROLLERAPP_H
#define CONTROLLERAPP_H

#include <iostream>
#include <regex>
#include "parametrizedapp.h"
#include "utils.h"

namespace ns3 {

struct ControllerApp : ParametrizedApp
{
    ControllerApp() : ParametrizedApp("/home")
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
        auto uriParts = Utils::split(p_interest->toUri(), '?');
        if (uriParts.size() < 2)
        {
            std::cerr << "Invalid interest uri" << std::endl;
            return;
        }
        auto interestParts = Utils::split(uriParts.at(0), '/');
        if (interestParts.size() < 4)
        {
            std::cerr << "Invalid interest" << std::endl;
            return;
        }

        if (interestParts.at(InterestPart::INTEREST_NAME) == lightSensorInterestName)
        {
            std::cout << "Light data from "
                      << interestParts.at(InterestPart::DEVICE_NAME)
                      << " : "
                      << interestParts.at(InterestPart::INTEREST_VALUE)
                      << std::endl;
            lightValue = std::stod(interestParts.at(InterestPart::INTEREST_VALUE));
        }
        else if (interestParts.at(InterestPart::INTEREST_NAME) == occupantionSensorInterestName)
        {
            std::cout << "Occupation data from "
                      << interestParts.at(InterestPart::DEVICE_NAME)
                      << " : "
                      << interestParts.at(InterestPart::INTEREST_VALUE)
                      << std::endl;
            peopleInside += std::stod(interestParts.at(InterestPart::INTEREST_VALUE));
        }

        ndn::App::OnInterest(p_interest);
    }

private:
    double lightValue = 0.0;
    int peopleInside = 0;
    const char* lightSensorInterestName = "luminocity";
    const char* occupantionSensorInterestName = "occupation";

    enum InterestPart
    {
        SYSTEM_NAME = 0,
        INTEREST_NAME,
        INTEREST_TYPE,
        DEVICE_NAME,
        INTEREST_VALUE
    };
};

}

#endif // CONTROLLERAPP_H
