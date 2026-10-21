import tkinter as tk
import socket
import threading
import os
from datetime import datetime
from PIL import Image, ImageTk, ImageDraw
import server
 
PUERTO = 8080
 
def crear_celda_libre(ancho, alto):
    img = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([0, 0, ancho, alto], radius=18, fill=(78, 169, 59))
    return img
 
def crear_celda_ocupada(ancho, alto):
    img = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([0, 0, ancho, alto], radius=18, fill=(239, 68, 68))
    return img
 
class ParqueaderoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("PARQUEADERO MCN")
        self.root.geometry("1280x800")
        self.root.configure(bg="#f4f4f4")
 
        self.parqueadero = server.ParqueaderoServidor()
        self.TOTAL_CELDAS = 18
        self.celdas_info = [{"ocupado": False, "placa": "", "hora_ingreso": None} for _ in range(18)]
        self.ruta = os.path.dirname(os.path.abspath(__file__))
        self.ANCHO, self.ALTO = 225, 160
 
        enc = tk.Frame(root, bg="#0a192f", height=120)
        enc.pack(fill="x")
        enc.pack_propagate(False)
        tk.Label(enc, text="PARQUEADERO MCN", font=("Segoe UI", 34, "bold"), bg="#0a192f", fg="#ffffff").pack(pady=(25, 5))
        tk.Label(enc, text="Sistema de Monitoreo", font=("Segoe UI", 14), bg="#0a192f", fg="#d4af37").pack()
 
        self.lbl_estado = tk.Label(root, text="INICIANDO...", font=("Segoe UI", 12, "bold"), bg="#f4f4f4", fg="#f59e0b")
        self.lbl_estado.pack(pady=12)
 
        f = tk.Frame(root, bg="#f4f4f4")
        f.pack(pady=5)
        tk.Label(f, text="Placa:", font=("Segoe UI", 11, "bold"), bg="#f4f4f4").grid(row=0, column=0, padx=5)
        self.ent = tk.Entry(f, font=("Segoe UI", 11), width=15)
        self.ent.grid(row=0, column=1, padx=5)
        tk.Button(f, text="Enviar", command=self.procesar_manual, bg="#4ea93b", fg="#ffffff", font=("Segoe UI", 10, "bold")).grid(row=0, column=2, padx=5)
 
        self.img_libre = ImageTk.PhotoImage(crear_celda_libre(self.ANCHO, self.ALTO))
        self.img_ocup = ImageTk.PhotoImage(crear_celda_ocupada(self.ANCHO, self.ALTO))
        self.img_carro = None
        self.cargar_carro()
 
        self.canvas = tk.Canvas(root, width=1230, height=600, bg="#f4f4f4", bd=0, highlightthickness=0)
        self.canvas.pack(pady=5)
 
        pie = tk.Frame(root, bg="#f4f4f4")
        pie.pack(pady=5)
        tk.Label(pie, text=" DISPONIBLE", fg="#4ea93b", font=("Segoe UI", 12, "bold"), bg="#f4f4f4").grid(row=0, column=0, padx=40)
        tk.Label(pie, text=" OCUPADO", fg="#ef4444", font=("Segoe UI", 12, "bold"), bg="#f4f4f4").grid(row=0, column=1, padx=40)
 
        threading.Thread(target=self.servidor_red, daemon=True).start()
        self.actualizar_pantalla()
 
    def cargar_carro(self):
        try:
            resample = getattr(Image, "Resampling", Image).LANCZOS
            img = Image.open(os.path.join(self.ruta, "Carro.png")).convert("RGBA").resize((105, 105), resample)
            self.img_carro = ImageTk.PhotoImage(img)
        except:
            pass
 
    def servidor_red(self):
        s = socket.socket()
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("0.0.0.0", PUERTO))
        s.listen(5)
        ip_local = socket.gethostbyname(socket.gethostname())
        self.root.after(0, lambda: self.lbl_estado.config(
            text=f"SERVIDOR ACTIVO — IP: {ip_local} — PUERTO {PUERTO}", fg="#4ea93b"))
        while True:
            conn, addr = s.accept()
            threading.Thread(target=self.atender_cliente, args=(conn,), daemon=True).start()
 
    def atender_cliente(self, conn):
        try:
            while True:
                tam = int.from_bytes(conn.recv(4), "big")
                if not tam:
                    break
                placa = conn.recv(tam).decode().strip()
                if not placa:
                    continue
                self.parqueadero.procesarPlacaEntrante(placa)
                conn.sendall(b"OK")
        except:
            pass
        finally:
            conn.close()
 
    def procesar_manual(self):
        placa = self.ent.get().strip()
        if placa:
            self.parqueadero.procesarPlacaEntrante(placa)
            self.ent.delete(0, tk.END)
 
    def leer_estado(self):
        texto = self.parqueadero.obtenerEstadoParqueadero()
        ant = [c.copy() for c in self.celdas_info]
        for i in range(18):
            self.celdas_info[i]["ocupado"] = False
            self.celdas_info[i]["placa"] = ""
        for linea in texto.split("\n"):
            if "Espacio [" in linea and "]:" in linea:
                partes = linea.split("]: ")
                try:
                    idx = int(partes[0].replace("Espacio [", "")) - 1
                    if not (0 <= idx < 18):
                        continue
                    if "OCUPADO" in partes[1]:
                        placa = partes[1].split("(Placa: ")[1].split(")")[0]
                        self.celdas_info[idx]["ocupado"] = True
                        self.celdas_info[idx]["placa"] = placa
                        if not ant[idx]["ocupado"]:
                            self.celdas_info[idx]["hora_ingreso"] = datetime.now()
                    else:
                        self.celdas_info[idx]["ocupado"] = False
                        self.celdas_info[idx]["placa"] = ""
                        self.celdas_info[idx]["hora_ingreso"] = None
                except:
                    pass
 
    def dibujar(self):
        self.canvas.delete("all")
        pos = [(30 + c * 470, 15 + f * 185) for f in range(6) for c in range(3)]
        for i, (x, y) in enumerate(pos):
            if i >= 18:
                break
            info = self.celdas_info[i]
            cx, cy = x + self.ANCHO // 2, y + self.ALTO // 2
            if info["ocupado"]:
                self.canvas.create_image(cx, cy, image=self.img_ocup)
                self.canvas.create_text(cx, y + 22, text=f"CELDA {i+1}", fill="#ffffff", font=("Segoe UI", 13, "bold"))
                if self.img_carro:
                    self.canvas.create_image(cx, cy + 5, image=self.img_carro)
                self.canvas.create_text(cx, y + self.ALTO - 20, text=info["placa"], fill="#ffffff", font=("Segoe UI", 13, "bold"))
            else:
                self.canvas.create_image(cx, cy, image=self.img_libre)
                self.canvas.create_text(cx, y + 22, text=f"CELDA {i+1}", fill="#ffffff", font=("Segoe UI", 13, "bold"))
                self.canvas.create_text(cx, cy + 15, text="DISPONIBLE", fill="#ffffff", font=("Segoe UI", 16, "bold"))
 
    def actualizar_pantalla(self):
        self.leer_estado()
        self.dibujar()
        self.root.after(2000, self.actualizar_pantalla)
 
if __name__ == "__main__":
    root = tk.Tk()
    app = ParqueaderoApp(root)
    root.mainloop()