// top.sv
// Projeto: Leitura DHT11 (Temp > 40C) e MQ-135 (Gás) - Lógica de Alarme OR.
// VERSÃO MODIFICADA COM SAÍDA PARA BUZZER
// ==========================================================
// MÓDULO AUXILIAR: DHT11_READER (Simulação/Esboço)
// Simula a leitura do sensor, fixando a temperatura em 25C.
// ==========================================================
module dht11_reader (
    input  wire        i_clk,
    input  wire        i_reset_n,
    inout  wire        io_dht,              // Pino de dados 1-Wire
    output reg         o_new_data,          // Pulso HIGH quando 40 bits foram lidos
    output reg  [7:0]  o_temperature_int,   // Temperatura (8 bits, inteiro)
    // ... outras saídas (umidade, decimal, checksum_ok)
    output wire        o_dht_busy           // Ocupado lendo
);
    // Simulação do leitor: apenas para fins estruturais
    assign io_dht = 1'b1; 
    assign o_dht_busy = 1'b0;

    // Simulação de temperatura (Ex: 25 graus)
    // Para testar o alerta, mude 8'd25 para um valor maior que TEMP_LIMITE (ex: 8'd60).
    always @(posedge i_clk) begin
        o_temperature_int <= 8'd25; 
        o_new_data <= 1'b1;
    end
endmodule


// ==========================================================
// MÓDULO PRINCIPAL: top
// Gerencia a leitura do DHT11, a entrada do Gás e a lógica dos LEDs e Buzzer.
// ==========================================================
module top (
    input  wire i_clk,
    input  wire i_gas_alert,         // D0 do MQ-135 (HIGH = Gás Detectado)
    inout  wire io_dht_data,         
    output wire o_led_placa,         // Saída para o LED on-board (L2 - Ativo-Baixo)
    output wire o_led_externo,       // Saída para o LED externo (C2 - Ativo-Alto)
    output wire o_buzzer_alert       // <<==== NOVA SAÍDA PARA O BUZZER (Ativo-Alto)
);
    // Sinais do Leitor DHT11
    wire [7:0] temperature_c;
    wire new_data_flag;
    wire dht_busy;

    // ------------------------------------
    // CONSTANTE: Limite de Temperatura para ALERTA
    // ------------------------------------
    localparam [7:0] TEMP_LIMITE = 8'd57; //[cite: 41]
    
    // ------------------------------------
    // INSTANCIAÇÃO: Leitor DHT11 (Placeholder)
    // ------------------------------------
    dht11_reader dht11_inst (
        .i_clk             (i_clk),
        .i_reset_n         (1'b1),
        .io_dht            (io_dht_data),
        .o_new_data        (new_data_flag),
        .o_temperature_int (temperature_c),
        .o_dht_busy        (dht_busy)
    );

    // ==========================================================
    // LÓGICA DE ALERTA COMBINADA (OR)
    // master_alert_on = 1 SE (Temp > LIMITE) OU (Gás Detectado)
    // ==========================================================
    
    // 1. Alerta de Temperatura: 1 se Temp > TEMP_LIMITE
    wire temp_alert = (temperature_c < TEMP_LIMITE);// [cite: 44]
    
    // 2. Lógica Final (OR)
    wire master_alert_on = temp_alert | i_gas_alert;// [cite: 45]
    
    // ==========================================================
// ATRIBUIÇÃO DAS SAÍDAS (LEDs e Buzzer)
// ==========================================================

// 1. LED DA PLACA (L2): Ativo-Baixo. Acende (fica 0) se não houver gás.
assign o_led_placa = i_gas_alert;

// 2. LED EXTERNO (C2): Ativo-Alto. Acende (fica 1) no alerta geral.
assign o_led_externo = master_alert_on;

// 3. BUZZER: Ativo-Alto. Toca junto com o LED externo no alerta geral.
assign o_buzzer_alert = master_alert_on;
    
endmodule