<h1>Micro-II</h1> 
<div>Projeto final da disciplina de Microprocessadores e Microcontroladores II. O programa recebe um array de dados numericos qu representam as cores RGB e plotam uma imagem correspondente a essas cores. Executavel criado com o pyinstaller.</div><p>
<p>
  <div>As pastas compactadas Projeto_img e Ima_up, são os projetos para STM32F4xx, onde o Ima_up é o projeto responsável por receber os dados via USART e salvar na memória externa do da placa de desenvolvimento do STM32, já o Projeto_img, é responsável por ler os endereços de memória que foram gravados anteeriormente com os dados da imagem e mandá-los via USART.</div><p>
<p>
  <div>O arquivo grava_img.py é responsável por trasnformar as imagens em bytes e enviá-las via porta serial para o STM32.</div>
</p>
