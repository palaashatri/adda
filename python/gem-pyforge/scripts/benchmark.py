import time
from core.logger import logger

def run_benchmark():
    logger.info("Starting benchmark...")
    start = time.time()
    # Dummy benchmark
    time.sleep(1)
    logger.info(f"Benchmark completed in {time.time() - start:.2f} seconds.")

if __name__ == "__main__":
    run_benchmark()
