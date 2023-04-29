# udp-20-clients-server
Arquitetura cliente-servidor usando UDP: 20 clientes se comunicam simultaneamente com 1 servidor, mas não entre si.

## Estrutura
Nessa simulação foi usada uma conexão cabeada ponto a ponto entre o servidor e cada cliente, com a classe `PointToPointHelper` do NS3. A rede foi testada com uma aplicação Echo UDP, com as classes correspondentes do NS3. Não foi simulada perda de pacotes UDP.
 - Data rate: 5Mbps
 - Delay: 2ms
 - Tamanho do pacote: 1024
 - Pacotes enviados por vez: 1

## Resultados e Conclusão
A simulação rodou tranquilamente e foi possível visualizá-la no NetAnim. Todos os clientes enviavam os pacotes ao mesmo tempo, eles eram recebidos e respondidos pleo servidor ao mesmo tempo também.

Eu esperava que houvesse algum problema com a resposta do servidor, como um atraso na resposta, pois no mesmo instante ele recebia 20 pacotes de 20 clientes diferentes. Todavia, ele respondeu todos simultaneamente sem nenhum atraso ou perda. 

Acredito que isso tenha acontecido porque são 20 enlaces cabeados diferentes, e isso significaria que o servidor tem 20 "placas Ethernet" e 20 IPs diferentes. O servidor então atua como um roteador ou um switch.

Uma visão mais realista é considerar apenas 1 IP para esse servidor, mas não sei se é possível fazer isso usando PointToPoint. Coloquei 20 enlaces cabeados, o que significa que tenho:
- 1 placa de rede em cada um dos 20 clientes.
- 1 cabo ligando cada cliente ao servidor 
- 20 placas de rede no servidor, cada uma tem sim um só IP

Para fazer desse outro jeito preciso usar um BUS (para conexão com fio) ou o Wi-Fi (para sem fio).
