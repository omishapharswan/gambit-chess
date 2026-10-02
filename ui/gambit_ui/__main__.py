"""Entry point for `python -m gambit_ui`. Phase 1 only prints the version; the window comes in Phase 3."""
from gambit_ui import __version__

if __name__ == "__main__":
    print(f"Gambit UI {__version__}")
