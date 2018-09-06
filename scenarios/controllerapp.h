#ifndef CONTROLLERAPP_H
#define CONTROLLERAPP_H

#include <iostream>
#include "parametrizedapp.h"
#include "utils.h"

namespace ns3 {

struct ControllerApp : ParametrizedApp
{
/*    ControllerApp() : ParametrizedApp("/home")
    {}*/

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

        if (interestParts.at(Utils::InterestPart::INTEREST_NAME) == lightSensorInterestName)
        {
            std::cout << "Light data from "
                      << interestParts.at(Utils::InterestPart::DEVICE_NAME)
                      << " : "
                      << interestParts.at(Utils::InterestPart::INTEREST_VALUE)
                      << std::endl;
            lightValue = std::stod(interestParts.at(Utils::InterestPart::INTEREST_VALUE));
        }
        else if (interestParts.at(Utils::InterestPart::INTEREST_NAME) == occupantionSensorInterestName)
        {
            std::cout << "Occupation data from "
                      << interestParts.at(Utils::InterestPart::DEVICE_NAME)
                      << " : "
                      << interestParts.at(Utils::InterestPart::INTEREST_VALUE)
                      << std::endl;
            peopleInside += std::stoi(interestParts.at(Utils::InterestPart::INTEREST_VALUE));
        }

        if (interestParts.at(Utils::InterestPart::DEVICE_NAME) != "controller")
        {
            evalLightBulbControl();
            SendLightBulb();
        }

        ndn::App::OnInterest(p_interest);
    }

    void evalLightBulbControl()
    {
        lightBulbControl = (lightValue < 0.7 && peopleInside > 0);
    }

    void SendLightBulb()
    {
        m_log << lightValue << "\t" << (peopleInside > 0) << "\t" << lightBulbControl << std::endl;
        std::string prefix("/home/lightnode/set/controller/");
        prefix.append(std::to_string(lightBulbControl ? 1 : 0));
        SendInterestImpl(prefix);
    }

private:
    double lightValue = 0.0;
    int peopleInside = 0;
    bool lightBulbControl = false;
    std::ofstream m_log{"log.txt"};
    const char* lightSensorInterestName = "luminocity";
    const char* occupantionSensorInterestName = "occupation";

};

}

#endif // CONTROLLERAPP_H
