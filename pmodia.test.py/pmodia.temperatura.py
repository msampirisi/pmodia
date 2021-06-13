import smbus
import time

i2c_ch = 1
i2c_address = 0x0d
nroRepeticiones = 10

print(f"inicializando el bus en el canal {i2c_ch}. y en la direccion {i2c_address}.")
i2cbus = smbus.SMBus(i2c_ch)

print(f"Se realizaran {nroRepeticiones}.")
for x in range(nroRepeticiones):
    # seteo modo de lectura de temperatura
    val = i2cbus.write_byte_data(i2c_address, 0x80, 0x91)
    val = i2cbus.write_byte_data(i2c_address, 0x81, 0x00)
    # espero por el status correcto
    statusOK = False;
    while statusOK == False:
        status8F = i2cbus.read_byte_data(i2c_address, 0x8F)
        if (status8F & 0x1) == 1:
            statusOK = True
    # leo los 2 bytes que almacenan la temperatura 
    dataHi = i2cbus.read_byte_data(i2c_address, 0x92)
    dataLo = i2cbus.read_byte_data(i2c_address, 0x93)
    dataTempBase = (dataHi << 8) + dataLo
    # se realizan los calculos de acuerdo a la hoja de datos del modulo
    if dataTempBase < 8192:
        dataTempAjus = dataTempBase / 32.0
    else:
        dataTempAjus = (dataTempBase - 16384 ) / 32.0
    print("status:", status8F, bin(status8F)," - ", status8F & 0x1, " - ", "temp:",bin(dataHi),bin(dataLo),bin(dataTempBase), dataTempBase,dataTempAjus)
    time.sleep(1)
