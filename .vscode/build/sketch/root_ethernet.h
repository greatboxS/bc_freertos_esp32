#ifndef rooT_ethernet_h
#define rooT_ethernet_h
#include <Ethernet.h>
#include <EthernetClient.h>

typedef enum HttpMethod_t
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
};

uint8_t SS_PIN = 0;
uint8_t RST_PIN = 0;

char Host[32] = "Host: 10.4.3.41:32765";
const char *ConnectionAlive = "Connection: Keep-Alive";
const char *KeepAlive = "Keep-Alive: timeout=5, max=999";
const char *ConnectionClose = "Connection: close";
char RootUrl[256];

char ServerIp[20] = "10.4.2.23";
int ServerPort = 32764;

bool Is_getIP = false;

void (*received_data_Callback)(EthernetClient &);

uint8_t DeviceMac[6] = {0xF6, 0x6C, 0x08, 0x62, 0x05, 0x06};

IPAddress local_ip, gateway_ip, dns_ip, subnet_ip;

EthernetClient client;

void ethernet_init(uint8_t cs_pin, uint8_t rst_pin, void (*callback_func)(EthernetClient &stream))
{
    RST_PIN = rst_pin;
    SS_PIN = cs_pin;
    received_data_Callback = callback_func;
}

uint8_t ethernet_get_module_status()
{
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(\r\n");
        return 0;
    }

    if (Ethernet.linkStatus() == LinkOFF)
    {
        printf("Ethernet cable is not connected\r\n");
        return 2;
    }

    return 1;
}

void ethernet_listen()
{
    if (client.available() > 0)
    {
        while (client.available() > 0)
        {
            received_data_Callback(client);
        }
    }
}

void ethernet_set_mac(uint8_t *mac)
{
    printf("Ethernet setup device mac\r\n");
    memccpy(DeviceMac, mac, 0, sizeof(DeviceMac));
}

void ethernet_reset()
{
    printf("Ethernet module is resetting...");
    digitalWrite(RST_PIN, LOW);
    delay(100);
    digitalWrite(RST_PIN, HIGH);
    delay(3000);
}

void ethernet_apply_par_changed()
{
    printf("ApplyServer");
    snprintf(Host, sizeof(Host), "Host: %s:%d", ServerIp, ServerPort);
    printf("%s\r\n", Host);

}

void ethernet_set_server_port(uint16_t _port)
{
    printf("Set Port\r\n");
    ServerPort = _port;
    ethernet_apply_par_changed();
}

void ethernet_set_server_ip(const String &serverName)
{
    printf("Set server\r\n");
    memccpy(ServerIp, serverName.c_str(), 0, sizeof(ServerIp));
    ethernet_apply_par_changed();
}

void ethernet_set_server_ip(char *serverName)
{
    printf("Set server\r\n");
    memccpy(ServerIp, serverName, 0, sizeof(ServerIp));
    ethernet_apply_par_changed();
}

void ethernet_setup_server_par(const String &serverName, uint16_t _port)
{
    printf("Ethernet setup server parameters\r\n");
    ServerPort = _port;
    memccpy(ServerIp, serverName.c_str(), 0, sizeof(ServerIp));
    ethernet_apply_par_changed();
    printf("Setup done!\r\n");
}

void ethernet_setup_server_par(char *serverName, uint16_t _port)
{
    printf("Ethernet setup server parameters\r\n");
    ServerPort = _port;
    memccpy(ServerIp, serverName, 0, sizeof(ServerIp));
    ethernet_apply_par_changed();
    printf("Setup done!\r\n");
}

void ethernet_url_builder(char *url, uint8_t method)
{
    memset(RootUrl, 0, sizeof(RootUrl));
    switch (method)
    {
    case HTTP_GET:
        snprintf(RootUrl, sizeof(RootUrl), "GET /%s HTTP/1.1", url);
        break;
    case HTTP_POST:
        snprintf(RootUrl, sizeof(RootUrl), "POST /%s HTTP/1.1", url);
        break;
    case HTTP_PUT:
        snprintf(RootUrl, sizeof(RootUrl), "PUT /%s HTTP/1.1", url);
        break;
    case HTTP_DELETE:
        snprintf(RootUrl, sizeof(RootUrl), "DELETE /%s HTTP/1.1", url);
        break;
    }
}

void ethernet_url_builder(const String &url, uint8_t method)
{
    memset(RootUrl, 0, sizeof(RootUrl));
    switch (method)
    {
    case HTTP_GET:
        snprintf(RootUrl, sizeof(RootUrl), "GET /%s HTTP/1.1", url.c_str());
        break;
    case HTTP_POST:
        snprintf(RootUrl, sizeof(RootUrl), "POST /%s HTTP/1.1", url.c_str());
        break;
    case HTTP_PUT:
        snprintf(RootUrl, sizeof(RootUrl), "PUT /%s HTTP/1.1", url.c_str());
        break;
    case HTTP_DELETE:
        snprintf(RootUrl, sizeof(RootUrl), "DELETE /%s HTTP/1.1", url.c_str());
        break;
    }
}

void ethernet_make_request(char *url, uint8_t method, char *data = nullptr)
{
    printf("Ethernet make request\r\n");
    char temp[32]{0};

    ethernet_url_builder(url, method);

    client.println(RootUrl);
    client.println(Host);
    client.println(ConnectionClose);

    printf("%s\r\n", RootUrl);
    printf("%s\r\n", Host);
    printf("%s\r\n", ConnectionClose);

    if (data != nullptr && method == HTTP_POST)
    {
        int length = strlen(data);
        client.println("Content-Type: application/json");
        snprintf(temp, sizeof(temp), "Content-Length: %d", length);
        client.println(temp);
        client.println();
        client.println(data);

        printf("Content-Type: application/json");
        printf("%s\r\n", temp);
        printf("%s\r\n", data);
    }
    else
        client.println();
}

uint8_t ethernet_begin()
{
    printf("Begin config ethernet module\r\n");
    Ethernet.init(SS_PIN);

    int exception = Ethernet.begin(DeviceMac, 5000);

    if (exception == 0)
    {
        printf("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(\r\n");
        }
        else if (Ethernet.linkStatus() == LinkOFF)
        {
            printf("Ethernet cable is not connected.\r\n");
        }
    }
    else
    {
        Is_getIP = true;

        printf("Save ethernet ip\r\n");
        local_ip = Ethernet.localIP();
        gateway_ip = Ethernet.gatewayIP();
        dns_ip = Ethernet.dnsServerIP();
        subnet_ip = Ethernet.subnetMask();

        printf("Local Ip: %s, Gateway Ip: %s, DNS: %s, Subnet: %s\r\n",
               local_ip.toString().c_str(), gateway_ip.toString().c_str(), dns_ip.toString().c_str(), subnet_ip.toString().c_str());
    }

    return exception;
}

void ethernet_re_init()
{
    printf("ReInitialize Ethernet module with ip\r\n");
    Ethernet.init(SS_PIN);
    Ethernet.begin(DeviceMac, local_ip, dns_ip, gateway_ip, subnet_ip);
}
#endif