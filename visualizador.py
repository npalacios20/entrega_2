import tkinter as tk
import socket
import threading
import os
from datetime import datetime
from PIL import Image, ImageTk, ImageDraw
import server_lib as server

PUERTO = 8080

def crear_celda_libre(ancho, alto):
    img = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([0, 0, ancho, alto], radius=12, fill=(78, 169, 59))
    return img

def crear_celda_ocupada(ancho, alto):
    img = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([0, 0, ancho, alto], radius=12, fill=(220, 50, 50))
    return img

class ParqueaderoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("PARQUEADERO MCN")
        self.root.geometry("1400x750")
        self.root.configure(bg="#f0f0f0")
        self.parqueadero = server.ParqueaderoServidor()
        self.TOTAL_CELDAS = 15
        self.celdas_info = [
            {"ocupado": False, "placa": "", "hora_ingreso": None}
            for _ in range(15)
        ]
        self.ruta = os.path.dirname(os.path.abspath(__file__))
        self.ANCHO, self.ALTO = 230, 150
        
        enc = tk.Frame(root, bg="#0a192f", height=110)
        enc.pack(fill="x")
        enc.pack_propagate(False)
        tk.Label(enc, text="PARQUEADERO MCN", font=("Segoe UI", 34, "bold"),
                bg="#0a192f", fg="#ffffff").pack(pady=(20, 2))
        tk.Label(enc, text="Sistema de Monitoreo de Vehículos — 15 Espacios",
                font=("Segoe UI", 12), bg="#0a192f", fg="#d4af37").pack()
        
        self.lbl_estado = tk.Label(root,
            text="⚙ Conectado al servidor — Esperando placas automáticas...",
            font=("Segoe UI", 11, "bold"), bg="#f0f0f0", fg="#4ea93b")
        self.lbl_estado.pack(pady=10)
        
        self.img_libre = ImageTk.PhotoImage(crear_celda_libre(self.ANCHO, self.ALTO))
        self.img_ocup = ImageTk.PhotoImage(crear_celda_ocupada(self.ANCHO, self.ALTO))
        self.img_carro = None
        self.cargar_carro()
        
        self.canvas = tk.Canvas(root, width=1350, height=500, bg="#f0f0f0", bd=0, highlightthickness=0)
        self.canvas.pack(pady=5)
        
        pie = tk.Frame(root, bg="#f0f0f0")
        pie.pack(pady=5)
        tk.Label(pie, text=" ● DISPONIBLE", fg="#4ea93b",
                font=("Segoe UI", 11, "bold"), bg="#f0f0f0").grid(row=0, column=0, padx=40)
        tk.Label(pie, text=" ● OCUPADO", fg="#dc3232",
                font=("Segoe UI", 11, "bold"), bg="#f0f0f0").grid(row=0, column=1, padx=40)
        
        threading.Thread(target=self.servidor_red, daemon=True).start()
        self.actualizar_pantalla()

    def cargar_carro(self):
        try:
            resample = getattr(Image, "Resampling", Image).LANCZOS
            ruta_img = os.path.join(self.ruta, "Carro.png")
            img = Image.open(ruta_img).convert("RGBA").resize((95, 95), resample)
            self.img_carro = ImageTk.PhotoImage(img)
        except Exception as e:
            pass

    def servidor_red(self):
        s = socket.socket()
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("0.0.0.0", PUERTO))
        s.listen(5)
        ip_local = socket.gethostbyname(socket.gethostname())
        self.root.after(0, lambda: self.lbl_estado.config(
            text=f"⚙ Conectado al servidor — IP: {ip_local} — PUERTO {PUERTO}"))
        while True:
            conn, addr = s.accept()
            threading.Thread(target=self.atender_cliente, args=(conn,), daemon=True).start()

    def atender_cliente(self, conn):
        try:
            while True:
                tam_bytes = conn.recv(4)
                if len(tam_bytes) < 4:
                    break
                tam = int.from_bytes(tam_bytes, "big")
                if tam <= 0:
                    continue
                placa = conn.recv(tam).decode().strip()
                if not placa:
                    continue
                self.parqueadero.procesarPlacaEntrante(placa)
                conn.sendall(b"OK")
        except:
            pass
        finally:
            conn.close()

    def leer_estado(self):
        texto = self.parqueadero.obtenerEstadoParqueadero()
        ant = [c.copy() for c in self.celdas_info]
        for i in range(15):
            self.celdas_info[i]["ocupado"] = False
            self.celdas_info[i]["placa"] = ""
        for linea in texto.split("\n"):
            if "Espacio [" in linea and "]:" in linea:
                partes = linea.split("]: ")
                try:
                    idx = int(partes[0].replace("Espacio [", "")) - 1
                    if not (0 <= idx < 15):
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

    def formatear_tiempo(self, hora_ingreso):
        if not hora_ingreso:
            return ""
        transcurrido = datetime.now() - hora_ingreso
        horas = int(transcurrido.total_seconds() // 3600)
        minutos = int((transcurrido.total_seconds() % 3600) // 60)
        segundos = int(transcurrido.total_seconds() % 60)
        return f"{horas:02d}:{minutos:02d}:{segundos:02d}"

    def dibujar(self):
        self.canvas.delete("all")
        filas = 3
        columnas = 5
        margen_x = 50
        margen_y = 30
        espacio_x = self.ANCHO + 15
        espacio_y = self.ALTO + 10
        pos = []
        for f in range(filas):
            for c in range(columnas):
                pos.append((margen_x + c * espacio_x, margen_y + f * espacio_y))
        for i, (x, y) in enumerate(pos):
            if i >= 15:
                break
            info = self.celdas_info[i]
            cx = x + self.ANCHO // 2
            if info["ocupado"]:
                self.canvas.create_image(cx, y + self.ALTO // 2, image=self.img_ocup)
                self.canvas.create_text(cx, y + 22, text=f"CELDA {i+1}",
                    fill="#ffffff", font=("Segoe UI", 13, "bold"))
                if self.img_carro:
                    self.canvas.create_image(cx, y + 70, image=self.img_carro)
                self.canvas.create_text(cx, y + self.ALTO - 35, text=info["placa"],
                    fill="#ffffff", font=("Segoe UI", 13, "bold"))
                tiempo = self.formatear_tiempo(info["hora_ingreso"])
                self.canvas.create_text(cx, y + self.ALTO - 15, text=tiempo,
                    fill="#dddddd", font=("Segoe UI", 11))
            else:
                self.canvas.create_image(cx, y + self.ALTO // 2, image=self.img_libre)
                self.canvas.create_text(cx, y + 22, text=f"CELDA {i+1}",
                    fill="#ffffff", font=("Segoe UI", 13, "bold"))
                self.canvas.create_text(cx, y + self.ALTO // 2 + 10, text="DISPONIBLE",
                    fill="#ffffff", font=("Segoe UI", 15, "bold"))

    def actualizar_pantalla(self):
        self.leer_estado()
        self.dibujar()
        self.root.after(1000, self.actualizar_pantalla)

if __name__ == "__main__":
    root = tk.Tk()
    app = ParqueaderoApp(root)
    root.mainloop()
