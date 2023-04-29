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
    uint32_t numClients = 2;

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
    
    NodeContainer SC0(serverNode, clientNodes.Get(0)); //novo agrupamento de nodes com: server -> pos 0; client -> pos 1
    NetDeviceContainer serverclient0 = pointToPoint.Install(SC0);
    
    NS_LOG_INFO("Install channel server-client 1");
    NodeContainer SC1(serverNode, clientNodes.Get(1)); 
    NetDeviceContainer serverclient1 = pointToPoint.Install(SC1);

    // Cria <numclients> agrupamentos de 2 nodes com: server -> node 0; client[i] -> node 1
    std::vector<NodeContainer> nodeAdjacencyList (numClients);
    for(uint32_t i=0; i<nodeAdjacencyList.size (); ++i){
        nodeAdjacencyList[i] = NodeContainer (serverNode, clientNodes.Get (i)); //novo agrupamento de 2 nodes com: 
    }

    // Instala o enlace em cada par de (server, client[i]) criado
    std::vector<NetDeviceContainer> deviceAdjacencyList (numClients);
    for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i){
        deviceAdjacencyList[i] = pointToPoint.Install(nodeAdjacencyList[i]); 
    }
    
    // --- ENDERECAMENTO IP --- //
    // Aqui eh um endereco de IP por 'placa de rede cabeada', entao cada cliente tem um IP e o servidor tem 20 IPs
    // Sao 20 subredes tambem, entao sao 20 interfaces => 20 IPs base. 
    //  (comecando com 2)
    NS_LOG_INFO ("Set and Assign IP Addresses.");
    Ipv4AddressHelper adress;
    std::vector<Ipv4InterfaceContainer> interfaces (numClients);
    for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i){
        std::ostringstream subnet;
        subnet<<"10.1."<<i+1<<".0";
        adress.SetBase(subnet.str ().c_str (), "255.255.255.0");
        interfaces[i] = adress.Assign(deviceAdjacencyList[i]); //interface i == server 10.1.[i].1 - client[i] 10.1.[i].2
    }

    // --- APLICACAO --- //
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

    //Uma aplicacao por cliente
    std::vector<ApplicationContainer> clientApp(numClients);
    for(uint32_t i=0; i<clientApp.size (); ++i){
        UdpEchoClientHelper clientHelper(interfaces[i].GetAddress(0), serverPort); //pega o endereco ip do servidor naquela subrede
        clientHelper.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
        clientHelper.SetAttribute ("Interval", TimeValue (interPacketInterval));
        clientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
        clientApp[i] = clientHelper.Install(clientNodes.Get(i)); //instala app no cliente i
        clientApp[i].Start(Seconds(2.0));
        clientApp[i].Stop(Seconds(10.0));
    }
    
    // --- NETANIM --- //
    NS_LOG_INFO("Set animation.");
    AnimationInterface anim ("udp-20-clients-server-anim.xml");

    anim.SetConstantPosition(serverNode.Get(0), 300, 300); //node 0
    uint32_t x = 0, y = 0;
    for(uint32_t i=0; i<numClients; ++i){
        x = (200 + 10*i);
        y = (i <= 9) ? (300 + 10*i) : (490 - 10*i);
        anim.SetConstantPosition(clientNodes.Get(i), x, y); //gera uma semi-circunferencia 
    }

    // --- EXECUCAO --- //
    NS_LOG_INFO("Run simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    return 0;
}
