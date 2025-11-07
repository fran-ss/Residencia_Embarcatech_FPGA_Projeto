Com base no projeto de **Implementação Lógica de Detecção Rápida de Incêndio em FPGA**, aqui está um modelo de arquivo `README.md`:

# 🚨 Projeto: Detecção Rápida de Incêndio em FPGA (Field-Programmable Gate Array)

## ✍️ Autores
* [cite_start]FRANCIANE SANTOS SILVA [cite: 2]
* [cite_start]KLEBSON MAX SILVA MENDES [cite: 2]
* [cite_start]JOÃO VITOR DINIZ DE JESUS [cite: 2]
* [cite_start]MARIA LUCILENE DOS SANTOS PEDROSA [cite: 2]
* [cite_start]**Instituição:** Instituto Federal do Maranhão (IFMA), São Luís - MA, Brasil [cite: 3]

## ✨ Resumo do Projeto
[cite_start]Este projeto descreve o desenvolvimento de um **Sistema de Detecção e Alerta de Incêndios de alto desempenho** [cite: 12][cite_start], focado na segurança de ambientes críticos, como depósitos[cite: 7, 12].

[cite_start]A solução é centrada na utilização de um **FPGA** (Field-Programmable Gate Array), que permite o processamento ultrarrápido e paralelo [cite: 8, 13][cite_start], crucial para garantir uma latência mínima e resposta em tempo real (hard real-time)[cite: 43]. [cite_start]O sistema supera as limitações de desempenho de sistemas baseados em microcontroladores (MCUs), que introduzem latência devido ao processamento sequencial e *overhead* de software[cite: 11, 25, 41, 42].

O sistema opera através de:
* [cite_start]**Monitoramento:** Sensores de temperatura e umidade (**DHT11**) e um sensor de monóxido de carbono (**MQ-7**)[cite: 9, 14].
* [cite_start]**Processamento:** A lógica de decisão é implementada em HDL no FPGA para realizar a **fusão de dados**[cite: 10, 15, 73].
* [cite_start]**Alerta:** Ao detectar o padrão de um princípio de incêndio (e.g., aumento súbito de temperatura e presença de CO) [cite: 10][cite_start], o sistema aciona simultaneamente um **LED (alerta visual)** e um **Buzzer (alerta sonoro)**[cite: 10, 15].

---

## 🎯 Objetivos

### Objetivo Geral
[cite_start]Desenvolver e validar um protótipo de sistema de alerta de incêndio de latência ultrabaixa baseado em FPGA e BitDogLab, integrando os dados dos sensores DHT11 e MQ-7 para um acionamento de alerta dual[cite: 31].

### Objetivos Específicos
* [cite_start]Implementar em System Verilog a interface de comunicação para aquisição de dados em paralelo dos sensores DHT11 e MQ-7[cite: 33].
* [cite_start]Projetar uma Lógica de Decisão no FPGA capaz de realizar a fusão lógica dos dados sensoriais e comparar os valores com limiares de segurança pré-definidos[cite: 34].
* [cite_start]Implementar a lógica de acionamento síncrono para o LED e o Buzzer, garantindo um alerta dual imediato após a detecção[cite: 35].
* [cite_start]Demonstrar a confiabilidade e o desempenho em tempo real do protótipo em simulações e testes funcionais controlados[cite: 37].

---

## 🛠️ Tecnologias e Materiais

| Categoria | Componente/Tecnologia | Detalhes |
| :--- | :--- | :--- |
| **Hardware Principal** | FPGA Board (Placa de FPGA) | [cite_start]Utilizada: Placa Colorlight 19 com chip Lattice ECP45 [cite: 102] |
| **Plataforma de Dev** | BitDogLab Development Board | [cite_start]Ambiente de prototipagem para mapeamento de pinos e depuração [cite: 36, 94] |
| **Sensores** | Sensor DHT11 | [cite_start]Medição de temperatura e umidade [cite: 9, 14, 64] |
| **Sensores** | Sensor MQ-7 | [cite_start]Detecção de monóxido de carbono (CO) [cite: 9, 14, 67] |
| **Alertas** | LED RGB e Buzzer | [cite_start]Subsistema de alerta dual (visual e sonoro) [cite: 95, 99] |
| **Linguagem de Descrição**| System Verilog (HDL) | [cite_start]Utilizada para codificar a lógica de hardware (processamento paralelo) [cite: 57, 106] |

---

## ⚙️ Lógica de Detecção

[cite_start]O sistema é dividido em três módulos principais[cite: 107]:

1.  [cite_start]**Módulo de Aquisição de Dados:** Realiza a leitura simultânea e paralela dos protocolos dos sensores DHT11 e MQ-7[cite: 107, 116].
2.  [cite_start]**Módulo de Lógica de Decisão:** Implementa a fusão de dados e a condição booleana de alerta[cite: 108].
3.  [cite_start]**Módulo de Alerta Dual:** Garante o acionamento síncrono do LED e do Buzzer[cite: 82].

### Condição de Alerta
O alerta é acionado a partir de uma condição composta de fusão de dados, onde:
* [cite_start]A **temperatura passa de $35^{\circ}C$** **OU** há **identificação de fumaça** (presença significativa de CO)[cite: 74, 108].

[cite_start]Além do LED e Buzzer, o sistema utiliza o *access point* da própria placa BitDogLab para gerar uma **interface HTML** de aviso de incêndio[cite: 111].

---

## 🖼️ Imagens do Protótipo

### Sensor DHT11 (Temperatura e Umidade)

### Sensor MQ-7 (Monóxido de Carbono)

### Protótipo em Funcionamento

### Interface HTML de Alerta

---

## ✅ Conclusão
[cite_start]O projeto demonstrou a **viabilidade do uso de FPGAs** em sistemas de detecção de incêndio, provando um funcionamento estável e uma **resposta imediata** (baixa latência) às variações ambientais[cite: 125, 126]. [cite_start]A implementação da lógica em hardware reconfigurável atende ao requisito de processamento paralelo e confiável, validando o potencial desta arquitetura para missões críticas de segurança[cite: 11, 126].

[cite_start]A resposta do sistema (acionamento do alerta dual e interface HTML) foi validada com sucesso em testes controlados[cite: 117].

---

Gostaria de obter mais detalhes sobre a **lógica de decisão (Módulo de Lógica de Decisão)** implementada em System Verilog?
