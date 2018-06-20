/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-window.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/application.h"
#include <iostream>
//#include "ns3/ndn-consumer.hpp"


namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

struct ParametrizedApp : ndn::App
{
    ParametrizedApp(std::string p_ownPrefix, std::vector<std::string> p_interestedPrefixes = {})
        : m_ownPrefix(std::move(p_ownPrefix)),
          m_interestedPrefixes(std::move(p_interestedPrefixes))
    {}
    void StartApplication() override
    {
        ndn::App::StartApplication();
        ndn::FibHelper::AddRoute(GetNode(), m_ownPrefix, m_face, 0);
    }

    virtual void SendInterestImpl(const std::string& p_prefix, ndn::time::milliseconds p_time = ndn::time::seconds(1))
    {
        std::string interest_ctor_arg = p_prefix;
        std::cout << "sent interest " << interest_ctor_arg << std::endl;
        auto interest = std::make_shared<ndn::Interest>(interest_ctor_arg);
        interest->setInterestLifetime(p_time);
        m_transmittedInterests(interest, this, m_face);
        m_appLink->onReceiveInterest(*interest);
    }
    void OnInterest(std::shared_ptr<const ndn::Interest> p_interest) override
    {
        ndn::App::OnInterest(p_interest);
        std::cout << "on interest" << std::endl;
    }
    void OnData(std::shared_ptr<const ndn::Data> p_data) override
    {
        ndn::App::OnData(p_data);
        std::cout << "on data" << std::endl;
    }
protected:
    int m_counter = 0;
    std::string m_ownPrefix;
    std::vector<std::string> m_interestedPrefixes;
};

struct ControllerApp : ParametrizedApp
{
    ControllerApp() : ParametrizedApp("/controller",{
                        "/lightsensor",
                        "/motionsensor/0",
                        "/moionnsensor/1"
                      })
    {
    }

    static TypeId GetTypeId()
    {
        static TypeId tid = 
            TypeId("ControllerApp")
                .SetParent<ParametrizedApp>()
                .AddConstructor<ControllerApp>();
        return tid;
    }
    void StartApplication() override
    {
        ParametrizedApp::StartApplication();
        Simulator::Schedule(Seconds(1.0), &ControllerApp::SendInterests, this);
    }
    void SendInterests()
    {
        for(const auto& p_prefixToSend : m_interestedPrefixes)
        {
            SendInterestImpl(p_prefixToSend);
        }
        Simulator::Schedule(Seconds(1.0), &ControllerApp::SendInterests, this);
    }
};

struct LightSensorApp : ParametrizedApp //jakoś się uogulni sensor
{
    LightSensorApp() : ParametrizedApp("/lightsensor")
    {}
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("LightSensorApp")
                .SetParent<ndn::App>()
                .AddConstructor<LightSensorApp>();
        return tid;
    }
    void OnInterest(std::shared_ptr<const ndn::Interest> p_interest) override
    {
        ndn::App::OnInterest(p_interest);
        std::cout << "got interest in light sensor xD" << std::endl;
    }
};

NS_OBJECT_ENSURE_REGISTERED(LightSensorApp);
NS_OBJECT_ENSURE_REGISTERED(ControllerApp);

int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxPackets", UintegerValue(20));

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  //
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate24Mbps"));

  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  Ptr<UniformRandomVariable> randomizer = CreateObject<UniformRandomVariable>();
  randomizer->SetAttribute("Min", DoubleValue(10));
  randomizer->SetAttribute("Max", DoubleValue(100));

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator", "X", PointerValue(randomizer),
                                "Y", PointerValue(randomizer), "Z", PointerValue(randomizer));

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  NodeContainer nodes;
  nodes.Create(2);
  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);

  // 2. Install Mobility model
  mobility.Install(nodes);

  // 3. Install NDN stack
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/best-route");


  // Installing applications

  std::cout << "cout działa xD" << std::endl;
  // Consumer
  //ndn::AppHelper consumerHelper("ns3::ndn::Consumer");
  ndn::AppHelper lightSensorHelper("LightSensorApp");
  lightSensorHelper.Install(nodes.Get(1));
  ndn::AppHelper controllerHelper("ControllerApp");
  controllerHelper.Install(nodes.Get(0));
  
  //ndn::AppHelper consumerHelper("YoloConsumer");
  //ndn::AppHelper consumerHelper("YoloConsumer");
  // Consumer will request /prefix/0, /prefix/1, ...
  /*consumerHelper.SetPrefix("/prefix/sub");
  consumerHelper.SetAttribute("Frequency", StringValue("0.1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(0));                        // first node*/

  // Producerk
 // ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  //producerHelper.SetPrefix("/prefix");
  //producerHelper.SetAttribute("PayloadSize", StringValue("16"));
 // producerHelper.Install(nodes.Get(2)); // last node

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
