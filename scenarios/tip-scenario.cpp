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
#include <vector>
//#include "ns3/pyviz.h"
#include "ns3/visualizer-module.h"



#include "parametrizedapp.h"
#include "controllerapp.h"
#include "lightnodeapp.h"
#include "sensorapp.h"

namespace ns3 {

class PcapWriter {
public:
  PcapWriter(const std::string& file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile(file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void
  TracePacket(Ptr<const Packet> packet)
  {
    static PppHeader pppHeader;
    pppHeader.SetProtocol(0x0077);

    m_pcap->Write(Simulator::Now(), pppHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};

NS_OBJECT_ENSURE_REGISTERED(ControllerApp);

NS_OBJECT_ENSURE_REGISTERED(LightNodeApp);
NS_OBJECT_ENSURE_REGISTERED(SensorApp);

enum NodeIndex
{
    NodeStationIndex = 0,
    NodeLightSensorIndex = 1,
    NodeControllerIndex = 2,
    NodeLightNodeIndex = 3,
    NodeOccupationSensorIndex = 4
};

int
main(int argc, char* argv[])
{

  CommandLine cmd;
  cmd.Parse(argc, argv);

    //ns3::PyViz pv;
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxPackets", UintegerValue(20));

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize

  // Creating nodes
  //
  WifiHelper wifi;
  wifi.EnableLogComponents();
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

  Ssid ssid = Ssid ("wifi-default");

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  Ptr<UniformRandomVariable> randomizer = CreateObject<UniformRandomVariable>();
  randomizer->SetAttribute("Min", DoubleValue(10));
  randomizer->SetAttribute("Max", DoubleValue(100));

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator", "X", PointerValue(randomizer),
                                "Y", PointerValue(randomizer), "Z", PointerValue(randomizer));

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  NodeContainer nodes;
  nodes.Create(5);
  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes.Get(NodeStationIndex));

  WifiMacHelper wifiMacHelperLightSensor;
  wifiMacHelperLightSensor.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer wifiNetDeviceLightSensor = wifi.Install(wifiPhyHelper, wifiMacHelperLightSensor, nodes.Get(NodeLightSensorIndex));

  WifiMacHelper wifiMacHelperController;
  wifiMacHelperController.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer wifiNetDeviceController = wifi.Install(wifiPhyHelper, wifiMacHelperController, nodes.Get(NodeControllerIndex));

  WifiMacHelper wifiMacHelperLightNode;
  wifiMacHelperLightNode.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer wifiNetDeviceLightNode = wifi.Install(wifiPhyHelper, wifiMacHelperLightNode, nodes.Get(NodeLightNodeIndex));

  WifiMacHelper wifiMacHelperOccupationSensor;
  wifiMacHelperOccupationSensor.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer wifiNetDeviceOccupationSensor = wifi.Install(wifiPhyHelper, wifiMacHelperOccupationSensor, nodes.Get(NodeOccupationSensorIndex));


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
  ndn::AppHelper controllerHelper("ControllerApp");
  controllerHelper.SetPrefix("/ctrl");
  controllerHelper.Install(nodes.Get(NodeControllerIndex));
  ndn::AppHelper lightNodeHelper("LightNodeApp");
  lightNodeHelper.SetPrefix("/home/lightnode");
  lightNodeHelper.Install(nodes.Get(NodeLightNodeIndex));
  ndn::AppHelper occupationSensorHelper("SensorApp");
  occupationSensorHelper.SetPrefix("/occupationSensor");
  occupationSensorHelper.SetAttribute(
        "DataPrefix",
        StringValue("/ctrl/occupation/publish/occupation-sensor-lvl0"));
  occupationSensorHelper.SetAttribute(
        "DataFile",
        StringValue("scenarios/occupation_data.txt"));
  occupationSensorHelper.Install(nodes.Get(NodeOccupationSensorIndex));
  ndn::AppHelper lightSensorHelper("SensorApp");
  lightSensorHelper.SetPrefix("/lightsensor");
  lightSensorHelper.SetAttribute(
        "DataPrefix",
        StringValue("/ctrl/luminocity/publish/light-sensor-0x01"));
  lightSensorHelper.SetAttribute(
        "DataFile",
        StringValue("scenarios/light_data.txt"));
  lightSensorHelper.Install(nodes.Get(NodeLightSensorIndex));

PcapWriter trace("ndn-simple-trace.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));
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
