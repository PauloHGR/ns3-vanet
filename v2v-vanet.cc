/*
 * wave-5-simple.cc
 *
 *  Created on: 3 de jul de 2020
 *      Author: paulo
 */

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
 *
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/seq-ts-header.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleOcb");
NodeContainer c;

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * usage:
 *  NodeContainer nodes;
 *  NetDeviceContainer devices;
 *  nodes.Create (2);
 *  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 *  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
 *  wifiPhy.SetChannel (wifiChannel.Create ());
 *  NqosWaveMacHelper wifi80211pMac = NqosWave80211pMacHelper::Default();
 *  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 *  devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
 *
 * The reason of not providing a 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need a high
 * MAC class that enables OCB mode.
 */

void ProcessedPacket (Ptr<Socket> socket)
{
	Address from;
	Ptr<Packet> pkt = socket->RecvFrom(from);
	SeqTsHeader seqTs;
	pkt->PeekHeader (seqTs);

	FILE *arqWrite;
	std::ostringstream arq;
	arqWrite = fopen("/home/paulo/Documentos/mestrado/ads/Trabalho final/v2v-rx.csv","a");
	arq << "Car: " << socket->GetNode()->GetId() << " Receive of the car: " << seqTs.GetSeq ()
			<< "," << Now ().GetSeconds ()
			<< "," << Now ().GetSeconds ();
	fprintf(arqWrite,"%s\n",arq.str().c_str());
	fclose(arqWrite);

	//std::cout << "Car: " << socket->GetNode()->GetId() << " Received response car: " << seqTs.GetSeq () <<" in: " << Now ().GetSeconds () << std::endl;
}

void ReceivePacket (Ptr<Socket> socket)
{

      Ipv4Address ipv4From;
      Address from;
      Ptr<Packet> pkt = socket->RecvFrom(from);
      SeqTsHeader seqTs;
      pkt->PeekHeader (seqTs);
      uint8_t *buffer = new uint8_t[pkt->GetSize()];
      pkt->CopyData(buffer, pkt->GetSize());
      std::string s = std::string((char*)buffer);

      ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();
      Ptr<MobilityModel> model = socket->GetNode()->GetObject<MobilityModel>();
      Ptr<MobilityModel> model2 = c.Get(seqTs.GetSeq ())->GetObject<MobilityModel>();


      FILE *arqWrite;
      std::ostringstream arq;
      arqWrite = fopen("/home/paulo/Documentos/mestrado/ads/Trabalho final/v2v.csv","a");
      arq << "Car: " << socket->GetNode()->GetId() << " Receive of the car: " << seqTs.GetSeq ()
				<< "," << Now ().GetSeconds ()
				<< "," << Now ().GetSeconds ();


      fprintf(arqWrite,"%s\n",arq.str().c_str());
      fclose(arqWrite);

      std::cout << "Car: " << socket->GetNode()->GetId() << ", Receive of the car: " << seqTs.GetSeq () << ", Seconds: " << Now ().GetSeconds ()
      				<< ", Packet: " << pkt->GetUid()
					<< std::endl;


      /*std::ostringstream msg;
      msg << "Processado!";
      Ptr<Packet> p  = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());

      SeqTsHeader seq;
      seq.SetSeq (socket->GetNode()->GetId());
      p->AddHeader (seq);

      socket->Connect(InetSocketAddress(ipv4From,80));
      socket->Send (p);
      //socket->Close();*/

}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, float pktInterval, uint32_t i )
{

  if (pktInterval >= 0)
    {
	  std::ostringstream msg;
	  msg << "Hello World!";
	  Ptr<Packet> p  = Create<Packet> ((uint8_t*) msg.str().c_str(), pktSize);


	  //Ptr<Ipv4> ipv4 = socket->GetNode()->GetObject<Ipv4> ();
	  //Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
	  //Ipv4Address ipAddr = iaddr.GetLocal ();

	  SeqTsHeader seqTs;
	  seqTs.SetSeq (i);
	  p->AddHeader (seqTs);

      socket->Send (p);
	  //std::cout << "Send of the car: " << socket->GetNode()->GetId() << "; IP: " << ipAddr << "; Packet: " << p->GetUid() << std::endl;
      //std::cout << "Send of the car: " << socket->GetNode()->GetId() << "; Packet: " << p->GetUid() << ", in: " << Now ().GetSeconds () << std::endl;
      Simulator::Schedule (Seconds(0.5), &GenerateTraffic,
                           socket, pktSize,pktCount, pktInterval - 0.5, i);
    }
  else
    {
      socket->Close ();
    }
}

int main (int argc, char *argv[])
{
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  uint32_t packetSize = 256; // bytes
  uint32_t numPackets = 1;
  float interval = 300.0; // seconds
  bool verbose = true;

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  // Convert to time object
  //Time interPacketInterval = Seconds (interval);

  c.Create (50);

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();

  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();

  if (verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));

  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
  // Tracing
  //wifiPhy.EnablePcap ("wave-5-simple", devices);

  Ns2MobilityHelper ns2 = Ns2MobilityHelper ("/home/paulo/Documentos/mestrado/pesquisa/cenario-5km/wave/esparso/wave.tcl");
  ns2.Install ();

  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  //for(uint32_t i = 0; i < 1; i++){
	  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
	  recvSink->Bind (local);
	  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
  //}

  for(uint32_t i = 0; i < 50; i++){

	  if(i != 0){

		  Ptr<Socket> source = Socket::CreateSocket (c.Get (i), tid);
		  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
		  //InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
		  source->SetAllowBroadcast (true);
		  source->Connect (remote);
		  //source->Bind(local);
		  //source->SetRecvCallback(MakeCallback(&ProcessedPacket));

		  Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
										  Seconds (0.5), &GenerateTraffic,
										  source, packetSize, numPackets, interval, i);
	  }
  }


  Simulator::Stop (Seconds (299.0));
  //AnimationInterface anim ("v2v.xml");
  Simulator::Run ();
  Simulator::Destroy ();
  std::cout << "Simulação encerrada!" << std::endl;

  return 0;
}
