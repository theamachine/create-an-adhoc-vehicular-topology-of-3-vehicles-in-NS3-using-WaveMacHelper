/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */
#include "ns3/command-line.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/seq-ts-header.h"
#include "ns3/wave-net-device.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/random-walk-2d-mobility-model.h"
#include "ns3/wifi-mode.h"

using namespace ns3;
/**
 * This simulation is to show the routing service of WaveNetDevice described in IEEE 09.4.
 *
 * note: although txPowerLevel is supported now, the "TxPowerLevels"
 * attribute of YansWifiPhy is 1 which means phy devices only support 1
 * levels. Thus, if users want to control txPowerLevel, they should set
 * these attributes of YansWifiPhy by themselves..
 */
class WaveNetDeviceExample
{
public:
  /// Send WSMP example function
  void SendWsmpExample (void);


private:

  /**
   * Send one WSMP packet function
   * \param channel the channel to use
   * \param seq the sequence
   * \param prio the priority
   * \param rate the data rate
   * \param size the packet size
   * \param i the node index
   */
  void SendOneWsmpPacket  (uint32_t channel, uint32_t seq, uint32_t prio, WifiMode rate, uint32_t size, uint32_t i);



  /**
   * Receive function
   * \param dev the device
   * \param pkt the packet
   * \param mode the mode
   * \param sender the sender address
   * \returns true if successful
   */
  bool Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);


  /// Create WAVE nodes function
  void CreateWaveNodes (void);


  NodeContainer nodes; ///< the nodes
  NetDeviceContainer devices; ///< the devices
};


void
WaveNetDeviceExample::CreateWaveNodes (void)
{
  //Question(1)
  //creating 3 vehicles 
  nodes = NodeContainer ();
  nodes.Create (3);


  //Question(2)
  MobilityHelper mobility;

  //Initializing the position
  double X_min = 0.0;
  double X_max = 20.0;
  double Y_min = 0.0;
  double Y_max = 20.0;

  Ptr<UniformRandomVariable> X = CreateObject<UniformRandomVariable> ();
  X->SetAttribute ("Min", DoubleValue (X_min));
  X->SetAttribute ("Max", DoubleValue (X_max));

  Ptr<UniformRandomVariable> Y = CreateObject<UniformRandomVariable> ();
  Y->SetAttribute ("Min", DoubleValue (Y_min));
  Y->SetAttribute ("Max", DoubleValue (Y_max));

  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", ns3::PointerValue(ns3::Ptr< ns3::RandomVariableStream>(X)),
                                 "Y", ns3::PointerValue(ns3::Ptr< ns3::RandomVariableStream>(Y)));


  // Adjusting bounds and speed
  double speed_min = 8.0;
  double speed_max = 13.0;
 
  Ptr<UniformRandomVariable> speed = CreateObject<UniformRandomVariable> ();
  speed->SetAttribute ("Min", DoubleValue (speed_min));
  speed->SetAttribute ("Max", DoubleValue (speed_max));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (0, 500, 0, 500)),
                             "Speed",ns3::PointerValue( ns3::Ptr< ns3::RandomVariableStream>(speed)));

  mobility.Install (nodes);


  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11); //Set the data link type of PCAP traces to be used. This function has to be called before EnablePcap(), so that the header of the pcap file can be written correctly. Here we set it to IEEE 802.11 Wireless LAN headers 
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  devices = waveHelper.Install (wavePhy, waveMac, nodes);

  for (uint32_t i = 0; i != devices.GetN (); ++i)
    {
      Ptr<WaveNetDevice> device = DynamicCast<WaveNetDevice> (devices.Get (i)); //we use DynamicCast to use a subclass of a base class
      device->SetReceiveCallback (MakeCallback (&WaveNetDeviceExample::Receive, this));  //Set the callback to be used to notify higher layers when a packet has been received.

    }

  // Tracing
  wavePhy.EnablePcap ("wave-simple-device", devices);
}

bool
WaveNetDeviceExample::Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender)
{
  SeqTsHeader seqTs;
  pkt->PeekHeader (seqTs);
  std::cout << "receive a packet: " << std::endl
            << "  reciever = " << dev->GetAddress() << "," << std::endl
            << "  sender = " << sender << "," << std::endl
            << "  sequence = " << seqTs.GetSeq () << "," << std::endl
            << "  sendTime = " << seqTs.GetTs ().As (Time::S) << "," << std::endl
            << "  recvTime = " << Now ().As (Time::S) << "," << std::endl
            << "  protocol = 0x" << std::hex << mode << std::dec  << std::endl;
  return true;
}

void
WaveNetDeviceExample::SendOneWsmpPacket  (uint32_t channel, uint32_t seq, uint32_t prio, WifiMode rate, uint32_t size, uint32_t i)
{
  Ptr<WaveNetDevice>  sender = DynamicCast<WaveNetDevice> (devices.Get (i));
  
  const static uint16_t WSMP_PROT_NUMBER = 0x88DC;   //WSM packets received from the lower layers with an Ethernet Type of 0x88DC are delivered to the WSM protocol. 
  Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
  
  //Adujsting priority, data rate, and transmission power
  const TxInfo txInfo = TxInfo (channel, prio, rate, WIFI_PREAMBLE_LONG, 8);

  // Adusting the packet size
  Ptr<Packet> p  = Create<Packet> (size);   

  SeqTsHeader seqTs;       //SeqTsHeader: Packet header to carry sequence number and timestamp
  seqTs.SetSeq (seq);      //Set the sequence number in the header
  p->AddHeader (seqTs);    //Add header to a packet
  sender->SendX  (p, bssWildcard, WSMP_PROT_NUMBER, txInfo);
}

void
WaveNetDeviceExample::SendWsmpExample ()
{
  CreateWaveNodes ();

  Ptr<WaveNetDevice>  vehicle_1 = DynamicCast<WaveNetDevice> (devices.Get (0));
  Ptr<WaveNetDevice>  vehicle_2 = DynamicCast<WaveNetDevice> (devices.Get (1));
  Ptr<WaveNetDevice>  vehicle_3 = DynamicCast<WaveNetDevice> (devices.Get (2));

  //Adjusting different data rates
  WifiMode rate1=WifiMode("OfdmRate12MbpsBW10MHz");
  WifiMode rate2=WifiMode("OfdmRate27MbpsBW10MHz");
  WifiMode rate3=WifiMode("OfdmRate9MbpsBW10MHz");
  WifiMode rate4=WifiMode("OfdmRate6MbpsBW10MHz");

  //Array to hold data rates for question(5)
  WifiMode rates[3] = {rate2, rate3, rate4};

  //Array to hold priorities for question(5)
  uint32_t priors[3] = {0, 5, 7};

  //Array to hold time intervals for question(5)
  double times[4] = {5, 10, 15, 20};
  

  // Question (4)
  Simulator::Schedule (Seconds (1.0), &WaveNetDeviceExample::SendOneWsmpPacket,  this, CCH, 1, 7, rate1, 500, 0);

  // Question(5)
  // Alternating access without immediate channel switch
  const SchInfo schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);   

  Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch,vehicle_1,schInfo);
  Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch,vehicle_2,schInfo);
  Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch,vehicle_3,schInfo);

  //variable to hold sequence number
  uint32_t seq_num = 2;

  //looping over time intervals
  for(uint32_t i=0; i<4 ; i++)
  {
    //looping over vehicles
    for(uint32_t j=0; j<3 ; j++)
    {
      Simulator::Schedule (Seconds (times[i]), &WaveNetDeviceExample::SendOneWsmpPacket,  this, SCH1, seq_num, priors[j], rates[j], 1000, j);
      seq_num++;
    }
  }

  //Question(3)
  Simulator::Stop (Seconds (20.0));
  
  Simulator::Run ();
  Simulator::Destroy ();
}


int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  WaveNetDeviceExample example;

  std::cout << "run WAVE WSMP routing service case:" << std::endl;
  example.SendWsmpExample ();   //WAVE systems use a highly efficient messaging protocol known as WAVE Short Message Protocol (WSMP)


  return 0;
}