def check_nth_bit(num, n):
    return(num>>n)&1

def AD5933_temperatura_byte(valor):
    if(check_nth_bit(valor, 13)) : 
        valorConvertido = (valor - 16384) / 32
    else:
        valorConvertido = valor / 32
    return valorConvertido

def AD5933_temperatura_int(valor):
    if(valor >= 8192) : 
        valorConvertido = (valor - 16384) / 32
    else:
        valorConvertido = valor / 32
    return valorConvertido

tablaTemperatura = [
    [-40.0, 0b11101100000000],
    [-30.0, 0b11110001000000],
    [-20.0, 0b11110011100000],
    [-10.0, 0b11111011000000],
    [ -0.03125, 0b11111111111111],
    [  0.0, 0b00000000000000],
    [  0.03125, 0b00000000000001],
    [ 10.0,0b00000101000000],
    [ 25.0,0b00001100100000],
    [ 50.0,0b00011001000000],
    [ 75.0,0b00100101100000],
    [100.0,0b00110010000000],
    [125.0,0b00111110100000],
    [150.0,0b01001011000000],
    [  0.0,0b01111111111111],
    [  0.0,0b11111111111111],
    [  0.0,0b00000000000001],
    [  0.0,0b10000000000001],
    [  0.0,0b10000000000000]
]
#!"^"
print("{0:_^14} {1:_^10} {2:_^10} {3:_^10} {4:_^10}".format("t.bin","t.int","t.tabla","t.calc.b","t.calc.i"))
for temperatura in tablaTemperatura:
    print(f"{format(temperatura[1],'014b')} {temperatura[1]: >10} {temperatura[0]: >10} {AD5933_temperatura_byte(temperatura[1]): >10} {AD5933_temperatura_int(temperatura[1]): >10}")