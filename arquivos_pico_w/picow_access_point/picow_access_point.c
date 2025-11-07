/**
 * Projeto: Servidor HTTP (Access Point) para Monitoramento de Incêndio (FPGA)
 * * Funcionalidades:
 * - Cria uma rede Wi-Fi "alarme_test" (senha "senha123")
 * - Interface web (http://192.168.4.1) que mostra o status do alarme
 * - Pisca o LED azul da placa (heartbeat) para indicar que o sistema está rodando
 * - Lê o pino GPIO 20 (conectado à FPGA)
 * - Se GPIO 20 estiver ALTO: 
 * - Mostra "INCÊNDIO DETECTADO!" na página web
 * - Aciona o Buzzer (GPIO 21)
 * - Se GPIO 20 estiver BAIXO:
 * - Mostra "Situação Normal" na página web
 * - Desliga o Buzzer
 */

#include <string.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "dhcpserver.h"
#include "dnsserver.h"

// Bibliotecas para o Buzzer (PWM) e clock
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// --- Definição do Pino de Alerta da FPGA ---
#define GPIO_IN_FPGA_ALERT 20 // Pino que recebe o sinal da FPGA (ex: E19)

// --- Definições do Buzzer (Baseadas no seu 1º código) ---
#define BUZZER_PIN         21     // Pino que aciona o buzzer
#define BUZZER_FREQUENCY   1000   // Frequência do tom em Hz
#define PWM_WRAP           4095   // Valor máximo do contador (0 a 4095)
#define DUTY_CYCLE         (PWM_WRAP / 2) // 50% de ciclo de trabalho

// --- Configurações do Servidor Web ---
#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"

// Template HTML: 
// <meta http-equiv=\"refresh\" content=\"5\"> atualiza a página a cada 5 segundos
#define HTML_BODY "<html><head><meta http-equiv=\"refresh\" content=\"5\"></head>" \
                  "<body><h1>STATUS DO ALARME DE INCENDIO</h1><p><b>%s</b></p></body></html>"
                  
#define PAGE_PATH "/"
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" PAGE_PATH "\n\n"

// --- Estruturas para o servidor TCP ---
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;


// --- Funções de controle do Buzzer (do seu 1º código) ---

/**
 * @brief Inicializa o PWM no pino do buzzer com a frequência desejada
 */
void pwm_init_buzzer(uint pin, uint frequency_hz) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência e nível máximo (wrap)
    pwm_config config = pwm_get_default_config();
    
    // Calcular o divisor de clock para atingir a frequência desejada
    float divider = (float)clock_get_hz(clk_sys) / (frequency_hz * (PWM_WRAP + 1));
    
    pwm_config_set_wrap(&config, PWM_WRAP); // Nível máximo
    pwm_config_set_clkdiv(&config, divider); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo (buzzer desligado)
    pwm_set_gpio_level(pin, 0);
}

/**
 * @brief Liga o som do buzzer (PWM em 50% duty cycle)
 */
void buzzer_on(uint pin) {
    pwm_set_gpio_level(pin, DUTY_CYCLE); 
}

/**
 * @brief Desliga o som do buzzer (PWM em 0% duty cycle)
 */
void buzzer_off(uint pin) {
    pwm_set_gpio_level(pin, 0); 
}


// --- Funções do Servidor TCP (sem modificações) ---
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

// --- GERAÇÃO DO CONTEÚDO WEB (MODIFICADO) ---
/**
 * @brief Gera o conteúdo HTML da página.
 * Esta função é chamada quando um navegador acessa o servidor.
 */
static int test_server_content(const char *request, char *result, size_t max_result_len) {
    int len = 0;
    
    // Verifica se a requisição é para a página principal ("/")
    if (strncmp(request, PAGE_PATH, sizeof(PAGE_PATH) - 1) == 0) {
        
        // --- LÓGICA DA FPGA (LEITURA) ---
        // Lê o pino de alerta da FPGA no momento da requisição
        bool incendi_detectado = gpio_get(GPIO_IN_FPGA_ALERT);
        
        char status_text[64];
        
        if (incendi_detectado) {
            // Se pino ALTO, há incêndio
            strcpy(status_text, "INCENDIO DETECTADO!");
        } else {
            // Se pino BAIXO, está normal
            strcpy(status_text, "Situacao Normal");
        }
        
        // Gera a página HTML com o status
        len = snprintf(result, max_result_len, HTML_BODY, status_text);
    }
    return len;
}

// --- RECEPÇÃO DE DADOS TCP (MODIFICADO) ---
/**
 * @brief Processa uma requisição HTTP (GET) do cliente.
 * Simplificado para não aceitar parâmetros (?alarme=X).
 */
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d err %d\n", p->tot_len, err);
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET);
            
            // Remove qualquer parâmetro (ex: ?...) ou espaço
            char *space = strchr(request, ' ');
            if (space) *space = 0;
            char *params = strchr(request, '?');
            if (params) *params = 0;

            // Gera o conteúdo da página lendo a FPGA
            con_state->result_len = test_server_content(request, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s\n", request);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            if (con_state->result_len > 0) {
                // Prepara cabeçalho HTTP 200 OK
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                                                 200, con_state->result_len);
            } else {
                // Se a página não for encontrada, redireciona para a raiz
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                                                 ipaddr_ntoa(con_state->gw));
            }

            // Envia os dados (cabeçalho e corpo HTML)
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

// --- Funções do Servidor TCP (sem modificações) ---
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK);
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb;
    con_state->gw = &state->gw;

    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("starting server on port %d\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n",TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    printf("Conecte-se a rede Wi-Fi '%s' (senha: senha123)\n", ap_name);
    printf("Acesse http://192.168.4.1 no navegador\n");
    printf("Pressione 'd' no terminal para desativar o access point\n");
    return true;
}

// Função para desativar o AP (pressione 'd' no terminal)
void key_pressed_func(void *param) {
    assert(param);
    TCP_SERVER_T *state = (TCP_SERVER_T*)param;
    int key = getchar_timeout_us(0);
    if (key == 'd' || key == 'D') {
        cyw43_arch_lwip_begin();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();
        state->complete = true;
    }
}


// --- FUNÇÃO PRINCIPAL (MODIFICADA) ---
int main() {
    stdio_init_all();

    // --- 1. INICIALIZAÇÃO DOS PINOS ---
    
    // Inicializa o pino da FPGA (Entrada)
    gpio_init(GPIO_IN_FPGA_ALERT);
    gpio_set_dir(GPIO_IN_FPGA_ALERT, GPIO_IN);
    gpio_pull_down(GPIO_IN_FPGA_ALERT); // Garante nível baixo se desconectado
    
    // Inicializa o PWM do Buzzer (Saída)
    pwm_init_buzzer(BUZZER_PIN, BUZZER_FREQUENCY);

    // --- 2. INICIALIZAÇÃO DO SERVIDOR ---
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return 1;
    }

    // Callback para desativar o AP
    stdio_set_chars_available_callback(key_pressed_func, state);

    // Configurações da rede
    const char *ap_name = "alarme_test";
    const char *password = "senha123";

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    #if LWIP_IPV6
    #define IP(x) ((x).u_addr.ip4)
    #else
    #define IP(x) (x)
    #endif

    ip4_addr_t mask;
    IP(state->gw).addr = PP_HTONL(CYW43_DEFAULT_IP_AP_ADDRESS);
    IP(mask).addr = PP_HTONL(CYW43_DEFAULT_IP_MASK);

    #undef IP

    // Inicia servidores DHCP e DNS
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    // Abre o servidor TCP
    if (!tcp_server_open(state, ap_name)) {
        DEBUG_printf("failed to open server\n");
        return 1;
    }

    // --- 3. LOOP PRINCIPAL (SUBSTITUI O SCHEDULER DO FREERTOS) ---
    
    bool led_state = true;       // Variável para controlar o estado do LED
    int heartbeat_counter = 0;   // Contador para o LED (piscar a cada 1s)

    state->complete = false;
    while(!state->complete) {
        
        // --- Tarefa 1: Lógica do Alarme (FPGA + Buzzer) ---
        // (Executa a cada 100ms)
        bool alert_active = gpio_get(GPIO_IN_FPGA_ALERT);
        
        if (alert_active) {
            buzzer_on(BUZZER_PIN);
        } else {
            buzzer_off(BUZZER_PIN);
        }
        
        // --- Tarefa 2: Lógica do LED "Heartbeat" ---
        heartbeat_counter++;
        // O loop roda a cada 100ms. 10 * 100ms = 1000ms = 1 segundo.
        if (heartbeat_counter >= 10) { 
            // CYW43_WL_GPIO_LED_PIN é o LED azul embutido
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
            led_state = !led_state;       // Inverte o estado do LED
            heartbeat_counter = 0;      // Reseta o contador
        }

        // --- Tarefa 3: Gerenciamento da Rede (WiFi e Servidor) ---
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        // Delay de 100ms (para o alarme responder rápido)
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(100));
#else
        sleep_ms(100);
#endif
    }
    
    // --- 4. DESLIGAMENTO ---
    buzzer_off(BUZZER_PIN); // Garante que o buzzer pare ao desligar
    tcp_server_close(state);
    dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    printf("Servidor desativado\n");
    return 0;
}