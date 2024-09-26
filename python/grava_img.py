import serial
import numpy as np
from PIL import Image

# Obtem o array da porta
def obtem_array(porta):
    arr = []
    porta_serial = serial.Serial("/dev/tty"+porta, baudrate=1000000)
    while True:
        byte_in = porta_serial.read(1)
        if byte_in==b'\xAA':
            break   
    print("Obtendo array...")
    try:
        while len(arr) < 691200:
            dados = porta_serial.read(1)
            arr += [int.from_bytes(dados, byteorder='big')]
            #arr += [dados]
            if len(arr)%100000==0:
                print(arr[len(arr)-1], type(dados))
        porta_serial.close()

    except KeyboardInterrupt:
        porta_serial.close()
        
    print(arr[0], arr[1], arr[2])
    array_imagem = np.array(arr)
    array_imagem = array_imagem.reshape((480, 480, 3))
    return array_imagem

# Transforma um array de dados em imagem e a salva no caminho 
# de saída
def transforma_img(array_data, caminho_saida):
    # Carregar os dados do arquivo de texto
    dados_do_arquivo = array_data
    # Redimensionar os dados de volta para o formato original
    altura, largura, _ = (480, 480, 3)  # Substitua pelas dimensões reais da sua imagem
    array_rgb = dados_do_arquivo.reshape(altura, largura, 3)

    # Converter o array NumPy de volta para uma imagem
    imagem = Image.fromarray(array_rgb.astype('uint8'))

    # Salvar ou exibir a imagem, conforme necessário
    imagem.save(f"{caminho_saida}")  # Substitua pelo caminho e nome desejados
    imagem.show()

# Transforma a imagem em um array e retorna o array
def obtem_img():
    # Carregar a imagem
    while True:
        try:
            nome = input("Digite o caminho da imagem: ")
            break
        except:
            print("Escolha uma caminho válido!!!")
        
    imagem = Image.open(nome)  # Substitua pelo caminho da sua imagem

    # Redimensionar a imagem para 480x480 (opcional, dependendo do tamanho original)
    imagem_redimensionada = imagem.resize((480, 480))

    # Converter a imagem para um array NumPy
    dados_imagem = np.array(imagem_redimensionada)

    # Se a imagem estiver em escala de cinza, converta para RGB
    #if len(dados_imagem.shape) == 2:
     #   dados_imagem = np.stack((dados_imagem,) * 3, axis=-1)

    
    arr = dados_imagem.flatten()
    
    # Exemplo: Imprimir os valores RGB do primeiro pixel
    print(dados_imagem.shape, dados_imagem[0, 0])

    return arr

# Realiza o up da imagem no STM32 e verifica se o dado foi
# escrito corretamente na mememória
def up_img(porta, array_img):
    porta_serial= serial.Serial("/dev/tty"+porta, baudrate=1000000)
    try:
        array_img = array_img.tolist()

        tam = len(array_img)#.size

        print("tamanho do array: ", tam, array_img[0:3])
        ant = 0
        while True:
            byte_in = porta_serial.read(1)
            if byte_in==b'\xAA':
                print("UP!")
                break
        
        for i in range(tam):
            convertion = array_img[i].to_bytes(1, byteorder='big')
            #print(convertion, end=" ")
            porta_serial.write(convertion)
            while True:
                dados = porta_serial.read(1)
                #if dados:
                #print(f"{convertion} X {dados }")
                if dados!= convertion:
                    ant = 1
                    break
                else: break
            if ant!=0: break



            if ((100*(i+1))/691200)%10==0:
                print(f"{(100*(i+1))/691200:.2f} %")
        if ant!=0: print("Quebra de dados...\n", f"{convertion}!={dados }")
        else: print("Finalizando envio...")

    except:
        print("erro!!!")
        porta_serial.close()

    finally:
        print("Fim do envio")
        porta_serial.close()


mode = int(input("1 - P/ Leitura de serial.\n2 - Para conversão e upload de imagem.\n3 - Teste de imagem.\nDigite a opção: "))
porta = input("Porta serial: ")
if mode==1:
    arr = obtem_array(porta)
    caminho = input("Caminho de destino + extensão: ")
    transforma_img(arr, caminho)


elif mode==2:
    up_img(porta, obtem_img())

elif mode==3:
    transforma_img(obtem_img(), "testes/imagem2_2.jpg")

    
# array_image = obtem_array()
# print("array obtido")

#imagem = Image.fromarray(array_image)
# caminho_out = input("Digite o caminho da imagem + titulo e extensão: ")

# transforma_img(array_image, caminho_out)


#imagem.save(caminho)

#imagem.show()