#ifndef esp_eeprom_config_h
#define esp_eeprom_config_h
#include "EEPROM.h"

//----------------------------------------------------------------------------------------
template <typename T>
void eeprom_write(T val, int addr)
{
    uint8_t size = sizeof(T);
    printf("Write %d bytes have value %d to address %d\r\n", size, val, addr);

    for (uint8_t i = size; i > 0; i--)
    {
        uint8_t value = 0;
        value = uint8_t(val >> ((i - 1) * 8));
        EEPROM.write(addr + (size - i), value);
        printf("Byte index %d, val %d\r\n", i, value);
    }
}

template <typename T>
T eeprom_read(int addr)
{
    T result = 0;
    uint8_t size = sizeof(T);

    for (uint8_t i = size; i > 0; i--)
    {
        T v = (T)EEPROM.read(addr + (size - i));
        printf("Byte index %d, val %d\r\n", i, v);

        result |= (T)(v << ((i - 1) * 8));
    }
    printf("Read %d bytes from address %d, return %d\r\n", sizeof(T), addr, result);
    return result;
}
//----------------------------------------------------------------------------------------

void eeprom_begin()
{
    EEPROM.begin(1024);
    touch_pad_intr_disable();
}

void eeprom_end()
{
    EEPROM.end();
    //touch_pad_intr_enable();
}
//----------------------------------------------------------------------------------------

void eeprom_read_user_info(User_t &user, int address)
{
    printf("Eeprom read user info\r\n");
    int id_addr = address;
    int code_addr = id_addr + sizeof(int);
    int rfid_addr = code_addr + sizeof(int);
    int dep_addr = rfid_addr + sizeof(TagNumber);
    int name_addr = dep_addr + sizeof(user.UserName);

    eeprom_begin();
    user.id = eeprom_read<int>(id_addr);
    user.UserCode = eeprom_read<int>(code_addr);

    for (size_t i = 0; i < sizeof(user.UserRFID); i++)
    {
        user.UserRFID[i] = eeprom_read<char>(i + rfid_addr);
    }

    char temp[10]{0};
    for (size_t i = 0; i < 10; i++)
    {
        temp[i] = eeprom_read<char>(i + dep_addr);
    }

    user.Department = String(temp);

    for (size_t i = 0; i < sizeof(user.UserName); i++)
    {
        user.UserName[i] = eeprom_read<char>(i + name_addr);
    }
    eeprom_end();
}

void eeprom_write_user_info(User_t &user, int address)
{
    printf("Eeprom write user info\r\n");
    int id_addr = address;
    int code_addr = id_addr + sizeof(int);
    int rfid_addr = code_addr + sizeof(int);
    int dep_addr = rfid_addr + sizeof(TagNumber);
    int name_addr = dep_addr + sizeof(user.UserName);

    eeprom_begin();
    eeprom_write<int>(user.id, id_addr);
    eeprom_write<int>(user.UserCode, code_addr);
    for (size_t i = 0; i < sizeof(user.UserRFID); i++)
    {
        eeprom_write(user.UserRFID[i], i + rfid_addr);
    }

    for (size_t i = 0; i < user.Department.length(); i++)
    {
        eeprom_write(user.Department[i], i + dep_addr);
    }

    for (size_t i = 0; i < sizeof(user.UserName); i++)
    {
        eeprom_write(user.UserName[i], i + name_addr);
    }
    eeprom_end();
}
//----------------------------------------------------------------------------------------

void eeprom_read_mac(uint8_t *mac, size_t len, int address)
{
    eeprom_begin();
    for (size_t i = address; i < address + len; i++)
    {
        *mac = EEPROM.read(i);
        ++mac;
    }
    eeprom_end();
}

void eeprom_write_mac(uint8_t *mac, size_t len, int address)
{
    eeprom_begin();
    for (size_t i = address; i < address + len; i++)
    {
        EEPROM.write(i, *mac);
        ++mac;
    }
    eeprom_end();
}

void eeprom_read_server_ip(char *serverIp, size_t len, int address)
{
    eeprom_begin();
    for (size_t i = address; i < address + len; i++)
    {
        *serverIp = EEPROM.read(i);
        ++serverIp;
    }
    eeprom_end();
}

void eeprom_write_server_ip(char *serverIp, size_t len, int address)
{
    eeprom_begin();
    for (size_t i = address; i < address + len; i++)
    {
        EEPROM.write(i, *serverIp);
        ++serverIp;
    }
    eeprom_end();
}

void eeprom_write_port(int port, int address)
{
    eeprom_begin();
    eeprom_write<int>(port, address);
    eeprom_end();
}

int eeprom_read_port(int address)
{
    eeprom_begin();
    int port = eeprom_read<int>(address);
    eeprom_end();
    return port;
}

void eeprom_write_server_par(char *serverIp, uint8_t len, int &port, int address)
{
    printf("Eeprom write server pars\r\n");
    int port_addr = address;
    int ip_addr = port_addr + sizeof(int);
    eeprom_write_port(port, port_addr);
    eeprom_write_server_ip(serverIp, len, ip_addr);
}

void eeprom_read_server_par(char *serverIp, uint8_t len, int &port, int address)
{
    printf("Eeprom read server pars\r\n");
    int port_addr = address;
    int ip_addr = port_addr + sizeof(int);
    port = eeprom_read_port(port_addr);
    eeprom_read_server_ip(serverIp, len, ip_addr);
}

//----------------------------------------------------------------------------------------

void eeprom_write_machine_info(int machineId, char *sysCode, char *machine_name, uint8_t len, int address)
{
    printf("Eeprom save machine info\r\n");
    eeprom_begin();
    int id_addr = address;
    int code_addr = id_addr + sizeof(id_addr);
    int name_addr = code_addr + 10;

    eeprom_write(machineId, id_addr);

    for (size_t i = 0; i < 10; i++)
    {
        EEPROM.write(code_addr + i, *sysCode);
        sysCode++;
    }

    for (size_t i = 0; i < len; i++)
    {
        EEPROM.write(name_addr + i, *machine_name);
        machine_name++;
    }

    eeprom_end();
}

void eeprom_read_machine_info(int &machineId, char *sysCode, char *machine_name, uint8_t len, int address)
{
    printf("Eeprom read machine info\r\n");
    eeprom_begin();
    int id_addr = address;
    int code_addr = id_addr + sizeof(id_addr);
    int name_addr = code_addr + 10;
    machineId = eeprom_read<int>(id_addr);

    for (size_t i = 0; i < 10; i++)
    {
        *sysCode = EEPROM.read(i + code_addr);
        sysCode++;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        *machine_name = EEPROM.read(i + name_addr);
        machine_name++;
    }

    eeprom_end();
}

void eeprom_read_machine_info(Machine_t &machine, int address)
{
    eeprom_read_machine_info(machine.id, machine.MachineCode, machine.MachineName, sizeof(machine.MachineName), address);
}

void eeprom_write_machine_info(Machine_t &machine, int address)
{
    eeprom_write_machine_info(machine.id, machine.MachineCode, machine.MachineName, sizeof(machine.MachineName), address);
}

//----------------------------------------------------------------------------------------
int eeprom_read_binterface_id(int address)
{
    eeprom_begin();
    int id = eeprom_read<int>(address);
    eeprom_end();
    return id;
}

void eeprom_write_binterface_id(int id, int address)
{
    eeprom_begin();
    eeprom_write<int>(id, address);
    eeprom_end();
}

#endif