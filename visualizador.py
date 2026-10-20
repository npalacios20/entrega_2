import socket
import threading
import tkinter as tk
from tkinter import scrolledtext
from datetime import datetime
import server_lib

class VisualizadorParqueadero:
    def __init__(self, root):
        self.root = root
        self.root.title("Visualizador de Parqueadero - Servidor C++ (SWIG) + Python")
        self.root.geometry("850x850")
        
        self.servidor_cpp = server_lib.ParqueaderoServidor()
        
        self.crear_interfaz()
        self.iniciar_servidor_socket()

    def crear_interfaz(self):
        top_frame = tk.Frame(self.root, bg="#f0f0f0")
        top_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        lbl_titulo = tk.Label(top_frame, text="Plano de Celdas de Parqueo (C1 - C18)", font=("Arial", 12, "bold"))
        lbl_titulo.pack(anchor="w", pady=5)
        
        self.grid_frame = tk.Frame(top_frame, bg="#e0e0e0", bd=2, relief=tk.GROOVE)
        self.grid_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.celdas_ui = {}
        for i in range(1, 19):
            row = (i - 1) // 3
            col = (i - 1) % 3
            
            f = tk.Frame(self.grid_frame, bg="green", bd=2, relief=tk.RAISED, width=220, height=70)
            f.grid(row=row, column=col, padx=5, pady=5, sticky="nsew")
            f.pack_propagate(False)
            
            lbl_id = tk.Label(f, text=f"C{i} [LIBRE]", font=("Arial", 10, "bold"), fg="white", bg="green")
            lbl_id.pack(expand=True)
            
            lbl_placa = tk.Label(f, text="", font=("Arial", 9), fg="white", bg="green")
            lbl_placa.pack(expand=True)
            
            self.celdas_ui[i] = {"frame": f, "lbl_id": lbl_id, "lbl_placa": lbl_placa}
            
        for c in range(3):
            self.grid_frame.columnconfigure(c, weight=1)
        for r in range(6):
            self.grid_frame.rowconfigure(r, weight=1)
            
        bottom_frame = tk.Frame(self.root)
        bottom_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        lbl_consola = tk.Label(bottom_frame, text="Consola de Eventos y Horas", font=("Arial", 11, "bold"))
        lbl_consola.pack(anchor="w")
        
        self.consola = scrolledtext.ScrolledText(bottom_frame, height=10, font=("Consolas", 10))
        self.consola.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.log_consola("Servidor Python Socket activo y escuchando en puerto 8080")

    def log_consola(self, texto):
        timestamp = datetime.now().strftime("[%H:%M:%S]")
        self.consola.insert(tk.END, f"{timestamp} {texto}\n")
        self.consola.see(tk.END)

    def actualizar_gui(self, res):
        timestamp = datetime.now().strftime("[%H:%M:%S]")
        
        if hasattr(res, 'estadoParqueadero') and res.estadoParqueadero:
            for linea in res.estadoParqueadero.split('\n'):
                linea_limpia = linea.strip()
                if "Placa:" in linea_limpia and "Acción:" in linea_limpia:
                    self.consola.insert(tk.END, f"[{timestamp}] {linea_limpia}\n")

        if res.placa:
            self.consola.insert(tk.END, f"[{timestamp}] Placa: {res.placa} | Acción: {res.accion} | Celda ID: {res.celdaId}\n")
            self.consola.see(tk.END)

        for i in range(1, 19):
            f = self.celdas_ui[i]["frame"]
            lbl_id = self.celdas_ui[i]["lbl_id"]
            lbl_placa = self.celdas_ui[i]["lbl_placa"]
            
            f.config(bg="green")
            lbl_id.config(text=f"C{i} [LIBRE]", bg="green")
            lbl_placa.config(text="", bg="green")

        estado_texto = self.servidor_cpp.obtenerEstadoParqueadero()
        for linea in estado_texto.split('\n'):
            if "Espacio [" in linea:
                partes = linea.split("]: ")
                id_espacio = int(partes[0].replace("Espacio [", ""))
                if "OCUPADO" in partes[1]:
                    placa = partes[1].split("(Placa: ")[1].replace(")", "")
                    self.celdas_ui[id_espacio]["frame"].config(bg="red")
                    self.celdas_ui[id_espacio]["lbl_id"].config(text=f"C{id_espacio} [OCUPADO]", bg="red")
                    self.celdas_ui[id_espacio]["lbl_placa"].config(text=placa, bg="red")

    def iniciar_servidor_socket(self):
        def escuchar():
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server_socket.bind(("127.0.0.1", 8080))
            server_socket.listen(5)
            
            while True:
                client_sock, addr = server_socket.accept()
                threading.Thread(target=self.manejar_cliente, args=(client_sock,), daemon=True).start()

        threading.Thread(target=escuchar, daemon=True).start()

    def manejar_cliente(self, client_sock):
        try:
            while True:
                data = client_sock.recv(1024)
                if not data:
                    break
                placa = data.decode('utf-8', errors='ignore').replace('\x00', '').strip()
                if placa:
                    res = self.servidor_cpp.procesarPlacaEntrante(placa)
                    self.root.after(0, lambda r=res: self.actualizar_gui(r))
                    client_sock.sendall(b"OK\n")
        except Exception as e:
            print(f"Error en socket: {e}")
        finally:
            try:
                client_sock.close()
            except:
                pass

if __name__ == "__main__":
    root = tk.Tk()
    app = VisualizadorParqueadero(root)
    root.mainloop()