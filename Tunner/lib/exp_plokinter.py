import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

def generar_senoidal(frecuencia):
    tiempo = np.arange(0, 1, 0.01)
    senal = np.sin(2 * np.pi * frecuencia * tiempo)
    return tiempo, senal

def graficar_senoidal():
    try:
        frecuencia = float(entry_frecuencia.get())
        tiempo, senal = generar_senoidal(frecuencia)
        
        # Limpiar el área de la gráfica anterior
        ax.cla()

        # Graficar la señal
        ax.plot(tiempo, senal)
        ax.set_title('Señal Senoidal')
        ax.set_xlabel('Tiempo')
        ax.set_ylabel('Amplitud')

        # Actualizar la ventana de la gráfica
        canvas.draw()

    except ValueError:
        print("Ingrese una frecuencia válida.")

# Crear la ventana principal
ventana = tk.Tk()
ventana.title("Generador de Señal Senoidal")

# Crear un cuadro de entrada para la frecuencia
label_frecuencia = ttk.Label(ventana, text="Frecuencia:")
label_frecuencia.pack(pady=5)
entry_frecuencia = ttk.Entry(ventana)
entry_frecuencia.pack(pady=5)

# Crear un botón para generar la señal
boton_generar = ttk.Button(ventana, text="Generar Señal", command=graficar_senoidal)
boton_generar.pack(pady=10)

# Configurar la gráfica con Matplotlib
fig, ax = plt.subplots()
canvas = FigureCanvasTkAgg(fig, master=ventana)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=1)

# Iniciar el bucle principal de la interfaz gráfica
ventana.mainloop()