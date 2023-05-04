// TOPOLOGIA E ATRIBUTOS: 
//  - 20 nodes em uma estrela, os pacotes vao das pontas da estrela ao seu centro.
//  - n0 -> server | n1 a n21 -> clients 
/*
          n2 n3 n4
           \ | /
            \|/
       n1---n0---n5
            /| \
           / |  \
          n8 n7  n6
*/
//  - Canal:
//      + versao 1.0 -> point to point, cabeada
//          * Data rate: 5Mbps
//          * Delay: 2ms
//      + versao 2.0 -> wireless (pesquisar)
//  - Aplicacao: Echo

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Udp20ClientServer");

int main (int argc, char *argv[]){
    // --- VARIAVEIS --- //
    uint32_t numClients = 3;

    // --- LOGGING --- //
    LogComponentEnable ("Udp20ClientServer", LOG_LEVEL_ALL);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
        
    // --- CRIACAO DOS NODES --- //
    // Agrupa os nodes em clientes e servidor, e tambem em todos os nodes
    NS_LOG_INFO("Create nodes");
    // NodeContainer serverNode;
    // NodeContainer clientNodes;
    // serverNode.Create(1);
    // clientNodes.Create(numClients);

    NodeContainer allNodes(numClients + 1); // node 0 -> server

    // --- PROTOCOLOS DA INTERNET --- //
    NS_LOG_INFO("Install internet protocols stack");
    InternetStackHelper internet;
    internet.Install (allNodes);

    // --- SETA ATRIBUTOS DO CANAL --- //
    //Ponto a ponto (cabeado)
    NS_LOG_INFO("Create point to point channel");
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    
    // --- INSTALA O CANAL NOS NODES --- //
    // Instala o enlace em cada par de (node[i+1] [client], node[0] [server]) criado e ja salva no NetDeviceContainer
    NetDeviceContainer allDevices;
    for(uint32_t i=0; i<numClients; ++i){
        allDevices.Add(pointToPoint.Install(allNodes.Get(0), allNodes.Get(i+1))); 
    }
    
    // --- ENDERECAMENTO IP --- //
    // UM endereco de IP para cada dispositivo na rede
    NS_LOG_INFO ("Set and Assign IP Addresses.");
    Ipv4AddressHelper adress;
    adress.SetBase("10.1.1.0", "255.255.255.0");
    
    Ipv4InterfaceContainer interface = adress.Assign(allDevices); // Atribui os IPs

    // // --- APLICACOES --- //
    NS_LOG_INFO ("Create server application.");
    uint16_t serverPort = 9;  // porta para ECHO
    UdpEchoServerHelper server(serverPort);
    ApplicationContainer serverApp = server.Install(allNodes.Get(0)); //instala a aplicacao servidor no node[0]
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    NS_LOG_INFO ("Create clients applications.");
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 1;
    Time interPacketInterval = Seconds (1.);

    //Atributos da aplicacao
    UdpEchoClientHelper clientHelper(interface.GetAddress(0), serverPort); //endereco 0 eh o endereco do servidor
    clientHelper.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    clientHelper.SetAttribute ("Interval", TimeValue (interPacketInterval));
    clientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));

    //UMa aplicacao por cliente, todas comecam ao mesmo tempo
    ApplicationContainer clientApps;
    for(uint32_t i=0; i<numClients; ++i){
        clientApps.Add(clientHelper.Install(allNodes.Get(i+1))); //instala app no node[i+1]
    }

    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    
    // // --- NETANIM --- //
    NS_LOG_INFO("Set animation.");
    AnimationInterface anim ("udp-20-clients-server-anim2.xml");

    anim.SetConstantPosition(allNodes.Get(0), 300, 300); //node 0
    uint32_t x = 0, y = 0;
    for(uint32_t i=0; i<numClients; ++i){
        x = (200 + 10*i);
        y = (i <= 9) ? (300 + 10*i) : (490 - 10*i);
        anim.SetConstantPosition(allNodes.Get(i+1), x, y); 
    }

    // --- EXECUCAO --- //
    NS_LOG_INFO("Run simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    return 0;
}
