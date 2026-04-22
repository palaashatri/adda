import sys
from pyforge.ui.main_window import MainWindow

def main():
    print("Initializing PyForge...")
    app = MainWindow()
    app.mainloop()

if __name__ == "__main__":
    main()
