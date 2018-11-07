import tkinter as tk
import pysftp
from PIL import Image, ImageTk


class MView(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        master.geometry("604x604")
        master.resizable(0, 0)
        master.title("Hologram Viewer")
        self.pack(fill=tk.BOTH, expand=1)
        self._connect_button = tk.Button(
            master=self, text="Connect to HAPI", command=self._connect, width=84, height=1)
        self._connect_button.place(x=3, y=3)
        self._choices = [None]
        self._tkvar = tk.StringVar(self)
        self._tkvar.set(None)
        self._run_label = tk.Label(self, text="Run Selection:")
        self._run_label.place(x=3, y=40)
        self._run_menu = tk.OptionMenu(self, self._tkvar, *self._choices)
        self._run_menu.place(x=83, y=35)
        self._load_button = tk.Button(
            self, text="Load Run", command=self._load_run, width=50, height=1)
        self._load_button.place(x=241, y=37.5)
        self._next_button = tk.Button(
            master=self, text="Next", command=self._next_image, width=41, height=2)
        self._next_button.place(x=304, y=68)
        self._prev_button = tk.Button(
            master=self, text="Prev", command=self._prev_image, width=41, height=2)
        self._prev_button.place(x=3, y=68)
        self._save_button = tk.Button(
            master=self, text="Save", command=self._save_current, width=84, height=2)
        self._save_button.place(x=3, y=110)

        test_image = Image.open("test2.png")
        tk_test_image = ImageTk.PhotoImage(test_image)
        image_label = tk.Label(self, image=tk_test_image)
        image_label.image = tk_test_image
        image_label.place(x=0, y=150)

    def _load_run(self):
        print("load")

    def _next_image(self):
        print("next")

    def _prev_image(self):
        print("prev")

    def _save_current(self):
        print("save")

    def _connect(self):
        print("connect")


root = tk.Tk()
app = MView(master=root)
app.mainloop()
