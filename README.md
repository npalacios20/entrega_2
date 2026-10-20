<img width="2230" height="987" alt="Parqueadero_MCN" src="https://github.com/user-attachments/assets/d12af842-230f-48b1-9cee-f4e548000faa" />

# ***PARQUEADERO MCN*** 
## Indice

➤ [Colaboradores del proyecto](#Colaboradores)
➤ [Descripción del proyecto](#descripción)
➤ [Objetivos del proyecto](#Objetivos)
➤ [Componentes del proyecto](#Componentes)

➤ [Funcionamiento del sistema](#Funcionamiento)
➤ [Diagrama UML](#Diagrama)
➤ [Tarifas](#Tarifas)
➤ [Contacto y redes](#Contacto)


### Descripción
Desarrollamos un sistema distribuido de gestión de parqueadero que genera y transmite placas automáticamente mediante sockets en C++, estableciendo comunicación entre un servidor y un cliente. Incorporamos una librería dinámica como puente que conecta el módulo de generación con un visualizador desarrollado en Python, el cual recibe y muestra en tiempo real la placa, la hora de ingreso y la celda asignada. El sistema detecta automáticamente si una placa se repite para liberar el espacio ocupado, permitiendo un monitoreo claro del ingreso y la salida de los vehículos.

### Objetivos
1. Implementar un sistema de comunicación entre procesos mediante sockets en lenguaje C++, donde un servidor genera placas de forma automática cada 2 a 5 segundos y transmite la información al cliente con la placa, la hora y la celda de parqueadero asignada.
   
2. Desarrollar la integración entre módulos mediante una librería dinámica, que permita conectar el sistema de generación en C++ con un visualizador en Python, logrando que los datos se muestren en pantalla de forma clara y en tiempo real.
   
3. Incorporar la lógica de detección de repetición de placas para identificar cuándo un vehículo ingresa o sale del parqueadero, liberando automáticamente la celda correspondiente y manteniendo el estado actualizado del sistema.

### Componentes
1. Servidor — Generador de Placas
• Desarrollado en C++, actúa como proceso principal del sistema.
• Genera placas de forma aleatoria cada 2 a 5 segundos.
• Asigna la hora actual y una celda de parqueadero disponible a cada vehículo que ingresa.
• Detecta cuando una placa ya existe → libera la celda ocupada y registra la salida del vehículo.
• Envía toda la información al cliente mediante sockets TCP.

2. Cliente — Receptor de Datos
• También implementado en C++, establece la conexión con el servidor.
• Recibe en tiempo real los datos transmitidos: placa, hora, celda y tipo de evento (ingreso o liberación).
• Transfiere la información recibida hacia la librería dinámica para su visualización.

3. Librería Dinámica
• Módulo compartido que funciona como puente entre el código en C++ y el visualizador.
• Expone funciones para conectarse al servidor y recibir los datos estructurados.
• Permite que el lenguaje de alto nivel (Python) acceda a la lógica de comunicación sin reescribirla.

4. Visualizador
• Desarrollado en Python, carga la librería dinámica y muestra la información de forma amigable.
• Presenta en pantalla: número de placa, hora del suceso, celda asignada y estado del vehículo (ingreso o liberación).
• Actualiza la información automáticamente a medida que llegan nuevos datos.

### Funcionamiento
### Diagrama
### Tarifas
El parqueadero MCN actualmente cuanta con 2 tarifas: 
💲 Tarifa por hora: **$8.000 COP**
💲 Tarifa por día: **$20.000 COP**

### Contactos
Para contactarse con el parqueadero MCN, puedes usar estos canales:
<img width="24" height="24" alt="image" src="https://github.com/user-attachments/assets/8743890e-bef9-4835-a53b-9f5d42ac4d02" />
[@parqueadero_MCN](https://instagram.com/parqueadero_MCN)
<img width="24" height="24" alt="image" src="https://github.com/user-attachments/assets/da1d9e48-f3cc-4531-b75e-767901106763" />
[+67 312 456 7890](https://wa.me/673124567890)

### Colaboradores
Este proyecto ha sido desarrollado por: 

❖ Martín Morales Muñoz    
❖ Leidy Carolina David Valle     
❖ Nilson Palacios Cardenas     

[🔝 Volver al Índice](#Indice)
