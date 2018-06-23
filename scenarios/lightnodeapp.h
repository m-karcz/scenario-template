#ifndef LIGHTNODEAPP_H
#define LIGHTNODEAPP_H

#include <iostream>
#include "parametrizedapp.h"
#include "utils.h"

namespace ns3 {

struct LightNodeApp : ParametrizedApp
{
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("LightNodeApp")
                .SetParent<ParametrizedApp>()
                .AddConstructor<LightNodeApp>();
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

        if (interestParts.at(Utils::InterestPart::INTEREST_TYPE) == "set")
        {
            std::cout << "Set light data from "
                      << interestParts.at(Utils::InterestPart::DEVICE_NAME)
                      << " : "
                      << interestParts.at(Utils::InterestPart::INTEREST_VALUE)
                      << std::endl;

            lighted = std::stoi(interestParts.at(Utils::InterestPart::INTEREST_VALUE)) == 1;
        }

        ndn::App::OnInterest(p_interest);
    }

private:
    bool lighted = false;
};

}
#endif // LIGHTNODEAPP_H
