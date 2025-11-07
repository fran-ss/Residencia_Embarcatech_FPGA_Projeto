
# 🚨 Projeto: Detecção Rápida de Incêndio em FPGA (Field-Programmable Gate Array)

## ✍️ Autores
* FRANCIANE SANTOS SILVA
* KLEBSON MAX SILVA MENDES
* JOÃO VITOR DINIZ DE JESUS 
* MARIA LUCILENE DOS SANTOS PEDROSA
* **Instituição:** Instituto Federal do Maranhão (IFMA), São Luís - MA, Brasil 

## ✨ Resumo do Projeto
Este projeto descreve o desenvolvimento de um **Sistema de Detecção e Alerta de Incêndios de alto desempenho**, focado na segurança de ambientes críticos, como depósitos.

A solução é centrada na utilização de um **FPGA** (Field-Programmable Gate Array), que permite o processamento ultrarrápido e paralelo, crucial para garantir uma latência mínima e resposta em tempo real (hard real-time). O sistema supera as limitações de desempenho de sistemas baseados em microcontroladores (MCUs), que introduzem latência devido ao processamento sequencial e *overhead* de software.

O sistema opera através de:
* **Monitoramento:** Sensores de temperatura e umidade (**DHT11**) e um sensor de monóxido de carbono (**MQ-7**).
* **Processamento:** A lógica de decisão é implementada em HDL no FPGA para realizar a **fusão de dados**.
* **Alerta:** Ao detectar o padrão de um princípio de incêndio (e.g., aumento súbito de temperatura e presença de CO), o sistema aciona simultaneamente um **LED (alerta visual)** e um **Buzzer (alerta sonoro)**.

---

## 🎯 Objetivos

### Objetivo Geral
Desenvolver e validar um protótipo de sistema de alerta de incêndio de latência ultrabaixa baseado em FPGA e BitDogLab, integrando os dados dos sensores DHT11 e MQ-7 para um acionamento de alerta dual.

### Objetivos Específicos
* Implementar em System Verilog a interface de comunicação para aquisição de dados em paralelo dos sensores DHT11 e MQ-7.
* Projetar uma Lógica de Decisão no FPGA capaz de realizar a fusão lógica dos dados sensoriais e comparar os valores com limiares de segurança pré-definidos.
* Implementar a lógica de acionamento síncrono para o LED e o Buzzer, garantindo um alerta dual imediato após a detecção.
* Demonstrar a confiabilidade e o desempenho em tempo real do protótipo em simulações e testes funcionais controlados.

---

## 🛠️ Tecnologias e Materiais

| Categoria | Componente/Tecnologia | Detalhes |
| :--- | :--- | :--- |
| **Hardware Principal** | FPGA Board (Placa de FPGA) | Utilizada: Placa Colorlight 19 com chip Lattice ECP45  |
| **Plataforma de Dev** | BitDogLab Development Board | Ambiente de prototipagem para mapeamento de pinos e depuração  |
| **Sensores** | Sensor DHT11 | Medição de temperatura e umidade  |
| **Sensores** | Sensor MQ-7 | Detecção de monóxido de carbono (CO) |
| **Alertas** | LED RGB e Buzzer | Subsistema de alerta dual (visual e sonoro)  |
| **Linguagem de Descrição**| System Verilog (HDL) | Utilizada para codificar a lógica de hardware (processamento paralelo)  |

---

## ⚙️ Lógica de Detecção

O sistema é dividido em três módulos principais:

1.  **Módulo de Aquisição de Dados:** Realiza a leitura simultânea e paralela dos protocolos dos sensores DHT11 e MQ-7.
2.  **Módulo de Lógica de Decisão:** Implementa a fusão de dados e a condição booleana de alerta.
3.  **Módulo de Alerta Dual:** Garante o acionamento síncrono do LED e do Buzzer.

### Condição de Alerta
O alerta é acionado a partir de uma condição composta de fusão de dados, onde:
* A **temperatura passa de $35^{\circ}C$** **OU** há **identificação de fumaça** (presença significativa de CO).

Além do LED e Buzzer, o sistema utiliza o *access point* da própria placa BitDogLab para gerar uma **interface HTML** de aviso de incêndio.

---

## 🖼️ Imagens do Protótipo

### Sensor DHT11 (Temperatura e Umidade)

<img width="450" height="569" alt="image" src="https://github.com/user-attachments/assets/e4c999e5-a304-443d-8509-ff2636035ccf" />

### Sensor MQ-7 (Monóxido de Carbono)

<img width="450" height="569" alt="image" src="https://github.com/user-attachments/assets/8ac3d007-17f6-4b28-8be5-e39024288136" />

### Protótipo em Funcionamento

<img width="450" height="569" alt="image" src="https://github.com/user-attachments/assets/da2abfad-7d15-4c8f-a037-2b80b204a850" />

### Interface HTML de Alerta

<img width="450" height="569" alt="image" src="https://github.com/user-attachments/assets/1ae8f5de-945a-4ba4-ab49-a1c396b2431c" />

---

## ✅ Conclusão
O projeto demonstrou a **viabilidade do uso de FPGAs** em sistemas de detecção de incêndio, provando um funcionamento estável e uma **resposta imediata** (baixa latência) às variações ambientais A implementação da lógica em hardware reconfigurável atende ao requisito de processamento paralelo e confiável, validando o potencial desta arquitetura para missões críticas de segurança.

[cite_start]A resposta do sistema (acionamento do alerta dual e interface HTML) foi validada com sucesso em testes controlados[cite: 117].

---

Gostaria de obter mais detalhes sobre a **lógica de decisão (Módulo de Lógica de Decisão)** implementada em System Verilog?
