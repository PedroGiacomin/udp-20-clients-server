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
    NodeContainer serverNode;
    NodeContainer clientNodes;
    serverNode.Create(1);
    clientNodes.Create(numClients);

    NodeContainer allNodes(serverNode, clientNodes);

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
    // Aqui eh como instalar um cabo entre cada um dos clientes e o servidor, entao serao 20 canais 
    //  (comecando com 2)

    // Cria <numclients> agrupamentos de 2 nodes com: server -> node 0; client[i] -> node 1
    std::vector<NodeContainer> nodeAdjacencyList (numClients);
    for(uint32_t i=0; i<nodeAdjacencyList.size (); ++i){
        nodeAdjacencyList[i] = NodeContainer (serverNode, clientNodes.Get (i)); //novo agrupamento de 2 nodes com: 
    }

    // Instala o enlace em cada par de (server, client[i]) criado
    std::vector<NetDeviceContainer> deviceAdjacencyList (numClients);
    //NetDeviceContainer allDevices;
    for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i){
        deviceAdjacencyList[i] = pointToPoint.Install(nodeAdjacencyList[i]); 
        //allDevices.Add(deviceAdjacencyList[i]); //Device container que contem todas as adjacencias
    }
    
    // --- ENDERECAMENTO IP --- //
    // Aqui eh um endereco de IP por 'placa de rede cabeada', entao cada cliente tem um IP e o servidor tem 20 IPs
    // Sao 20 subredes tambem, entao sao 20 interfaces => 20 IPs base. 
    //  (comecando com 2)
    NS_LOG_INFO ("Set and Assign IP Addresses.");
    Ipv4AddressHelper adress;
    adress.SetBase("10.1.1.0", "255.255.255.0");
    
    Ipv4InterfaceContainer interface;
    interface = adress.Assign(deviceAdjacencyList[1].Get(0)); // Atribui um IP ao servidor
    for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i){
        interface.Add(adress.Assign(deviceAdjacencyList[i].Get(1))); //Atribui um IP a cada client
    }

    // // --- APLICACOES --- //
    NS_LOG_INFO ("Create server application.");
    uint16_t serverPort = 9;  // porta para ECHO
    UdpEchoServerHelper server (serverPort);
    ApplicationContainer serverApp = server.Install(serverNode.Get(0)); //instala servidor nesse node
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
    std::vector<ApplicationContainer> clientApp(numClients);
    for(uint32_t i=0; i<clientApp.size (); ++i){
        clientApp[i] = clientHelper.Install(clientNodes.Get(i)); //instala app no cliente i
        clientApp[i].Start(Seconds(2.0*(i+1)));
        clientApp[i].Stop(Seconds(3.0*(i+1))); 
    }
    
    // // --- NETANIM --- //
    // NS_LOG_INFO("Set animation.");
    // AnimationInterface anim ("udp-20-clients-server-anim.xml");

    // anim.SetConstantPosition(serverNode.Get(0), 300, 300); //node 0
    // uint32_t x = 0, y = 0;
    // for(uint32_t i=0; i<numClients; ++i){
    //     x = (200 + 10*i);
    //     y = (i <= 9) ? (300 + 10*i) : (490 - 10*i);
    //     anim.SetConstantPosition(clientNodes.Get(i), x, y); //gera uma semi-circunferencia 
    // }

    // --- EXECUCAO --- //
    NS_LOG_INFO("Run simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    return 0;
}
