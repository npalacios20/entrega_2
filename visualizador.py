import socket
import threading
import tkinter as tk
from datetime import datetime
import server_lib

class VisualizadorParqueadero:
    def __init__(self, root):
        self.root = root
        self.root.title("Visualizador de Parqueadero - Servidor C++ (SWIG) + Python")
        self.root.geometry("850x650")
        self.root.config(bg="#f0f0f0")

        # Instanciar la lógica pura de C++
        self.parqueadero_core = server_lib.ParqueaderoServidor()

        self.crear_encabezado()
        self.crear_celdas_parqueo()
        self.crear_consola_eventos()

        # Iniciar servidor de sockets
        self.hilo_servidor = threading.Thread(target=self.iniciar_servidor_socket, daemon=True)
        self.hilo_servidor.start()

        self.actualizar_interfaz_completa()

    def crear_encabezado(self):
        header_frame = tk.Frame(self.root, bg="#f0f0f0")
        header_frame.pack(fill=tk.X, padx=15, pady=10)

        tk.Label(header_frame, bg="green", width=4, height=1).pack(side=tk.LEFT, padx=5)
        tk.Label(header_frame, text="Disponible", bg="#f0f0f0", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=5)

        tk.Label(header_frame, bg="red", width=4, height=1).pack(side=tk.LEFT, padx=15)
        tk.Label(header_frame, text="Ocupado", bg="#f0f0f0", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=5)

        self.lbl_reloj = tk.Label(header_frame, text="", bg="#f0f0f0", font=("Arial", 11, "bold"))
        self.lbl_reloj.pack(side=tk.RIGHT, padx=5)
        self.actualizar_reloj()

    def actualizar_reloj(self):
        ahora = datetime.now().strftime("Fecha Hora: %m/%d/%Y %H:%M:%S")
        self.lbl_reloj.config(text=ahora)
        self.root.after(1000, self.actualizar_reloj)

    def crear_celdas_parqueo(self):
        frame_contenedor = tk.LabelFrame(self.root, text=" Plano de Celdas de Parqueo (C1 - C6) ", font=("Arial", 11, "bold"), bg="#f0f0f0")
        frame_contenedor.pack(fill=tk.BOTH, expand=True, padx=15, pady=10)

        self.celdas_dict = {}
        for i in range(1, 7):
            fila = 0 if i <= 3 else 1
            columna = (i - 1) % 3

            cell_frame = tk.Frame(frame_contenedor, bg="green", bd=2, relief=tk.RAISED)
            cell_frame.grid(row=fila, column=columna, padx=15, pady=15, sticky="nsew")
            
            frame_contenedor.grid_rowconfigure(fila, weight=1)
            frame_contenedor.grid_columnconfigure(columna, weight=1)

            lbl_titulo = tk.Label(cell_frame, text=f"C{i} [LIBRE]", font=("Arial", 11, "bold"), bg="green", fg="white")
            lbl_titulo.pack(pady=(10, 2))

            lbl_detalle = tk.Label(cell_frame, text="---", font=("Arial", 10), bg="green", fg="white")
            lbl_detalle.pack(pady=(2, 10))

            self.celdas_dict[i] = {"frame": cell_frame, "titulo": lbl_titulo, "detalle": lbl_detalle}

    def crear_consola_eventos(self):
        frame_consola = tk.LabelFrame(self.root, text=" Consola de Eventos y Horas ", font=("Arial", 11, "bold"), bg="#f0f0f0")
        frame_consola.pack(fill=tk.BOTH, expand=True, padx=15, pady=10)

        self.texto_consola = tk.Text(frame_consola, height=8, bg="white", fg="black", font=("Consolas", 9))
        self.texto_consola.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

    def registrar_evento(self, mensaje):
        timestamp = datetime.now().strftime("%H:%M:%S")
        linea = f"[{timestamp}] {mensaje}\n"
        self.texto_consola.insert(tk.END, linea)
        self.texto_consola.see(tk.END)

    def actualizar_interfaz_completa(self):
        estado_texto = self.parqueadero_core.obtenerEstadoParqueadero()
        
        for i in range(1, 7):
            lineas = estado_texto.split("\n")
            for l in lineas:
                if f"Espacio [{i}]:" in l:
                    if "OCUPADO" in l:
                        placa_extraida = "OCUPADO"
                        if "(Placa: " in l:
                            placa_extraida = l.split("(Placa: ")[1].replace(")", "").strip()
                        
                        self.celdas_dict[i]["frame"].config(bg="red")
                        self.celdas_dict[i]["titulo"].config(text=f"C{i} [OCUPADO]", bg="red")
                        self.celdas_dict[i]["detalle"].config(text=placa_extraida, bg="red")
                    else:
                        self.celdas_dict[i]["frame"].config(bg="green")
                        self.celdas_dict[i]["titulo"].config(text=f"C{i} [LIBRE]", bg="green")
                        self.celdas_dict[i]["detalle"].config(text="---", bg="green")

    def iniciar_servidor_socket(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(("127.0.0.1", 8080))
        server_socket.listen(5)
        
        self.root.after(0, lambda: self.registrar_evento("🟢 Servidor Python Socket activo y escuchando en puerto 8080"))

        while True:
            try:
                cliente_sock, addr = server_socket.accept()
                hilo_cliente = threading.Thread(target=self.manejar_cliente, args=(cliente_sock,), daemon=True)
                hilo_cliente.start()
            except Exception as e:
                break

    def manejar_cliente(self, cliente_sock):
        try:
            while True:
                raw_tam = cliente_sock.recv(4)
                if not raw_tam:
                    break
                tam = int.from_bytes(raw_tam, byteorder='big')
                
                placa_bytes = cliente_sock.recv(tam)
                if not placa_bytes:
                    break
                placa = placa_bytes.decode('utf-8')

                # Procesar en la librería C++ a través de SWIG
                resultado = self.parqueadero_core.procesarPlacaEntrante(placa)

                self.root.after(0, lambda r=resultado: self.procesar_resultado_gui(r))

                respuesta = f"Acción: {resultado.accion} | Celda: {resultado.celdaId}\n"
                resp_bytes = respuesta.encode('utf-8')
                tam_resp = len(resp_bytes).to_bytes(4, byteorder='big')
                cliente_sock.sendall(tam_resp + resp_bytes)

        except Exception as e:
            self.root.after(0, lambda err=str(e): self.registrar_evento(f"❌ Error en socket: {err}"))
        finally:
            cliente_sock.close()

    def procesar_resultado_gui(self, resultado):
        self.registrar_evento(f"Placa: {resultado.placa} | Acción: {resultado.accion} | Celda ID: {resultado.celdaId}")
        self.actualizar_interfaz_completa()

if __name__ == "__main__":
    root = tk.Tk()
    app = VisualizadorParqueadero(root)
    root.mainloop()