import serial
import numpy as np
from PIL import Image, ImageTk 
import tkinter as tk
from tkinter import ttk
import threading
from tkinter import filedialog
from tkinter import PhotoImage
from tkinter import Toplevel
import serial.tools.list_ports
import time

porta = ""

# Obtem o array da porta
def obtem_array():
    global porta
    arr = []
    porta_serial = serial.Serial(porta, baudrate=1000000)
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
            
    except KeyboardInterrupt:
        porta_serial.close()
        
    array_imagem = np.array(arr)
    array_imagem = array_imagem.reshape((480, 480, 3))
    return array_imagem

# Transforma um array de dados em imagem e a salva no caminho 
# de saída
def save_image(imagem, caminho):
    imagem.save(caminho)


def transforma_img2(array_data):
    # Carregar os dados do arquivo de texto
    dados_do_arquivo = array_data
    # Redimensionar os dados de volta para o formato original
    altura, largura, _ = (480, 480, 3)  # Substitua pelas dimensões reais da sua imagem
    array_rgb = dados_do_arquivo.reshape(altura, largura, 3)

    # Converter o array NumPy de volta para uma imagem
    imagem = Image.fromarray(array_rgb.astype('uint8'))

    # Salvar ou exibir a imagem, conforme necessário
    # imagem.save(f"{caminho_saida}")  # Substitua pelo caminho e nome desejados
    imagem.show()

def obter_portas_com():
    # Obtém uma lista de portas COM disponíveis
    portas_disponiveis = [porta.device for porta in serial.tools.list_ports.comports()]

    # Atualiza as opções no combobox
    combobox_portas["values"] = portas_disponiveis

def selecionar_porta():
    global porta
    porta = combobox_portas.get()
    porta_sel_label.config(text=f"Porta COM Selecionada: {porta}")

def abrir_seletor_arquivo2():
    filepath = filedialog.asksaveasfilename(defaultextension=".jpg", filetypes=(("Imagens", "*.jpg"), ("Todos os arquivos", "*.*")))
    # label_caminho_arquivo2.config(text=f"Arquivo 2: {filepath}")
    return f"{filepath}"




def iniciar_progresso_thread():
    reset_label.config(text="Reset o STM")
    thread = threading.Thread(target=iniciar_progresso)
    thread.daemon = True
    thread.start()

def iniciar_progresso():
    selecionar_porta()
    # Configuração inicial
    progresso["value"] = 0
    porcentagem_label.config(text="Porcentagem: 0%")
    tempo_inicial = time.time()

    

    # Simulação de um processo que leva tempo
    global porta
    arr = []
    porta_serial = serial.Serial(porta, baudrate=1000000)
    while True:
        byte_in = porta_serial.read(1)
        porcentagem_label.config(text=f"Esperando comunicação de dados...")
        if byte_in==b'\xAA':
            reset_label.config(text="")
            break
    progresso["value"] = 1
    porcentagem_label.config(text="Porcentagem: 0%")
    tempo_inicial = time.time()
    print("Obtendo array...")
    try:
        while len(arr) < 691200:
            dados = porta_serial.read(1)
            arr += [int.from_bytes(dados, byteorder='big')]
            
            i = (len(arr)*100)//691200
            if len(arr)%1000==0:
                # print(arr[len(arr)-1], type(dados))    
                progresso['value'] = i
                porcentagem_label.config(text=f"Porcentagem: {i}%")
                tempo_decorrido = time.time() - tempo_inicial
                tempo_label.config(text=f"Tempo decorrido: {int(tempo_decorrido)} segundos")
                janela.update_idletasks()
            
            elif len(arr)%691200==0:
                # print(arr[len(arr)-1], type(dados))    
                progresso['value'] = i
                porcentagem_label.config(text=f"Conclusão: {i}%")
                tempo_decorrido = time.time() - tempo_inicial
                tempo_label.config(text=f"Tempo: {int(tempo_decorrido)} segundos")
                janela.update_idletasks()
                

    except KeyboardInterrupt:
        porta_serial.close()
    
    finally:
        try:
            porta_serial.close()
            tempo_decorrido = time.time() - tempo_inicial
            tempo_label.config(text=f"Tempo: {tempo_decorrido:.2f} segundos")
            janela.update_idletasks()
            array_imagem = np.array(arr)
            array_imagem = array_imagem.reshape((480, 480, 3))
            # caminho = abrir_seletor_arquivo2()
            transforma_img2(array_imagem)#, caminho)
            # exibir_imagem(caminho)
            # Abre automaticamente o seletor de arquivo 2 quando atinge 100%
        except EnvironmentError:
            print("erro!!!")
            porta_serial.close()


# Cria a janela principal
janela = tk.Tk()
janela.title("Serial Images")
janela.geometry("500x280")  # Tamanho da janela

# Seletor de Porta COM
frame_porta_com = tk.Frame(janela)
frame_porta_com.pack(pady=20)

#frame_porta_com2 = tk.Frame(janela)
#frame_porta_com2.pack(pady=0)

frame_porta_com3 = tk.Frame(janela)
frame_porta_com3.pack(pady=0)


combobox_portas = ttk.Combobox(frame_porta_com, state="readonly", width=20)
combobox_portas.grid(row=0, column=0, pady=5, sticky="w")

botao_obter_portas = tk.Button(frame_porta_com, text="Atualizar Portas COM", command=obter_portas_com)
botao_obter_portas.grid(row=0, column=1, pady=5, padx=(10, 0), sticky="w")

#botao_selecionar_porta = tk.Button(frame_porta_com2, text="Selecionar Porta", command=selecionar_porta)
#botao_selecionar_porta.pack(pady=5)



porta_sel_label = tk.Label(frame_porta_com3, text="")
porta_sel_label.pack()


# Barra de Progresso
frame_progresso = tk.Frame(janela)
frame_progresso.pack(pady=20)

reset_label = tk.Label(frame_progresso, text="")
reset_label.pack()

progresso = ttk.Progressbar(frame_progresso, orient="horizontal", length=400, mode="determinate")
progresso.pack(pady=10)

porcentagem_label = tk.Label(frame_progresso, text="Conclusão: 0%")
porcentagem_label.pack()



tempo_label = tk.Label(frame_progresso, text="Tempo: 0 segundos")
tempo_label.pack()

botao_iniciar_progresso = tk.Button(frame_progresso, text="Iniciar Leitura", command=iniciar_progresso_thread)
botao_iniciar_progresso.pack(pady=10)

# Inicia o loop principal da interface gráfica
janela.mainloop()

