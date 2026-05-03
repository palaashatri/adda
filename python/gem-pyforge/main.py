from app import PyForgeApp
from core.logger import logger

def main():
    logger.info("Starting PyForge...")
    app = PyForgeApp()
    # app.run() will start the mainloop and handle initial downloads
    app.run()

if __name__ == "__main__":
    main()
