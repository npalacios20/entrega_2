<img width="2230" height="987" alt="Parqueadero_MCN" src="https://github.com/user-attachments/assets/d12af842-230f-48b1-9cee-f4e548000faa" />

# ***PARQUEADERO MCN*** 
## Indice - Proyecto

➤ [Colaboradores del proyecto](Colaboradores)
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
### Funcionamiento
### Diagrama
### Tarifas
### Contactos
### Colaboradores

